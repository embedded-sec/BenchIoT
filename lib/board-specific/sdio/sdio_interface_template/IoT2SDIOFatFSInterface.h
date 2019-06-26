/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT file system module  R0.12c                             /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2017, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:

/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/----------------------------------------------------------------------------*/



//=================================================================================================
//
// This is a class to be used as an interface for IoT2 to utilize SDIO with FATFS.
//
//=================================================================================================


#ifndef IOT2_SDIO_FATFS_INTERFACE_H
#define IOT2_SDIO_FATFS_INTERFACE_H



//=========================================== INCLUDES ==========================================//

// mbed files
#include "mbed.h"


//======================================== DEFINES & GLOBALS ====================================//


//========================================= FUNCTIONS ===========================================//
    
class IoT2SDIOFatFSInterface
{
public:
	/// Constructor
	IoT2SDIOFatFSInterface();

	/// Destructor
	~IoT2SDIOFatFSInterface();

	/// @brief Initializes the sdio interface
	/// @param None
	/// @retval None
	void init(char *path);

	/// @brief DeInitialize the sdio interface
	/// @param None
	/// @retval None
	void deInit(char *path);

	/// @brief Mount the file system to FATFS module
	/// @param FIL object
	/// @param Logical driver number (Path)
	/// @retval File function return code to indicate success or an error (i.e. FRESULT).
	void mount(FATFS* fs,  const TCHAR* path);

	/// @brief Create (format) a FATFS
	/// @param Logical driver number (Path)
	/// @param Format option
	/// @param Size of allocation unit (cluster) [byte]
	/// @param Pointer to working buffer
	/// @param Size of working buffer
	/// @retval File function return code to indicate success or an error (i.e. FRESULT).
	void mkfs(const TCHAR* path, BYTE opt, DWORD au, void* work, UINT len);


	/// @brief Open file
	/// @param Pointer to FIL object
	/// @param Pointer to the file path
	/// @param File access permissions and flags
	/// @retval File function return code to indicate success or an error (i.e. FRESULT).
	void open(FIL* fp, const TCHAR* path, BYTE mode);

	/// @brief Close file
	/// @retval File function return code to indicate success or an error (i.e. FRESULT).
	void close(FIL* fp);

	/// @brief Wrote to a file
	/// @param Pointer to FIL object
	/// @param Pointer to the data buffer to write
	/// @param Size of bytes to write
	/// @param Pointer to the size of byte to write
	/// @retval File function return code to indicate success or an error (i.e. FRESULT).
	void write(FIL* fp, const void* buff, UINT btw, UINT* bw);

	/// @brief Read from a file
	/// @param Pointer to FIL object
	/// @param Pointer to the data buffer to read
	/// @param Size of bytes to read
	/// @param Pointer to the size of byte to read
	/// @retval File function return code to indicate success or an error (i.e. FRESULT).
	void read(FIL* fp, void* buff, UINT btr, UINT* br);

	/// @brief Function to catch errors. Implemented as a simple infinite loop
	/// @param None
	/// @retval None
	void error();

	/// FATFS functions result variable
	FRESULT res;

};

//================================================== FUNCTIONS ==================================================//




#endif	// IOT2_SDIO_FATFS_INTERFACE_H //