#include <iostream>
#include<stdio.h>
#include <map>
#include <cstring>
#include <algorithm>
#include <cmath>
using namespace std;

double** Read_TSP(char* path, int& n);

double** Read_TSP(int CITES, char* path);

double** Read_CVRP(char* path, int& n, int &track, double &capacity);
