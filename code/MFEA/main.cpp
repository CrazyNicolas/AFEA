#include <iostream>
#include "Population.h"
#include "Individual.h"
#include "Ackley.h"
#include "Rastrigin.h"
#include "Sphere.h"
#include "Matrix.h"
#include "TSP.h"
#include "Problem.h"
#include <algorithm>
#include <iostream>
#include <cstring>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <time.h>
using namespace std;
#pragma warning(disable : 4996)
int Epoch = 500;
int dim = 30, dim2 = 20, num_pro = 3, popul = 100;

Population population = Population(dim, popul, num_pro, 1, 0.3);
Matrix MA = Matrix::Rand(dim, dim);
Matrix opt(1, dim);
Ackley ackley_1;
Matrix MR = Matrix::Rand(dim, dim);
Rastrigin rastrigin_1;
Sphere sphere;
Ackley ackley_2;
Rastrigin rastrigin_2;
TSP tsp16, tsp22, tsp52;
Problem** problem_set;
double** map = new double*[100], x[100], y[100];
void read(char* path, int n)
{
	FILE* file = fopen(path, "r");
	int s;
	for (int i = 0; i < n; i++)
	{
		fscanf(file, "%d %lf %lf", &s, &x[i], &y[i]);
		printf("%f %f\n", x[i], y[i]);
	}
	for(int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			map[i][j] = map[j][i] = sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));
		}
	}
}
int main()
{
	clock_t st = clock();
	srand((unsigned)time(0));
	for (int i = 0; i < 100; i++)
	{
		map[i] = new double[100];
	}

	read((char*)"TSP16.txt", 16);
	Matrix map16(map, 16, 16);
	read((char*)"TSP22.txt", 22);
	Matrix map22(map, 22, 22);
	read((char*)"TSP52.txt", 52);
	Matrix map52(map, 52, 52);

	problem_set = new Problem*[num_pro];
	ackley_1.Init(MA, dim, opt, 0, 50);
	rastrigin_1.Init(MR, dim, opt, 0, 50);
	sphere.Init(dim, opt, 0, 50);
	ackley_2.Init(MA.cut(dim2, dim2), dim2, opt, 0, 50);
	rastrigin_2.Init(MR.cut(dim2, dim2), dim2, opt, 0, 50);
	tsp16.Init(16, map16);
	tsp22.Init(22, map22);
	tsp52.Init(52, map52);

	problem_set[0] = &tsp16;
	problem_set[1] = &tsp22;
	problem_set[2] = &tsp52;
	//problem_set[3] = &ackley_1;
	//problem_set[4] = &ackley_2;
	population.Init_Evaluate(problem_set, popul);

	int epoch = 0;
	while (epoch < Epoch)
	{
		epoch++;
		population.getOffspring(problem_set);
		population.Select();

		for (int i = 0; i < num_pro; i++)
		{
			printf("%d %lf\n", i, problem_set[i]->solve(population.Get_Best(i).gene));
		}
		printf("\n");
	}
	clock_t en = clock();
	cout << double(en - st) / CLOCKS_PER_SEC << endl;
}

