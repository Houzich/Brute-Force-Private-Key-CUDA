#include <stdafx.h>
#include <stdio.h>
#include <stdint.h>


#include "KernelStride.hpp"
#include "Helper.h"
#include <GPU.h>
#include "../Tools/utils.h"

int stride_class::privkey_to_hash160(uint64_t grid, uint64_t block) 
{
	gl_privkey_to_hash_search << <(unsigned int)grid, (unsigned int)block, 0, dt->stream1 >> > (dt->dev.ret);
	return 0;
}




int stride_class::memsetGlobal()
{
	if (cudaMemsetAsync(dt->dev.ret, 0, sizeof(retStruct), dt->stream1) != cudaSuccess) { fprintf(stderr, "cudaMemset Board->dev.ret failed!"); return -1; }
	return 0;
}


int stride_class::start(uint64_t grid, uint64_t block)
{
	if (memsetGlobal() != 0) return -1;
	if (privkey_to_hash160(grid, block) != 0) return -1;

	return 0;
}

int stride_class::end()
{
	cudaError_t cudaStatus = cudaSuccess;


	//if (DeviceSynchronize("end") != cudaSuccess) return -1; //
	cudaStatus = cudaMemcpy(dt->host.ret, dt->dev.ret, sizeof(retStruct), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy ret failed!");
		return -1;
	}

	return 0;
}