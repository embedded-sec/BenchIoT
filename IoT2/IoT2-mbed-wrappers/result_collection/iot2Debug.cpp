//=================================================================================================
//
// This file is used to print error messages when debugging is enabled
//
//=================================================================================================



//=========================================== INCLUDES ==========================================//

#include "mbed.h"
#include "iot2Debug.h"
//======================================== DEFINES & GLOBALS ====================================//



//========================================= FUNCTIONS ===========================================//

void iot2Error(void){
    
    // error led
    DigitalOut error_led(IoT2_ERROR_LED);
    while(1){
        error_led = !error_led;
        wait(1.0);
    }
}


// if iot2 debugging is enabled
#if IoT2_DEBUG == 1
// serial for debugging
Serial debug_serial(USBTX, USBRX, NULL, 9600);
void iot2SerialDebugMsg(const char* msg){


    // print error msg
    debug_serial.printf("[DEBUG] %s\r\n", msg);
}


#else   // IOT2_DEBUG == 0 //

// If iot2 debugging is disabled
void iot2SerialDebugMsg(const char* msg){
    // debugging is disabled, so this is an empty function
}
    

#endif // IOT2_RESULTS_COLLECTOR //
