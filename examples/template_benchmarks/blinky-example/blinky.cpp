/* This is a simple application template, it just blinks LEDs and prints 
 * a message using a serial. The baudrate is mbed's default (i.e. 9600)
*/


#include "mbed.h"
#include "blinky.h"

DigitalOut led1(LED1);

Serial pc(USBTX, USBRX, 9600);
uint8_t num_iterations = 10;


/// used to start the energy measurements, this happens after enabling measuring
/// all metrics
extern DigitalOut energyGPIO;

void benchmarkMain(void){

    
    /// The following sequence is used to start the measurements. In the 
    /// benchmarks this is done after establishing the connection at the end
    /// of iot2InitTCP(...). However, in this simple example we are not using
    /// the network and thus this was added here instead. Collecting
    /// the metrics is done automatically at the main.cpp file.
    //**************************************************************************
    /// Beginning of measurement start sequence.

    // reset counters and enable all metrics
    iot2CallSVC(IOT2_RESERVED_SVC,IOT2_DWT_REST_SETUP_SVC_NUM);
    // enable measuremets
    COLLECT_IOT2_METRICS = 1;

    // initialize priv thread counter
    iot2RecordPrivThread();

    // set the energy gpio to 1 for energy measurement
    energyGPIO = 1;

    /// End of measurement start sequence.
    //**************************************************************************

    for(uint8_t i = 0; i < num_iterations; i++) {
        led1 = !led1;
        wait(0.2);
        pc.printf("Blinky-example application Running!\r\n");
    }

    
}
