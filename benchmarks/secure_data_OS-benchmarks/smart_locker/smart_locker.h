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


//#sdio-------------------------------------------------------------------------
#include "iot2_fs_wrapper.h"
//#sdio-------------------------------------------------------------------------


//======================================== DEFINES & GLOBALS ====================================//


#define SHA256_LENGTH        32
#define PIN_LENGTH           5
#define DATASET_SIZE         100
#define NUM_SERVER_INQUIRIES 30
#define LOCKER_LED           IoT2_GENERAL_LED
#define BENCH_RESULT_SIZE    (9)            // result,<bool>\n
#define NUM_BENCH_RESULTS    2*DATASET_SIZE
#define CONTAIN_PACKAGE_CMD  "CONTAIN_PACKAGE_FOR? "
#define INVALID_REQ_RESPONSE "INVALID_REQ\n"
#define NO_PACKAGE_AVAILABLE  "NO_PACKAGE_AVAILABLE\n"

// benchmark datasets
const char* const bench_dataset[DATASET_SIZE] = {
                        "72477",
                        "73619",
                        "62601",
                        "53161",
                        "52939",
                        "30242",
                        "12519",
                        "95497",
                        "38661",
                        "64046",    // 10
                        "76169",
                        "21840",
                        "66964",
                        "80865",
                        "16656",
                        "44057",
                        "30585",
                        "70208",
                        "18093",
                        "42021",
                        "12031",
                        "30093",
                        "61110",
                        "53227",
                        "38563",
                        "69319",
                        "22897",
                        "13316",
                        "35645",
                        "27078",
                        "14227",
                        "97557",
                        "31607",
                        "55553",
                        "96604",
                        "72773",
                        "35875",
                        "15053",
                        "50338",
                        "38535",
                        "89143",
                        "71371",
                        "13583",
                        "33330",
                        "55861",
                        "58052",
                        "21877",
                        "98233",
                        "44700",
                        "17959",
                        "11055",
                        "20515",
                        "78869",
                        "60449",
                        "86559",
                        "68192",
                        "39957",
                        "70526",
                        "96706",
                        "16328",
                        "54421",
                        "68612",
                        "35340",
                        "94658",
                        "24010",
                        "30665",
                        "76868",
                        "25386",
                        "75774",
                        "75675",
                        "13480",
                        "12928",
                        "57608",
                        "45408",
                        "21832",
                        "72342",
                        "78320",
                        "43261",
                        "83535",
                        "30645",
                        "90546",
                        "37906",
                        "27649",
                        "12126",
                        "29528",
                        "56528",
                        "68340",
                        "24171",
                        "82202",
                        "76946",
                        "99735",
                        "15400",
                        "18086",
                        "71771",
                        "30650",
                        "83973",
                        "32527",
                        "20319",
                        "33286",
                        "30604"
                            };

const char* const worng_pins_dataset[DATASET_SIZE] = {
                        "17111",
                        "10081",
                        "22950",
                        "93655",
                        "22036",
                        "74234",
                        "15802",
                        "17615",
                        "40510",
                        "65178",    // 10
                        "23877",
                        "64185",
                        "52221",
                        "45072",
                        "11536",
                        "74360",
                        "21649",
                        "46289",
                        "34313",
                        "26957",
                        "67138",
                        "48769",
                        "68024",
                        "84240",
                        "90736",
                        "93515",
                        "86619",
                        "27191",
                        "48228",
                        "50128",
                        "62111",
                        "50924",
                        "58379",
                        "35516",
                        "66807",
                        "49374",
                        "86828",
                        "28955",
                        "20666",
                        "84991",
                        "30614",
                        "20801",
                        "24656",
                        "73344",
                        "32386",
                        "12222",
                        "78694",
                        "69012",
                        "17737",
                        "38238",
                        "73373",
                        "18884",
                        "32989",
                        "52255",
                        "86933",
                        "32557",
                        "69891",
                        "61999",
                        "73617",
                        "50816",
                        "34576",
                        "43121",
                        "40968",
                        "89092",
                        "45900",
                        "37194",
                        "75903",
                        "92387",
                        "26818",
                        "27924",
                        "34606",
                        "42643",
                        "70247",
                        "89858",
                        "35395",
                        "23527",
                        "87743",
                        "57950",
                        "22903",
                        "66857",
                        "67511",
                        "95906",
                        "84610",
                        "81724",
                        "41846",
                        "14885",
                        "21516",
                        "99774",
                        "80262",
                        "21398",
                        "72311",
                        "41607",
                        "81289",
                        "97812",
                        "58937",
                        "71877",
                        "81765",
                        "67282",
                        "33663",
                        "48311"
                            };

// holds the benchmark result in format <hash, hash_res, result, occupied result>
extern char bench_result[NUM_BENCH_RESULTS][BENCH_RESULT_SIZE];


// constant to be used to format the results in bench_result
const char result_prefix[8] = "result,";

extern Serial pc;
//========================================= FUNCTIONS ===========================================//


void benchmarkMain(void);


void dropPackageHandler(void);


void pickupPackageHandler(void);

bool checkPickupPin(unsigned char* user_pin, uint8_t res_cntr);

void storeBenchResult(bool dataset_result, uint8_t res_cntr);

void displayMsg(IoT2_DisplayInterface &display, uint8_t* msg);

bool isPinHash(unsigned char *entered_pin, uint8_t locker_idx);

void handleSmartLockerReq(char* req, size_t req_size);

#endif  // SMART_LOCKER //