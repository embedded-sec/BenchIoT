//=================================================================================================
//
// This file has the implementationfor the benchmark itself. The main file only calls the 
// benchmark main function (where the benchmark main function is named after each benchmark).
//
//=================================================================================================



//=========================================== INCLUDES ==========================================//

#include "mbed.h"

#include "iot2_fs_wrapper.h"

//======================================== DEFINES & GLOBALS ====================================//

FATFS usd_fatfs;
char usd_root_path[4];
IoT2SDIOFatFSInterface sdio;

//========================================= FUNCTIONS ===========================================//


void iot2fs_init(void){
    sdio.init(usd_root_path);
}

void iot2fs_fmount(void){
    sdio.mount(&usd_fatfs, (TCHAR const*)usd_root_path);
}

IOT2FILE* iot2fs_fopen(IOT2FILE* fd, const char* file_name, const char* ap){

    BYTE permissions = 0;
    // check which permission are given (e.g., "w", "r+", "a"...etc)
    // permission are mapped to CHAN fatfs from here
    // http://elm-chan.org/fsw/ff/doc/open.html
    if (strlen(ap)==1){
        if (strncmp(ap, "r", 1) == 0){
            permissions = FA_READ;
        }
        else if(strncmp(ap, "w", 1) == 0){
            permissions = FA_CREATE_ALWAYS | FA_WRITE | FA_OPEN_APPEND;
        }
        else if(strncmp(ap, "a", 1) == 0){
            permissions = FA_OPEN_APPEND | FA_WRITE;
        }
        // Non-valid permisison, through an error
        else{
            iot2Error();
        }

    }
    else if(strlen(ap)==2){
        if(strncmp(ap, "r+", 2) == 0){
            permissions = FA_READ | FA_WRITE;
        }
        else if (strncmp(ap, "w+", 2) == 0){
            permissions = FA_CREATE_ALWAYS | FA_WRITE | FA_READ;
        }
        else if (strncmp(ap, "a+", 2) == 0){
            permissions = FA_OPEN_APPEND | FA_WRITE | FA_READ;
        }
        else if(strncmp(ap, "wx", 2) == 0){
            permissions = FA_CREATE_NEW | FA_WRITE;
        }
        // Invlid permission, through an error
        else{
            iot2Error();
        }
    }

    else if(strlen(ap) == 3){
        if(strncmp(ap, "w+x", 3) == 0){
            permissions = FA_CREATE_NEW | FA_WRITE | FA_READ;
        }
        // invalid permission, through an error
        else{
            iot2Error();
        }
    }
    // permissions are either 1 or 2 in length, so trigger an error
    else{
        iot2Error();
    }

    sdio.open(fd, (TCHAR const*)file_name, permissions);

    return fd;
}


ssize_t iot2fs_fwrite(const void* buff, size_t size, size_t count, IOT2FILE* fd){

    // for sdio, size is not used
    (void)size;
    ssize_t bytes_written = 0;

    sdio.write(fd, (uint8_t*)buff, count, (UINT*)&bytes_written);

    return bytes_written;
}


ssize_t iot2fs_fread(void* buff, size_t size, size_t count, IOT2FILE* fd){

    // for sdio, size is not used
    (void)size;
    ssize_t bytes_written = 0;

    sdio.read(fd, (uint8_t*)buff, count, (UINT*)&bytes_written);

    return bytes_written;
}


int iot2fs_fseek(IOT2FILE* fd, signed long offset, int origin){

    // for sdio version, origin (i.e., SEEK_*) option is not used
    // and it is assumed that the offset is handled prior to 
    // prior to calling fseek
    if (f_lseek(fd, offset) != FR_OK){
        // an error occured
        return -1;
    }
    return 0;
}


signed long iot2fs_ftell(IOT2FILE* fd){
   signed long offset = fd->fptr;
   return offset; 
}


signed long iot2fs_fsize(IOT2FILE* fd){
    return f_size(fd);
}


int iot2fs_fclose(IOT2FILE* fd){
    sdio.close(fd);
    return 0;
}

// should return 0 on success, negative otherwise
int iot2fs_remove(const char* pathname){
    return f_unlink(pathname);
}


void iot2fs_deinit(void){
    sdio.deInit(usd_root_path);
}