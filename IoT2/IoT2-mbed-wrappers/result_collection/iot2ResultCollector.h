//=================================================================================================
//
// This file is used to send the metric results after the benchmark has finished.
//
//=================================================================================================


#ifndef IOT2_RESULT_COLLECTOR_H
#define IOT2_RESULT_COLLECTOR_H


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


void iot2SendResults(void);


#endif  // IOT2_RESULT_COLLECTOR //