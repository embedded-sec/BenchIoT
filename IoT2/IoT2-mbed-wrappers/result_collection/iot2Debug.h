//=================================================================================================
//
// This file is used to send the metric results after the benchmark has finished.
//
//=================================================================================================


#ifndef IOT2_DEBUG_H
#define IOT2_DEBUG_H


//=========================================== INCLUDES ==========================================//

// IoT2 configuration and special interface files
#include "IoT2_Config.h"

// mbed files
#include "mbed.h"

// stdlib
#include <stdint.h>
#include <string.h>
#include <inttypes.h>


//======================================== DEFINES & GLOBALS ====================================//



//========================================= FUNCTIONS ===========================================//

void iot2Error(void);

void iot2SerialDebugMsg(const char* msg);


#endif  // IOT2_DEBUG_H //