#include <stdafx.h>

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



#include "Dispatcher.h"
#include "GPU.h"
#include "KernelStride.hpp"
#include "Helper.h"


#include "cuda_runtime.h"
#include "device_launch_parameters.h"


#include "../Tools/tools.h"
#include "../Tools/utils.h"
#include "../config/Config.hpp"
#include "../Tools/segwit_addr.h"


const int8_t mapBase58[256] = {
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
	-1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
	22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
	-1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
	47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
};

int Generate_Mnemonic_And_Hash(void)
{
	cudaError_t cudaStatus = cudaSuccess;
	bool find = false;
	size_t round = 0;

	ConfigClass config;
	try {
		parse_gonfig(&config, "config.cfg");
	}
	catch (...) {
		return -1;
	}

	std::string Test = "511111111111111111111111111111111111111111111111111";


	Devices_Info();
	// Choose which GPU to run on, change this on a multi-GPU system.
	uint32_t num_device = 0;
#ifndef GENERATE_INFINITY
	std::cout << "\n\nEnter number of device: ";
	std::cin >> num_device;
#endif //GENERATE_INFINITY
	cudaStatus = cudaSetDevice(num_device);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		return -1;
	}

	size_t num_wallets_gpu = config.cuda_grid * config.cuda_block;


	std::cout << "\nNUM WALLETS IN PACKET GPU: " << tools::formatWithCommas(num_wallets_gpu) << std::endl << std::endl;
	std::cout << "Seach in pivate key: \"" << config.priv_key << "\"" << std::endl;
	std::cout << "Looking for an address: \"" << config.address << "\"" << std::endl;
	std::string hash160hex;
	data_class *Data = new data_class();
	stride_class* Stride = new stride_class(Data);

	if (config.is_legacy_address)
	{
		tools::decodeAddressBase58(config.address, hash160hex);
		tools::decodeAddressBase58(config.address, (uint8_t*)Data->user_hash160);
	}
	else
	{
		tools::decodeAddressBase32(config.address, hash160hex);
		tools::decodeAddressBase32(config.address, (uint8_t*)Data->user_hash160);
	}
	std::cout << "HASH160: \"" << hash160hex << "\"" << std::endl;


	if (Data->Malloc(config.cuda_grid, config.cuda_block, (uint32_t)config.num_symbols_find) != 0) {
		std::cout << "Error Board->Malloc()!" << std::endl;
		goto Error;
	}
	Data->wif_key = config.priv_key;
	for (int i = 0; i < Data->wif_key.length(); i++)
	{
		Data->user_private_key_wif[i] = mapBase58[Data->wif_key[i]];
	}

	std::cout << "START GENERATE ADDRESSES!" << std::endl;


	if (cudaMemcpyToSymbol(num_keys_in_packet, &Data->num_keys_round, sizeof(uint32_t), 0, cudaMemcpyHostToDevice) != cudaSuccess)
	{
		fprintf(stderr, "cudaMemcpyToSymbol to num_keys_in_packet failed!");
		goto Error;
	}
	if (cudaMemcpyToSymbol(num_sumbols, &Data->num_symbols_find, sizeof(uint32_t), 0, cudaMemcpyHostToDevice) != cudaSuccess)
	{
		fprintf(stderr, "cudaMemcpyToSymbol to num_sumbols failed!");
		goto Error;
	}
	if (cudaMemcpyToSymbol(wif_const, Data->user_private_key_wif, 52, 0, cudaMemcpyHostToDevice) != cudaSuccess)
	{
		fprintf(stderr, "cudaMemcpyToSymbol to wif_const failed!");
		goto Error;
	}
	if (cudaMemcpyToSymbol(hash_addr, Data->user_hash160, sizeof(uint32_t) * 5, 0, cudaMemcpyHostToDevice) != cudaSuccess)
	{
		fprintf(stderr, "cudaMemcpyToSymbol to hash_addr failed!");
		goto Error;
	}

	while (!find)
	{
		tools::start_time();
		uint8_t random_bytes[52] = { 0 };
		tools::Generate_Bytes_For_PrivKey_Wif_Rundom(Data->num_symbols_find, random_bytes);
		if (cudaMemcpyToSymbolAsync(random_seed, random_bytes, 52, 0, cudaMemcpyHostToDevice, Data->stream1) != cudaSuccess)
		{
			fprintf(stderr, "cudaMemcpyToSymbol to rundom_seed failed!");
			goto Error;
		}

		if (Stride->start(config.cuda_grid, config.cuda_block) != 0) {
			printf("Error START!!\n");
			goto Error;
		}

		if (Stride->end() != 0) {
			printf("Error END!!\n");
			goto Error;
		}

		if (Data->host.ret->found == 1)
		{
			std::string key_hex = tools::bytesToHexString(Data->host.ret->priv_key, SIZE_PRIVKEY_BYTES);

			tools::Add_Find_Seed_In_File(key_hex, config.address.c_str());
			std::cout << "\n!!!FOUND!!!\n!!!FOUND!!!\n!!!FOUND!!!\n!!!FOUND!!!\n";
			std::cout << "!!!FOUND KEY " << key_hex << " FOR ADDRESS " << config.address << std::endl;
			std::cout << "!!!FOUND!!!\n!!!FOUND!!!\n!!!FOUND!!!\n!!!FOUND!!!\n";
			break;
		}
		float delay;
		tools::stop_time_and_calc(&delay);
		round++;
		std::cout << "\rSPEED: " << std::setw(8) << std::fixed << tools::formatWithCommas((float)(Data->num_keys_round) / (delay / 1000.0f)) << " KEYS PER SECOND, ROUND: " << round;
	}


	std::cout << "" << std::endl;
	std::cout << "\n\nEND!" << std::endl;

	// cudaDeviceReset must be called before exiting in order for profiling and
	// tracing tools such as Nsight and Visual Profiler to show complete traces.
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
		return -1;
	}

	return 0;
Error:
	std::cout << "\n\nERROR!" << std::endl;
	// cudaDeviceReset must be called before exiting in order for profiling and
	// tracing tools such as Nsight and Visual Profiler to show complete traces.
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
		return -1;
	}

	return -1;
}







