#pragma once
#include "Problem.h"
class CVRP :
	public Problem
{
public:
	Matrix map;
	int track;
	double *demand, capacity;
	CVRP();
	void Init(int, Matrix, double*, int, double);
	double solve(Matrix);

	~CVRP();
};

