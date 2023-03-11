#pragma once
#include <string>



struct ConfigClass
{
public:
	uint64_t cuda_grid = 0;
	uint64_t cuda_block = 0;
	uint64_t num_symbols_find = 0;
	std::string priv_key = "";
	std::string address = "";
	bool is_legacy_address = true;
public:
	ConfigClass()
	{
	}
	~ConfigClass()
	{
	}
};


int parse_gonfig(ConfigClass* config, std::string path);
