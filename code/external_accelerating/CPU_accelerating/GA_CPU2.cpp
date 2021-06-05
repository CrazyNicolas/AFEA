#include <iostream>
#include <map>
#include <cstring>
#include <algorithm>
#include <cmath>
#include "oneapi/tbb.h"
#include "../../IOtool/IOtool.h"
#pragma warning(disable : 4996)
using namespace oneapi::tbb;

const int N = 50, LENGTH = 260, popul_size = 500;
int n, len, Epoch = 100, Count = 50, chain_length = 100, rule;
double Pc = 0.9, Pm = 0.05, cool_rate = 0.9, initial_T = 100, termination_T = 0.01, eps = 1e-5;
double** Map;

unsigned Random[N * N];

void Init_Random()
{
	for (int i = 0; i < n * n; i++)
		Random[i] = rand();
}

double Distance(int a, int b)
{
	return Map[a][b];
}

class INDIV
{
public:
	int length, state;
	int gene[LENGTH];
	bool constrain()
	{
		return true;
	}
	void Init()
	{
		length = len;
		for (int i = 0; i < length; i++)
			gene[i] = i + 1;
		int i = 0, len = length - 1, x;
		while (len > 0)
		{
			x = rand() % len + 1;
			std::swap(gene[i], gene[i + x]);
			i++; len--;
		}
		state = rand() % 2;
	}
	void print()
	{
		for (int i = 0; i < length; i++)
			printf("%d ", gene[i]);
		printf("\n");
	}
	double evaluation()
	{
		double res = 0;
		for (int i = 0; i < length; i++)
		{
			res += Distance(gene[i], gene[(i + 1) % length]);
		}
		return res;
	}
	void Mutation()
	{
		int pos1 = rand() % length;
		int pos2 = rand() % length;
		if(pos1 > pos2)
			std::swap(pos1, pos2);
		while (pos1 < pos2)
		{
			std::swap(gene[pos1++], gene[pos2--]);
		}
	}
};
INDIV population[N * N], sub_popul[N * N];

void PMX(INDIV Pa, INDIV Pb, INDIV& Ca, INDIV& Cb)
{
	int x = rand() % len;
	int y = rand() % len;
	while (x == y)
	{
		y = rand() % len;
	}
	if (x > y) std::swap(x, y);
	std::map<int, int> pre, nex;
	for (int i = 0; i < len; i++)
	{
		Ca.gene[i] = Pb.gene[i];
		Cb.gene[i] = Pa.gene[i];
	}
	bool* vis1 = new bool[len + 1];
	bool* vis2 = new bool[len + 1];
	bool* vis = new bool[len + 1];
	for (int i = 0; i <= len; i++)
		vis1[i] = vis2[i] = vis[i] = false;
	//Pa.print(); Pb.print();
	for (int i = x; i <= y; i++)
	{
		int xx = Pa.gene[i], yy = Pb.gene[i];
		vis1[yy] = vis2[xx] = 1;
		if (xx == yy)
			continue;
		nex[xx] = yy;
		pre[yy] = xx;
	}
	for (int i = x; i <= y; i++)
	{
		int xx = Pa.gene[i], yy = xx;
		if (vis[xx])
			continue;
		vis[xx] = 1;
		int Left = 0, Right = 0;
		while (pre[xx] > 0 && !vis[pre[xx]])
		{
			xx = pre[xx];
			vis[xx] = 1;
		}
		Left = xx;
		while (nex[yy] > 0 && !vis[nex[yy]])
		{
			yy = nex[yy];
			vis[yy] = 1;
		}
		Right = yy;
		pre[Left] = Right;
		nex[Right] = Left;
	}

	//Ca.gene.print(); Cb.gene.print();

	for (int i = 0; i < len; i++)
	{
		if (i >= x && i <= y)
			continue;
		int xx = Pa.gene[i], yy = Pb.gene[i];
		while (vis1[xx])
		{
			xx = nex[xx];
		}
		Ca.gene[i] = xx;
		vis1[xx] = 1;
		while (vis2[yy])
		{
			yy = nex[yy];
		}
		Cb.gene[i] = yy;
		vis2[yy] = 1;
	}
	//Ca.print(); Cb.print();
	//printf("\n");

	delete[] vis1;
	delete[] vis2;
	delete[] vis;
}

class Init
{
public:

	void operator() (const blocked_range<int>& r) const
	{
		srand(Random[r.begin()]);
		for (int i = r.begin(); i < r.end(); i++)
		{
			population[i].Init();
		}
	}
};

class SCGA
{
public:

	void operator() (const blocked_range<int>& r) const
	{
		srand(Random[r.begin()]);
		for (int i = r.begin(); i < r.end(); i++)
		{
			int x = i / n, y = i % n, K = 0;
			INDIV Pa = population[i];
			INDIV Pb = population[x * n + (y + 1) % n];
			for(int j = -1; j <= 1; j++)
				for (int k = -1; k <= 1; k++)
				{
					int index = (x + j + n) % n * n + (y + k + n) % n;
					if (index == i)
						continue;
					K += population[index].state;
					if (population[index].state > 0 && Pb.evaluation() > population[index].evaluation())
						Pb = population[index];
				}
			if (Pa.state == 1)
			{
				if (!((rule == 1 && (K == 2 || K == 3)) || ((rule == 2) && (K > 0 && K < 5)) || ((rule == 3) && (K % 2 == 0))))
					Pa.state = 0;
			}
			else
			{
				if (((rule == 1) && (K == 3)) || ((rule == 2) && (K >= 4 && K <= 7)) || ((rule == 3) && (K % 2 == 1)))
					Pa.state = 1;
			}
			population[i].state = Pa.state;
			if (Pa.state == 0)
				continue;
			INDIV Ca = Pa, Cb = Pb;
			PMX(Pa, Pb, Ca, Cb);
			Ca.Mutation(); Cb.Mutation();
			if (Ca.evaluation() > Cb.evaluation())
				std::swap(Ca, Cb);
			if (Ca.evaluation() < Pa.evaluation())
				sub_popul[i] = Ca;
			else
				sub_popul[i] = Pa;
		}
	}
};

class SA
{
public:
	INDIV Sa(INDIV x) const
	{
		double T = initial_T;
		INDIV res = x;
		while (T > termination_T)
		{
			for (int i = 1; i <= chain_length; i++)
			{
				INDIV y = x;
				y.Mutation();
				double df = y.evaluation() - x.evaluation(), P = exp(-df / T);
				if (df < 0 || P > (double(rand()) / RAND_MAX))
				{
					x = y;
				}
				if (x.evaluation() < res.evaluation())
					res = x;
			}
			T = T * cool_rate;
		}
		return res;
	}
	void operator() (const blocked_range<int>& r) const
	{
		srand(Random[r.begin()]);
		for (int i = r.begin(); i < r.end(); i++)
		{
			if (population[i].state > 0)
			{
				population[i] = Sa(sub_popul[i]);
			}
		}
	}
};


int main()
{
	srand((unsigned)time(0));
	n = 10; rule = 2;

	Map = Read_TSP((char*)"TSP48i.txt", len);
	//for (int i = 1; i <= n; i++)
	//{
	//	for (int j = 1; j <= n; j++)
	//		printf("%lf ", Map[i][j]);
	//	printf("\n");
	//}
	Init_Random();
	parallel_for(blocked_range<int>(0, n * n, 5), Init());
	int epoch = 0;
	while (epoch < Epoch)
	{
		Init_Random();
		parallel_for(blocked_range<int>(0, n * n, 5), SCGA());
		Init_Random();
		parallel_for(blocked_range<int>(0, n * n, 5), SA());
		epoch++;
		INDIV best = population[0]; int live = 0;
		for (int i = 1; i < n * n; i++)
		{
			if (population[i].state > 0)
				live++;
			if (best.evaluation() > population[i].evaluation())
				best = population[i];
		}
		best.print();
		printf("%d %lf\n", live, best.evaluation());
	}
}
