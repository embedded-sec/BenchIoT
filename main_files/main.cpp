//=================================================================================================
//
// The main file sets up the iot2 evaluation functions, calls the benchmark main function and then
// collect the result using iot2 API.
//
//=================================================================================================


//=========================================== INCLUDES ==========================================//

#include "benchmark.h"

//======================================== DEFINES & GLOBALS ====================================//

DigitalOut benchLed(IoT2_GREEN_LED, 0);
DigitalOut energyGPIO(IoT2_ENERGY_GPIO, 0);
extern Serial pc;

//============================================= MAIN ============================================//

int main() {
    // setup the MPU / initialize privThread counter
    //iot2CallSVC(4);

    
    //--------------------------------------------------------------------------
    // SETUP SERIAL MESSAGES. DO NOT MODIFY THEM
    //--------------------------------------------------------------------------
#if (IoT2_OS_BENCHMARKS ==1)
    // print welcome message
    pc.printf(" Welcome to IoT2\r\n");
    // notify the serial process of the beginning of the benchmark
    pc.printf("[IoT2] benchmark: START\r\n");
#else
    //iot2SerialDebugMsg("[IoT2] benchmark: START");
    //iot2SerialDebugMsg(" Welcome to IoT2");
    // notify the serial process of the beginning of the benchmark
    pc.printf("[IoT2] benchmark: START\r\n");
    pc.printf(" Welcome to IoT2\r\n");

#endif
    //--------------------------------------------------------------------------
    // END OF SETUP SERIAL MESSAGES.
    //--------------------------------------------------------------------------

    // flip led to signal the begining of the benchmark
    benchLed = 1;
    
    // start the benchmark
    benchmarkMain();

    // set the energy gpio to 0 to end energy measurement
    energyGPIO = 0;
    
    // end IoT2 measurements
    iot2CallSVC(IOT2_RESERVED_SVC, IOT2_END_MEASUREMENT_SVC_NUM);
    
    // flip the led to indicate the end of the benchmark
    benchLed = 0;

    // send the benchmark results to collect it through the serial
    iot2SendResults();

    // end the benchmark
    iot2EndBenchmark();
}
