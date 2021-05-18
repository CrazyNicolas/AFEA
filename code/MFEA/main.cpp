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
int dim = 0, dim2 = 20, num_pro = 2, popul = 100;


Matrix MA = Matrix::Rand(dim, dim);
Matrix opt(1, dim);
Ackley ackley_1;
Matrix MR = Matrix::Rand(dim, dim);
Rastrigin rastrigin_1;
Sphere sphere;
Ackley ackley_2;
Rastrigin rastrigin_2;
TSP tsp16, tsp22, tsp48, tsp52;
Problem** problem_set;
double** map = new double*[100], x[100], y[100];
void read(char* path, int n)	//Read in n demension TSP coordinate data from path and store in map
{
	FILE* file = fopen(path, "r");
	int s;
	for (int i = 0; i < n; i++)
	{
		fscanf(file, "%d %lf %lf", &s, &x[i], &y[i]);
		//printf("%f %f\n", x[i], y[i]);
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
	read((char*)"TSP48.txt", 48);
	Matrix map48(map, 48, 48);

	problem_set = new Problem*[num_pro];
	ackley_1.Init(MA, dim, opt, 0, 50);
	rastrigin_1.Init(MR, dim, opt, 0, 50);
	sphere.Init(dim, opt, 0, 50);
	ackley_2.Init(MA.cut(dim2, dim2), dim2, opt, 0, 50);
	rastrigin_2.Init(MR.cut(dim2, dim2), dim2, opt, 0, 50);
	tsp16.Init(16, map16);
	tsp22.Init(22, map22);
	tsp48.Init(48, map48);
	tsp52.Init(52, map52);

	//double op16[16] = { 1, 14, 13, 12, 7, 6, 15, 5, 11, 9, 10, 16, 3, 2, 4, 8 };
	double op16[16] = { 4, 2, 3, 1, 16, 12, 13, 14, 6, 7, 10, 9, 11, 5, 15, 8 };
	double op22[22] = { 1,14,13,12,7,6,15,5,11,9,10,19,20,21,16,3,2,17,22,4,18,8 };
	double op48[48] = { 1,8,38,31,44,18,7,28,6,37,19,27,17,43,30,36,46,33,20,47,21,32,39,48,5,42,24,10,45,35,4,26,2,29,34,41,16,22,3,23,14,25,13,11,12,15,40,9 };
	double op52[52] = { 1,49,32,45,19,41,8,9,10,43,33,51,11,52,14,13,47,26,27,28,12,25,4,6,15,5,24,48,38,37,40,39,36,35,34,44,46,16,29,50,20,23,30,2,7,42,21,17,3,18,31,22 };
	printf("%lf\n", tsp16.solve(Matrix(op16, 16)));
	printf("%lf\n", tsp22.solve(Matrix(op22, 22)));
	printf("%lf\n", tsp48.solve(Matrix(op48, 48)));
	printf("%lf\n", tsp52.solve(Matrix(op52, 52)));

	problem_set[0] = &tsp16;
	problem_set[1] = &tsp22;
	//problem_set[2] = &tsp48;
	//problem_set[3] = &tsp22;
	//problem_set[4] = &sphere;

	for (int i = 0; i < num_pro; i++)
		dim = max(dim, problem_set[i]->dim);

	Population population(dim, popul, num_pro, 1, 0.3);
	population.Init_Evaluate(problem_set, popul);

	int epoch = 0;
	while (epoch < Epoch)
	{
		epoch++;
		population.getOffspring(problem_set);
		population.Select();

		for (int i = 0; i < num_pro; i++)
		{
			printf("%d: ", i);
			//population.Get_Best(i).gene.print();
			population.Get_Best(i).Print_Solution(problem_set[i]->dim);
			printf("%lf\n\n", problem_set[i]->solve(population.Get_Best(i).gene));
		}
		//for (int i = 0; i < popul; i++)
		//	printf("%lf ", problem_set[0]->solve(population[i].gene));
		//printf("\n\n");
		//system("pause");
	}
	for (int i = 0; i < popul; i++)
	{
		printf("%d    ", population[i].skill_factor);
		population[i].gene.print();
	}
	printf("\n\n");
	clock_t en = clock();
	cout << double(en - st) / CLOCKS_PER_SEC << endl;
}

