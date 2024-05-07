#include "../include/core/base/AnthemDefs.h"
#include "../include/core/base/AnthemLogger.h"
#include <cstdio>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "device_functions.h"

typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t i32;
typedef int64_t i64;


#define DCONST const
DCONST u32 LOG_BLOCK_SIZE = 10;
DCONST u32 BLOCK_SIZE = (1 << LOG_BLOCK_SIZE);
DCONST u32 LOG_NUM_BLOCKS = 8;
DCONST u32 NUM_BLOCKS = 1 << LOG_NUM_BLOCKS;
DCONST u32 TOTAL_SIZE = BLOCK_SIZE * NUM_BLOCKS;
DCONST u32 BIT_PER_PASS = 2;
DCONST u32 BLOCK_SUM_SIZE = NUM_BLOCKS * (1 << BIT_PER_PASS);
DCONST u32 MAX_BITS = 32;
DCONST u32 BIT_CANDIDATES = 1<<BIT_PER_PASS;
DCONST char* SAVE_RESULT = ANTH_ASSET_TEMP_DIR"radixresultcuda.txt";
#undef DCONST


u32* hData;
u32* hResult;
u32* hDataCopy;
u32* dBufferGlobal[2];
u32* dLocalPrefixSum;
u32* dBlockPrefixSum;

double cudaRadixTime = 0.0;
double cudaRadixTimeAll = 0.0;
double stdSortTime = 0.0;
double cpuRadixTime = 0.0;

struct CPURadix {
	// Code enclosed in this struct from: 
	// https://www.geeksforgeeks.org/radix-sort/
	int getMax(u32 a[], int n) {
		u32 i, max = 0;
		for (i = 0; i < n; i++) {
			if (max < a[i])max = a[i];
		}
		return max;
	}
	void countSort(u32 a[], int n, int exp) {
		u32* output = new u32[n];
		u32 i, buckets[10] = { 0 };
		u32* temp = new u32[n];
		for (int i = 0; i < n; i++) {
			buckets[(a[i] / exp) % 10]++;
		}
		for (int i = 1; i < 10; i++) {
			buckets[i] += buckets[i - 1];
		}
		for (int i = n - 1; i >= 0; i--) {
			output[buckets[(a[i] / exp) % 10] - 1] = a[i];
			buckets[(a[i] / exp) % 10]--;
		}
		for (int i = 0; i < n; i++) {
			a[i] = output[i];
		}
	}
	void radixSort(u32 a[], int n) {
		u32 exp;
		u32 max = getMax(a, n);
		for (exp = 1; max / exp > 0; exp *= 10) {
			countSort(a, n, exp);
		}
	}
};

__global__ void radixSortLocalHistogram(const int iter,u32* dLocalPrefixSum,u32* dBlockPrefixSum,u32* dSrc) {
	u32 tId = threadIdx.x;
	u32 bId = blockIdx.x;
	u32 gId = bId * BLOCK_SIZE + tId;
	u32* dBuffer = dSrc;
	__shared__ u32 localBins[BLOCK_SIZE];
	__shared__ u32 localCounter[BIT_CANDIDATES];

	if(tId<BIT_CANDIDATES){
		localCounter[tId] = 0;
	}
	__syncthreads();

	u32 curVal = dBuffer[gId];
	u32 curBit = (curVal>>(iter*BIT_PER_PASS)) & (BIT_CANDIDATES-1);

	atomicAdd(&localCounter[curBit], 1);
	__syncthreads();

	for(i32 i=0;i<BIT_CANDIDATES;i++){
		localBins[tId] = (curBit==i)?1:0;
		
		__syncthreads();
		for(i32 j=0;j<LOG_BLOCK_SIZE;j++){
			u32 base = (1<<(j+1))-1;
			u32 increment = (1<<(j+1));
			if(tId>=base&&(tId-base)%increment==0){
				localBins[tId] += localBins[tId-increment/2];
			}
			__syncthreads();
		}
		for(i32 j= LOG_BLOCK_SIZE-2;j>=0;j--){
			u32 base = (1<<(j+1))-1+(1<<(j));
			u32 increment = (1<<(j+1));
			if(tId>=base&&(tId-base)%increment==0){
				localBins[tId] += localBins[tId-increment/2];
			}
			__syncthreads();
		}
		if(curBit==i){
			dLocalPrefixSum[gId] = localBins[tId]-1;
		}
		__syncthreads();
	}
	if(tId<BIT_CANDIDATES){
		dBlockPrefixSum[tId*NUM_BLOCKS+bId] = localCounter[tId];
	}
}

__global__ void radixSortBlockPrefix(u32* dBlockPrefixSum){
	u32 tId = threadIdx.x;
	__shared__ u32 localCounter[NUM_BLOCKS * BIT_CANDIDATES];
	__shared__ u32 srcValue[NUM_BLOCKS * BIT_CANDIDATES];

	srcValue[tId] = dBlockPrefixSum[tId];
	localCounter[tId] = srcValue[tId];
	__syncthreads();
	for(i32 i=0;i<BIT_PER_PASS*LOG_NUM_BLOCKS;i++){
		u32 base = (1<<(i+1))-1;
		u32 increment = (1<<(i+1));
		if(tId>=base&&(tId-base)%increment==0){
			localCounter[tId] += localCounter[tId-increment/2];
		}
		__syncthreads();
	}
	for(i32 i= BIT_PER_PASS * LOG_NUM_BLOCKS -2;i>=0;i--){
		u32 base = (1<<(i+1))-1+(1<<(i));
		u32 increment = (1<<(i+1));
		if(tId>=base&&(tId-base)%increment==0){
			localCounter[tId] += localCounter[tId-increment/2];
		}
		__syncthreads();
	}
	dBlockPrefixSum[tId] = localCounter[tId] - srcValue[tId];
}

__global__ void radixSortScatter(const int iter,u32* dBlockPrefixSum,u32* dLocalPrefixSum, u32* dSrc,u32* dDst) {
	u32 tId = threadIdx.x;
	u32 bId = blockIdx.x;
	u32 gId = bId * BLOCK_SIZE + tId;
	u32* dBuffer = dSrc;

	u32 curVal = dBuffer[gId];
	u32 curBit = (curVal>>(iter*BIT_PER_PASS)) & (BIT_CANDIDATES-1);
	u32 blockPrefix = dBlockPrefixSum[curBit* NUM_BLOCKS +bId];
	u32 localPrefix = dLocalPrefixSum[gId];
	u32 scatterPos = blockPrefix + localPrefix;
	dDst[scatterPos] = curVal;
}

void radixSortExec(){
	for(i32 i=0;i<MAX_BITS/BIT_PER_PASS;i++){
		radixSortLocalHistogram AT_KARG2(NUM_BLOCKS, BLOCK_SIZE) (i,dLocalPrefixSum,dBlockPrefixSum,dBufferGlobal[i & 1]);
		radixSortBlockPrefix AT_KARG2(1, NUM_BLOCKS * BIT_CANDIDATES) (dBlockPrefixSum);
		radixSortScatter AT_KARG2(NUM_BLOCKS, BLOCK_SIZE) (i,dBlockPrefixSum,dLocalPrefixSum, dBufferGlobal[i & 1], dBufferGlobal[1-(i & 1)]);
	}
}

void radixSort() {

	cudaMalloc(&dBufferGlobal[0], TOTAL_SIZE * sizeof(u32));
	cudaMalloc(&dBufferGlobal[1], TOTAL_SIZE * sizeof(u32));
	cudaMalloc(&dLocalPrefixSum, TOTAL_SIZE * sizeof(u32));
	cudaMalloc(&dBlockPrefixSum, BLOCK_SUM_SIZE * sizeof(u32));
	auto start1 = std::chrono::high_resolution_clock::now();
	cudaMemcpy(dBufferGlobal[0], hData, TOTAL_SIZE * sizeof(u32), cudaMemcpyHostToDevice);
	auto start = std::chrono::high_resolution_clock::now();
	radixSortExec();
	cudaDeviceSynchronize();
	auto end = std::chrono::high_resolution_clock::now();
	cudaRadixTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	//cudaMemcpy(hResult,dBlockPrefixSum, NUM_BLOCKS * BIT_CANDIDATES * sizeof(u32), cudaMemcpyDeviceToHost);
	cudaMemcpy(hResult, dBufferGlobal[MAX_BITS/BIT_PER_PASS & 1], TOTAL_SIZE*sizeof(u32), cudaMemcpyDeviceToHost);
	auto end1 = std::chrono::high_resolution_clock::now();
	cudaRadixTimeAll = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

	cudaFree(dBufferGlobal[0]);
	cudaFree(dBufferGlobal[1]);
	cudaFree(dLocalPrefixSum);
	cudaFree(dBlockPrefixSum);

}

void testRadixSort() {
	hData = new u32[TOTAL_SIZE];
	hResult = new u32[TOTAL_SIZE];
	for (i32 i = 0; i < TOTAL_SIZE; i++) {
		hData[i] = (rand()%32767)*(rand()%65536);
	}

	// CUDA Radix Sort
	radixSort();
	FILE* fp = fopen(SAVE_RESULT, "w");
	for (i32 i = 0; i < TOTAL_SIZE; i++) {
		fprintf(fp, "%d ", hResult[i]);
		if(i%64==63){
			fprintf(fp, "\n");
		}
	}

	// Std::sort
	hDataCopy = new u32[TOTAL_SIZE];
	memcpy(hDataCopy, hData, TOTAL_SIZE * sizeof(u32));
	auto start = std::chrono::high_resolution_clock::now();
	std::sort(hDataCopy, hDataCopy + TOTAL_SIZE);
	auto end = std::chrono::high_resolution_clock::now();
	stdSortTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	// CPU Radix Sort
	CPURadix cpuRadix;
	memcpy(hDataCopy, hData, TOTAL_SIZE * sizeof(u32));
	start = std::chrono::high_resolution_clock::now();
	cpuRadix.radixSort(hDataCopy, TOTAL_SIZE);
	end = std::chrono::high_resolution_clock::now();
	cpuRadixTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();


	// Result
	ANTH_LOGI("CUDA Radix Sort Time (Kernel):", cudaRadixTime / 1000.0, " ms");
	ANTH_LOGI("CUDA Radix Sort Time (Total) :", cudaRadixTimeAll / 1000.0, " ms");
	ANTH_LOGI("Std::sort Time               :", stdSortTime / 1000.0, " ms");
	ANTH_LOGI("CPU Radix Sort Time          :", cpuRadixTime / 1000.0, " ms");

	delete[] hData;
	delete[] hResult;
	delete[] hDataCopy;

}

int main() {
	for (i32 i = 0; i < 5; i++) {
		ANTH_LOGI("Test ",i);
		testRadixSort();
	}
	cudaDeviceReset();
	return 0;
}