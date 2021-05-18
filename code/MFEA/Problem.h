#pragma once
#include "Matrix.h"
class Problem
{
protected:
	
public:
	int dim;
	virtual double solve(Matrix) = 0;
	//virtual ~Problem();
};

