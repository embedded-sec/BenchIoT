//=================================================================================================
//
// This is the header file for the benchmark itself. The main file only calls the benchmark main
// function.
//
//=================================================================================================


#ifndef SMART_LOCKER_H
#define SMART_LOCKER_H


//=========================================== INCLUDES ==========================================//

// mbed files
#include "mbed.h"

// board specific files
#include "benchmark_target.h"

// mbedtls files
#include <stdint.h>
#include <string.h>
#include "mbedtls/sha256.h"
#include "mbedtls/platform.h"

// IoT2 configuration and special interface files
#include "IoT2_Config.h"
#include "iot2TCP.h"
#include "IoT2_DisplayInterface.h"
#include "iot2Debug.h" 


//filesystem wrapper------------------------------------------------------------
#include "iot2_fs_wrapper.h"
//------------------------------------------------------------------------------

#include "PID.h"

//======================================== DEFINES & GLOBALS ====================================//


#define DATASET_SIZE        30
#define NUM_REMOTE_CMDS     120
#define NUM_DEVICE_INPUTS   20
#define THERMOSTAT_LED      IoT2_GENERAL_LED
#define TEMP_ERROR          -1

// smart thermostat cmds
#define SET_TEMP_CMD            "SET_TEMPERATURE "
#define GET_TEMP_CMD            "GET_TEMPERATURE"
#define INVALID_CMD_RESPONSE    "INVALID CMD"
#define SET_TEMP_RESPONSE       "TEMP SET: "
#define GET_TEMP_RESPONSE       "CURRENT TEMP: "
#define SET_TEMP_RESP_LEN       10
#define GET_TEMP_RESP_LEN       14
#define MAX_TEMP                35.0
#define MIN_TEMP                18.0

// board specific files
#if defined(TARGET_STM32F469NI)

#define ADC_PIN         PC_0    /// The pin used to read ADC for temperature

#elif defined(TARGET_K64F)

#define ADC_PIN         PTB2    /// The pin used to read ADC for temperature

#else
#error "[-] ERROR: Please configure ADC_PIN for your board in smart_thermostat.h"

#endif  // TARGET_*

// benchmark datasets
const float thermostat_dataset[DATASET_SIZE] = {
            22,
            24,
            27,
            25,
            18,
            23,
            27,
            18,
            22,
            19,
            25,
            18,
            18,
            24,
            24,
            23,
            20,
            24,
            22,
            23,
            26,
            26,
            18,
            19,
            18,
            27,
            19,
            19,
            25,
            20
                  };


extern Serial pc;
//========================================= FUNCTIONS ===========================================//


void benchmarkMain(void);

void displayMsg(IoT2_DisplayInterface &display, uint8_t* msg);

void readAdcHandler(void);

void logHwAdc(void);

void handleCmd(char* cmd, size_t cmd_size);

int getInputTemp(char* cmd, uint8_t temp_idx, size_t cmd_len);

void setInvalidCmdResponse(char* buff, size_t buff_size);

void setValidResponse(char* buff, size_t buff_size, const char* response_buff);

void cpBuff(char* cmd, size_t cmd_size, const char* buff, size_t buff_len);

// this fuction assumes ascii_buff length is 3 ( 2 chars + '\0')
void getAsciiTemp(char* ascii_buff);

void readUserTemp(void);

#endif  // SMART_LOCKER //