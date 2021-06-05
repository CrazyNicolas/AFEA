#include <iostream>
#include <map>
#include <cstring>
#include <algorithm>
#include <cmath>
#include "oneapi/tbb.h"
#include "../../IOtool/IOtool.h"

using namespace oneapi::tbb;
const int N = 505, sub_size = 30, popul_size = 1000, LENGTH = 505;
int n, len, Epoch = 150, Count = 50;
double Pc = 0.95, Pm = 0.05, eps = 1e-5;
double **Map;
//struct Node
//{
//	double x, y;
//	double dist(Node b)
//	{
//		return sqrt((x - b.x) * (x - b.x) + (y - b.y) * (y - b.y));
//	}
//}node[N];
class INDIV
{
public:
	int length, level;
	int gene[LENGTH];
	bool constrain()
	{
		return true;
	}
	double evaluation()
	{
		double res = 0;
		for (int i = 0; i < length; i++)
		{
			res += Map[gene[i]][gene[(i + 1) % length]];
		}
		return res;
	}
	void Mutation()
	{
		for (int i = 0; i < level; i++)
		{
			int pos1 = rand() % length;
			int pos2 = rand() % length;
			std::swap(gene[pos1], gene[pos2]);
		}
	}
};
INDIV population[popul_size];
bool check(INDIV a)
{
	bool vis[N];
	memset(vis, 0, sizeof(vis));
	for (int i = 0; i < a.length; i++)
	{
		if (vis[a.gene[i]])
			return false;
		vis[a.gene[i]] = 1;
	}
	return true;
}
bool fullCheck()
{
	for (int i = 0; i < sub_size * 7; i++)
		if (!check(population[i]))
			return false;
	return true;
}
void Crossover(INDIV p1, INDIV p2, INDIV& s1, INDIV& s2)
{
	int x = rand() % (p1.length);
	int y = rand() % (p2.length);
	if (x > y) std::swap(x, y);
	s1 = p2; s2 = p1;
	bool vis1[N];
	bool vis2[N];
	std::memset(vis1, 0, sizeof(vis1));
	std::memset(vis2, 0, sizeof(vis2));
	for (int i = x; i <= y; i++)
	{
		vis1[p2.gene[i]] = true;
		vis2[p1.gene[i]] = true;
	}
	int a = 0, b = 0;
	for (int i = 0; i < x; i++)
	{
		while (a < p1.length && vis1[p1.gene[a]])
			a++;
		if (a < p1.length)
			s1.gene[i] = p1.gene[a++];
		while (b < p2.length && vis2[p2.gene[b]])
			b++;
		if (b < p2.length)
			s2.gene[i] = p2.gene[b++];
	}
	for (int i = y + 1; i < p1.length; i++)
	{
		while (a < p1.length && vis1[p1.gene[a]])
			a++;
		if(a < p1.length)
			s1.gene[i] = p1.gene[a++];
		while (b < p2.length && vis2[p2.gene[b]])
			b++;
		if(b < p2.length)
			s2.gene[i] = p2.gene[b++];
	}
}

bool cmp_INDIV(INDIV a, INDIV b)
{
	return a.evaluation() < b.evaluation();
}
class GA
{
public:

	void operator() (const blocked_range<int>& r) const
	{
		printf("%d\n", r.begin());
		int lev = r.begin() / sub_size < 2 ? 1 : r.begin() / sub_size < 4 ? 2 : 3;
		for (int i = r.begin(); i != r.end(); i++)
		{
			population[i].level = lev;
		}
		for (int i = r.begin(); i < r.end(); i += 2)
		{
			double pp = (double)(rand() % 100) / 100.0;
			if (pp > Pc)
				continue;
			INDIV tmp[4]; tmp[0] = population[i]; tmp[1] = population[i + 1];

			/*INDIV ss1, ss2;
			INDIV *s1 = &ss1, *s2 = &ss2;*/
			tmp[3].level = tmp[2].level = population[i].level;
			tmp[3].length = tmp[2].length = population[i].length;
			Crossover(tmp[0], tmp[1], tmp[2], tmp[3]);


			std::sort(tmp, tmp + 4, cmp_INDIV);
			//std::cout << lev << std::endl;
			population[i] = tmp[0]; population[i + 1] = tmp[1];
			//delete tmp[2], tmp[3];
		}

		for (int i = r.begin(); i != r.end(); i++)
		{
			double pp = (double)(rand() % 100) / 100.0;
			if (pp > lev * Pm)
				continue;
			population[i].Mutation();
		}
		std::sort(population + r.begin(), population + r.end(), cmp_INDIV);

	}
};

class Select2Pass
{
public:
	void operator() (const blocked_range<int>& r) const
	{
		bool vis[sub_size + 5];
		std::memset(vis, 0, sizeof(vis));
		int id = r.begin() / sub_size + 1, len1 = sub_size / 3, len2 = len1 * 2;
		if (id > 3)
			return;
		for (int i = 0; i < len1; i++)
		{
			int pos = rand() % len2;
			while (vis[pos])
			{
				pos = rand() % len2;
			}
			vis[pos] = 1;
			std::swap(population[r.begin() + len1 + pos], population[(id * 2 - 1) * sub_size + i]);
		}
		for (int i = 0; i < len1; i++)
		{
			int pos = rand() % len2;
			while (vis[pos])
			{
				pos = rand() % len2;
			}
			vis[pos] = 1;
			std::swap(population[r.begin() + len1 + pos], population[id * 2 * sub_size + i]);
		}
		printf("%d\n", id);
		std::sort(population + r.begin(), population + r.end(), cmp_INDIV);

	}
};

class Init
{
public:

	void operator() (const blocked_range<int>& r) const
	{
		bool vis[LENGTH];
		int lev = r.begin() / sub_size < 2 ? 1 : r.begin() / sub_size < 4 ? 2 : 3;
		for (int i = r.begin(); i < r.end(); i++)
		{
			memset(vis, 0, sizeof(vis));
			INDIV tmp;
			tmp.length = len;
			tmp.level = lev;
			for (int j = 0; j < len; j++)
			{
				int cur = rand() % n + 1;
				while (vis[cur])
				{
					cur = rand() % n + 1;
				}
				vis[cur] = 1;
				tmp.gene[j] = cur;
			}
			population[i] = tmp;
		}
	}
};

int main()
{
	srand((unsigned)time(0));
//	std::cin >> n; len = n;
//	for (int i = 1; i <= n; i++)
//	{
//		int x;
//		std::cin >> x >> node[i].x >> node[i].y;
//	}
    Read_TSP(Map, len);
	parallel_for(blocked_range<int>(0, sub_size * 8, sub_size), Init());
	double last = 1e9;
	int count = 0, epoch = 0;
	while (epoch < Epoch && count < Count)
	{
		parallel_for(blocked_range<int>(0, sub_size * 8, sub_size), GA());


		parallel_for(blocked_range<int>(0, 4 * sub_size, sub_size), Select2Pass());
		if (fabs(population[0].evaluation() - last) < eps)
			count++;
		else
			count = 0;
		if(last > population[0].evaluation())
			last = population[0].evaluation();
		printf("%lf\n", last);

		epoch++;
	}

	for (int i = 0; i < len; i++)
	{
		printf("%d ", population[0].gene[i]);
	}
	printf("\n%lf\n", population[0].evaluation());
	for (int i = 0; i < population[0].length; i++)
		std::cin >> population[0].gene[i];
	//population[1].gene[105] = { 1 14 13 12 7 6 15 5 11 9 10 19 20 21 16 3 2 17 22 4 18 8 };
	printf("\n%lf\n", population[0].evaluation());
}
