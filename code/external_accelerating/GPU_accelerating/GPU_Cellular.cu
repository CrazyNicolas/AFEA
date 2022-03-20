#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<curand.h>
#include<iostream>
#include<curand_kernel.h>
#include<algorithm>
using namespace std;
// Cellular Model, so we define a grid like group
#define BLOCK_COL 10
#define BLOCK_ROW 10
#define THREADS_PER_BLOCK 52
// Some constants for Genetic operation
#define MUTATION_PROBABILITY 0.300

//SBX const
#define ETA 2

//define a struct for sorting with key
typedef struct{
	float value;
	int key;
}SortObj;
bool compare(SortObj a, SortObj b){
	return a.value < b.value;
}

//read TSP data from *.tsp file
double** Read_TSP(char* path)
{
	unsigned int a;
	freopen(path, "r", stdin);
	double** res = new double* [BLOCK_ROW*BLOCK_COL];
	double* x = new double[THREADS_PER_BLOCK];
	double* y = new double[THREADS_PER_BLOCK];
	for (unsigned int i = 0; i < THREADS_PER_BLOCK; i++)
	{
		res[i] = new double[THREADS_PER_BLOCK];
		std::cin >> a;	//Some benchmark data have additional index values before coordinates.
		std::cin >> x[i] >> y[i];
	}
	fclose(stdin);
	for (unsigned int i = 0; i < THREADS_PER_BLOCK; i++)
		for (unsigned int j = 0; j < THREADS_PER_BLOCK; j++)
			res[i][j] = res[j][i] = sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));
	delete[] x, y;
	return res;
}


unsigned int Rand(unsigned int X, unsigned int Y){
	return rand()%(Y-X+1) + X;
}
void init_group(unsigned int arr[][THREADS_PER_BLOCK]){
	unsigned int r,temp;
	for(unsigned int bid = 0; bid < BLOCK_COL * BLOCK_ROW; bid++){
		for(unsigned int i = 0; i < THREADS_PER_BLOCK; i++) arr[bid][i] = i;
		for(unsigned int i = 0; i < THREADS_PER_BLOCK; i++){
			r = Rand(0,THREADS_PER_BLOCK-1);
			temp = arr[bid][i];
			arr[bid][i] = arr[bid][r];
			arr[bid][r] = temp;
		}
		// for(unsigned int i = 1; i <=THREADS_PER_BLOCK; i++){
		// 	printf("%3d ",arr[bid][i-1]);
		// 	if(i % 10 == 0) printf("\n");
		// }
		//printf("\n");
	}
}

// this crossover are not suitable for TSP instead refer to SBX_crossover below
__global__ void crossover(unsigned int *parents, unsigned int *offsprings, unsigned int *randoms, unsigned int pitch){
	unsigned int tid = threadIdx.x;
	unsigned int bid = blockIdx.x;
	// define 2 crossover points 
	__shared__ unsigned int mother,a,b;
	if(tid == 0){
		mother = randoms[bid * 3]%4; // up down right left
		a = randoms[bid * 3 + 1]%THREADS_PER_BLOCK;
		b = randoms[bid * 3 + 2]%THREADS_PER_BLOCK;
	}
	__syncthreads();
	unsigned int *parent = (unsigned int*)((char*)parents + bid * pitch);
	unsigned int *offspring = (unsigned int*)((char*)offsprings + bid * pitch);
	offspring[tid] = parent[tid];
	__syncthreads();
	if(a > b){
		unsigned int temp;
		temp = a;
		a = b;
		b = temp;
	}

	// cellular model, so the bound need to be considered 
	if(tid >= a && tid <= b){
		if(mother == 0){// up
			unsigned int *parent_prime = (unsigned int*)((char*)parents + ((bid -1 + BLOCK_ROW)%BLOCK_ROW) * pitch);
			offspring[tid] = parent_prime[tid];
		}
		if(mother == 1){//down
			unsigned int *parent_prime = (unsigned int*)((char*)parents + ((bid +1)%BLOCK_ROW) * pitch);
			offspring[tid] = parent_prime[tid];
		}
		if(mother == 2){//left
			offspring[tid] = parent[(tid- 1+ BLOCK_COL)%BLOCK_COL];
		}
		if(mother == 3){//right
			offspring[tid] = parent[(tid +1)%BLOCK_COL];
		}
	}
	
}

//SBX
__global__ void SBX_crossover(unsigned int *parents, float *pre_offsprings, 
	unsigned int *randoms, float* random_SBX, unsigned int pitch){
	unsigned int tid = threadIdx.x;
	unsigned int bid = blockIdx.x;
	__shared__ unsigned int mother;
	__shared__ float beta;
	if(tid == 0){
		mother = randoms[bid]%4; // up down right left
		if(random_SBX[bid] > 0.5){
			beta = pow( (double)2 * random_SBX[bid] , (double)1 / (1 + ETA));
		}else{
			beta = pow((double)1/(2 - 2*random_SBX[bid]), (double)1/(1+ETA));
		}
	}
	__syncthreads();
	unsigned int *parent = (unsigned int*)((char*)parents + bid * pitch);
	unsigned int *pre_offspring = (unsigned int*)((char*)pre_offsprings + bid * pitch);
	if(mother == 0){// up
		unsigned int *parent_prime = (unsigned int*)((char*)parents + (((bid-BLOCK_ROW + BLOCK_ROW*BLOCK_COL)/BLOCK_ROW)%BLOCK_ROW*BLOCK_ROW + bid%BLOCK_ROW) * pitch);
		pre_offspring[tid] = 0.5 * ((1+beta) * parent_prime[tid] + (1-beta) * parent[tid]);
	}
	if(mother == 1){//down
		unsigned int *parent_prime = (unsigned int*)((char*)parents + ((((bid+BLOCK_ROW)/BLOCK_ROW)%BLOCK_ROW)*BLOCK_ROW + bid%BLOCK_ROW) * pitch);
		pre_offspring[tid] = 0.5 * ((1+beta) * parent_prime[tid] + (1-beta) * parent[tid]);
	}
	if(mother == 2){//left
		unsigned int *parent_prime = (unsigned int*)((char*)parents + (bid/BLOCK_ROW*BLOCK_ROW + (bid -1)%BLOCK_ROW) * pitch);
		pre_offspring[tid] = 0.5 * ((1+beta) * parent_prime[tid] + (1-beta) * parent[tid]);
	}
	if(mother == 3){//right
		unsigned int *parent_prime = (unsigned int*)((char*)parents + (bid/BLOCK_ROW*BLOCK_ROW + (bid +1)%BLOCK_ROW) * pitch);
		pre_offspring[tid] = 0.5 * ((1+beta) * parent_prime[tid] + (1-beta) * parent[tid]);
	}
	

}

// randomly choose 2 vertices and swag them
__global__ void mutation(unsigned int *group, unsigned int *randoms, float *pms,unsigned int pitch){
	unsigned int tid = threadIdx.x;
	unsigned int bid = blockIdx.x;
	__shared__ unsigned int a,b;
	__shared__ float pm;
	a = randoms[bid * 2] % THREADS_PER_BLOCK;
	b = randoms[bid * 2 + 1] %THREADS_PER_BLOCK;
	pm = pms[bid];
	if(tid == a && pm <= MUTATION_PROBABILITY){
		unsigned int *individual = (unsigned int*)((char*)group + bid * pitch);
		unsigned int temp = individual[a];
		individual[a] = individual[b];
		individual[b] = temp;
	}
}

// calculate fitness, actually a reduction operation
__global__ void calculate_fitness(unsigned int *group, double *DIS, double *immediates, 
	unsigned int pitch, unsigned int pitch_dis){
	unsigned int tid = threadIdx.x;
	unsigned int bid = blockIdx.x;
	__shared__ unsigned int *individual;
	individual=(unsigned int*)((char*)group + bid * pitch);
	__shared__ double *immediate;
	immediate = (double*)((char*)immediates + bid * pitch);
	__syncthreads();
	double *DIS_row = (double*)((char*)DIS + individual[tid] * pitch_dis);
	// boundary case
	if(tid == (THREADS_PER_BLOCK-1)){
		immediate[tid] = DIS_row[individual[0]];
	}else{
		//printf("%.2lf\n",DIS_row[individual[tid+1]]);
		immediate[tid] = DIS_row[individual[tid+1]];
	}
}

__global__ void reduce_fitness(double *immediates, double *fitness, unsigned int pitch){
	unsigned int tid = threadIdx.x;
	unsigned int bid = blockIdx.x;
	__shared__ double *individual;
	individual=(double*)((char*)immediates + bid * pitch);
	__syncthreads();
	for(unsigned int i = blockDim.x/2; i > 0; i/=2){
		if(tid < i){
			individual[tid] = individual[tid] + individual[tid + i];
		} 
	}
	__syncthreads();
	if(tid == 0){
		fitness[bid] = individual[0];
		//printf("%.2lf\n",individual[0]);
	}
}


// using REDUCTION to calculate fitness then Natural Selection
__global__ void select_offspring(unsigned int *parents, unsigned int *offsprings, 
	double* f_parents, double *f_offspring, unsigned int pitch){
	unsigned int tid = threadIdx.x;
	unsigned int bid = blockIdx.x;
	__shared__ unsigned int *parent;
	__shared__ unsigned int *offspring;
	parent = (unsigned int*)((char*)parents + bid * pitch);
	offspring = (unsigned int*)((char*)offsprings + bid * pitch);
	__syncthreads();
	if(f_parents[bid] > f_offspring[bid]){
		parent[tid] = offspring[tid];
	}
	__syncthreads();
	if(f_parents[bid] > f_offspring[bid]){
		if(tid == 0) f_parents[bid] = f_offspring[bid];
	}
}
int main(){
	srand((unsigned)time(NULL));
	//init Distance Matrix, This peocedure can also be written as a kernel!!!
	double DIS[THREADS_PER_BLOCK][THREADS_PER_BLOCK];
	double **dis = Read_TSP((char*)"berlin52.tsp");
	//test if read correctly
	for(unsigned int i = 0; i < THREADS_PER_BLOCK; i++){
		for(unsigned int j = 0; j < THREADS_PER_BLOCK; j++){
			DIS[i][j] = dis[i][j];
			cout<<DIS[i][j]<<" ";
		}
		cout<<endl;
	}

	//allocate DM to GPU
	size_t pitch_dis;
	double *DIS_gpu;
	cudaMallocPitch((void**)&DIS_gpu, &pitch_dis, 
		THREADS_PER_BLOCK * sizeof(double), THREADS_PER_BLOCK);
	cudaMemcpy2D(DIS_gpu,pitch_dis,DIS,THREADS_PER_BLOCK*sizeof(double), 
		THREADS_PER_BLOCK*sizeof(double), THREADS_PER_BLOCK, cudaMemcpyHostToDevice);

	//init a group on CPU
	unsigned int group[BLOCK_COL*BLOCK_ROW][THREADS_PER_BLOCK];
	init_group(group);
	//test init
	// for(unsigned int i = 0; i < BLOCK_ROW * BLOCK_COL; i++){
	// 	for(unsigned int j = 0; j < THREADS_PER_BLOCK; j++){
	// 		cout<<group[i][j]<<" ";
	// 	}
	// 	cout<<endl;
	// }
	unsigned int *group_gpu;
	size_t pitch;

	// allocate this group on GPU
	cudaMallocPitch((void**)&group_gpu,&pitch,THREADS_PER_BLOCK * sizeof(unsigned int),
		BLOCK_COL*BLOCK_ROW);
	cudaMemcpy2D(group_gpu, pitch, group, THREADS_PER_BLOCK * sizeof(unsigned int), 
		THREADS_PER_BLOCK*sizeof(unsigned int), BLOCK_ROW*BLOCK_COL, cudaMemcpyHostToDevice);
	printf("pitch = %d, dis_pitch = %d\n",(unsigned int)pitch,(unsigned int)pitch_dis);

	// allocate pre_offsprings in CPU
	float pre_offsprings[BLOCK_ROW*BLOCK_COL][THREADS_PER_BLOCK];
	// allocate pre_offspring in GPU
	float *pre_offsprings_gpu;
	cudaMallocPitch((void**)&pre_offsprings_gpu, &pitch, THREADS_PER_BLOCK * sizeof(unsigned int), 
		BLOCK_ROW * BLOCK_COL);
	// allocate offsprings(same size with group) on GPU
	unsigned int *offsprings_gpu;
	cudaMallocPitch((void**)&offsprings_gpu, &pitch, THREADS_PER_BLOCK * sizeof(unsigned int), 
		BLOCK_ROW * BLOCK_COL);
	// allocate offsprings on CPU
	unsigned int offsprings[BLOCK_ROW*BLOCK_COL][THREADS_PER_BLOCK];

	//allocate immediates matrix on GPU
	double *immediates_gpu;
	cudaMallocPitch((void**)&immediates_gpu,&pitch, THREADS_PER_BLOCK * sizeof(double), 
		BLOCK_ROW * BLOCK_COL);
	cudaMemset2D(immediates_gpu, pitch, 0, pitch, BLOCK_ROW * BLOCK_COL);

	// allocate fitness of individuals in group on CPU
	//double fitness_parents[BLOCK_ROW * BLOCK_COL];
	//double fitness_offsprings[BLOCK_ROW * BLOCK_COL];
	// allocate fitness of individuals in group on GPU
	double *fitness_parents_gpu;
	cudaMalloc((void**)&fitness_parents_gpu, BLOCK_ROW*BLOCK_COL*sizeof(double));
	double *fitness_offsprings_gpu;
	cudaMalloc((void**)&fitness_offsprings_gpu, BLOCK_ROW*BLOCK_COL*sizeof(double));

	// prepare a DEVICE random valuable generator
	curandGenerator_t gen;
	curandCreateGenerator(&gen,CURAND_RNG_PSEUDO_XORWOW);

	//devlaration and allocation of DEVICE rvs
	unsigned int *crossover_rv_gpu;
	float *crossover_rv_SBX_gpu;
	unsigned int *mutation_location_gpu;
	float *mutation_rv_gpu;
	cudaMalloc((void**)&crossover_rv_gpu, 
		BLOCK_ROW * BLOCK_COL * sizeof(unsigned int));
	cudaMalloc((void**)&crossover_rv_SBX_gpu, BLOCK_ROW*BLOCK_COL*sizeof(float));
	cudaMalloc((void**)&mutation_location_gpu, 
		2 * BLOCK_ROW * BLOCK_COL * sizeof(unsigned int));
	cudaMalloc((void**)&mutation_rv_gpu, BLOCK_ROW*BLOCK_COL*sizeof(float));

	//define struct 2D array for sorting pre_offsprings in crossover_SBX
	SortObj **sobjs = new SortObj*[BLOCK_ROW*BLOCK_COL];
	for(int i = 0; i < BLOCK_ROW * BLOCK_COL; i++){
		sobjs[i] = new SortObj[THREADS_PER_BLOCK];
	}

	//start iterating process
	unsigned int epoch = 500;
	time_t start = clock(); //calculating time
	while(--epoch){
		// firstly, generate all neccessary rvs for crossover
		curandGenerate(gen,crossover_rv_gpu,BLOCK_ROW*BLOCK_COL);
		curandGenerateUniform(gen,crossover_rv_SBX_gpu,BLOCK_ROW*BLOCK_COL);// 0-1 uniform
		// do cross over
		SBX_crossover<<<BLOCK_ROW * BLOCK_COL, THREADS_PER_BLOCK>>>(group_gpu,pre_offsprings_gpu,
			crossover_rv_gpu, crossover_rv_SBX_gpu, pitch);// 500 epoch only take 0.005s to execute
		//now we got SBX result: pre_offsprings sort on cpu and copy to offsprings_gpu
		//1. copy out pre_offsprings 
		cudaMemcpy2D(pre_offsprings, THREADS_PER_BLOCK*sizeof(float), 
			pre_offsprings_gpu, pitch, THREADS_PER_BLOCK*sizeof(float), BLOCK_ROW*BLOCK_COL, cudaMemcpyDeviceToHost);
		//2. construct sorting obj
		for(int i = 0; i < BLOCK_ROW * BLOCK_COL; i++){
			for(int j = 0; j < THREADS_PER_BLOCK; j++){
				sobjs[i][j].value = pre_offsprings[i][j];
				sobjs[i][j].key = j;
			}
		}
		//3. sorting
		for(int i=0; i<BLOCK_ROW*BLOCK_COL; i++){
			sort(sobjs[i],sobjs[i]+THREADS_PER_BLOCK,compare);
			for(int j =0; j <THREADS_PER_BLOCK; j++){//set offsprings
				offsprings[i][sobjs[i][j].key] = j;
			}
		}

		//4. copy offsprings to offsprings_gpu
		cudaMemcpy2D(offsprings_gpu, pitch, offsprings, THREADS_PER_BLOCK*sizeof(float)
			, THREADS_PER_BLOCK*sizeof(float), BLOCK_ROW*BLOCK_COL, cudaMemcpyHostToDevice);
		// generate all necessary rvs for mutation
		curandGenerate(gen,mutation_location_gpu,2*BLOCK_ROW*BLOCK_COL);
		curandGenerateUniform(gen,mutation_rv_gpu,BLOCK_ROW*BLOCK_COL);// 0-1 uniform
		mutation<<<BLOCK_ROW * BLOCK_COL, THREADS_PER_BLOCK>>>(offsprings_gpu, 
			mutation_location_gpu, mutation_rv_gpu,pitch);// 500 epoch only take 0.001s to execute

		//calculate fitness
		calculate_fitness<<<BLOCK_ROW*BLOCK_COL,THREADS_PER_BLOCK>>>(group_gpu,DIS_gpu,immediates_gpu,pitch,pitch_dis);
		reduce_fitness<<<BLOCK_ROW*BLOCK_COL,pitch/sizeof(double)>>>(immediates_gpu,fitness_parents_gpu,pitch);
		//thrust is slow
		// for(unsigned int i= 0; i < BLOCK_ROW * BLOCK_COL; i++){
		// 	double *immediate_gpu = (double*)((char*)immediates_gpu + i * pitch);
		// 	thrust::device_vector<double> dvec(immediate_gpu,immediate_gpu + THREADS_PER_BLOCK);
			
		// 	fitness_parents[i] = thrust::reduce(dvec.begin(),dvec.end(),(double)0, thrust::plus<double>());
		// }
		calculate_fitness<<<BLOCK_ROW*BLOCK_COL,THREADS_PER_BLOCK>>>(offsprings_gpu,DIS_gpu,immediates_gpu,pitch,pitch_dis);
		reduce_fitness<<<BLOCK_ROW*BLOCK_COL,pitch/sizeof(double)>>>(immediates_gpu,fitness_offsprings_gpu,pitch);
		// for(unsigned int i= 0; i < BLOCK_ROW * BLOCK_COL; i++){
		// 	double *immediate_gpu = (double*)((char*)immediates_gpu + i * pitch);
		// 	thrust::device_vector<double> dvec(immediate_gpu,immediate_gpu+THREADS_PER_BLOCK);
			
		// 	fitness_offsprings[i] = thrust::reduce(dvec.begin(),dvec.end(),(double)0, thrust::plus<double>());
		// }
		
		select_offspring<<<BLOCK_ROW * BLOCK_COL, THREADS_PER_BLOCK>>>(group_gpu,offsprings_gpu,
			fitness_parents_gpu,fitness_offsprings_gpu,pitch);

		

		
	}
	time_t end = clock();
	double time_used = (double)(end - start) / CLOCKS_PER_SEC;
	printf("Time Used: %.5f; Time Used Per Epoch: %.5f\n",time_used,time_used/500);

	//in the end, find the best individual and his fitness
	double result[BLOCK_ROW * BLOCK_COL];
	cudaMemcpy(result, fitness_parents_gpu, 
		BLOCK_COL*BLOCK_ROW*sizeof(double), cudaMemcpyDeviceToHost);
	for(unsigned int i =0; i < BLOCK_ROW; i++){
		for(unsigned int j = 0; j < BLOCK_COL; j++){
			printf("%.2lf\t",result[i * BLOCK_ROW +j]);
		}
		printf("\n");
	}
	printf("\n");
	cudaMemcpy2D(group, THREADS_PER_BLOCK * sizeof(unsigned int), group_gpu, pitch, 
		THREADS_PER_BLOCK*sizeof(unsigned int), BLOCK_ROW*BLOCK_COL, cudaMemcpyDeviceToHost);

	for(unsigned int i =0; i <BLOCK_ROW * BLOCK_COL; i++){
		for(unsigned int j = 0; j < THREADS_PER_BLOCK; j++){
			printf("%d ",group[i][j]);
		}

		printf("\n\n");
	}

	//DEBUG-START
	// calculate_fitness<<<BLOCK_ROW*BLOCK_COL,THREADS_PER_BLOCK>>>(group_gpu,
	// 	DIS_gpu,immediates_gpu,pitch,pitch_dis);
	// reduce_fitness<<<BLOCK_ROW*BLOCK_COL,pitch/sizeof(double)>>>(immediates_gpu,
	// 		fitness_parents_gpu,pitch);
	// double res[BLOCK_ROW * BLOCK_COL];
	// cudaMemcpy(res, fitness_parents_gpu,
	//  BLOCK_ROW*BLOCK_COL*sizeof(double), cudaMemcpyDeviceToHost);
	// for(unsigned int i =0; i < BLOCK_ROW; i++){
	// 	for(unsigned int j = 0; j < BLOCK_COL; j++){
	// 		printf("%.2lf\t",res[i * BLOCK_ROW +j]);
	// 	}
	// 	printf("\n");
	// }

	//DEBUG-END
	cudaFree(group_gpu);
	cudaFree(offsprings_gpu);
	cudaFree(crossover_rv_gpu);
	cudaFree(mutation_rv_gpu);
	return 0;
}

