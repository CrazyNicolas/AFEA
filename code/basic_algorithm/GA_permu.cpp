#include <iostream>
#include <algorithm>
#include <cstring>
#include <time.h>
#include <map>
#include "../IOtool/IOtool.h"

using namespace std;
const int N = 300, LENGTH = 130;
int n, len, Epoch = 2500, P, p;
double Pm = 0.06, alpha = 1.0;
double*** Map;

double Distance(int a, int b)
{
	return Map[p][a][b];
}

class INDIV
{
public:
	int length;
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
		int i = 0, leng = length - 1, x;
		while (leng > 0)
		{
			x = rand() % leng + 1;
			swap(gene[i], gene[i + x]);
			i++; leng--;
		}
	}
	void print()
	{
		for (int i = 0; i < length; i++)
			printf("%d ", gene[i]);
	}
	double evaluation()
	{
		double res = 0;
		for (int i = 0; i < length; i++)
		{
			res += Map[p][gene[i]][gene[(i + 1) % length]];
		}
		return res;
	}
	void Mutation()
	{
		int a = rand() % length, b = rand() % length;
		while(a == b)
			b = rand() % length;
		swap(gene[a], gene[b]);
	}

};
INDIV population[N];

bool cmp(INDIV &a, INDIV &b)
{
    return a.evaluation() < b.evaluation();
}

void PMX(INDIV Pa, INDIV Pb, INDIV& Ca, INDIV& Cb)
{
	int x = rand() % len;
	int y = rand() % len;
	if (x > y) swap(x, y);
	map<int, int> pre, nex;
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
	//Pa.gene.print(); Pb.gene.print();
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
	//Ca.gene.print(); Cb.gene.print();
	//printf("\n");

	delete[] vis1;
	delete[] vis2;
	delete[] vis;
}


void getOffspring()
{
	int index = n, cur_size = n * 2, num_candidate = n;
	int* candidate = new int[n];
	for (int i = 0; i < n; i++)
		candidate[i] = i;
	while (index < cur_size)
	{
		int Xa = rand() % num_candidate, Xb = rand() % num_candidate;
		while (Xa == Xb)
		{
			Xb = rand() % num_candidate;
		}
		int Pa = candidate[Xa], Pb = candidate[Xb];
		swap(candidate[Xa], candidate[--num_candidate]);
		swap(candidate[Xb], candidate[--num_candidate]);
		INDIV Ca, Cb; Ca.Init(); Cb.Init();
		PMX(population[Pa], population[Pb], Ca, Cb);
		Ca.Mutation();  Cb.Mutation();
		population[index++] = Ca;  population[index++] = Cb;
	}
	delete[] candidate;
}

void select()
{
    sort(population, population + n * 2, cmp);
    /*
    int index = 1, s = n - index;
	while (index < n)
	{
		int x = rand() % s + index, y = rand() % s + index;
		if (population[x].evaluation() > population[y].evaluation())
			swap(population[index++], population[x]);
		else
			swap(population[index++], population[y]);
	}
	*/
}

int main(int argc, char* argv[])
{
    P = atoi(argv[2]);
    char* path = argv[1];
	Map = Read_TSP_batch(path, P);
	//for (int i = 1; i <= len; i++)
	//{
	//	for (int j = 1; j <= len; j++)
	//		printf("%lf ", Map[i][j]);
	//	printf("\n");
	//}
	srand(unsigned(time(0)));
	for(p = 0; p < P; p++)
    {
        n = 100;
        len = Map[p][0][0];
        for (int i = 0; i < n; i++)
        {
            population[i].Init();
        }
        int epoch = 0;
        while (epoch < Epoch)
        {
            epoch++;
            getOffspring();
            select();
        }
        population[0].print();
        printf("%lf\n", population[0].evaluation());
	}
}
