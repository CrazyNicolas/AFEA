// Plain Differential Evolutionary Algorithm implementation
#include<stdio.h>
#include<stdlib.h>
#include<cstdlib>
#include<ctime>
#include<iostream>
#include<algorithm>
#include <string.h>
//#include "../IOtool/IOtool.h"
using namespace std;

//scaling factor between 0-2
#define F 0.8
//crossover rate
#define CR 0.6
//group size
#define GROUP_SIZE 20
//number of cites
#define CITES 52
//iteration
#define EPOCH 500

//define a struct for sorting with key
typedef struct{
	double value;
	int key;
}SortObj;
bool compare(SortObj a, SortObj b){
	return a.value < b.value;
}

double** Read_TSP(char* path)
{
	int a;
	freopen(path, "r", stdin);
	double **res = new double*[CITES];
	double* x = new double[CITES];
	double* y = new double[CITES];
	for (int i = 0; i < CITES; i++)
	{
		res[i] = new double[CITES];
		cin >> a;	//Some benchmark data have additional index values before coordinates.
		cin >> x[i] >> y[i];
	}
	fclose(stdin);
	for (int i = 0; i < CITES; i++)
		for (int j = 0; j < CITES; j++)
			res[i][j] = res[j][i] = sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));
	
	delete[] x, y;
	return res;
}


// find nearest neighbor
int find_nearest(double **dis, int * visited, int i){
	int nearest = -1;
	double min_dis = 999999;
	for(int j = 0; j < CITES; j++){
		if(dis[i][j] < min_dis && visited[j] == 0){
			nearest = j;
			min_dis = dis[i][j];
		}
	}
	visited[nearest] = 1;
	return nearest;
}
// init group with greedy strategy
int** init_group(double **dis){
	//allocating 2D array for group
	int** group = new int*[GROUP_SIZE];
	for(int i = 0; i < GROUP_SIZE; i++){
		group[i] = new int[CITES];
		//a visited table is needed
		int visited[CITES];
		for(int k = 0; k < CITES; k++) visited[k] = 0;
		//randomly pick a start
		int start = rand()%CITES;
		group[i][0] = start;
		visited[start] = 1;
		for(int j = 1; j < CITES; j++){
			int nearest = find_nearest(dis,visited,start);
			start = nearest;
			group[i][j] = nearest; 
		}
	}
	return group;
}

// init group with randome strategy, rv_threshold controls greedy
int ** init_group(double **dis, double rv_threshold){
	//allocating 2D array for group
	int** group = new int*[GROUP_SIZE];
	for(int i = 0; i < GROUP_SIZE; i++){
		group[i] = new int[CITES];
		//a visited table is needed
		int visited[CITES];
		for(int k = 0; k < CITES; k++) visited[k] = 0;
		//randomly pick a start
		int start = rand()%CITES;
		group[i][0] = start;
		visited[start] = 1;
		for(int j = 1; j < CITES; j++){
			double rv = (double)rand()/RAND_MAX;
			if(rv < rv_threshold){
				int nearest = find_nearest(dis,visited,start);
				start = nearest;
				group[i][j] = nearest; 
			}else{
				int next = (start+1+rand()%CITES)%CITES;
				while(visited[next] == 1){
					next = (start+1+rand()%CITES)%CITES;
				}
				group[i][j] = next;
				visited[next] = 1;
				start = next;
			}
		}
	}
	return group;
}

//get fitness
double getfitness(int *indiv, double **dis){
	double total_distance = 0;
	for(int i = 0; i < CITES; i++){
		if(i < CITES-1) total_distance += dis[indiv[i]][indiv[i+1]];
		else total_distance += dis[indiv[i]][indiv[0]];
	}

	return 1.0/total_distance;
}

//mutation
int* mutation(int **group, int index){
	int r1 = rand()%GROUP_SIZE;
	while(r1 == index){
		r1 = rand()%GROUP_SIZE;
	}
	int r2 = rand()%GROUP_SIZE;
	while(r2 == index || r2 == r1){
		r2 = rand()%GROUP_SIZE;
	}
	int r3 = rand()%GROUP_SIZE;
	while(r3 == index || r3 == r2 || r3 == r1){
		r3 = rand()%GROUP_SIZE;
	}
	SortObj *sobjs = new SortObj[CITES];
	for(int i = 0; i < CITES; i++){
		sobjs[i].value = group[r1][i] + F * (group[r2][i] - group[r3][i]);
		sobjs[i].key = i;
	}
	sort(sobjs, sobjs + CITES, compare);
	int *result = new int[CITES];
	for(int i = 0; i < CITES; i++){
		result[sobjs[i].key] = i;
	}
	delete[] sobjs;
	return result;

}

int* crossover(int* x, int* v, double** dis){
	double rv = (double)rand()/RAND_MAX;
	if(rv > CR) return x;
	int current_x = 0;
	int current_v = -1; //default
	//find where v[i] = x[current_x]
	for(int k = 0; k < CITES; k++){
		if(v[k] == x[current_x]){
			current_v = k;
			break;
		}
	}
	//to judge if a city has been choosen
	int* visited = new int[CITES];
	for(int i = 0; i < CITES; i++){
		visited[i] = 0;
	}
	//offspring
	int* offspring = new int[CITES];
	//right last index of offspring
	int index = 0;
	visited[x[current_x]] = 1;
	offspring[index++] = x[current_x];
	while(index < CITES){//end condition

		//find next unvisited point in x
		int idx_x = (current_x + 1) % CITES;
		while(visited[x[idx_x]] == 1){
			idx_x = (idx_x + 1) % CITES;

		}

		//find next unvisited point in v
		int idx_v = (current_v + 1) % CITES;
		while(visited[v[idx_v]] == 1){
			idx_v = (idx_v + 1) % CITES;
		}

		if(dis[x[current_x]][x[idx_x]] < dis[v[current_v]][v[idx_v]]){
			offspring[index++] = x[idx_x];
			visited[x[idx_x]] = 1;
			current_x = idx_x;
			for(int k = 0; k < CITES; k++){
				if(v[k] == x[current_x]){
					current_v = k;
				}
			}
		}else{
			offspring[index++] = v[idx_v];
			visited[v[idx_v]] = 1;
			current_v = idx_v;
			for(int c = 0; c < CITES; c++){
				if(x[c] == v[current_v]){
					current_x = c;
				}
			}
		}
	}

	return offspring;
	delete[] visited;

}

int main(int argc, char **argv){
	//random seed
	srand(time(0));
	//load the dis matrix
	double **dis = Read_TSP((char*)"berlin52.tsp");
	//test dis mat read correctly
	// for(int i = 0; i < CITES; i++){
	// 	for(int j = 0; j < CITES; j++){
	// 		printf("%.2f\t",dis[i][j]);
	// 	}
	// 	cout<<endl;
	// }
	// cout<<endl;
	// init it
	int **group = init_group(dis,0.5);
	//TEST if __init__ is successful
	// for(int i = 0 ; i < GROUP_SIZE; i++){
	// 	for(int j = 0; j < CITES; j++){
	// 		cout<<group[i][j]<<" ";
	// 	}
	// 	cout<<endl;
	// }

	//every generation, offspings needs same space as group
	int **offspings = new int*[GROUP_SIZE];
	//begin the iteration
	int epoch = EPOCH;
	
	while(epoch--){
		//best fitness
		double best_fitness = 0;
		clock_t start = clock();
		int* variant;
		for(int i = 0; i < GROUP_SIZE; i++){
			variant = mutation(group,i);
			offspings[i] = crossover(group[i],variant,dis);
		}
		for(int i = 0; i < GROUP_SIZE; i++){
			double f_parent = getfitness(group[i],dis);
			double f_offspring = getfitness(offspings[i],dis);
			if(f_offspring > f_parent){
				group[i] = offspings[i];
			}
			if(f_offspring > best_fitness){
				best_fitness = f_offspring;
			}
			if(f_parent > best_fitness){
				best_fitness = f_parent;
			}
		}
		clock_t end = clock();
		printf("epoch %d, time used: %.3f s, best_fitness: %lf\n",EPOCH - epoch,(double)(end - start)/CLOCKS_PER_SEC, best_fitness);

	}

	//DEBUG-START
	// for(int i = 0 ; i < GROUP_SIZE; i ++){
	// 	int *v = mutation(group,i);
	// 	for(int j =0 ; j < CITES; j++){
	// 		cout<<group[i][j]<<" ";
	// 	}
	// 	cout<<endl;
	// 	for(int j =0 ; j< CITES; j++){
	// 		cout<<v[j]<<" ";
	// 	}
	// 	cout<<endl;

	// 	int *res = crossover(group[i],v,dis);
	
	// 	for(int j =0 ; j< CITES; j++){
	// 		cout<<res[j]<<" ";
	// 	}
	// 	cout<<endl;
	// 	cout<<endl;
	// }
	//DEBUG-END

	// free spaces dynamically allocated before
	for(int i = 0; i < GROUP_SIZE; i++){
		delete[] group[i];
		delete[] offspings[i];
	}
	delete[] group;
	delete[] offspings;
	return 0;
}