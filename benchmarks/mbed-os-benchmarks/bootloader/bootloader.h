//=================================================================================================
//
// This is the header file for the benchmark itself. The main file only calls the benchmark main
// function.
//
//=================================================================================================


#ifndef SMART_LIGHT_H
#define SMART_LIGHT_H


//=========================================== INCLUDES ==========================================//

// mbed files
#include "mbed.h"

// board specific files
#include "benchmark_target.h"

// // mbedtls files
// #include <stdint.h>
// #include <string.h>

// IoT2 configuration and special interface files
#include "IoT2_Config.h"
#include "iot2TCP.h"
#include "iot2Debug.h"




//======================================== DEFINES & GLOBALS ====================================//


extern Serial pc;
//========================================= FUNCTIONS ===========================================//


void benchmarkMain(void);



#endif  // SMART_LIGHT //
