
#include "Config.hpp"
#include <tao/config.hpp>

int check_gonfig(ConfigClass* config)
{
	if (config->num_symbols_find >= 52)
	{
		std::cerr << "Error parse config file, num_bytes_find = " << config->num_symbols_find << std::endl;
		return -1;
	}
	if (config->priv_key.size() != 52)
	{
		std::cerr << "Error parse config file, priv_key.size() not 52 = " << config->priv_key.size() << ", priv_key: " << config->priv_key << std::endl;
		return -1;
	}
	if (config->address[0] == '1' && config->address.size() != 34)
	{
		std::cerr << "Error parse config file, address.size() not 34 = " << config->address.size() << ", address: " << config->address << std::endl;
		return -1;
	}
	else if (config->address[0] == 'b' && config->address.size() != 42)
	{
		std::cerr << "Error parse config file, address.size() not 42 = " << config->address.size() << ", address: " << config->address << std::endl;
		return -1;
	}




	return 0;
}


int parse_gonfig(ConfigClass* config, std::string path)
{
	try {
		const tao::config::value v = tao::config::from_file(path);
		config->cuda_grid = access(v, tao::config::key("cuda_grid")).get_unsigned();
		config->cuda_block = access(v, tao::config::key("cuda_block")).get_unsigned();
		config->num_symbols_find = access(v, tao::config::key("num_symbols_find")).get_unsigned();
		config->priv_key = access(v, tao::config::key("priv_key")).get_string();
		config->address = access(v, tao::config::key("address")).get_string();

		if (check_gonfig(config))
		{
			std::cerr << "Error parse config file, check_gonfig()" << std::endl;
			throw;
		}
		if (config->address[0] == '1')
		{
			config->is_legacy_address = true;
		}
		else if (config->address[0] == 'b')
		{
			config->is_legacy_address = false;
		}

	}
	catch (std::runtime_error& e) {
		std::cerr << "Error parse config file " << path << " : " << e.what() << '\n';
		throw;
	}
	catch (...) {
		std::cerr << "Error parse config file, unknown exception occured" << std::endl;
		throw;
	}
	return 0;
}


