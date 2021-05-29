#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<curand.h>
#include<iostream>
#include<curand_kernel.h>
using namespace std;

/*in this paper each thread are responsible for one chorosome*/
#define NUM_OF_POPULATION 20
#define CITES 14
#define Pc 0.9
#define Pm 0.3
#define Ps 0.8

#define EPOCH 500
//read TSP data from *.tsp file
double** Read_TSP(char* path)
{
	unsigned int a;
	freopen(path, "r", stdin);
	double** res = new double* [NUM_OF_POPULATION];
	double* x = new double[CITES];
	double* y = new double[CITES];
	for (unsigned int i = 0; i < CITES; i++)
	{
		res[i] = new double[CITES];
		std::cin >> a;	//Some benchmark data have additional index values before coordinates.
		std::cin >> x[i] >> y[i];
	}
	fclose(stdin);
	for (unsigned int i = 0; i < CITES; i++)
		for (unsigned int j = 0; j < CITES; j++)
			res[i][j] = res[j][i] = sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));
	delete[] x, y;
	return res;
}

unsigned int Rand(unsigned int X, unsigned int Y){
	return rand()%(Y-X+1) + X;
}
void init_group(unsigned int arr[][CITES]){
	unsigned int r,temp;
	for(unsigned int bid = 0; bid < NUM_OF_POPULATION; bid++){
		for(unsigned int i = 0; i < CITES; i++) arr[bid][i] = i;
		for(unsigned int i = 0; i < CITES; i++){
			r = Rand(0,CITES-1);
			temp = arr[bid][i];
			arr[bid][i] = arr[bid][r];
			arr[bid][r] = temp;
		}
		// for(unsigned int i = 1; i <=CITES; i++){
		// 	printf("%3d ",arr[bid][i-1]);
		// 	if(i % 10 == 0) printf("\n");
		// }
		//printf("\n");
	}
}


__global__ void get_fitness(unsigned int *Group, double *dis, double* fitness, unsigned int pitch, unsigned int pitch_dis){
	unsigned int tid = threadIdx.x;
	unsigned int *individual = (unsigned int*)((char*)Group + tid*pitch);
	double tour_dis = 0.0;
	for(unsigned int i = 0; i < CITES; i++){
		//printf("%d\n",individual[i]);
		double *dis_temp = (double*)((char*)dis + individual[i] * pitch_dis);
		if(i == CITES -1){
			//printf("%.2lf\n",dis_temp[0]);
			tour_dis += dis_temp[0];
		}else{
			//printf("%.2lf\n",dis_temp[i+1]);
			tour_dis += dis_temp[individual[i+1]];
		}
	}
	fitness[tid] = tour_dis;
}

__global__ void get_offsprings(unsigned int *parents, unsigned int *offsprings, double *dis,
	unsigned int *location_crossover, float *rv_crossover,
	unsigned int *location_mutation, float *rv_mutation, 
	double *fitness_offsprings, unsigned int pitch, unsigned int pitch_dis){
	/*each thread responsible for generating one offspring*/
	unsigned int tid = threadIdx.x;
	//crossover
	unsigned int *parent = (unsigned int*)((char*)parents + tid*pitch);
	unsigned int *offspring = (unsigned int*)((char*)offsprings + tid*pitch);
	unsigned int cl = location_crossover[tid]%CITES;
	if(rv_crossover[tid] < Pc){
		for(unsigned int i = 0; i < CITES; i++){
			offspring[i] = parent[(cl+i)%CITES];
		}
	}else{
		for(unsigned int i = 0; i < CITES; i++){
			offspring[i] = parent[i];
		}
	}

	// mutation
	if(rv_mutation[tid] < Pm){
		unsigned int a = location_mutation[tid * 2]%CITES;
		unsigned int b = location_mutation[tid * 2 + 1]%CITES;
		unsigned int temp = offspring[a];
		offspring[a] = offspring[b];
		offspring[b] = temp;
	}

	// get fitness
	double tour_dis = 0.0;
	for(unsigned int i = 0; i < CITES; i++){
		//printf("%d\n",individual[i]);
		double *dis_temp = (double*)((char*)dis + offspring[i] * pitch_dis);
		if(i == CITES -1){
			//printf("%.2lf\n",dis_temp[0]);
			tour_dis += dis_temp[0];
		}else{
			//printf("%.2lf\n",dis_temp[i+1]);
			tour_dis += dis_temp[offspring[i+1]];
		}
	}
	fitness_offsprings[tid] = tour_dis;

}

__global__ void select(unsigned int *parents, unsigned int *offsprings, 
	unsigned int *candidate_select, float *rv_select, 
	double *fitness_parents, double *fitness_offsprings,unsigned int pitch){
	int tid = threadIdx.x;
	unsigned int temp[CITES];
	double fitness_temp;
	unsigned int pidx = candidate_select[tid*2]%NUM_OF_POPULATION;
	unsigned int oidx = candidate_select[tid*2+1]%NUM_OF_POPULATION;
	unsigned int *parent = (unsigned int*)((char*)parents + pidx*pitch);
	unsigned int *offspring = (unsigned int*)((char*)offsprings + oidx*pitch);
	if(fitness_offsprings[oidx] > fitness_parents[pidx]){
		for(int i = 0; i < CITES; i++){
			temp[i] = parent[i];
			fitness_temp = fitness_parents[pidx];
		}
	}else{
		for(int i = 0; i < CITES; i++){
			temp[i] = offspring[i];
			fitness_temp = fitness_offsprings[oidx];
		}
	}
	__syncthreads();
	unsigned int *parent_tid = (unsigned int*)((char*)parents + tid*pitch);
	for(int i =0; i < CITES; i++){
		parent_tid[i] = temp[i];
	}
	fitness_parents[tid] = fitness_temp;
}

__global__ void print_dis(double* dis, unsigned int pitch_dis){
	for(unsigned int i = 0; i < CITES ; i++){
		double *dis_temp = (double*)((char*)dis + i*pitch_dis);
		for(unsigned int j = 0; j < CITES; j++){
			printf("%.2lf ",dis_temp[j]);
		}
		printf("\n");
	}
}

int main(unsigned int argc, char **argv){
	srand((unsigned)time(NULL));
	//init Distance Matrix
	double DIS[CITES][CITES];
	double **dis = Read_TSP((char*)"burma14.tsp");
	//test if read correctly
	for(unsigned int i = 0; i < CITES; i++){
		for(unsigned int j = 0; j < CITES; j++){
			DIS[i][j] = dis[i][j];
			cout<<DIS[i][j]<<" ";
		}
		cout<<endl;
	}

	//allocate DM to GPU
	size_t pitch_dis;
	double *DIS_gpu;
	cudaMallocPitch((void**)&DIS_gpu, &pitch_dis, 
		CITES * sizeof(double), CITES);
	cudaMemcpy2D(DIS_gpu,pitch_dis,DIS,CITES*sizeof(double), 
		CITES*sizeof(double), CITES, cudaMemcpyHostToDevice);

	//init a group on CPU
	unsigned int group[NUM_OF_POPULATION][CITES];
	init_group(group);
	//test init
	// for(unsigned int i = 0; i < NUM_OF_POPULATION; i++){
	// 	for(unsigned int j = 0; j < CITES; j++){
	// 		cout<<group[i][j]<<" ";
	// 	}
	// 	cout<<endl;
	// }
	unsigned int *group_gpu;
	size_t pitch;

	// allocate this group on GPU
	cudaMallocPitch((void**)&group_gpu,&pitch,CITES * sizeof(unsigned int),
		NUM_OF_POPULATION);
	cudaMemcpy2D(group_gpu, pitch, group, CITES * sizeof(unsigned int), 
		CITES*sizeof(unsigned int), NUM_OF_POPULATION, cudaMemcpyHostToDevice);
	printf("pitch = %d, dis_pitch = %d\n",(unsigned int)pitch,(unsigned int)pitch_dis);

	// allocate offsprings(same size with group) on GPU
	unsigned int *offsprings_gpu;
	cudaMallocPitch((void**)&offsprings_gpu, &pitch, CITES * sizeof(unsigned int), 
		NUM_OF_POPULATION);
	// allocate offsprings on CPU
	unsigned int offsprings[NUM_OF_POPULATION][CITES];


	// allocate fitness_parent on GPU
	double *fitness_parents_gpu;
	cudaMalloc((void**)&fitness_parents_gpu, NUM_OF_POPULATION*sizeof(double));
	// allocate fitness_offspring on GPU
	double *fitness_offsprings_gpu;
	cudaMalloc((void**)&fitness_offsprings_gpu, NUM_OF_POPULATION*sizeof(double));

	// prepare a DEVICE random valuable generator
	curandGenerator_t gen;
	curandCreateGenerator(&gen,CURAND_RNG_PSEUDO_XORWOW);

	// allocate rvs shown in this paper
	unsigned int *location_crossover;
	float *rv_crossover;
	unsigned int *location_mutation;
	float *rv_mutation;
	unsigned int *candidate_select;
	float *rv_select;
	cudaMalloc((void**)&location_crossover, sizeof(unsigned int) * NUM_OF_POPULATION);
	cudaMalloc((void**)&rv_crossover, sizeof(float) * NUM_OF_POPULATION);
	cudaMalloc((void**)&location_mutation, sizeof(unsigned int) * NUM_OF_POPULATION * 2);
	cudaMalloc((void**)&rv_mutation, sizeof(float) * NUM_OF_POPULATION);
	cudaMalloc((void**)&candidate_select, sizeof(unsigned int) * NUM_OF_POPULATION * 2);
	cudaMalloc((void**)&rv_select, sizeof(float) * NUM_OF_POPULATION);

	//main iteration
	//kernel 1
	get_fitness<<<1,NUM_OF_POPULATION>>>(group_gpu,DIS_gpu,
		fitness_parents_gpu,pitch,pitch_dis);
	int epoch = EPOCH;
	while(epoch--){
		//kernel 2
		curandGenerate(gen,location_crossover,NUM_OF_POPULATION);
		curandGenerateUniform(gen,rv_crossover,NUM_OF_POPULATION);
		curandGenerate(gen,location_mutation,2*NUM_OF_POPULATION);
		curandGenerateUniform(gen,rv_mutation,NUM_OF_POPULATION);
		get_offsprings<<<1,NUM_OF_POPULATION>>>(group_gpu,offsprings_gpu,DIS_gpu,
			location_crossover,rv_crossover,location_mutation,rv_mutation,
			fitness_offsprings_gpu,pitch,pitch_dis);
		curandGenerate(gen,candidate_select,2*NUM_OF_POPULATION);
		curandGenerateUniform(gen,rv_select,NUM_OF_POPULATION);
		select<<<1,NUM_OF_POPULATION>>>(group_gpu,offsprings_gpu,candidate_select,
			rv_mutation,fitness_parents_gpu,fitness_offsprings_gpu,pitch);

	}
	//DEBUG-START
	/* test get_fitness()*/
	// get_fitness<<<1,NUM_OF_POPULATION>>>(group_gpu,DIS_gpu,
	// 	fitness_parents_gpu,pitch,pitch_dis);
	// double fitness[NUM_OF_POPULATION];
	// cudaMemcpy(fitness, fitness_parents_gpu, NUM_OF_POPULATION*sizeof(double), 
	// 	cudaMemcpyDeviceToHost);
	// for(unsigned int i = 0; i < NUM_OF_POPULATION; i++){
	// 	cout<<fitness[i]<<" ";
	// }
	/* test dis successfully tranfered to GPU*/
	//print_dis<<<1,1>>>(DIS_gpu,pitch_dis);

	/*test get_offsprings()*/
	// curandGenerate(gen,location_crossover,NUM_OF_POPULATION);
	// curandGenerateUniform(gen,rv_crossover,NUM_OF_POPULATION);
	// curandGenerate(gen,location_mutation,2*NUM_OF_POPULATION);
	// curandGenerateUniform(gen,rv_mutation,NUM_OF_POPULATION);
	// get_offsprings<<<1,NUM_OF_POPULATION>>>(group_gpu,offsprings_gpu,DIS_gpu,
	// 	location_crossover,rv_crossover,location_mutation,rv_mutation,
	// 	fitness_offsprings_gpu,pitch,pitch_dis);

	// cudaMemcpy2D(offsprings, CITES*sizeof(unsigned int), offsprings_gpu, pitch, 
	// 	CITES*sizeof(unsigned int), NUM_OF_POPULATION, cudaMemcpyDeviceToHost);
	// for(int i =0 ; i < NUM_OF_POPULATION; i++){
	// 	for(int j = 0 ; j < CITES; j++){
	// 		cout<<offsprings[i][j]<<" ";
	// 	}
	// 	cout<<endl;
	// }
	// double fitness[NUM_OF_POPULATION];
	// cudaMemcpy(fitness, fitness_offsprings_gpu, NUM_OF_POPULATION*sizeof(double), 
	// 	cudaMemcpyDeviceToHost);
	// for(unsigned int i = 0; i < NUM_OF_POPULATION; i++){
	// 	cout<<fitness[i]<<" ";
	// }
	/*test select*/
	// curandGenerate(gen,candidate_select,2*NUM_OF_POPULATION);
	// curandGenerateUniform(gen,rv_select,NUM_OF_POPULATION);
	// select<<<1,NUM_OF_POPULATION>>>(group_gpu,offsprings_gpu,candidate_select,
	// 	rv_mutation,fitness_parents_gpu,fitness_offsprings_gpu,pitch);
	cudaMemcpy2D(offsprings, CITES*sizeof(unsigned int),group_gpu, pitch, 
		CITES*sizeof(unsigned int), NUM_OF_POPULATION, cudaMemcpyDeviceToHost);
	for(int i =0 ; i < NUM_OF_POPULATION; i++){
		for(int j = 0 ; j < CITES; j++){
			cout<<offsprings[i][j]<<" ";
		}
		cout<<endl;
	}
	double fitness[NUM_OF_POPULATION];
	cudaMemcpy(fitness, fitness_parents_gpu, NUM_OF_POPULATION*sizeof(double), 
		cudaMemcpyDeviceToHost);
	for(unsigned int i = 0; i < NUM_OF_POPULATION; i++){
		cout<<fitness[i]<<" ";
	}

	//DEBUG-END

	return 0;
}