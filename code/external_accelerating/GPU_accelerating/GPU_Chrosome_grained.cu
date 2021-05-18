#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<curand.h>
#include<curand_kernel.h>
#include<thrust/device_vector.h>
// Cellular Model, so we define a grad like group
#define BLOCK_COL 10
#define BLOCK_ROW 10
#define THREADS_PER_BLOCK 52
// Some constants for Genetic operation
#define MUTATION_PROBABILITY 0.300
int Rand(int X, int Y){
	return rand()%(Y-X+1) + X;
}
void init_group(int arr[][THREADS_PER_BLOCK]){
	int r,temp;
	for(int bid = 0; bid < BLOCK_COL * BLOCK_ROW; bid++){
		for(int i = 0; i < THREADS_PER_BLOCK; i++) arr[bid][i] = i;
		for(int i = 0; i < THREADS_PER_BLOCK; i++){
			r = Rand(0,THREADS_PER_BLOCK-1);
			temp = arr[bid][i];
			arr[bid][i] = arr[bid][r];
			arr[bid][r] = temp;
		}
		// for(int i = 1; i <=THREADS_PER_BLOCK; i++){
		// 	printf("%3d ",arr[bid][i-1]);
		// 	if(i % 10 == 0) printf("\n");
		// }
		//printf("\n");
	}
}

__global__ void crossover(int *parents, int *offsprings, unsigned int *randoms, int pitch){
	int tid = threadIdx.x;
	int bid = blockIdx.x;
	// define 2 crossover points 
	__shared__ int mother,a,b;
	if(tid == 0){
		mother = randoms[bid * 3]%4; // up down right left
		a = randoms[bid * 3 + 1]%THREADS_PER_BLOCK;
		b = randoms[bid * 3 + 2]%THREADS_PER_BLOCK;
	}
	__syncthreads();
	int *parent = (int*)((char*)parents + bid * pitch);
	int *offspring = (int*)((char*)offsprings + bid * pitch);
	offspring[tid] = parent[tid];
	__syncthreads();
	if(a > b){
		int temp;
		temp = a;
		a = b;
		b = temp;
	}

	// cellular model, so the bound need to be considered 
	if(tid >= a && tid <= b){
		if(mother == 0){// up
			int *parent_prime = (int*)((char*)parents + ((bid -1 + BLOCK_ROW)%BLOCK_ROW) * pitch);
			offspring[tid] = parent_prime[tid];
		}
		if(mother == 1){//down
			int *parent_prime = (int*)((char*)parents + ((bid +1)%BLOCK_ROW) * pitch);
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

// randomly choose 2 vertices and swag them
__global__ void mutation(int *group, unsigned int *randoms, float *pms,int pitch){
	int tid = threadIdx.x;
	int bid = blockIdx.x;
	__shared__ int a,b;
	__shared__ float pm;
	a = randoms[bid * 2];
	b = randoms[bid * 2 + 1];
	pm = pms[bid];
	if(tid == a && pm <= MUTATION_PROBABILITY){
		int *individual = (int*)((char*)group + bid * pitch);
		int temp = individual[a];
		individual[a] = individual[b];
		individual[b] = temp;
	}
}

// calculate fitness, actually a reduction operation
__global__ void calculate_fitness(int *group, float *DIS, float *immediates, 
	int pitch, int pitch_dis){
	int tid = threadIdx.x;
	int bid = blockIdx.x;
	__shared__ int *individual;
	individual=(int*)((char*)group + bid * pitch);
	__shared__ float *immediate;
	immediate = (float*)((char*)immediates + bid * pitch);
	__syncthreads();
	float *DIS_row = (float*)((char*)DIS + individual[tid] * pitch_dis);
	// boundary case
	if(tid == (THREADS_PER_BLOCK-1)){
		immediate[tid] = DIS_row[individual[0]];
	}else{
		immediate[tid] = DIS_row[individual[tid+1]];
	}
}

__global__ void reduce_fitness(float *immediates, float *fitness, int pitch){
	int tid = threadIdx.x;
	int bid = blockIdx.x;
	__shared__ float *individual;
	individual=(float*)((char*)immediates + bid * pitch);
	__syncthreads();
	for(int i = blockDim.x/2; i > 0; i/=2){//todo:  fixup 2^n set rhem as 0
		if(tid < i){
			individual[tid] = individual[tid] + individual[tid + i];
		} 
	}
	__syncthreads();
	if(tid == 0){
		fitness[bid] = individual[0];
	}
}


// using REDUCTION to calculate fitness then Natural Selection
__global__ void select_offspring(int *parents, int *offsprings, 
	float* f_parents, float *f_offspring, int pitch){
	int tid = threadIdx.x;
	int bid = blockIdx.x;
	__shared__ int *parent;
	__shared__ int *offspring;
	parent = (int*)((char*)parents + bid * pitch);
	offspring = (int*)((char*)offsprings + bid * pitch);
	__syncthreads();
	if(f_parents[bid] < f_offspring[bid]){
		parent[tid] = offspring[tid];
	}
}
int main(){
	srand((unsigned)time(NULL));

	//init Distance Matrix, This peocedure can also be written as a kernel!!!
	float DIS[THREADS_PER_BLOCK][THREADS_PER_BLOCK];
	for(int i = 0; i < THREADS_PER_BLOCK; i++){
		for(int j = 0; j < THREADS_PER_BLOCK; j++){
			//For convenience to TEST set 0
			DIS[i][j] = 0;
			//TODO: calculate from real data
		}
	}
	//allocate DM to GPU
	size_t pitch_dis;
	float *DIS_gpu;
	cudaMallocPitch((void**)&DIS_gpu, &pitch_dis, 
		THREADS_PER_BLOCK * sizeof(float), THREADS_PER_BLOCK);
	cudaMemcpy2D(DIS_gpu,pitch_dis,DIS,THREADS_PER_BLOCK*sizeof(float), 
		THREADS_PER_BLOCK*sizeof(float), THREADS_PER_BLOCK, cudaMemcpyHostToDevice);

	//init a group on CPU
	int group[BLOCK_COL*BLOCK_ROW][THREADS_PER_BLOCK];
	init_group(group);
	int *group_gpu;
	size_t pitch;

	// allocate this group on GPU
	cudaMallocPitch((void**)&group_gpu,&pitch,THREADS_PER_BLOCK * sizeof(int),
		BLOCK_COL*BLOCK_ROW);
	cudaMemcpy2D(group_gpu, pitch, group, THREADS_PER_BLOCK * sizeof(int), 
		THREADS_PER_BLOCK*sizeof(int), BLOCK_ROW*BLOCK_COL, cudaMemcpyHostToDevice);
	printf("pitch = %d\n",(int)pitch);

	// allocate offspring(same size with group) 
	int *offsprings_gpu;
	cudaMallocPitch((void**)&offsprings_gpu, &pitch, THREADS_PER_BLOCK * sizeof(int), 
		BLOCK_ROW * BLOCK_COL);

	//allocate immediates matrix on GPU
	float *immediates_gpu;
	cudaMallocPitch((void**)&immediates_gpu,&pitch, THREADS_PER_BLOCK * sizeof(float), 
		BLOCK_ROW * BLOCK_COL);
	cudaMemset2D(immediates_gpu, pitch, 0, pitch, BLOCK_ROW * BLOCK_COL);

	// allocate fitness of individuals in group on CPU
	//float fitness_parents[BLOCK_ROW * BLOCK_COL];
	//float fitness_offsprings[BLOCK_ROW * BLOCK_COL];
	// allocate fitness of individuals in group on GPU
	float *fitness_parents_gpu;
	cudaMalloc((void**)&fitness_parents_gpu, BLOCK_ROW*BLOCK_COL*sizeof(float));
	float *fitness_offsprings_gpu;
	cudaMalloc((void**)&fitness_offsprings_gpu, BLOCK_ROW*BLOCK_COL*sizeof(float));

	// prepare a DEVICE random valueble generator
	curandGenerator_t gen;
	curandCreateGenerator(&gen,CURAND_RNG_PSEUDO_XORWOW);

	//devlaration and allcation of DEVICE rvs
	unsigned int *crossover_rv_gpu;
	unsigned int *mutation_location_gpu;
	float *mutation_rv_gpu;
	cudaMalloc((void**)&crossover_rv_gpu, 
		3 * BLOCK_ROW * BLOCK_COL * sizeof(unsigned int));
	cudaMalloc((void**)&mutation_location_gpu, 
		2 * BLOCK_ROW * BLOCK_COL * sizeof(unsigned int));
	cudaMalloc((void**)&mutation_rv_gpu, BLOCK_ROW*BLOCK_COL*sizeof(float));


	//start iterating process
	int epoch = 500;
	time_t start = clock(); //calculating time
	while(--epoch){
		// firstly, generate all neccessary rvs for crossover
		curandGenerate(gen,crossover_rv_gpu,3*BLOCK_ROW*BLOCK_COL);
		// do cross over
		crossover<<<BLOCK_ROW * BLOCK_COL, THREADS_PER_BLOCK>>>(group_gpu,offsprings_gpu,
			crossover_rv_gpu, pitch);// 500 epoch only take 0.005s to execute

		// generate all necessary rvs for mutation
		curandGenerate(gen,mutation_location_gpu,2*BLOCK_ROW*BLOCK_COL);
		curandGenerateUniform(gen,mutation_rv_gpu,BLOCK_ROW*BLOCK_COL);// 0-1 uniform
		mutation<<<BLOCK_ROW * BLOCK_COL, THREADS_PER_BLOCK>>>(offsprings_gpu, 
			mutation_location_gpu, mutation_rv_gpu,pitch);// 500 epoch only take 0.001s to execute

		//calculate fitness
		calculate_fitness<<<BLOCK_ROW*BLOCK_COL,THREADS_PER_BLOCK>>>(group_gpu,DIS_gpu,immediates_gpu,pitch,pitch_dis);
		reduce_fitness<<<BLOCK_ROW*BLOCK_COL,pitch/sizeof(float)>>>(immediates_gpu,fitness_parents_gpu,pitch);
		//thrust is slow
		// for(int i= 0; i < BLOCK_ROW * BLOCK_COL; i++){
		// 	float *immediate_gpu = (float*)((char*)immediates_gpu + i * pitch);
		// 	thrust::device_vector<float> dvec(immediate_gpu,immediate_gpu + THREADS_PER_BLOCK);
			
		// 	fitness_parents[i] = thrust::reduce(dvec.begin(),dvec.end(),(float)0, thrust::plus<float>());
		// }
		calculate_fitness<<<BLOCK_ROW*BLOCK_COL,THREADS_PER_BLOCK>>>(offsprings_gpu,DIS_gpu,immediates_gpu,pitch,pitch_dis);
		reduce_fitness<<<BLOCK_ROW*BLOCK_COL,pitch/sizeof(float)>>>(immediates_gpu,fitness_offsprings_gpu,pitch);
		// for(int i= 0; i < BLOCK_ROW * BLOCK_COL; i++){
		// 	float *immediate_gpu = (float*)((char*)immediates_gpu + i * pitch);
		// 	thrust::device_vector<float> dvec(immediate_gpu,immediate_gpu+THREADS_PER_BLOCK);
			
		// 	fitness_offsprings[i] = thrust::reduce(dvec.begin(),dvec.end(),(float)0, thrust::plus<float>());
		// }
		
		select_offspring<<<BLOCK_ROW * BLOCK_COL, THREADS_PER_BLOCK>>>(group_gpu,offsprings_gpu,
			fitness_parents_gpu,fitness_offsprings_gpu,pitch);

		// data_recording<<<BLOCK_ROW * BLOCK_COL, THREADS_PER_BLOCK>>>();
	}
	time_t end = clock();
	double time_used = (double)(end - start) / CLOCKS_PER_SEC;
	printf("Time Used: %.5f; Time Used Per Epoch: %.5f\n",time_used,time_used/500);
	cudaFree(group_gpu);
	cudaFree(offsprings_gpu);
	cudaFree(crossover_rv_gpu);
	cudaFree(mutation_rv_gpu);
	return 0;
}

