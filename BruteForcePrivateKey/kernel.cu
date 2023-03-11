
#include <stdafx.h>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
//#include <synchapi.h>

#include <Dispatcher.h>
#include <thread>

int main()
{
    
    Generate_Mnemonic_And_Hash();


    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(100));
    }

    return 0;
}

