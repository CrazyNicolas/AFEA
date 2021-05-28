#include <iostream>
#include <algorithm>
#include <cstring>
#include <time.h>
#include <map>
#include "IOtool.h"

using namespace std;
const int N = 300, LENGTH = 260;
int n, len, Epoch = 10000;
double Pm = 0.06, alpha = 1.0;
double** Map;

double Distance(int a, int b)
{
	return Map[a][b];
}

class INDIV
{
public:
	int length;
	double gene[LENGTH], momentum[LENGTH];
	bool constrain()
	{
		return true;
	}
	void Init()
	{
		length = len;
		memset(momentum, 0, sizeof(momentum));
		for (int i = 0; i < length; i++)
			gene[i] = double(rand()) / RAND_MAX;
	}
	void print()
	{
		for (int i = 0; i < length; i++)
			printf("%lf ", gene[i]);
		printf("\n");
	}
	double Ackley()
	{
		double res = 0, sum1 = 0, sum2 = 0;
		for (int i = 0; i < length; i++)
		{
			sum1 += (gene[i] - 0.5) * (gene[i] - 0.5);
			sum2 += cos(2 * acos(-1) * (gene[i] - 0.5));
		}
		sum1 /= length; sum2 /= length;
		return -20 * exp(-0.2 * sqrt(sum1)) - exp(sum2) + 20 + exp(1);
	}
	double evaluation()
	{
		//return Ackley();
		struct Node
		{
			double value; int id;
			bool operator < (Node x)
			{
				return value < x.value;
			}
		};
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
	bool operator < (INDIV x)
	{
		return evaluation() < x.evaluation();
	}
};
INDIV population[N];

void SBX(INDIV Pa, INDIV Pb, INDIV& Ca, INDIV& Cb)
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
		SBX(population[Pa], population[Pb], Ca, Cb);
		Ca.Momentum_Mutation();  Cb.Momentum_Mutation();
		population[index++] = Ca;  population[index++] = Cb;
	}
	delete[] candidate;
}

int main()
{
	Map = Read_TSP((char*)"TSP16i.txt", len);
	//for (int i = 1; i <= len; i++)
	//{
	//	for (int j = 1; j <= len; j++)
	//		printf("%lf ", Map[i][j]);
	//	printf("\n");
	//}
	srand(unsigned(time(0)));
	n = 100;// len = 30;
	for (int i = 0; i < n; i++)
	{
		population[i].Init();
	}
	int epoch = 0;
	while (epoch < Epoch)
	{
		epoch++;
		getOffspring();
		sort(population, population + n * 2);
		for(int i = 0; i < 10; i++)
			population[i].print();
		//printf("%lf\n\n", population[0].evaluation());
		printf("\n\n");
	}
	population[0].print();
	printf("%lf\n", population[0].evaluation());
}