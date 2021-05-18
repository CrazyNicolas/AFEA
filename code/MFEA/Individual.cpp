#include "Individual.h"
#include "Matrix.h"
#include "Ackley.h"
#include "Rastrigin.h"
#include "Sphere.h"
#include <algorithm>
#include <cmath>
#include <time.h>
using namespace std;

Individual::Individual(int _length, int num_problems): gene(1, _length)		//Initializa individuals in populations
{
	length = _length;
	scalar_fitness = skill_factor = -1;
	for (int i = 0; i < num_problems; i++)
		factorial_cost[i] = 1e9;
}


void Individual::Update_Cost(Problem **problem_set, int num_problems)
{
	for (int i = 0; i < num_problems; i++)
		factorial_cost[i] = problem_set[i]->solve(gene);
}

void Individual::Init()
{
	for(int i = 0; i < length; i++)
		gene[0][i] = double(rand()) / RAND_MAX;
}

void Individual::Int_Init()
{
	for (int i = 0; i < length; i++)
		gene[0][i] = i + 1;
	int i = 0, len = length - 1, x;
	while (len > 0)
	{
		x = rand() % len + 1;
		swap(gene[0][i], gene[0][i + x]);
		i++; len--;
	}
}

bool Individual::operator<(Individual& obj) const
{
	return scalar_fitness > obj.scalar_fitness;
}

void Individual::Print_Solution(int dim)
{
	Matrix var = gene.cut(1, dim);
	struct Node
	{
		double value; int id;
		bool operator < (Node x)
		{
			return value < x.value;
		}
	};
	Node* node = new Node[dim];
	for (int i = 0; i < dim; i++)
	{
		node[i].value = var[0][i];
		node[i].id = i;
	}
	sort(node, node + dim);
	int* path = new int[dim];
	for (int i = 0; i < dim; i++)
	{
		path[node[i].id] = i;
	}
	for (int i = 0; i < dim; i++)
		printf("%d ", path[i] + 1);
	printf("\n");
	delete[] node;
	delete[] path;
}

Individual::~Individual()
{
}
