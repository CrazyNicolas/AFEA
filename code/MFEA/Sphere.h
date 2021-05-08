#pragma once
#include "Matrix.h"
#include "Problem.h"
class Sphere: public Problem
{
private:
	Matrix opt;
	int dim;
	double center, range;
public:
	void Init(int, Matrix, double, double);
	virtual double solve(Matrix);
	~Sphere();
};

