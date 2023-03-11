#pragma once
#include "stdafx.h"
#include <stdint.h>


#include "Helper.h"

class stride_class
{
public:
	data_class* dt;
public:

	stride_class(data_class* data)
	{
		dt = data;
	}

private:

public:

	int privkey_to_hash160(uint64_t grid, uint64_t block);
	int memsetGlobal();
	int start(uint64_t grid, uint64_t block);
	int end();
};

