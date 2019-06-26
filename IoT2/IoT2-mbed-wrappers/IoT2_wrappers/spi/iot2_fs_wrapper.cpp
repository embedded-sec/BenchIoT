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

SDBlockDevice sd(SPI_MOSI, SPI_MISO, SPI_CLK, SPI_CS);
FATFileSystem fs("sd", &sd);

//========================================= FUNCTIONS ===========================================//


void iot2fs_init(void){
    // empty since it is already initilized through globals
}

void iot2fs_fmount(void){
    // empty since it is already initilized through globals
}

IOT2FILE* iot2fs_fopen(IOT2FILE* fd, const char* file_name, const char* ap){

    char sd_filename[32] = {'\0'};
    if(snprintf(sd_filename, 31, "/sd/%s", file_name) < 0){
        iot2SerialDebugMsg("[-] ERROR: Cannot format filename correctly");
        iot2Error();
    }

    return fopen(sd_filename, ap);
  
}


ssize_t iot2fs_fwrite(const void* buff, size_t size, size_t count, IOT2FILE* fd){

    return fwrite(buff, size, count, fd);
}


ssize_t iot2fs_fread(void* buff, size_t size, size_t count, IOT2FILE* fd){

    return fread(buff, size, count, fd);
}


int iot2fs_fseek(IOT2FILE* fd, signed long offset, int origin){

    return fseek(fd, offset, origin);
}


signed long iot2fs_ftell(IOT2FILE* fd){
    return ftell(fd);
}


signed long iot2fs_fsize(IOT2FILE* fd){
    fseek(fd, 0, SEEK_END);
    signed long size_fd = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    return size_fd;
}


int iot2fs_fclose(IOT2FILE* fd){
    
    if (fclose(fd) != 0){
        iot2SerialDebugMsg("[-] ERROR: Cannot close file");
        iot2Error();
    }

    return 0;
}

int iot2fs_remove(const char* pathname){
    char sd_filename[32] = {'\0'};
    if(snprintf(sd_filename, 31, "/sd/%s", pathname) < 0){
        iot2SerialDebugMsg("[-] ERROR: Cannot format filename correctly");
        iot2Error();
    }
    if (remove(sd_filename) != 0){
        iot2SerialDebugMsg("[-] ERROR: Cannot delete file file");
        iot2Error();
    }

    return 0;
}

void iot2fs_deinit(void){
    // empty function
}

void iot2fs_ferror(IOT2FILE *fd){
    if (fd == NULL){
        iot2SerialDebugMsg("[-] ERROR in opening/closing file");
        iot2Error();
    }
}