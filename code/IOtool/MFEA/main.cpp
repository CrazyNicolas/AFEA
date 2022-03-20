#include <iostream>
#include "Population.h"
#include "Individual.h"
#include "Ackley.h"
#include "Rastrigin.h"
#include "Sphere.h"
#include "Matrix.h"
#include "TSP.h"
#include "CVRP.h"
#include "Problem.h"
#include <Windows.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <string>
#include <time.h>
#include "../IOtool/IOtool.h"
using namespace std;
#pragma warning(disable : 4996)

/* parameter setting */
int Epoch = 500;
int dim,				// Dimension of individuals (maximal length of TSP instances)
	num_pro,			// Number of problems
	popul = 100;		// Size of population
double cr = 0.5;		// Crossover rate
Problem** problem_set;
TSP* tsp = NULL;
CVRP* cvrp = NULL;
string exePath = "";

bool Config()
{
	FILE* f = freopen((exePath + "config.txt").c_str(), "r", stdin);
	string SIZE, CR, ITER, PROBLEM, DATAPATH; int pos;
	getline(cin, SIZE);
	pos = SIZE.find("=");
	if (pos == string::npos)
		return false;
	popul = atoi(SIZE.substr(pos + 1, SIZE.size() - pos - 1).c_str());

	getline(cin, CR);
	pos = CR.find("=");
	if (pos == string::npos)
		return false;
	cr = atof(CR.substr(pos + 1, CR.size() - pos - 1).c_str());

	getline(cin, ITER);
	pos = ITER.find("=");
	if (pos == string::npos)
		return false;
	Epoch = atoi(ITER.substr(pos + 1, ITER.size() - pos - 1).c_str());

	getline(cin, PROBLEM);
	pos = PROBLEM.find("=");
	if (pos == string::npos)
		return false;
	PROBLEM = PROBLEM.substr(pos + 1, PROBLEM.size() - pos - 1);

	getline(cin, DATAPATH);
	pos = DATAPATH.find("=");
	if (pos == string::npos)
		return false;
	problem_set = new Problem*[num_pro];
	DATAPATH = DATAPATH.substr(pos + 1, DATAPATH.size() - pos - 1);
	DATAPATH += " ";
	string path = ""; double*** map; int len;
	for (int i = 0; i < DATAPATH.size(); i++)
	{
		if (DATAPATH[i] == ' ')
		{
			if (path.size() < 1)
				continue;
			if (PROBLEM == "TSP")
			{
				map = Read_TSP_batch((char*)(exePath + path).c_str(), num_pro);
				tsp = new TSP[num_pro];
				for (int j = 0; j < num_pro; j++)
				{
					len = map[j][0][0];
					map[j][0][0] = 0;
					Matrix M(map[j], len + 1, len + 1);
					tsp[j].Init(len, M);
					problem_set[j] = &tsp[j];
					dim = max(dim, len);
				}
			}
			else if (PROBLEM == "CVRP")
			{
				int truck; double capacity;
				map = Read_CVRP_batch((char*)(exePath + path).c_str(), num_pro);
				cvrp = new CVRP[num_pro];
				for (int j = 0; j < num_pro; j++)
				{
					len = map[j][0][0]; truck = map[j][1][1]; capacity = map[j][2][2];
					map[j][0][0] = map[j][1][1] = map[j][2][2] = 0;
					Matrix M(map[j], len, len);
					cvrp[j].Init(len - 1, M, map[j][len], truck, capacity);
					problem_set[j] = &cvrp[j];
					dim = max(dim, len - 1);
				}
			}
			else
				return false;
			path = "";
		}
		else
			path += DATAPATH[i];
	}
}

/* Interface: ./MFEA num_of_problem */
int main(int argc, char* argv[])
//int main()
{
	num_pro = atoi(argv[1]);
	//num_pro = 32;
	srand((unsigned)time(0));
	char exeFullPath[256]; // Full path
	GetModuleFileNameA(NULL, exeFullPath, 256);
	exePath = (string)exeFullPath;    // Get full path of the file
	int ppp = exePath.find_last_of('\\', exePath.length());
	exePath = exePath.substr(0, ppp);
	exePath += "\\";
	//exePath = "";
	if (!Config())
	{
		printf("Configuration error.\n");
		return 0;
	}
	//double** map; int len;
	//num_pro = 3;
	//problem_set = new Problem * [num_pro]; tsp = new TSP[num_pro];
	//char* path[3] = { (char*)"TSP16.txt", (char*)"TSP22.txt", (char*)"TSP48.txt" };
	//for (int i = 0; i < num_pro; i++)
	//{
	//	map = Read_TSP(path[i], len);
	//	Matrix M(map, len + 1, len + 1);
	//	tsp[i].Init(len, M);
	//	problem_set[i] = &tsp[i];
	//	dim = max(dim, len);
	//}
	popul = max(popul * num_pro, 100);
	Population population = Population(dim, popul, num_pro, cr);
	population.Init_Evaluate(problem_set, popul);
	int epoch = 0;
	while (epoch < Epoch)
	{
		epoch++;
		population.getOffspring(problem_set);
		population.Select();
		//for (int i = 0; i < num_pro; i++)
		//{
		//	population.Get_Best(i).Print_Solution(problem_set[i]->dim);
		//	printf("%lf\n", problem_set[i]->solve(population.Get_Best(i).gene));
		//}
		//printf("\n");

	}
	for (int i = 0; i < num_pro; i++)
	{
		population.Get_Best(i).Print_Solution(problem_set[i]->dim);
		printf("%lf\n", problem_set[i]->solve(population.Get_Best(i).gene));
	}

}

