#include "CVRP.h"
#include <algorithm>
#include <cstring>
#include <iostream>
using namespace std;
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

//double CVRP::solve(Matrix var)
//{
//	int* path = new int[dim];
//	double res = 0; int index = 0, tra = track;
//	for (int i = 0; i < var.b; i++)
//	{
//		if (var[0][i] <= dim)
//			path[index++] = var[0][i];
//	}
//	double cur = 0;
//	for (int i = 0; i < dim; i++)
//	{
//		if (cur + demand[path[i]] >= capacity)
//		{
//			res += map[path[i - 1]][0];
//			tra--;
//			cur = 0;
//		}
//		if (cur <= eps)
//			res += map[0][path[i]];
//		else
//			res += map[path[i - 1]][path[i]];
//		cur += demand[path[i]];
//	}
//	if (cur > eps)
//	{
//		res += map[path[dim - 1]][0];
//		tra--;
//	}
//	if (tra < 0)
//		res = 1e9;
//	delete[] path;
//	return res;
//}

double CVRP::solve(Matrix var)
{
	//int* path = new int[dim + 1];
	//double res = 0; int index = 1, tra = track;
	//for (int i = 0; i < var.b; i++)
	//{
	//	if (var[0][i] <= dim)
	//		path[index++] = var[0][i];
	//}
	//double* V = new double[dim + 1];
	//V[0] = 0;
	//for (int i = 1; i <= dim; i++)
	//	V[i] = 1e9;
	//for (int i = 1; i <= dim; i++)
	//{
	//	double load = 0, cost = 0; int j = i;
	//	do
	//	{
	//		load += demand[path[j]];
	//		if (i == j)
	//			cost = map[0][path[j]] + map[path[j]][0];
	//		else
	//			cost = cost - map[path[j - 1]][0] + map[path[j - 1]][path[j]] + map[path[j]][0];
	//		if (load <= capacity)
	//		{
	//			if (V[i - 1] + cost < V[j])
	//			{
	//				V[j] = V[i - 1] + cost;
	//			}
	//			j++;
	//		}
	//	} while (j <= dim && load <= capacity);
	//}
	//res = V[dim];
	//delete[] V, path;
	//return res;
	int* path = new int[dim + 1];
	double res = 0; int index = 1;
	for (int i = 0; i < var.b; i++)
	{
		if (var[0][i] <= dim)
			path[index++] = var[0][i];
	}
	double** dp = new double* [dim + 1];
	double** de = new double* [dim + 1];
	for (int i = 0; i <= dim; i++)
	{
		dp[i] = new double[track + 1];
		de[i] = new double[track + 1];
		for (int j = 0; j <= track; j++)
			dp[i][j] = de[i][j] = 1e9;
	}
	dp[1][1] = map[0][path[1]] + map[path[1]][0];
	de[1][1] = demand[path[1]];
	for (int i = 2; i <= dim; i++)
	{
		for (int j = 1; j <= min(i, track); j++)
		{
			if (de[i - 1][j] + demand[path[i]] <= capacity)
			{
				if (dp[i - 1][j] - map[path[i - 1]][0] + map[path[i - 1]][path[i]] + map[path[i]][0] <= dp[i - 1][j - 1] + map[path[i]][0] + map[0][path[i]])
				{
					dp[i][j] = dp[i - 1][j] - map[path[i - 1]][0] + map[path[i - 1]][path[i]] + map[path[i]][0];
					de[i][j] = de[i - 1][j] + demand[path[i]];
				}
				else
				{
					dp[i][j] = dp[i - 1][j - 1] + map[path[i]][0] + map[0][path[i]];
					de[i][j] = demand[path[i]];
				}
			}
			else
			{
				dp[i][j] = dp[i - 1][j - 1] + map[path[i]][0] + map[0][path[i]];
				de[i][j] = demand[path[i]];
			}
		}
	}
	res = dp[dim][track];
	for (int i = 0; i <= dim; i++)
	{
		delete[] de[i];
		delete[] dp[i];
	}
	delete[] de;
	delete[] dp;
	delete[] path;
	return res;
}

CVRP::~CVRP()
{
}
