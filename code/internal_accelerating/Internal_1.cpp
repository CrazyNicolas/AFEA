#include "Internal_1.h"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <time.h>
#include <cmath>
#include <map>
#include "../IOtool/IOtool.h"

using namespace std;

Internal_1::Internal_1()
{
	n = 100;
	Epoch = 1000;
	Pm = 0.06;
	alpha = 1.0;
}


void Internal_1::SBX(INDIV Pa, INDIV Pb, INDIV& Ca, INDIV& Cb)
{
	double mu = 3;
	for (int i = 0; i < len; i++)
	{
		double u = double(rand()) / RAND_MAX;
		double Beta = u <= 0.5 ? pow(2 * u, (1 / (mu + 1))) : pow(1 / (2 - 2 * u), (1 / (mu + 1)));
		Ca.gene[i] = min(max(0.5 * (Pa.gene[i] + Pb.gene[i]) + 0.5 * Beta * (Pb.gene[i] - Pa.gene[i]), 0.0), 1.0);
		Cb.gene[i] = min(max(0.5 * (Pb.gene[i] + Pa.gene[i]) + 0.5 * Beta * (Pa.gene[i] - Pb.gene[i]), 0.0), 1.0);
	}
}

void Internal_1::getOffspring()
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
		INDIV Ca, Cb; Ca.Init(len, Pm, Map); Cb.Init(len, Pm, Map);
		SBX(population[Pa], population[Pb], Ca, Cb);
		Ca.Momentum_Mutation();  Cb.Momentum_Mutation();
		population[index++] = Ca;  population[index++] = Cb;
	}
	delete[] candidate;
}

void Internal_1::solve(char* data_path)
{
	Map = Read_TSP(data_path, len);
	srand(unsigned(time(0)));
	for (int i = 0; i < n; i++)
	{
		population[i].Init(len, Pm, Map);
	}
	int epoch = 0;
	while (epoch < Epoch)
	{
		epoch++;
		getOffspring();
		sort(population, population + n * 2);
	}
	population[0].print();
}

Internal_1::~Internal_1()
{

}
