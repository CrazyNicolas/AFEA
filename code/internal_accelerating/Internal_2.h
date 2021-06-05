#pragma once
#include <iostream>
#include <algorithm>
#include <cstring>
#include <time.h>
#include <vector>
#include <map>
#include <cmath>
#include "../IOtool/IOtool.h"

class Internal_2
{
public:
	Internal_2();
	static const int N = 300, LENGTH = 260;

	int n,			 // Size of population
		Epoch,		 // Number of iterations
		len;				 // Length of TSP (number of nodes)
	double Pm,		 // Mutation rate
	   	   Pc;		 // Parameter proposed in paper. See readme
	double** Map;			 // Distance matrix
	struct INDIV
	{
		int length; 
		int gene[LENGTH];
		double Pm, Pc, ** Map;
		bool constrain()
		{
			return true;
		}
		void Init(int len, double _Pm, double _Pc, double** _Map)
		{
			length = len; Pm = _Pm; Pc = _Pc; Map = _Map;
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
			printf("\n");
		}
		double evaluation() const
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
			if (double(rand()) / RAND_MAX < Pm)
			{
				int a = rand() % length, b = rand() % length;
				while (a == b)
					b = rand() % length;
				swap(gene[a], gene[b]);
			}
		}
		bool operator < (const INDIV x) const
		{
			return evaluation() < x.evaluation();
		}
	};
	INDIV population[N];

	double Distance(int a, int b);
	void CX2(INDIV Pa, INDIV Pb, INDIV & Ca, INDIV & Cb);
	void getOffspring();
	void solve(char* data_path);
	~Internal_2();
};


