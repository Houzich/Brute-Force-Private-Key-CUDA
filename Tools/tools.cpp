#include "main.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <set>
#include <random>
#include <fstream>
#include <filesystem>

#include "../BruteForcePrivateKey/stdafx.h"
#include "tools.h"
#include "utils.h"
#include "base58.h"
#include "segwit_addr.h"




namespace tools {

	int Generate_Bytes_For_PrivKey_Wif_Rundom(size_t num_symbols, uint8_t* bytes_out)
	{
		static size_t count = 0;
		if (count == 0) srand((unsigned int)time(NULL));
		if ((count % (1024 * 10)) == 0)
		{
			srand((unsigned int)time(NULL));
		}

		for (int i = 0; i < num_symbols; i++)
		{
			bytes_out[i] = rand() % 256;
		}
		count++;
		return 0;
	}

	void Add_Find_Seed_In_File(std::string seed_hexstr, const char* address) {
		std::ofstream out;
		out.open(FILE_PATH_RESULT, std::ios::app);
		if (out.is_open())
		{
			std::time_t result = std::time(nullptr);
			out << seed_hexstr << "," << (const char*)address << "," << std::asctime(std::localtime(&result));
		}
		else
		{
			printf("\n!!!ERROR create file %s!!!\n", FILE_PATH_RESULT);
		}
		out.close();
	}

}
