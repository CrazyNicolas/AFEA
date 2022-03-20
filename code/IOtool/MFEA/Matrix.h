#pragma once
class Matrix
{
private:
	
public:
	int a, b;
	double** mat;
	//double mat[150][150];
	Matrix();
	Matrix(int, int);
	Matrix(double[], int);
	Matrix(double**, int, int);
	Matrix(const Matrix&);
	double* operator[](int);
	Matrix operator+ (Matrix&) const;
	Matrix operator+ (double) const;
	Matrix operator- (Matrix&) const;
	Matrix operator- (double) const;
	Matrix operator* (Matrix&) const;
	Matrix operator* (double) const;
	Matrix& operator=(Matrix);
	Matrix trans();
	Matrix cut(int, int) ;
	static Matrix Rand(int, int);
	void print();
	~Matrix();
};

