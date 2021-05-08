#include "Individual.h"
#include "Matrix.h"
#include "Ackley.h"
#include "Rastrigin.h"
#include "Sphere.h"
#include <algorithm>
#include <cmath>
#include <time.h>
using namespace std;

Individual::Individual(int _length, int num_problems): gene(1, _length)
{
	length = _length;
	scalar_fitness = skill_factor = -1;
	//gene = Matrix(1, length);
	//factorial_cost = new double[num_problems];
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
	//srand((unsigned)time(0));
	for(int i = 0; i < length; i++)
		gene[0][i] = double(rand()) / RAND_MAX;
}

bool Individual::operator<(Individual& obj) const
{
	return scalar_fitness > obj.scalar_fitness;
}

Individual::~Individual()
{
	//delete gene;
	//delete[] factorial_cost;
}
