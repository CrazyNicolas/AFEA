#pragma once
#include "Individual.h"
#include "Matrix.h"
#include "Problem.h"

class Population
{
public:
	Individual popul[300];
	int factorial_rank[300][20];
	double cr_rate;
	int size, num_problems, dim;

	Population(int, int, int, double);
	void Init();
	void Init_Evaluate(Problem**, int);
	void Update_Factor(int);
	void SBX(Individual, Individual, Individual&, Individual&);
	void PMX(Individual, Individual, Individual&, Individual&);
	void OX(Individual, Individual, Individual&, Individual&);
	void OBX(Individual, Individual, Individual&, Individual&);
	Individual Gaussian_Mu(Individual);
	Individual Swap_Mu(Individual);
	void getOffspring(Problem**);
	void Select();
	Individual Get_Best(int);
	Individual operator[](int);
	~Population();
};

