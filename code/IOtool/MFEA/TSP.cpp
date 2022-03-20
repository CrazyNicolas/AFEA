#include "TSP.h"
#include <algorithm>
#include <iostream>
using namespace std;
TSP::TSP()
{
}

void TSP::Init(int _dim, Matrix _map)
{
	dim = _dim; map = _map;
}

double TSP::solve(Matrix var)
{
	/*
	var = var.cut(1, dim);
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
	double res = 0;
	for (int i = 0; i < dim; i++)
	{
		res += map[path[i]][path[(i + 1) % dim]];
	}
	delete[] node;
	delete[] path;
	*/
	int* path = new int[dim];
	double res = 0; int index = 0;
	for (int i = 0; i < var.b; i++)
	{
		if (var[0][i] <= dim)
			path[index++] = var[0][i];
	}
	for (int i = 0; i < dim; i++)
	{
		res += map[path[i]][path[(i + 1) % dim]];
	}
	delete[] path;
	return res;
}

TSP::~TSP()
{
}
