#include <stdio.h>
#include <time.h>
__global__ void kernel(float *arr)
{
	arr[threadIdx.x] = arr[threadIdx.x] + threadIdx.x;
}
__global__ void add(int *a, int *b, int *c, int num)
{
	c[threadIdx.x] = a[threadIdx.x] + b[threadIdx.x];
}

__global__ void reduce(int *a, int *b)
{
	int tid = threadIdx.x;
	__shared__ int sData[16];
	sData[tid] = a[tid];
	__syncthreads();
	int i = 16;
	while(i >= 1){
		i/=2;
		if(tid < i ){
			sData[tid] = sData[tid] + sData[tid + i];
		}
		__syncthreads();
	}

	if( tid == 0){
		b[tid] = sData[tid];
	}
}

void cpuSum(int *a, int *b){
	b[0] = 0;
	for(int i = 0; i < 16; i++){
		b[0] += a[i];
	}
}
int main(int argc, char **argv)
{
	////basic opration
	// cudaSetDevice(0);
	// float *aGPU;
	// cudaMalloc((void**)&aGPU,16*sizeof(float));
	// float a[16] = {0};
	// cudaMemcpy(aGPU,a,16*sizeof(float),cudaMemcpyHostToDevice);
	// kernel<<<1,16>>>(aGPU);
	// cudaMemcpy(a,aGPU,16*sizeof(float),cudaMemcpyDeviceToHost);
	// for(int i = 0; i <16; i++)
	// {
	// 	printf("%.2f\t",a[i]);
	// }
	// cudaFree(aGPU);
	// cudaDeviceReset();
	// printf("\n");
	// int gpuCount = -1;
	// cudaGetDeviceCount(&gpuCount);
	// printf("This PC has %d GPUs\n",gpuCount);

	// //look up some information of GPU
	// cudaDeviceProp prop;
	// cudaGetDeviceProperties(&prop,0);
	// printf("max thread per block: %d\n",prop.maxThreadsPerBlock);
	// printf("Total memory: %zd\n",prop.totalConstMem);
	
	//// add operation
	// cudaSetDevice(0);
	// const int num = 10;
	// int a[num],b[num],c[num];
	// int *a_gpu, *b_gpu, *c_gpu;
	// for(int i = 0 ; i < num; i++)
	// {
	// 	a[i] = i;
	// 	b[i] = i * i;
	// }
	// cudaMalloc((void**)&a_gpu,sizeof(int) * num);
	// cudaMalloc((void**)&b_gpu,sizeof(int) * num);
	// cudaMalloc((void**)&c_gpu,sizeof(int) * num);

	// //copy data
	// cudaMemcpy(a_gpu, a, num * sizeof(int), cudaMemcpyHostToDevice);
	// cudaMemcpy(b_gpu, b, num * sizeof(int), cudaMemcpyHostToDevice);

	// //do
	// add<<<1,num>>>(a_gpu,b_gpu,c_gpu,num);

	// // get data
	// cudaMemcpy(c, c_gpu, num * sizeof(int), cudaMemcpyDeviceToHost);

	// // io
	// for(int i = 0; i < num; i++){
	// 	printf("%d\n",c[i]);
	// }


	// Reduce运算
	const int num = 16;
	int a[num];
	int b[1];
	int *aGpu;
	int *bGpu;
	for(int i = 0 ; i < num; i++){
		a[i] = i * (i+1);
	}
	cudaMalloc((void**)&aGpu,num*sizeof(int));
	cudaMalloc((void**)&bGpu, 1*sizeof(int));
	cudaMemcpy(aGpu, a, num*sizeof(int), cudaMemcpyHostToDevice);

	//do
	reduce<<<1,num>>>(aGpu,bGpu);
	cudaMemcpy(b, bGpu, sizeof(int), cudaMemcpyDeviceToHost);
	printf("sum = %d\n",b[0]);

	//comparing performance of GPU and CPU
	// run 10000 times to calculate the total time
	clock_t startTime,endTime;
	startTime = clock();
	for(int i = 0; i < 1000000; i++){
		reduce<<<1,num>>>(aGpu,bGpu);
	}
	endTime = clock();
	printf("1000000 times on GPU need: %.3f seconds\n",
		(double)(endTime-startTime)/CLOCKS_PER_SEC);

	// cpu case
	startTime = clock();
	for(int i = 0; i < 1000000; i++){
		cpuSum(a,b);
	}
	endTime = clock();
	printf("1000000 times on CPU need: %.3f seconds\n",
		(double)(endTime-startTime)/CLOCKS_PER_SEC);
	cudaFree(aGpu);
	cudaFree(bGpu);
	return 0;
}