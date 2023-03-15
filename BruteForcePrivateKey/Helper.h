#pragma once
#include <stdint.h>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <omp.h>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"


class host_buffers_class
{
public:
	retStruct* ret = NULL;
	uint64_t memory_size = 0;
public:
	host_buffers_class()
	{
	}

	static std::string FormatWithCommas(uint64_t value)
	{
		std::stringstream ss;
		ss.imbue(std::locale("en_US.UTF-8"));
		ss << std::fixed << value;
		return ss.str();
	}
	int alignedMalloc(void** point, uint64_t size, uint64_t* all_ram_memory_size, std::string buff_name) {
		*point = _aligned_malloc(size, 4096);
		if (NULL == *point) { fprintf(stderr, "_aligned_malloc (%s) failed! Size: %s", buff_name.c_str(), FormatWithCommas(size).data()); return 1; }
		*all_ram_memory_size += size;
		//std::cout << "MALLOC RAM MEMORY SIZE (" << buff_name << "): " << std::to_string((float)size / (1024.0f * 1024.0f)) << " MB\n";
		return 0;
	}
	int mallocHost(void** point, uint64_t size, uint64_t* all_ram_memory_size, std::string buff_name) {
		if (cudaMallocHost((void**)point, size) != cudaSuccess) {
			fprintf(stderr, "cudaMallocHost (%s) failed! Size: %s", buff_name.c_str(), FormatWithCommas(size).data()); return -1;
		}
		*all_ram_memory_size += size;
		//std::cout << "MALLOC RAM MEMORY SIZE (" << buff_name << "): " << std::to_string((float)size / (1024.0f * 1024.0f)) << " MB\n";
		return 0;
	}
	int Malloc()
	{
		memory_size = 0;
		if (mallocHost((void**)&ret, sizeof(retStruct), &memory_size, "ret") != 0) return -1;
		std::cout << "MALLOC ALL RAM MEMORY SIZE (HOST): " << std::to_string((float)memory_size / (1024.0f * 1024.0f)) << " MB\n";
		return 0;
	}

	~host_buffers_class()
	{
		cudaFreeHost(ret);
	}

};

class device_buffers_class
{
public:
	retStruct* ret = NULL;

	uint64_t memory_size = 0;
public:
	device_buffers_class()
	{
	}

	std::string FormatWithCommas(uint64_t value)
	{
		std::stringstream ss;
		ss.imbue(std::locale(""));
		ss << std::fixed << value;
		return ss.str();
	}
	int cudaMallocDevice(uint8_t** point, uint64_t size, uint64_t* all_gpu_memory_size, std::string buff_name) {
		//cudaError_t cudaStatus = cudaSuccess;
		if (cudaMalloc(point, size) != cudaSuccess) {
			fprintf(stderr, "cudaMalloc (%s) failed! Size: %s", buff_name.c_str(), FormatWithCommas(size).data()); return -1;
		}
		*all_gpu_memory_size += size;
		//std::cout << "MALLOC GPU MEMORY SIZE (" << buff_name << "): " << std::to_string((float)size / (1024.0f * 1024.0f)) << " MB\n";
		return 0;
	}
	int Malloc()
	{
		memory_size = 0;
		if (cudaMallocDevice((uint8_t**)&ret, sizeof(retStruct), &memory_size, "ret") != 0) return -1;
		std::cout << "MALLOC ALL MEMORY SIZE (GPU): " << std::to_string((float)(memory_size) / (1024.0f * 1024.0f)) << " MB\n";
		return 0;
	}
	~device_buffers_class()
	{
		cudaFree(ret);
	}
};


class data_class
{
public:
	device_buffers_class dev;
	host_buffers_class host;

	cudaStream_t stream1 = NULL;
	size_t num_keys_round = 0;

	std::string wif_key = "";
	uint32_t num_symbols_find = 0;
	uint8_t* rundom_bytes;
	uint8_t user_private_key_wif[SIZE_PRIVKEY_WIF] = { 0 };
	uint8_t user_hash160[SIZE_HASH160] = { 0 };
	
public:
	data_class()
	{

	}
	std::string FormatWithCommas(uint64_t value)
	{
		std::stringstream ss;
		ss.imbue(std::locale(""));
		ss << std::fixed << value;
		return ss.str();
	}
	int alignedMalloc8(uint8_t** point, uint64_t size, std::string buff_name) {
		*point = (uint8_t*)_aligned_malloc(size, 4096);
		if (NULL == *point) { fprintf(stderr, "_aligned_malloc (%s) failed! Size: %s", buff_name.c_str(), FormatWithCommas(size).data()); return 1; }
		//std::cout << "MALLOC RAM MEMORY SIZE (" << buff_name << "): " << std::to_string((float)size / (1024.0f * 1024.0f)) << " MB\n";
		return 0;
	}
	int Malloc(size_t cuda_grid, size_t cuda_block, uint32_t num_symbols_find)
	{
		size_t num_wallet = cuda_grid * cuda_block;

		if (cudaStreamCreate(&stream1) != cudaSuccess) { fprintf(stderr, "cudaStreamCreate failed!  stream1"); return -1; }
		if (dev.Malloc() != 0) return -1;
		if (host.Malloc() != 0) return -1;
		this->num_keys_round = num_wallet * 2;
		this->num_symbols_find = num_symbols_find;
		if (alignedMalloc8(&rundom_bytes, this->num_symbols_find, "rundom_bytes") != 0) return -1;
		return 0;
	}
	~data_class()
	{
		cudaStreamDestroy(stream1);
		_aligned_free(rundom_bytes);
	}
};

cudaError_t DeviceSynchronize(std::string name_kernel);
void Devices_Info(void);

