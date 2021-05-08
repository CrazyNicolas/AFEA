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
	//Individual(const Individual&);
	Individual(int, int);
	void Update_Cost(Problem**, int);
	void Init();
	bool operator < (Individual&) const;
	~Individual();
};

