#include "IOtool.h"
#include <iostream>
#include <map>
#include <cstring>
#include <algorithm>
#include <cmath>
using namespace std;
#pragma warning(disable : 4996)

double** Read_TSP(char* path, int& n)
{
 //	int a;
 //	FILE *f = freopen(path, "r", stdin);
 //	std::cin >> n;
	//cout << path << " " << n << endl;
 //	for (int i = 1; i <= n; i++)
 //	{
 //		res[i] = new double[n + 1];
 //		std::cin >> a;	//Some benchmark data have additional index values before coordinates.
 //		std::cin >> x[i] >> y[i];
 //	}
 //	fclose(stdin);
 //	for (int i = 1; i <= n; i++)
 //		for (int j = 1; j <= n; j++)
 //			res[i][j] = res[j][i] = sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));
	FILE* file = fopen(path, "r");
	int s;
	fscanf(file, "%d", &n);
	double** res = new double* [n + 1];
	res[0] = new double[n + 1];
	double* x = new double[n + 1];
	double* y = new double[n + 1];

	for (int i = 1; i <= n; i++)
	{
		res[i] = new double[n + 1];
		fscanf(file, "%d %lf %lf", &s, &x[i], &y[i]);
		//printf("%f %f\n", x[i], y[i]);
	}
	for (int i = 1; i <= n; i++)
	{
		for (int j = 1; j <= n; j++)
		{
			res[i][j] = res[j][i] = sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));
		}
	}
	file = NULL;
	delete[] x, y;
	return res;

}

double** Read_TSP(int CITES, char* path)
{
	int a;
	freopen(path, "r", stdin);
	double **res = new double*[CITES];
	double* x = new double[CITES];
	double* y = new double[CITES];
	for (int i = 0; i < CITES; i++)
	{
		res[i] = new double[CITES];
		cin >> a;	//Some benchmark data have additional index values before coordinates.
		cin >> x[i] >> y[i];
	}
	fclose(stdin);
	for (int i = 0; i < CITES; i++)
		for (int j = 0; j < CITES; j++)
			res[i][j] = res[j][i] = sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));

	delete[] x, y;
	return res;
}

double** Read_CVRP(char* path, int& n, int &track, double &capacity)
{
	FILE* file = fopen(path, "r");
	int s;
	fscanf(file, "%d %d %lf", &n, &track, &capacity);
	double** res = new double* [n + 1];
	res[n] = new double[n + 1];
	double* x = new double[n + 1];
	double* y = new double[n + 1];

	for (int i = 0; i < n; i++)
	{
		res[i] = new double[n + 1];
		fscanf(file, "%d %lf %lf", &s, &x[i], &y[i]);
	}
	for (int i = 0; i < n; i++)
	{
		fscanf(file, "%d %lf", &s, &res[n][i]);
	}
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			res[i][j] = res[j][i] = sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));
		}
	}
	file = NULL;
	delete[] x, y;
	return res;
}