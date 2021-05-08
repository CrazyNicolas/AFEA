#include "Matrix.h"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <time.h>
using namespace std;

Matrix::Matrix() {}

Matrix::Matrix(int _a, int _b)
{
	a = _a; b = _b;
	mat = new double*[a];
	for (int i = 0; i < a; i++)
		mat[i] = new double[b];
	for (int i = 0; i < a; i++)
		//memset(mat[i], 0, sizeof(mat[i]));
		for (int j = 0; j < b; j++)
			mat[i][j] = 0;
}

Matrix::Matrix(double one_dim[], int length)
{
	a = 1; b = length;
	mat = new double* [a];
	mat[0] = new double[b];
	for (int i = 0; i < b; i++)
		mat[0][i] = one_dim[i];
}

Matrix::Matrix(double** two_dim, int _a, int _b)
{
	a = _a; b = _b;
	mat = new double* [a];
	for (int i = 0; i < a; i++)
		mat[i] = new double[b];
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
			mat[i][j] = two_dim[i][j];
}

Matrix::Matrix(const Matrix& M)
{
	a = M.a;  b = M.b;
	mat = new double* [a];
	for (int i = 0; i < a; i++)
		mat[i] = new double[b];
	for (int i = 0; i < a; i++)
		//memcpy(mat[i], M.mat[i], b * sizeof(int));
		for (int j = 0; j < b; j++)
			mat[i][j] = M.mat[i][j];
}

double* Matrix::operator[](int i)
{
	return mat[i];
}

Matrix Matrix::operator+(Matrix& m) const
{
	Matrix res(a, b);
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
			res[i][j] = mat[i][j] + m[i][j];
	return res;
}

Matrix Matrix::operator+(double x) const
{
	Matrix res(a, b);
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
			res[i][j] = mat[i][j] + x;
	return res;
}

Matrix Matrix::operator-(Matrix& m) const
{
	Matrix res(a, b);
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
			res[i][j] = mat[i][j] - m[i][j];
	return res;
}

Matrix Matrix::operator-(double x) const
{
	Matrix res(a, b);
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
			res[i][j] = mat[i][j] - x;
	return res;
}

Matrix Matrix::operator*(Matrix& m) const
{
	Matrix res(a, m.b);
	for (int i = 0; i < a; i++)
		for (int j = 0; j < m.b; j++)
			for (int k = 0; k < b; k++)
				res[i][j] += mat[i][k] * m[k][j];
	return res;
}

Matrix Matrix::operator*(double x) const
{
	Matrix res(a, b);
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
				res[i][j] = mat[i][j] * x;
	return res;
}

Matrix Matrix::trans()
{
	Matrix res(b, a);
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
			res[j][i] = mat[i][j];
	return res;
}

Matrix Matrix::cut(int c, int d)
{
	Matrix res(c, d);
	for (int i = 0; i < min(c, a); i++)
		for (int j = 0; j < min(d, b); j++)
			res[i][j] = mat[i][j];
	return res;
}

Matrix Matrix::Rand(int _a, int _b)
{
	Matrix res(_a, _b);
	srand((unsigned)time(0));
	for (int i = 0; i < _a; i++)
		for (int j = 0; j < _b; j++)
			res[i][j] = (double(rand()) / RAND_MAX - 0.5) * 0.4;
	return res;
}

void Matrix::print()
{
	for (int i = 0; i < a; i++)
	{
		for (int j = 0; j < b; j++)
			printf("%lf ", mat[i][j]);
		printf("\n");
	}

}

Matrix::~Matrix()
{
	/*for (int i = 0; i < a; i++)
		delete[] mat[i];
	delete[] mat;*/
}
