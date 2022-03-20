#pragma once
#include "Matrix.h"
#include "Problem.h"

class Rastrigin: public Problem
{
private:
	Matrix M;
	Matrix opt;
	int dim;
	double center, range;
public:
	void Init(Matrix, int, Matrix, double, double);
	virtual double solve(Matrix);
	~Rastrigin();
};

