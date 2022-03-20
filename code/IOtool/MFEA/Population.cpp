#include "Population.h"
#include <map>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <time.h>
using namespace std;


Population::Population(int _dim, int _size, int _num_problems, double cr)
{
	srand((unsigned)time(0));
	size = _size; num_problems = _num_problems;
	cr_rate = cr;
	dim = _dim;
	factorial_rank = new int*[size * 2];
	for (int i = 0; i < size * 2; i++)
		factorial_rank[i] = new int[num_problems];

	popul = new Individual[size * 2];
	for (int i = 0; i < size * 2; i++)
	{
		Individual tmp(dim, num_problems);
		popul[i] = tmp;
		//popul[i].Init();
		popul[i].Int_Init();
	}
}

void Population::Init()
{
	for (int i = 0; i < size; i++)
		popul[i].Init();
}

void Population::Update_Factor(int cur_size)
{
	struct Id_Cost
	{
		int id; double cost;
		bool operator < (Id_Cost ic)
		{
			return cost < ic.cost;
		}
	};
	Id_Cost** id_cost = new Id_Cost*[num_problems];
	for(int i = 0; i < num_problems; i++)
		id_cost[i] = new Id_Cost[cur_size];
	for (int i = 0; i < cur_size; i++)
	{
		for (int j = 0; j < num_problems; j++)
		{
			id_cost[j][i].cost = popul[i].factorial_cost[j];
			id_cost[j][i].id = i;
		}
	}
	for (int i = 0; i < num_problems; i++)
		std::sort(id_cost[i], id_cost[i] + cur_size);
	for(int i = 0; i < num_problems; i++)
		for (int j = 0; j < cur_size; j++)
		{
			factorial_rank[id_cost[i][j].id][i] = j + 1;
		}
	for (int i = 0; i < cur_size; i++)
	{
		double fit = 0; int fac;
		for(int j = 0; j < num_problems; j++)
			if (1.0 / double(factorial_rank[i][j]) > fit)
			{
				fit = 1.0 / double(factorial_rank[i][j]);
				fac = j;
			}
		popul[i].scalar_fitness = fit;
		popul[i].skill_factor = fac;
	}
	for (int i = 0; i < num_problems; i++)
		delete[] id_cost[i];
	delete[] id_cost;
}

void Population::Init_Evaluate(Problem** problem_set, int cur_size)
{
	
	for (int i = 0; i < cur_size; i++)
	{
		popul[i].Update_Cost(problem_set, num_problems);
	}

	Update_Factor(cur_size);
}

void Population::SBX(Individual Pa, Individual Pb, Individual& Ca, Individual& Cb)
{
	double mu = 2; int len = dim;
	for (int i = 0; i < len; i++)
	{	
		double u = double(rand()) / RAND_MAX; 
		double Beta = u <= 0.5 ? pow(2 * u, (1 / (mu + 1))) : pow(1 / (2 - 2 * u), (1 / (mu + 1)));
		Ca.gene[0][i] = min(max(0.5 * (Pa.gene[0][i] + Pb.gene[0][i]) + 0.5 * Beta * (Pb.gene[0][i] - Pa.gene[0][i]), 0.0), 1.0);
		Cb.gene[0][i] = min(max(0.5 * (Pb.gene[0][i] + Pa.gene[0][i]) + 0.5 * Beta * (Pa.gene[0][i] - Pb.gene[0][i]), 0.0), 1.0);
	}
}

void Population::PMX(Individual Pa, Individual Pb, Individual& Ca, Individual& Cb)
{
	int x = rand() % dim;
	int y = rand() % dim;
	if (x > y) swap(x, y);
	map<int, int> pre, nex;
	for (int i = 0; i < dim; i++)
	{
		Ca.gene[0][i] = Pb.gene[0][i];
		Cb.gene[0][i] = Pa.gene[0][i];
	}
	bool* vis1 = new bool[dim + 1];
	bool* vis2 = new bool[dim + 1];
	bool *vis = new bool[dim + 1];
	for (int i = 0; i <= dim; i++)
		vis1[i] = vis2[i] = vis[i] = false;
	//Pa.gene.print(); Pb.gene.print();
	for (int i = x; i <= y; i++)
	{
		int xx = int(round(Pa.gene[0][i])), yy = int(round(Pb.gene[0][i]));
		vis1[yy] = vis2[xx] = 1;
		if (xx == yy)
			continue;
		nex[xx] = yy;
		pre[yy] = xx;
	}
	for (int i = x; i <= y; i++)
	{
		int xx = int(round(Pa.gene[0][i])), yy = xx;
		if (vis[xx])
			continue;
		vis[xx] = 1;
		int Left = 0, Right = 0;
		while (pre[xx] > 0 && !vis[pre[xx]])
		{
			xx = pre[xx];
			vis[xx] = 1;
		}
		Left = xx;
		while (nex[yy] > 0 && !vis[nex[yy]])
		{
			yy = nex[yy];
			vis[yy] = 1;
		}
		Right = yy;
		pre[Left] = Right;
		nex[Right] = Left;
	}

	//Ca.gene.print(); Cb.gene.print();

	for (int i = 0; i < dim; i++)
	{
		if (i >= x && i <= y)
			continue;
		int xx = int(round(Pa.gene[0][i])), yy = int(round(Pb.gene[0][i]));
		while (vis1[xx])
		{
			xx = nex[xx];
		}
		Ca.gene[0][i] = xx;
		vis1[xx] = 1;
		while (vis2[yy])
		{
			yy = nex[yy];
		}
		Cb.gene[0][i] = yy;
		vis2[yy] = 1;
	}
	//Ca.gene.print(); Cb.gene.print();
	//printf("\n");
	delete[] vis1;
	delete[] vis2;
	delete[] vis;
	for (int i = 0; i < Ca.length; i++)
		if (Ca.gene[0][i] != Pa.gene[0][i])
			return;
}

void Population::OX(Individual Pa, Individual Pb, Individual& Ca, Individual& Cb)
{
	int x = rand() % dim;
	int y = rand() % dim;
	while (x == y)
		y = rand() % dim;
	if (x > y) swap(x, y);
	bool* vis1 = new bool[dim + 1];
	bool* vis2 = new bool[dim + 1];
	for (int i = 0; i <= dim; i++)
		vis1[i] = vis2[i] = false;
	for (int i = x; i <= y; i++)
	{
		Ca.gene[0][i] = Pb.gene[0][i];
		vis1[int(Ca.gene[0][i])] = true;
		Cb.gene[0][i] = Pa.gene[0][i];
		vis2[int(Cb.gene[0][i])] = true;
	}
	int ia = 0, ib = 0;
	for (int i = 0; i < dim; i++)
	{
		if (i >= x && i <= y)
			continue;
		while (vis1[int(Pa.gene[0][ia])])
			ia++;
		Ca.gene[0][i] = Pa.gene[0][ia++];
		while (vis2[int(Pb.gene[0][ib])])
			ib++;
		Cb.gene[0][i] = Pb.gene[0][ib++];
	}
	delete[] vis1;
	delete[] vis2;
	for (int i = 0; i < Ca.length; i++)
		if (Ca.gene[0][i] != Pa.gene[0][i])
			return;
}

void Population::OBX(Individual Pa, Individual Pb, Individual& Ca, Individual& Cb)
{
	bool* vis1 = new bool[dim + 1];
	bool* vis2 = new bool[dim + 1];
	bool* vis = new bool[dim + 1];
	for (int i = 0; i <= dim; i++)
		vis1[i] = vis2[i] = vis[i] = false;
	for (int i = 0; i < dim / 2; i++)
	{
		int x = rand() % dim;
		while (vis[x])
			x = rand() % dim;
		vis[x] = true;
		Ca.gene[0][x] = Pb.gene[0][x];
		Cb.gene[0][x] = Pa.gene[0][x];
		vis1[int(Ca.gene[0][x])] = vis2[int(Cb.gene[0][x])] = true;
	}
	int ia = 0, ib = 0;
	for (int i = 0; i < dim; i++)
	{
		if (vis[i])
			continue;
		while (vis1[int(Pa.gene[0][ia])])
			ia++;
		Ca.gene[0][i] = Pa.gene[0][ia++];
		while (vis2[int(Pb.gene[0][ib])])
			ib++;
		Cb.gene[0][i] = Pb.gene[0][ib++];
	}
	delete[] vis1;
	delete[] vis2;
	delete[] vis;
	for (int i = 0; i < Ca.length; i++)
		if (Ca.gene[0][i] != Pa.gene[0][i])
			return;
	//Pa.Print_Solution(31); Pb.Print_Solution(31);

}

void Population::SEC(Individual Pa, Individual Pb, Individual& Ca, Individual& Cb)
{
	int x = rand() % dim;
	int y = rand() % dim;
	while (x == y)
		y = rand() % dim;
	if (x > y) swap(x, y);
	bool* vis = new bool[dim + 1];
	for (int i = 0; i <= dim; i++)
		vis[i] = false;
	for (int i = x; i <= y; i++)
		vis[int(Pa.gene[0][i])] = true;
	for(int i = 0; i < dim; i++)
		Ca.gene[0][i] = Pa.gene[0][i];
	int ia = x;
	for (int i = 0; i < dim; i++)
	{
		Cb.gene[0][i] = Pb.gene[0][i];
		if (vis[int(Cb.gene[0][i])])
			swap(Cb.gene[0][i], Ca.gene[0][ia++]);
	}
	for (int i = 0; i < Ca.length; i++)
		if (Ca.gene[0][i] != Pa.gene[0][i])
			return;
	//Pa.Print_Solution(31); Pb.Print_Solution(31);

}

Individual Population::Gaussian_Mu(Individual P)
{
	Individual res = P; double len = dim, mum = 5;
	for (int i = 0; i < dim; i++)
	{
		if (double(rand()) / RAND_MAX < 1.0 / len)
		{
			double u = double(rand()) / RAND_MAX;
			if (u <= 0.5)
			{
				res.gene[0][i] *= pow(2 * u, 1 / (1 + mum));
			}
			else
			{
				res.gene[0][i] = (res.gene[0][i] - 1) * pow(2 - 2 * u, 1 / (1 + mum)) + 1;
			}
		}
	}
	return res;
}

Individual Population::Swap_Mu(Individual P)
{
	Individual res = P;
	int a = rand() % dim, b = rand() % dim;
	swap(res.gene[0][a], res.gene[0][b]);
	return res;
}

void Population::getOffspring(Problem** problem_set)
{
	int index = size, cur_size = size * 2;
	while (index < cur_size)
	{
		int Pa = rand() % size, Pb = rand() % size;
		while (Pa == Pb)
		{
			Pb = rand() % size;
		}

		Individual Ca(dim, num_problems), Cb(dim, num_problems);
		int ProA = popul[Pa].skill_factor, ProB = popul[Pb].skill_factor;
		if (popul[Pa].skill_factor == popul[Pb].skill_factor || double(rand()) / RAND_MAX < cr_rate)
		{
			//SBX(popul[Pa], popul[Pb], Ca, Cb);
			//PMX(popul[Pa], popul[Pb], Ca, Cb);
			//OX(popul[Pa], popul[Pb], Ca, Cb);
			OBX(popul[Pa], popul[Pb], Ca, Cb);
			//SEC(popul[Pa], popul[Pb], Ca, Cb);

			if (double(rand()) / RAND_MAX < 0.5)
			{
				Ca.factorial_cost[ProA] = problem_set[ProA]->solve(Ca.gene);
			}
			else
				Ca.factorial_cost[ProB] = problem_set[ProB]->solve(Ca.gene);

			if (double(rand()) / RAND_MAX < 0.5)
				Cb.factorial_cost[ProA] = problem_set[ProA]->solve(Cb.gene);
			else
				Cb.factorial_cost[ProB] = problem_set[ProB]->solve(Cb.gene);
		}
		else
		{
			//Ca = Gaussian_Mu(popul[Pa]);
			Ca = Swap_Mu(popul[Pa]);
			Ca.factorial_cost[ProA] = problem_set[ProA]->solve(Ca.gene);

			//Cb = Gaussian_Mu(popul[Pb]);
			Cb = Swap_Mu(popul[Pb]);
			Cb.factorial_cost[ProB] = problem_set[ProB]->solve(Cb.gene);
		}

		popul[index++] = Ca;  popul[index++] = Cb;

	}
	Update_Factor(size * 2);
}

void Population::Select()
{
	//std::sort(popul + size, popul + int(size * (1 + off_scale)));
	//int begin = 0;
	//if (popul[size].scalar_fitness < popul[0].scalar_fitness)
	//	begin = 1;
	//for (int i = begin; i < size; i++)
	//	std::swap(popul[i], popul[i + size]);
	std::sort(popul, popul + (int(size * 2)));
	int index = num_problems, s = size - index;
	while (index < size)
	{
		int x = rand() % s + index, y = rand() % s + index;
		if (popul[x].scalar_fitness > popul[y].scalar_fitness)
			swap(popul[index++], popul[x]);
		else
			swap(popul[index++], popul[y]);
	}
	Update_Factor(size);
}

Individual Population::Get_Best(int prob)
{
	for (int i = 0; i < size; i++)
	{
		if (factorial_rank[i][prob] == 1)
			return popul[i];
	}
}

Individual Population::operator[](int i)
{
	return popul[i];
}

Population::~Population()
{
	delete[] popul;
	for (int i = 0; i < size; i++)
		delete[] factorial_rank[i];
	delete[] factorial_rank;
}
