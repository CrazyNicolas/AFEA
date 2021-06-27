#pragma once
#include "Matrix.h"
#include "Problem.h"
class Individual
{
public:
	Matrix gene;
	double factorial_cost[20];
	double scalar_fitness;
	int skill_factor, length;

	Individual() {};
	Individual(int, int);
	Individual(const Individual& I);
	Individual& operator=(Individual);
	void Update_Cost(Problem**, int);
	void Init();
	void Int_Init();
	bool operator < (Individual&) const;
	void Print_Solution(int);
	~Individual();
};

