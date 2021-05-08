#include "Rastrigin.h"
#include "Matrix.h"
#include <cmath>
#include <algorithm>



void Rastrigin::Init(Matrix _M, int _dim, Matrix _opt, double _center, double _range)
{
	M = _M; dim = _dim; opt = _opt; center = _center; range = _range;
}

double Rastrigin::solve(Matrix var)
{
	var = var.cut(1, dim);
	var = (var - 0.5) * range + center;
	var = (var - opt).trans();
	var = (M * var).trans();
	double res = 10.0 * dim;
	for (int i = 0; i < dim; i++)
		res += var[0][i] * var[0][i] - 10 * cos(2 * acos(-1) * var[0][i]);
	return res;
}


Rastrigin::~Rastrigin()
{

}
