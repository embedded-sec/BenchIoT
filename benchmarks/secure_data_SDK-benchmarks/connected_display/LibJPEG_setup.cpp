//=================================================================================================
//
// This file has the implementationfor the benchmark itself. The main file only calls the 
// benchmark main function (where the benchmark main function is named after each benchmark).
//
//=================================================================================================



//=========================================== INCLUDES ==========================================//

#include "LibJPEG_setup.h"

//======================================== DEFINES & GLOBALS ====================================//



//========================================= FUNCTIONS ===========================================//

ssize_t jf_read_file (JFILE  *file, uint8_t *buf, uint32_t sizeofbuf)
{
    static ssize_t bytes ;
    f_read (file, buf , sizeofbuf, (UINT*)&bytes); 
    return bytes; 
}

ssize_t jf_write_file (JFILE  *file, uint8_t *buf, uint32_t sizeofbuf)
{
    static ssize_t bytes ;  
    f_write (file, buf , sizeofbuf, (UINT*)&bytes); 
    return bytes;
}