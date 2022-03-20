#pragma once
#include "Problem.h"
#include "Matrix.h"
class TSP :
	public Problem
{
public:
	//int dim;
	Matrix map;
	TSP();
	void Init(int, Matrix);
	double solve(Matrix);
	~TSP();
};

