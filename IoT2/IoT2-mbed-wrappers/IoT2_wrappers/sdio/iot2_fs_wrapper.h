//=================================================================================================
//
// This file is used to unify the API of SDIO and SPI for a file system on uSD card.
//
//=================================================================================================


#ifndef IOT2_FS_WRAPPER_H
#define IOT2_FS_WRAPPER_H


//=========================================== INCLUDES ==========================================//

// mbed files
#include "mbed.h"

// IoT2 files
#include "IoT2_Config.h"
#include "iot2Debug.h"

// board specific files
// Class header file
#include "IoT2SDIOFatFSInterface.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

// FATFS lib
#include "ff.h"
//======================================== DEFINES & GLOBALS ====================================//

// Use IOT2FILE to use a consistent API for either SPI/SDIO
typedef FIL IOT2FILE;



//========================================= FUNCTIONS ===========================================//


void iot2fs_init(void);

void iot2fs_fmount(void);

IOT2FILE* iot2fs_fopen(IOT2FILE* fd, const char* file_name, const char* ap);

ssize_t iot2fs_fwrite(const void* buff, size_t size, size_t count, IOT2FILE* fd);

ssize_t iot2fs_fread(void* buff, size_t size, size_t count, IOT2FILE* fd);

int iot2fs_fseek(IOT2FILE* fd, signed long offset, int origin);

signed long iot2fs_ftell(IOT2FILE* fd);

signed long iot2fs_fsize(IOT2FILE* fd);

int iot2fs_fclose(IOT2FILE* fd);

int iot2fs_remove(const char* pathname);

void iot2fs_deinit(void);


#endif  // IOT2_FS_WRAPPER_H //