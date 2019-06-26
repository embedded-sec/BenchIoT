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

// mbedtls files
#include <stdint.h>
#include <string.h>

// IoT2 configuration and special interface files
#include "IoT2_Config.h"
#include "iot2TCP.h"
#include "iot2Debug.h" 


//#sdio-------------------------------------------------------------------------
#include "iot2_fs_wrapper.h"
//#sdio-------------------------------------------------------------------------


//======================================== DEFINES & GLOBALS ====================================//


#define TCP_BUFF_SIZE             64
#define DATASET_SIZE              100
#define NUM_SERVER_INQUIRIES      30
#define NUM_BENCH_RESULTS         2*DATASET_SIZE
#define LIGHT_CONFIG_LENGTH       9
#define NUM_RECV_CONFIG_CMDS      3
#define INVALID_REQ_RESPONSE      "INVALID_REQ"
#define CMD_RESPONSE_HEADER       "CMD_EXECUTED:"

/// smart light commands
#define SET_LIGHT_ON_PERIOD_CMD   "LIGHT_ON_PERIOD "        // LIGHT_ON_PERIOD start,end
#define SET_LIGHT_OFF_PERIOD_CMD  "LIGHT_OFF_PERIOD "       // LIGHT_OFF_PERIOD start,end
#define TURN_LIGHT_ON_CMD         "TURN_LIGHT_ON"
#define TURN_LIGHT_OFF_CMD        "TURN_LIGHT_OFF"
#define KEEP_LIGHT_ON_CMD         "KEEP_LIGHT_ON"

// depending on the targeted hardware, turning na LED can
// be through pullup or pull down. To avoid such inconsistency,
// the below directives can be configured (i.e., 0/1) according
// the the hardware used
#define LED_ON                0
#define LED_OFF               1


extern Serial pc;
//========================================= FUNCTIONS ===========================================//


void benchmarkMain(void);

void checkLightHandler(void);

void motionDetectionHandler(void);

void handleSmartLightCmd(char* cmd, size_t cmd_size);

bool verifyConfigTimeFormat(char *cmd, uint8_t periods_config_idx);

void setPeriodConfig(char *cmd, char *start, char *end, uint8_t periods_config_idx);

void setInvalidReqResponse(char *buff, size_t buff_size);

void setCmdReponseHeader(char *buff, size_t buff_size);

void handleNonConfigCmd(char *buff, size_t buff_size);

void cpyCmdBuff(char *buff, char *cmd_cpy, uint8_t buff_idx, uint8_t cmd_cpy_size);

void applyConfigs(char *time_buff);

bool isLightOnPeriod(char *time_buff);

bool isLightOffTime(char *time_buff);

int32_t convTimeStrToSeconds(char *time_buff);

#endif  // SMART_LIGHT //