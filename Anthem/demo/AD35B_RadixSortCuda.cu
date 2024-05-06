#include "../include/core/base/AnthemDefs.h"
#include <cstdio>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"


__global__ void helloWorld() {
	printf("%s\n", "Hello World,CUDA");
}

int main() {
	helloWorld AT_KARG2(2,2) ();
	printf("%s\n", "Hello World,x86");
	cudaDeviceReset();
	return 0;
}