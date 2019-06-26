//=================================================================================================
//
// This is the header file for the benchmark itself. The main file only calls the benchmark main
// function (where the benchmark main function is named after each benchmark).
//
//=================================================================================================


#ifndef CONNECTED_DISPLAY_H
#define CONNECTED_DISPLAY_H


//=========================================== INCLUDES ==========================================//

// mbed files
#include "mbed.h"

// board specific files
#include "benchmark_target.h"
#include "IoT2_Config.h"
#include "IoT2_DisplayInterface.h"
#include "iot2_fs_wrapper.h"
#include "iot2TCP.h"

// libjpeg files
#include <stdint.h>
#include <string.h>
#include "jpeglib.h"
#include "LibJPEG_setup.h"

//======================================== DEFINES & GLOBALS ====================================//

// cmds
#define PUT_FILE_CMD       "put"
#define GET_FILE_CMD       "get"
#define LOGIN_CMD          "login"
#define PASS_CMD           "pass"
#define INVALID_CMD        "INVALID"

#define IMAGE_HEIGHT        256
#define IMAGE_WIDTH         256//320
#define IMAGE_QUALITY       90
#define IMAGE_COMPONENTS    3   // 3 for RGB
#define DISPLAY_CENTER_YPOS 430
#define TOTAL_IMGS          20
#define SWAP_RB


//========================================= FUNCTIONS ===========================================//


void benchmarkMain(void);

void encodeBmpToJpeg(JFILE *bmpFd, JFILE *jpegFd, uint16_t imgQuality, uint16_t imgW,
                     uint16_t imgH, uint8_t imgComp);

void decodeJpegToBMP(JFILE *jpegFd, JFILE *bmpFd, uint16_t imgQuality, uint16_t imgW, 
                     uint16_t imgH, uint8_t imgComp, IoT2_DisplayInterface *disp);


void setupImageName(char* imgFullPath, const char* imgName, const char* imgExt, 
                    size_t imgFullPathSize , size_t imgNameSize, size_t imgExtSize, uint8_t index);

void handleConnDispCmd(char* cmd, size_t cmd_size, TCPSocket *socket);

void handleInvalidCmd(char *buff, size_t buff_size, TCPSocket *socket);

void handlePutCmd(char *buff, size_t buff_size, TCPSocket *socket);

int32_t parseFileSize(char *buff, size_t buff_size, uint8_t start_idx);

void handleInvalidCmd(char *buff, size_t buff_size, TCPSocket *socket);

void recvImgFile(TCPSocket *socket, char *filename_buff, size_t filename_buff_size,
               int32_t file_size, char* buff, size_t buff_size);


void sendImgFile(TCPSocket *socket, char *filename_buff, size_t filename_buff_size,
                char* buff, size_t buff_size);

#endif  // CONNECTED_DISPLAY_H //