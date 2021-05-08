#pragma once
#include "Matrix.h"
#include "Problem.h"

class Ackley: public Problem
{
private:
	Matrix M;
	Matrix opt;
	int dim;
	double range, center;
public:
	Ackley();
	void Init(Matrix, int, Matrix, double, double);
	virtual double solve(Matrix);
	~Ackley();
};

