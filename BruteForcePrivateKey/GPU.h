#pragma once

#include <stdint.h>
#include "stdafx.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

__global__ void gl_privkey_to_hash_search(retStruct* __restrict__ ret);
extern __device__ __constant__ uint32_t num_keys_in_packet[1];
extern __device__ __constant__ uint32_t num_sumbols[1];
extern __device__ __constant__ uint8_t random_seed[52];
extern __device__ __constant__ uint8_t wif_const[52];
extern __device__ __constant__ uint32_t hash_addr[5];
