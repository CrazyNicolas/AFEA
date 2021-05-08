#pragma once
#include "Individual.h"
#include "Matrix.h"
#include "Problem.h"

class Population
{
public:
	Individual popul[300];
	int factorial_rank[300][20];
	double off_scale, cr_rate;
	int size, num_problems;

	Population(int, int, int, double, double);
	void Init();
	void Init_Evaluate(Problem**, int);
	void Update_Factor(int);
	void SBX(Individual, Individual, Individual&, Individual&);
	Individual Gaussian_Mu(Individual);
	void getOffspring(Problem**);
	void Select();
	Individual Get_Best(int);
	Individual operator[](int);
	~Population();
};

