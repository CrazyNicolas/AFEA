#include "Matrix.h"
#include <cstring>
#include <algorithm>
#include <Windows.h>
#include <cmath>
#include <time.h>
using namespace std;

Matrix::Matrix() 
{
	mat = NULL;
}

Matrix::Matrix(int _a, int _b)		//Create a matrix with size a * b and fitted with 0s
{
	a = _a; b = _b;
	mat = new double*[a];
	for (int i = 0; i < a; i++)
		mat[i] = new double[b];
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
			mat[i][j] = 0;
}

Matrix::Matrix(double one_dim[], int length)		//Create an one dimension matrix by copying an array
{
	a = 1; b = length;
	mat = new double* [a];
	mat[0] = new double[b];
	for (int i = 0; i < b; i++)
		mat[0][i] = one_dim[i];
}

Matrix::Matrix(double** two_dim, int _a, int _b)		//Create a matrix by copying a two dimension array
{
	a = _a; b = _b;
	mat = new double* [a];
	for (int i = 0; i < a; i++)
		mat[i] = new double[b];
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
			mat[i][j] = two_dim[i][j];
}

Matrix::Matrix(const Matrix& M)		//Override copy function
{
	a = M.a;  b = M.b;
	mat = new double* [a];
	for (int i = 0; i < a; i++)
		mat[i] = new double[b];
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
			mat[i][j] = M.mat[i][j];
}

/* Override [] + - * operator */

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

Matrix& Matrix::operator=(Matrix m)
{
	if (mat != NULL)
	{
		for (int i = 0; i < a; i++)
		{
			delete[] mat[i];
			mat[i] = NULL;
		}
		delete[] mat;
		mat = NULL;
	}
	a = m.a; b = m.b;
	mat = new double*[a];
	for (int i = 0; i < a; i++)
	{
		mat[i] = new double[b];
		for (int j = 0; j < b; j++)
			mat[i][j] = m[i][j];
	}
	return *this;
}

Matrix Matrix::trans()		//Transpose
{
	Matrix res(b, a);
	for (int i = 0; i < a; i++)
		for (int j = 0; j < b; j++)
			res[j][i] = mat[i][j];
	return res;
}

Matrix Matrix::cut(int c, int d)		//Intercept a part of matrix
{
	Matrix res(c, d);
	for (int i = 0; i < min(c, a); i++)
		for (int j = 0; j < min(d, b); j++)
			res[i][j] = mat[i][j];
	return res;
}

Matrix Matrix::Rand(int _a, int _b)		//Create a random matrix
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
			printf("%.0lf ", mat[i][j]);
			//printf("%d ", int(mat[i][j]));
		printf("\n");
	}

}

Matrix::~Matrix()
{
	if (mat != NULL)
	{
		for (int i = 0; i < a; i++)
		{
			delete[] mat[i];
			mat[i] = NULL;
		}
		delete[] mat;
		mat = NULL;
	}
	mat = NULL;
}
