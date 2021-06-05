#include "CVRP.h"

double eps = 1e-5;

CVRP::CVRP()
{
}

void CVRP::Init(int _dim, Matrix _map, double* _demand, int _track, double _capacity)
{
	dim = _dim;
	map = _map;
	demand = new double[dim + 1];
	for (int i = 0; i <= dim; i++)
		demand[i] = _demand[i];
	track = _track;
	capacity = _capacity;
}

double CVRP::solve(Matrix var)
{
	int* path = new int[dim];
	double res = 0; int index = 0, tra = track;
	for (int i = 0; i < var.b; i++)
	{
		if (var[0][i] <= dim)
			path[index++] = var[0][i];
	}
	double cur = 0;
	for (int i = 0; i < dim; i++)
	{
		if (cur + demand[path[i]] >= capacity)
		{
			res += map[path[i - 1]][0];
			tra--;
			cur = 0;
		}
		if (cur <= eps)
			res += map[0][path[i]];
		else
			res += map[path[i - 1]][path[i]];
		cur += demand[path[i]];
	}
	if (cur > eps)
	{
		res += map[path[dim - 1]][0];
		tra--;
	}
	if (tra < 0)
		res = 1e9;
	delete[] path;
	return res;
}

CVRP::~CVRP()
{
}
