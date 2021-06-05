#pragma once
#include <iostream>
#include <algorithm>
#include <cstring>
#include <time.h>
#include <vector>
#include <map>
#include <cmath>

using namespace std;
class Internal_1
{
public:
	Internal_1();
	static const int N = 300, LENGTH = 260;
	/* parameter setting */

	int n,				 // Size of population
		Epoch,			 // Number of iterations
		len;				 // Length of TSP (number of nodes)
	double Pm,			 // Mutation rate
		   alpha;		 // Parameter proposed in paper. See readme
	double** Map;		 // Distance matrix
	struct INDIV
	{
		int length;
		double gene[LENGTH], momentum[LENGTH];
		double Pm, alpha, ** Map;
		bool constrain()
		{
			return true;
		}
		void Init(int len, double _Pm, double ** _Map)
		{
			length = len; Pm = _Pm;
			Map = _Map;
			memset(momentum, 0, sizeof(momentum));
			for (int i = 0; i < length; i++)
				gene[i] = double(rand()) / RAND_MAX;
		}
		struct Node
		{
			double value; int id;
			bool operator < (const Node x) const
			{
				return this->value < x.value;
			}
		};
		void print()
		{
			Node* node = new Node[length];
			for (int i = 0; i < length; i++)
			{
				node[i].value = gene[i];
				node[i].id = i;
			}
			sort(node, node + length);
			int* path = new int[length];
			for (int i = 0; i < length; i++)
			{
				path[node[i].id] = i + 1;
			}
			for (int i = 0; i < length; i++)
				printf("%d ", path[i]);
			printf("\n");
			delete[] node;
			delete[] path;
		}
		double evaluation() const
		{
			Node* node = new Node[length];
			for (int i = 0; i < length; i++)
			{
				node[i].value = gene[i];
				node[i].id = i;
			}
			sort(node, node + length);
			int* path = new int[length];
			for (int i = 0; i < length; i++)
			{
				path[node[i].id] = i + 1;
			}
			double res = 0;
			for (int i = 0; i < length; i++)
			{
				res += Map[path[i]][path[(i + 1) % length]];
			}
			delete[] node;
			delete[] path;
			return res;
		}
		double Gaussian(double x)
		{
			double mum = 5, res = x;
			double u = double(rand()) / RAND_MAX;
			if (u <= 0.5)
			{
				res *= pow(2 * u, 1 / (1 + mum));
			}
			else
			{
				res = (x - 1) * pow(2 - 2 * u, 1 / (1 + mum)) + 1;
			}
			return res;
		}
		void Momentum_Mutation()
		{
			for (int i = 0; i < length; i++)
			{
				if (double(rand()) / RAND_MAX < Pm)
				{
					double y = gene[i];
					gene[i] = gene[i] + alpha * momentum[i] + Gaussian(gene[i]);
					momentum[i] = gene[i] - y;
				}
			}
		}
		bool operator < (const INDIV x) const
		{
			return evaluation() < x.evaluation();
		}
	};
	INDIV population[N];

	void SBX(INDIV Pa, INDIV Pb, INDIV& Ca, INDIV& Cb);
	void getOffspring();
	void solve(char* data_path);
	~Internal_1();
};