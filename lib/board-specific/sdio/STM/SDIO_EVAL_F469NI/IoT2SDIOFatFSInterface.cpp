/**
  *
  *  Portions COPYRIGHT 2017 STMicroelectronics
  *  Copyright (C) 2017, ChaN, all right reserved.
  *
  ******************************************************************************
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/*
* This is a class to be used for the STM32479I-EVAL board to utilize SDIO with
* FATFS.
*/


// Check if the target is the evaluation board, otherwise through an error
#if !defined(TARGET_STM32F469NI)
#error "[-] This library is written for the STM32479I-EVAL board. Please add support to your board" \
		" or modify this error handling code if the support has been added already."
#endif	// TARGET_STM32F469NI //

//=========================================== INCLUDES ==========================================//

// Class header file
#include "IoT2SDIOFatFSInterface.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"


//======================================== DEFINES & GLOBALS ====================================//

extern const Diskio_drvTypeDef  SD_Driver;

//========================================= FUNCTIONS ===========================================//
/// Constructor
IoT2SDIOFatFSInterface::IoT2SDIOFatFSInterface(){

}

/// Destructor
IoT2SDIOFatFSInterface::~IoT2SDIOFatFSInterface(){

}

/// @brief Initializes the sdio interface
/// @param None
/// @retval None
void IoT2SDIOFatFSInterface::init(char *path){

	// Link FATFS driver 
	if(FATFS_LinkDriver(&SD_Driver, path) != 0){
		// Call our error function. Implemented this way to avoid any change of the error
		// handler because of the vendor
		this->error();	
	}
}

/// @brief DeInitialize the sdio interface
/// @param None
/// @retval None
void IoT2SDIOFatFSInterface::deInit(char *path){
	FATFS_UnLinkDriver(path);
}

/// @brief Mount the file system to FATFS module
/// @param FIL object
/// @param Logical driver number (Path)
/// @retval File function return code to indicate success or an error (i.e. FRESULT).
void IoT2SDIOFatFSInterface::mount(FATFS* fs,  const TCHAR* path){
	if(f_mount(fs, path, 0) != FR_OK){
		this->error();
	}
}

/// @brief Create (format) a FATFS
/// @param Logical driver number (Path)
/// @param Format option
/// @param Size of allocation unit (cluster) [byte]
/// @param Pointer to working buffer
/// @param Size of working buffer
/// @retval File function return code to indicate success or an error (i.e. FRESULT).
void IoT2SDIOFatFSInterface::mkfs(const TCHAR* path, BYTE opt, DWORD au, void* work, UINT len){

	if(f_mkfs(path, opt, 0, work, len) != FR_OK){
		// Call our error function. Implemented this way to avoid any change of the error
		// handler because of the vendor
		this->error();
	}
}

/// @brief Open file
/// @param Pointer to FIL object
/// @param Pointer to the file path
/// @param File access permissions and flags
/// @retval File function return code to indicate success or an error (i.e. FRESULT).
void IoT2SDIOFatFSInterface::open(FIL* fp, const TCHAR* path, BYTE mode){
	
	if(f_open(fp, path, mode) != FR_OK){
		// Call our error function. Implemented this way to avoid any change of the error
		// handler because of the vendor
		this->error();
	}
}

/// @brief Close file
/// @retval File function return code to indicate success or an error (i.e. FRESULT).
void IoT2SDIOFatFSInterface::close(FIL* fp){

	// Make sure file closed correctly
	if (f_close(fp) != FR_OK){
		// Call our error function. Implemented this way to avoid any change of the error
		// handler because of the vendor
		this->error();
	}
}

/// @brief Wrote to a file
/// @param Pointer to FIL object
/// @param Pointer to the data buffer to write
/// @param Size of bytes to write
/// @param Pointer to the size of byte to write
/// @retval File function return code to indicate success or an error (i.e. FRESULT).
void IoT2SDIOFatFSInterface::write(FIL* fp, const void* buff, UINT btw, UINT* bw){

	 res = f_write(fp, buff, btw, bw);

	 // Check the return value & the size of the written bytes
	 if (!(btw) ||  (res != FR_OK)){
	 	// Call our error function. Implemented this way to avoid any change of the error
		// handler because of the vendor
		this->error();
	 }
}

/// @brief Read from a file
/// @param Pointer to FIL object
/// @param Pointer to the data buffer to read
/// @param Size of bytes to read
/// @param Pointer to the size of byte to read
/// @retval File function return code to indicate success or an error (i.e. FRESULT).
void IoT2SDIOFatFSInterface::read(FIL* fp, void* buff, UINT btr, UINT* br){
	
	res = f_read(fp, buff, btr, br);

	// check the return value & the size of bytes read
	if (!(btr) || (res != FR_OK)){
		// Call our error function. Implemented this way to avoid any change of the error
		// handler because of the vendor
		this->error();
	}
}

/// @brief Function to catch errors. Implemented as a simple infinite loop
/// @param None
/// @retval None
void IoT2SDIOFatFSInterface::error(){

	while(1){
		// An error occured! Loop inifinitely
	};
}




