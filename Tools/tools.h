#pragma once
#include <vector>
#include <string>
#include "../BruteForcePrivateKey/stdafx.h"
namespace tools {

	int Generate_Bytes_For_PrivKey_Wif_Rundom(size_t num_symbols, uint8_t* bytes_out);
	void Add_Find_Seed_In_File(std::string seed_hexstr, const char* address);
}