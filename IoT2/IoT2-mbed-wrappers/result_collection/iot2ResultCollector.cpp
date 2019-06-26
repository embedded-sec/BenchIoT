//=================================================================================================
//
// This file is used to send the metric results after the benchmark has finished.
//
//=================================================================================================



//=========================================== INCLUDES ==========================================//

#include "mbed.h"
#include "iot2ResultCollector.h"

//======================================== DEFINES & GLOBALS ====================================//




//messages to use for starting/ending result collection
const char start_collector_msg[] = "collect_results: START";
const char end_collector_msg[] = "collect_results: END";

// serial for debugging
Serial result_collector(USBTX, USBRX, NULL, 9600);

//========================================= FUNCTIONS ===========================================//

// if result collector is enabled
#if (defined(IOT2_RESULTS_COLLECTOR))

void iot2SendResults(void){

    // buffer to be used to send the result of each metric
    char result_buff[128] = {0};

    //-------------------------------------------------------------------------------------------//
    //                   send start message to notify the result collector                       //
    //-------------------------------------------------------------------------------------------//

    result_collector.printf("[IoT2] %s\r\n", start_collector_msg);
    
    //-------------------------------------------------------------------------------------------//
    //                          send IoT2 CORE METRICS results                                   //
    //-------------------------------------------------------------------------------------------//
    
    //--------------------------------------------------------------------------------------------
    // (1) send total runtime results
    //--------------------------------------------------------------------------------------------
    // elavate to read DWT
    iot2CallSVC(IOT2_RESERVED_SVC, IOT2_ELAVATE_PRIV_SVC_NUM);
    // Write the result of the cycle counter to the IoT2 Total runtime counter
    IoT2GlobalCounter.RuntimeCycles.totalRuntime += DWT->CYCCNT;



    uint64_t total_runtime = (RuntimeCntr->numOverflows * IOT2_CYCCNT_OVERFLOW_LIMIT)+
                             RuntimeCntr->totalRuntime;
    if (snprintf(result_buff, sizeof(result_buff), 
        "TotalRuntime_cycles:%" PRIu64"", total_runtime) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on total_runtime\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);


    //--------------------------------------------------------------------------------------------
    // (2) send ISR_cycles results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "ISR_cycles:%" PRIu64 "", ISRCntr->metricCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on ISR_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);




    //--------------------------------------------------------------------------------------------
    // (3) send SVC_cycles results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "SVC_cycles:%" PRIu64 "", SVCCntr->metricCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on SVC_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);

    //--------------------------------------------------------------------------------------------
    // (4) send PendSV_cycles results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "PendSV_cycles:%" PRIu64 "", PendSVCntr->metricCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on PendSV_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);


    //--------------------------------------------------------------------------------------------
    // (5) send SysTick_cycles results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "SysTick_cycles:%" PRIu64 "", SysTickCntr->metricCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on SysTick_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);


    //--------------------------------------------------------------------------------------------
    // (6) send InitCycles results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "Init_cycles:%" PRIu64 "", InitCntr->metricCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on Init_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);



    //--------------------------------------------------------------------------------------------
    // (7) send IoT2Cycles results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "IoT2Overhead_cycles:%" PRIu64 "", IoT2Cntr->metricCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on IoT2_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);

    //--------------------------------------------------------------------------------------------
    // (8) send PrivThreadCycles results
    //--------------------------------------------------------------------------------------------


    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "PrivThread_cycles:%" PRIu64 "", PrivThreadCntr->metricCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on PrivThread_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);


    //--------------------------------------------------------------------------------------------
    // (9) send IOBoundCycles results
    //--------------------------------------------------------------------------------------------
/*
    memset(result_buff, 0, sizeof(result_buff));
    
    // add throughput cycles to IOBoundCntr
    IOBoundCntr->metricCntr += IoT2GlobalCounter.IoT2ThroughputCycles.metricCntr;

    if (snprintf(result_buff, sizeof(result_buff), 
        "IOBound_cycles:%" PRIu64 "", IOBoundCntr->metricCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on IOBound_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);
*/

    //--------------------------------------------------------------------------------------------
    // (10) send SleepCycles results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "Sleep_cycles:%" PRIu64 "", SleepCntr->metricCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on Sleep_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);


    //--------------------------------------------------------------------------------------------
    // (11) send IoT2throughput results
    //--------------------------------------------------------------------------------------------
/*
    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "IoT2throughput:%f", IoT2GlobalCounter.IoT2throughput) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on IoT2throughput\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);

*/
    //--------------------------------------------------------------------------------------------
    // (12) send Priv* code size results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));
    uint64_t total_priv_regs_size = 0;
    for (uint8_t i = 0; i < IoT2_MAX_PRIV_CODE_REGS; i++){
        total_priv_regs_size += IoT2MPU.PrivRegions[i].size;
    }
    if (snprintf(result_buff, sizeof(result_buff), 
        "TotalPriv_code:%" PRIu64 "", total_priv_regs_size) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on total_priv_code\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);

    //--------------------------------------------------------------------------------------------
    // (13) send DEP results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));
    uint8_t dep_res = IoT2MPU.DEP & MPU->CTRL;//IoT2MPU.MPU_ENABLED;

    if (snprintf(result_buff, sizeof(result_buff), 
        "DEP_Enabled:%" PRIu8 "", dep_res) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on DEP_Enabled\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);


    //--------------------------------------------------------------------------------------------
    // (14) send PrivCycles  results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));
    IoT2GlobalCounter.PrivCycles.metricCntr = ISRCntr->metricCntr +
                                              SVCCntr->metricCntr +
                                              PendSVCntr->metricCntr +
                                              SysTickCntr->metricCntr +
                                              PrivThreadCntr->metricCntr;
    if (snprintf(result_buff, sizeof(result_buff), 
        "TotalPriv_cycles:%" PRIu64 "", IoT2GlobalCounter.PrivCycles.metricCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on CPU_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);

    //--------------------------------------------------------------------------------------------
    // (15) send ExceptionCntr  results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "Exception_cycles:%" PRIu64 "", ExceptionCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on CPU_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);

    //--------------------------------------------------------------------------------------------
    // (16) send ExceptionOverheadCntr  results
    //--------------------------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "EXCCNT_cycles:%" PRIu32 "", ExceptionOverheadCntr) < 0){

        // an error occured!
        result_collector.printf("[-] ERROR: could not perform snprintf on CPU_cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);


    //-------------------------------------------------------------------------------------------//
    //                   send end message to notify the result collector                         //
    //-------------------------------------------------------------------------------------------//
    
    result_collector.printf("[IoT2] %s\r\n", end_collector_msg);
    //result_collector.printf("[IoT2] %c\r\n",result_collector.getc());
    
}


#else   // IOT2_RESULTS_COLLECTOR //

// If result collection is disabled
void iot2SendResults(void){
    // result collection is disable, so link the empty function.
    
}

#endif // IOT2_RESULTS_COLLECTOR //