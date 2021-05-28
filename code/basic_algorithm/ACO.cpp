#include <iostream>
#include<ctime>
#include<string.h>
#include<cstdlib>
using namespace std;
#include<algorithm>
#include <cmath>
#define CITES 52
#define NUM_OF_ANTS CITES
#define Q 100
#define RHO 0.6
#define ALPHA 1
#define BETA 5
#define TAU 1e-6 
#define EPOCH 500
double eps = 7e-6;
double** Read_TSP(char* path)
{
	int a;
	freopen(path, "r", stdin);
	double** res = new double* [CITES];
	double* x = new double[CITES];
	double* y = new double[CITES];
	for (int i = 0; i < CITES; i++)
	{
		res[i] = new double[CITES];
		std::cin >> a;	//Some benchmark data have additional index values before coordinates.
		std::cin >> x[i] >> y[i];
	}
	fclose(stdin);
	for (int i = 0; i < CITES; i++)
		for (int j = 0; j < CITES; j++)
			res[i][j] = res[j][i] = sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));
	delete[] x, y;
	return res;
}

int main()
{
	srand(time(NULL));
    int epoch = EPOCH;
    //Distance matrix read from TSP data
	double** dis = Read_TSP((char*)"berlin52.tsp");
	//test dis
	/*for (int i = 0; i < CITES; i++) {
		for (int j = 0; j < CITES; j++) {
			cout<<dis[i][j]<<" ";
		}
		cout << endl;
	}*/

	// initialize phermenon between 2 cites
	double** Pheromone = new double* [CITES];
	for (int i = 0; i < CITES; i++) {
		Pheromone[i] = new double[CITES];
		for (int j = 0; j < CITES; j++) {
			Pheromone[i][j] = TAU;
		}
	}
    while (epoch--) {
		//START-TIME
		clock_t start = clock();

		// initialize start point of every ant
		// initialize tour set of every ant
		// initialize visited table for ever ant
		int** Tour = new int* [NUM_OF_ANTS];
		int** Visited = new int* [NUM_OF_ANTS];
		for (int i = 0; i < NUM_OF_ANTS; i++) {
			Tour[i] = new int[CITES];
			Visited[i] = new int[CITES];
			for (int j = 0; j < CITES; j++) {
				if (j == 0) Tour[i][j] = rand() % CITES;
				else Tour[i][j] = -1;
				Visited[i][j] = 0;
			}
			Visited[i][Tour[i][0]] = 1;
		}
		
		//test init
		/*for (int i = 0; i < NUM_OF_ANTS; i++) {
			for (int j = 0; j < CITES; j++) {
				cout<<visited[i][j]<<" ";
			}
			cout << endl;
		}*/
		// in every iteration, all ants construct total tour step by step
		for (int index = 1; index < CITES - 1; index++){
			//inner loop, each ant take a move 
			for (int ant = 0; ant < NUM_OF_ANTS; ant++) {
				double rv = (double)rand() / RAND_MAX;
				double probability_sum = 0.0;
				double Sum = 0.0;
				for (int city = 0; city < CITES; city++) {
					if (Visited[ant][city] == 0) {
						
						Sum += pow(Pheromone[Tour[ant][index - 1]][city], ALPHA) / pow(dis[Tour[ant][index - 1]][city], BETA);
					}
				}
				for (int city = 0; city < CITES; city++){
					if (Visited[ant][city] == 0){
						double probability_this_city = pow(Pheromone[Tour[ant][index - 1]][city], ALPHA) / pow(dis[Tour[ant][index - 1]][city], BETA) / Sum;
						if ((rv >= probability_sum - eps) && (rv <= probability_sum + probability_this_city + eps)){
							//this city has been choosen
							Tour[ant][index] = city;
							Visited[ant][city] = 1;
							//printf("index=%d city=%d\n", index, city);
							break;
						}
						else{
							probability_sum += probability_this_city;
						}
					}
				}

			}
		}
		//find last rest one city for each ant
		for (int ant = 0; ant < NUM_OF_ANTS; ant++) {
			for (int city = 0; city < CITES; city++) {
				if (Visited[ant][city] == 0) {
					//cout << city << endl;
					Tour[ant][CITES - 1] = city;
				}
			}
		}

		//calculate Tour distance for every ant
		double L_k[NUM_OF_ANTS];
		for(int ant = 0; ant < NUM_OF_ANTS; ant++) {
			L_k[ant] = 0;
			for (int i = 0; i < CITES; i++) {
				if (i == CITES - 1) {
					L_k[ant] += dis[Tour[ant][i]][Tour[ant][0]];
				}
				else {
					int a = Tour[ant][i];
					int b = Tour[ant][i + 1];
					L_k[ant] += dis[a][b];
				}
			}
		}
		//sort L_k to get best result in each epoch
		double best_result;
		sort(L_k, L_k + NUM_OF_ANTS);
		best_result = L_k[0];


		//test calculating correctly?
		/*for (int i = 0; i < NUM_OF_ANTS; i++) {
			cout << L_k[i] << " ";
		}*/

		//update Phermenon
		for (int i = 0; i < CITES; i++) {
			for (int j = 0; j < CITES; j++) {
				Pheromone[i][j] = (1 - RHO) * Pheromone[i][j];
			}
		}
		for (int i = 0; i < CITES; i++) {
			for (int j = 0; j < CITES; j++) {
				for (int ant = 0; ant < NUM_OF_ANTS; ant++) {
					bool has_ij = false;
					for (int k = 0; k < CITES; k++) {
						if (Tour[ant][k] == i && Tour[ant][k + 1] == j) {
							has_ij = true;
							break;
						}
					}
					if (has_ij) {
						Pheromone[i][j] += Q / L_k[ant];
					}
				}
			}
		}

		//test Tour is correctly constructed?
		/*for (int i = 0; i < NUM_OF_ANTS; i++) {
			for (int j = 0; j < CITES; j++) {
				cout << Tour[i][j] << " ";
			}
			cout << endl;
		}*/

		for (int i = 0; i < NUM_OF_ANTS; i++) {
			delete[] Tour[i];
			delete[] Visited[i];
		}
		delete[] Tour;
		delete[] Visited;
		
		//END-TIME
		clock_t end = clock();
		cout << "epoch " << EPOCH - epoch << ": Time used: " << (double)(end - start) / CLOCKS_PER_SEC << "  best result: " << best_result << endl;

    }
	for (int i = 0; i < CITES; i++) {
		delete[] Pheromone[i];
	}
	delete[] Pheromone;



    return 0;
}


