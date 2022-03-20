#include "Sphere.h"



void Sphere::Init(int _dim, Matrix _opt, double _center, double _range)
{
	dim = _dim; opt = _opt; center = _center; range = _range;
}

double Sphere::solve(Matrix var)
{
	var = var.cut(1, dim);
	var = (var - 0.5) * range + center;
	var = var - opt;
	Matrix vari = var.trans();
	return (var * vari)[0][0];
}


Sphere::~Sphere()
{
}
