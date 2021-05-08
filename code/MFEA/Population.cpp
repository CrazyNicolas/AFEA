#include "Population.h"
#include <map>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <time.h>
using namespace std;


Population::Population(int dim, int _size, int _num_problems, double off, double cr)
{
	srand((unsigned)time(0));
	size = _size; num_problems = _num_problems;
	off_scale = off; cr_rate = cr;
	/*factorial_rank = new int*[size * 2];
	for (int i = 0; i < size * 2; i++)
		factorial_rank[i] = new int[num_problems];

	popul = new Individual[size * 2];*/
	for (int i = 0; i < size * 2; i++)
	{
		Individual tmp(dim, num_problems);
		popul[i] = tmp;
		popul[i].Init();
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
	//srand((unsigned)time(0));
	double mu = 2; int len = Pa.length;
	for (int i = 0; i < len; i++)
	{	
		double u = double(rand()) / RAND_MAX; 
		double Beta = u <= 0.5 ? pow(2 * u, (1 / (mu + 1))) : pow(1 / (2 - 2 * u), (1 / (mu + 1)));
		Ca.gene[0][i] = min(max(0.5 * (Pa.gene[0][i] + Pb.gene[0][i]) + 0.5 * Beta * (Pb.gene[0][i] - Pa.gene[0][i]), 0.0), 1.0);
		Cb.gene[0][i] = min(max(0.5 * (Pb.gene[0][i] + Pa.gene[0][i]) + 0.5 * Beta * (Pa.gene[0][i] - Pb.gene[0][i]), 0.0), 1.0);
		//*Ca.gene[0][i] = 0.5 * (*Pa.gene[0][i] + *Pb.gene[0][i]) - 0.5 * Beta * (*Pb.gene[0][i] - *Pa.gene[0][i]);
		//*Cb.gene[0][i] = 0.5 * (*Pa.gene[0][i] + *Pb.gene[0][i]) - 0.5 * Beta * (*Pb.gene[0][i] - *Pa.gene[0][i]);
	}
}

Individual Population::Gaussian_Mu(Individual P)
{
	//srand((unsigned)time(0));
	Individual res = P; double len = P.length, mum = 5;
	for (int i = 0; i < len; i++)
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

void Population::getOffspring(Problem** problem_set)
{
	//srand((unsigned)time(0));
	int index = size;
	while (index < size * (1 + off_scale))
	{
		int Pa = double(rand()) / RAND_MAX * size, Pb = double(rand()) / RAND_MAX * size;
		Pa = std::min(Pa, size - 1);
		int len = popul[Pa].length;
		while (Pb == Pa || Pb == size)
		{
			Pb = double(rand()) / RAND_MAX * size;
		}
		Individual Ca(len, num_problems), Cb(len, num_problems);
		int ProA = popul[Pa].skill_factor, ProB = popul[Pb].skill_factor;
		if (popul[Pa].skill_factor == popul[Pb].skill_factor || double(rand()) / RAND_MAX < cr_rate)
		{
			SBX(popul[Pa], popul[Pb], Ca, Cb);

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
			Ca = Gaussian_Mu(popul[Pa]);
			Ca.factorial_cost[ProA] = problem_set[ProA]->solve(Ca.gene);

			Cb = Gaussian_Mu(popul[Pb]);
			Cb.factorial_cost[ProB] = problem_set[ProB]->solve(Cb.gene);
		}
		popul[index++] = Ca;  popul[index++] = Cb;
	}
	Update_Factor(size * (1 + off_scale));
}

void Population::Select()
{
	//std::sort(popul + size, popul + int(size * (1 + off_scale)));
	//int begin = 0;
	//if (popul[size].scalar_fitness < popul[0].scalar_fitness)
	//	begin = 1;
	//for (int i = begin; i < size; i++)
	//	std::swap(popul[i], popul[i + size]);
	std::sort(popul, popul + (int(size * (1 + off_scale))));
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
	/*delete[] popul;
	for (int i = 0; i < size; i++)
		delete[] factorial_rank[i];
	delete[] factorial_rank;*/
}
