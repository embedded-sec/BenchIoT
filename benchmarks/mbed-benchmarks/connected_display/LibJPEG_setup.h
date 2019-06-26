//=================================================================================================
//
// This header file is used to remap LibJPEG macros in order to work with mbed-SDK/mbed-OS
//
//=================================================================================================


#ifndef LIBJPEG_SETUP_H
#define LIBJPEG_SETUP_H


//=========================================== INCLUDES ==========================================//

#include "ff.h"


//======================================== DEFINES & GLOBALS ====================================//

/// LibJpeg is a C only library, thus we need to map directly to ChAN fat filesystem
/// library to avoid C++ name mangling.
#define JFILE     FIL
#define JMALLOC   malloc    
#define JFREE     free  

//========================================= FUNCTIONS ===========================================//


ssize_t jf_read_file (JFILE  *file, uint8_t *buf, uint32_t sizeofbuf);
ssize_t jf_write_file (JFILE  *file, uint8_t *buf, uint32_t sizeofbuf);

#define JFREAD(file,buf,sizeofbuf)  \
   jf_read_file (file,buf,sizeofbuf)

#define JFWRITE(file,buf,sizeofbuf)  \
   jf_write_file (file,buf,sizeofbuf)


#endif  // LIBJPEG_SETUP_H //