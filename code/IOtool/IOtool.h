#pragma once
#include <iostream>
#include <map>
#include <cstring>
#include <algorithm>
#include <cmath>
using namespace std;
#pragma warning(disable : 4996)

double** Read_TSP(char* path, int& n);

double** Read_TSP(int CITES, char* path);

double** Read_CVRP(char* path, int& n, int &track, double &capacity);