#include "Ackley.h"
#include "Matrix.h"
#include <cmath>
#include <algorithm>


Ackley::Ackley() {}

void Ackley::Init(Matrix _M, int _dim, Matrix _opt, double _center, double _range)
{
	M = _M; dim = _dim; opt = _opt; center = _center; range = _range;
}

double Ackley::solve(Matrix var)
{
	var = var.cut(1, dim);
	var = var * (range * 2) + center - range;
	var = (var - opt);
	var = var.trans();
	var = (M * var);
	var = var.trans();
	double sum1 = 0, sum2 = 0;
	for (int i = 0; i < dim; i++)
	{
		sum1 += var[0][i] * var[0][i];
		sum2 += cos(2 * acos(-1) * var[0][i]);
	}
	double avg1 = sum1 / dim, avg2 = sum2 / dim;
	double res = -20 * exp(-0.2 * sqrt(avg1)) - exp(avg2) + 20 + exp(1);
	return res;
}


Ackley::~Ackley()
{
}
