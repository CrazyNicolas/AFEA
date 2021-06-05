#include "Internal_2.h"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <time.h>
#include <cmath>
#include <vector>
#include <map>
using namespace std;

Internal_2::Internal_2()
{
	n = 100;
	Epoch = 1000;
	Pm = 0.01;
	Pc = 0.8;
}

double Internal_2::Distance(int a, int b)
{
	return Map[a][b];
}

void Internal_2::CX2(INDIV Pa, INDIV Pb, INDIV& Ca, INDIV& Cb)
{
	int a = rand() % len, b = rand() % len;
	while (b == a)
		b = rand() % len;
	if (a > b)
		swap(a, b);
	bool vis1[LENGTH + 1], vis2[LENGTH + 1];
	memset(vis1, 0, sizeof(vis1));
	memset(vis2, 0, sizeof(vis2));
	int ia = 0, ib = 0;
	for (int i = 0; i < a; i++)
	{
		Ca.gene[ia++] = Pa.gene[i];
		Cb.gene[ib++] = Pb.gene[i];
		vis1[Pa.gene[i]] = vis2[Pb.gene[i]] = 1;
	}
	double La = 0, Lb = 0;
	for (int i = a; i < b; i++)
	{
		La += Distance(Pa.gene[i], Pa.gene[i + 1]);
		Lb += Distance(Pb.gene[i], Pb.gene[i + 1]);
	}
	if (La < Lb)
	{
		for (int i = a; i <= b; i++)
		{
			if (!vis1[Pa.gene[i]])
				Ca.gene[ia++] = Pa.gene[i];
			if (!vis2[Pa.gene[i]])
				Cb.gene[ib++] = Pa.gene[i];
			vis1[Pa.gene[i]] = vis2[Pa.gene[i]] = 1;
		}
	}
	else
	{
		for (int i = a; i <= b; i++)
		{
			if (!vis1[Pb.gene[i]])
				Ca.gene[ia++] = Pb.gene[i];
			if (!vis2[Pb.gene[i]])
				Cb.gene[ib++] = Pb.gene[i];
			vis1[Pb.gene[i]] = vis2[Pb.gene[i]] = 1;
		}
	}
	for (int i = b + 1; i < len; i++)
	{
		if (!vis1[Pb.gene[i]])
			Ca.gene[ia++] = Pb.gene[i];
		if (!vis2[Pa.gene[i]])
			Cb.gene[ib++] = Pa.gene[i];
		vis1[Pb.gene[i]] = vis2[Pa.gene[i]] = 1;
	}
	if (ia >= len && ib >= len)
		return;
	vector<int> miss1, miss2;
	for (int i = 1; i <= len; i++)
	{
		if (!vis1[i])
			miss1.push_back(i);
		if (!vis2[i])
			miss2.push_back(i);
	}
	INDIV Ca1 = Ca, Ca2 = Ca;
	for (int i = ia - 1; i >= 0; i--)
	{
		swap(Ca2.gene[i], Ca2.gene[i + miss1.size()]);
	}
	for (int i = 0; i < miss1.size(); i++)
	{
		Ca1.gene[ia + i] = Ca2.gene[i] = miss1[i];
	}
	if (Ca1.evaluation() < Ca2.evaluation())
		Ca = Ca1;
	else
		Ca = Ca2;
	INDIV Cb1 = Cb, Cb2 = Cb;
	for (int i = ib - 1; i >= 0; i--)
	{
		swap(Cb2.gene[i], Cb2.gene[i + miss2.size()]);
	}
	for (int i = 0; i < miss2.size(); i++)
	{
		Cb1.gene[ib + i] = Cb2.gene[i] = miss2[i];
	}
	if (Cb1.evaluation() < Cb2.evaluation())
		Cb = Cb1;
	else
		Cb = Cb2;
	//delete[] vis1, vis2;
}

void Internal_2::getOffspring()
{
	int index = n, cur_size = n * 2, num_candidate = n;
	int* candidate = new int[n];
	for (int i = 0; i < n; i++)
		candidate[i] = i;
	while (num_candidate > 0)
	{
		int Xa = rand() % num_candidate, Xb = rand() % num_candidate;
		while (Xa == Xb)
		{
			Xb = rand() % num_candidate;
		}
		int Pa = candidate[Xa], Pb = candidate[Xb];
		swap(candidate[Xa], candidate[--num_candidate]);
		swap(candidate[Xb], candidate[--num_candidate]);
		if (double(rand()) / RAND_MAX < Pc)
		{
			INDIV Ca, Cb; Ca.Init(len, Pm, Pc, Map); Cb.Init(len, Pm, Pc, Map);
			CX2(population[Pa], population[Pb], Ca, Cb);
			Ca.Mutation();  Cb.Mutation();
			population[index++] = Ca;  population[index++] = Cb;
		}
	}
	sort(population, population + index);
	delete[] candidate;
}

void Internal_2::solve(char* data_path)
{
	Map = Read_TSP(data_path, len);
	srand(unsigned(time(0)));
	for (int i = 0; i < n; i++)
	{
		population[i].Init(len, Pm, Pc, Map);
	}
	int epoch = 0;
	while (epoch < Epoch)
	{
		epoch++;
		getOffspring();
	}
	population[0].print();
	//res = new int[len];
	//for (int i = 0; i < len; i++)
	//	res[i] = population[0].gene[i];
}


Internal_2::~Internal_2()
{
}
