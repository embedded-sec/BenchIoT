//=================================================================================================
//
// IoT2-lib: A library for functions to be generally used accross all benchmarks. Generally are 
// architecture independent (i.e. only depenends on cmsis lib) and do not use a specific board
// feature as much as possible.
//
//=================================================================================================


//========================================== INCLUDES ===========================================//

#include "IoT2Lib.h"
// IoT2 configuration and special interface files
#include "IoT2_Config.h"

//====================================== DEFINES & GLOBALS ======================================//


IoT2CntrStruct IoT2GlobalCounter = {
    // Total runtime
    .RuntimeCycles = {0},
    // Core metrics
    .ISRCycles = {0},
    .SVCCycles = {0},
    .PendSVCycles = {0},
    .SysTickCycles = {0},
    .InitCycles = {0},
    .IoT2Cycles = {0},
    // Security metrics
    .PrivCycles = {0},
    .PrivThreadCycles = {0},
    .UnprivCycles = {0},
    // Performance metrics
    .IOBoundCycles = {0},
    .CPUCycles = {0},
    .SleepCycles = {0},
    // extra counter for debugging
    .DebugCycles = {0},
    // Throughput
    .IoT2ThroughputCycles ={0},
    .IoT2throughputBytes = 0,
    .IoT2throughput = 0

};

// Total runtime
IoT2RuntimeCntr *volatile RuntimeCntr = &(IoT2GlobalCounter.RuntimeCycles);
// Core metrics
IoT2MetricCntr *volatile ISRCntr = &(IoT2GlobalCounter.ISRCycles);
IoT2MetricCntr *volatile SVCCntr = &(IoT2GlobalCounter.SVCCycles);
IoT2MetricCntr *volatile PendSVCntr = &(IoT2GlobalCounter.PendSVCycles);
IoT2MetricCntr *volatile SysTickCntr = &(IoT2GlobalCounter.SysTickCycles);
IoT2MetricCntr *volatile InitCntr = &(IoT2GlobalCounter.InitCycles);
IoT2MetricCntr *volatile SleepCntr = &(IoT2GlobalCounter.SleepCycles);
IoT2MetricCntr *volatile IoT2Cntr = &(IoT2GlobalCounter.IoT2Cycles);
IoT2MetricCntr *volatile PrivThreadCntr = &(IoT2GlobalCounter.PrivThreadCycles);
IoT2MetricCntr *volatile IOBoundCntr = &(IoT2GlobalCounter.IOBoundCycles);
// Initially we start at user (thread) mode, so we set up both to point to UserCycles
IoT2MetricCntr *volatile CurrCntr = &(IoT2GlobalCounter.InitCycles);
IoT2MetricCntr *volatile PrevCntr = &(IoT2GlobalCounter.InitCycles);
IoT2MetricCntr *volatile DebugCntr = &(IoT2GlobalCounter.DebugCycles);
IoT2ExecModeStrct volatile iot2ExecMode = {USER_MODE};
enum IOT2_OVERHEAD_TYPE iot2OverheadType = ENTRY_OVERHEAD;


#if IoT2_OS_BENCHMARKS==0

                                  // entry, exit
uint8_t iot2_entr_exit_arr[3][2] = { {129,   56},     // iot2ISRTrampoline
                                     {129,   129},    // iot2ISRTrampoline
                                     {53,    54}};    // iot2SVC 

#else
// if using the OS benchmarks

                                  // entry, exit
uint8_t iot2_entr_exit_arr[3][2] = { {129,   56},     // iot2ISRTrampoline
                                     {98,    19},     // iot2(Start/End)ExceptionTrampoline
                                     {53,    54}};    // iot2SVC 


#endif // IoT2_OS_BENCHMARKS==0

uint32_t iot2FuncOverhead = 0;          // holds the overhead of collecting IoT2Cntr function



uint32_t iot2PrivState = 0;
enum IoT2_PROCESSOR_STATE volatile iot2ProcessorState = IoT2_RUN;
enum IoT2_VECTOR_TABLE_STATE volatile iot2VectorTableState = VECTOR_TABLE_WRITABLE;


// The original IRQ is at offset of NVIC_NUM_VECTORS
__attribute__((used)) void (*origIRQHandler)(void);
IoT2BenchHandlerStrct benchHandlerStrct = {0};

uint32_t volatile origExceptionNUM = 0; // bits[0:8] represent exception number
uint32_t volatile iot2LR = 0;           // holds the value of LR with no preemption
uint32_t volatile iot2PreemptLR = 0;    // holds the value of LR preemption
uint32_t volatile origLR = 0;           // Global to be used to restore original LR
__attribute__((used)) uint32_t volatile tempReg = 0;


// MPU Variables
IoT2MPUMetrics IoT2MPU = {
    .MPU_ENABLED = 0,                   // assume mpu is disabled
    .BACKGROUND_REGION_ENABLED = 0,     // Assume background region is disabled
    .DEP = 1,                           // Assume DEP is enabled
    .PrivRegions = {{0}}
};

int volatile COLLECT_IOT2_METRICS = 0;
uint64_t volatile PrivThreadHelper = 0;
uint64_t volatile ExceptionCntr = 0;
uint32_t volatile ExceptionOverheadCntr = 0;



//------------------------------------------------------------------------------

/// The svc handler is invoked by calling svc 99, we then use IoT2SvcNum as an 
/// index in the IoT2SvcTable to call the desired handler
const uint8_t IoT2SvcHandlerNum = 99;

/// This variable is used to setup calling the correct SVC handler for IoT2 from
/// IOT2_SVC_TABLE
uint8_t volatile IOT2_SVC_NUM = 0;

/// This flag is used to indicate wether the called SVC should be handled by 
/// the default handler or by IoT2, 1 = handle by IoT2, 0 = Default handler
/// It is mainly used to call SVCs for IoT2 before switching SCB->VTOR to IoT2 
/// after taking control of VTOR IoT2 uses SVC 99 for all its SVCs
volatile int IOT2_SVC_FLAG = 0;


uint32_t* OldVTOR = NULL;

uint32_t PrivThreadStatus = 0;
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------//
//                                      Mbed specific                                            //
//-----------------------------------------------------------------------------------------------//




#if IoT2_OS_BENCHMARKS==0

uint8_t volatile BM_SVC_NUM = 0;
/// Include this handler if the user did not include the function already. In
/// case SVC_Handler is implemented by the user, then the body of this function
/// should be included at the beginning of the SVC_Handler to enable IoT2Lib 
/// to setup the measurements of the runtime metrics.

//void SVC_Handler();

void SVC_Handler(void){
    if(IOT2_SVC_FLAG){
        __disable_irq();
        iot2Init();
        __enable_irq();

    }
    // The use handler should go here

/* remove later
    if (BM_SVC_NUM == 1){
        iot2EndAllMeasurements();
    }
    else if (BM_SVC_NUM == 2){
        iot2svcElavatePriv();
    }
    else if (BM_SVC_NUM == 3){
        iot2svcDeElavatePriv();
    }
    else if (BM_SVC_NUM == 4){
        #if SECURE_DATA
            config_mpu();
        #else
            iot2RecordPrivThread();
        #endif // SECURE_DATA
    }
    else if (BM_SVC_NUM == 5){
        #if SECURE_DATA
        enable_secure_data_mpu();
        #endif
    }
    else if (BM_SVC_NUM == 6){
        #if SECURE_DATA
        disable_secure_data_mpu();
        #endif
    }
*/
}


#endif //IoT2_OS_BENCHMARKS==0


//========================================== FUNCTIONS ==========================================//

/// This function calls iot2Init internally, it sets up the configurations to 
/// call iot2Init as an SVC. iot2Init then takes control of the vector table
/// this function is called once at initialization stage
__attribute__((used)) void iot2Setup(void){
    // this flag is used to indicate that iot2 should be used, it is later
    // set to 0 in iot2Init
    IOT2_SVC_FLAG = 1;
    // the first SVC should use svc_num 0 as it calls the default SVC, which
    // is annotated to call iot2Init
    iot2CallSVC(0, IOT2_SWITCH_VEC_TABLE_SVC_NUM); // 0: svc_num, 2: index of the handler in IoT2SvcTable

}


__attribute__((used)) void iot2Init(void){

#if (defined(IOT2_ISR_RUNTIME))

    iot2SwitchVectorTable();

#endif      // IOT2_ISR_RUNTIME

    // make sure to set IOT2_SVC_FLAG = 0
    IOT2_SVC_FLAG = 0;

    //iot2HWsetup();
    // here we only enable DWT to measure init cycles, all metrics are enabled
    // after establishing the connection

    // update exception overhead and reset exception overhead. This is important
    // to avoid an overflow in the measurement in the EXCCNT is larger than the
    // intended exception measurement, which can occur at the begininng of
    // of the application execution
    ExceptionOverheadCntr += DWT->EXCCNT;
    DWT->EXCCNT = 0;
    // synchronize here
    __DSB();
    __ISB();
}

///
/// 1- Set the USERSETMPEND bit in CCR so that ISR can be triggered in unprivileged mode
/// 2- Enable any needed IRQs for the current application
///

void iot2HWsetup(void){
    // check DEP in the beginning of the application
    iot2CheckPrivCodeAndDEP();
    SCB->CCR |= SCB_CCR_USERSETMPEND_Msk;               // Allow unprivleged code to triggere ISR
    iot2SetupCounters();                                // Enable DWT counters

}

void iot2EnableSWI(IRQn_Type IRQn){
    NVIC_EnableIRQ(IRQn);//[iot2-debug]
}

///
/// Triggers an IRQ from software
///
void iot2TriggerSWI(IRQn_Type IRQn){
    NVIC->STIR = (uint32_t)(NVIC_STIR_INTID_Msk & IRQn); 
}

/// @brief Higher level API to trigger software interrupt for a benchmark
void iot2TriggerBenchSoftIRQ(void(*benchIrq)(), IRQn_Type benchIrqNum){

    // update the function pointer in benchHandlerStrct
    benchHandlerStrct.benchIRQHandler = benchIrq;
    benchHandlerStrct.benchIRQNum = (uint32_t) benchIrqNum + NUM_CORTEX_M_CORE_VECTORES;
    benchHandlerStrct.benchFlag = BENCH_FLAG_ON;
    // Make sure the code executes in order to redirect the flow to the benchmark
    // IRQ handler.
    //__DSB();
    //__ISB();
    // trigger software interrupt
    iot2TriggerSWI(benchIrqNum);
}


//-----------------------------------------------------------------------------------------------//
//                                      DWT related functions                                    //
//-----------------------------------------------------------------------------------------------//

/// @brief: Reset all DWT counters
/// @param: None
void iot2ResetCounters(void){

    // reset all DWT counters before initilizing them
    DWT->CYCCNT = 0;        // Cycle counter
    DWT->CPICNT = 0;        // Instruction cycle counter for multi-cycle instructions
    DWT->EXCCNT = 0;        // Exception overhead cycle counter
    DWT->SLEEPCNT = 0;      // Sleep overhead cycle counter
    DWT->LSUCNT = 0;        // Load-store unit cycle counter for multi-cycle ld/str instructions
    DWT->FOLDCNT = 0;       // Foleded instructions cycle counter
}

/// @brief: Enable DWT counters
/// @param: None
void iot2EnableCounters(void){

    // enable debugging (more precisly enables DWT/ITM/ETM/TPIU units)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // Enable the different DWT counters
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk     // Enable cycle count register (DWT_CYCCNT)
                | DWT_CTRL_EXCTRCENA_Msk    // Enable exception trace
                | DWT_CTRL_CPIEVTENA_Msk    // Enable CPI count register
                | DWT_CTRL_EXCEVTENA_Msk    // Enable exception overhead count register
                | DWT_CTRL_SLEEPEVTENA_Msk  // Enable sleep counter register
                | DWT_CTRL_LSUEVTENA_Msk    // Enable LSU (Load Store Unit) counter register
                | DWT_CTRL_FOLDEVTENA_Msk;  // Enable fold count register
}

///
/// Setup the DWT unit counters
///
void iot2SetupCounters(void){

    // reset all counters
    iot2ResetCounters();
    // enable all counters
    iot2EnableCounters();

}


void iot2DisableCounters(void){
    // Halt all DWT counters. 
    DWT->CTRL &= ~(DWT_CTRL_CYCCNTENA_Msk
                | DWT_CPICNT_CPICNT_Msk
                | DWT_EXCCNT_EXCCNT_Msk
                | DWT_SLEEPCNT_SLEEPCNT_Msk
                | DWT_LSUCNT_LSUCNT_Msk
                | DWT_FOLDCNT_FOLDCNT_Msk);
}


/// @brief: Collect results from the global IoT2 counter
/// @param: None
void iot2CollectResults(){

    float estimated_time = (float)IoT2GlobalCounter.IoT2ThroughputCycles.metricCntr/IoT2_PROCESSOR_SPEED;
    // calculate total throughput
    if(estimated_time == 0){
        IoT2GlobalCounter.IoT2throughput = 0;
    }
    else{
        IoT2GlobalCounter.IoT2throughput = IoT2GlobalCounter.IoT2throughputBytes/estimated_time;
    }
      

}


void iot2EndAllMeasurements(void){
    
    // Make sure PrivThreadCntr state is marked as Finished
    iot2HaltPrivThreadCntr();
    // stop COLLECT_IOT@_METRICS
    COLLECT_IOT2_METRICS = 0;
    // diable DWT counters 
    iot2DisableCounters();
    //iot2CollectResults();// might delete this call


}


__attribute__((used, optimize("-O0"))) void iot2EndBenchmark(void){
    __asm("bkpt\n");
}

//-----------------------------------------------------------------------------------------------//
//                               IoT2 runtime measurement functions                              //
//-----------------------------------------------------------------------------------------------//


void iot2UpdateProcessorState(void){
    if (iot2ProcessorState == IoT2_SLEEP){
        iot2ProcessorState = IoT2_RUN;
    }
}



void iot2UpdateExecMode(void){

    // set the prevCntr to point to the current one, the prevCntr is used in 
    // case of preemption
    PrevCntr = CurrCntr;
    // get the exception number
    origExceptionNUM = SCB->ICSR & 0x1FF; // bits[0:8] represent exception number
    switch(origExceptionNUM){
        
        case SVCall_EXCEPTION_NUM:
            CurrCntr = &(IoT2GlobalCounter.SVCCycles);
            iot2ExecMode.currMode = SVC_MODE;
            iot2FuncOverhead =  iot2_entr_exit_arr[2][iot2OverheadType];
            break;

        case PendSV_EXCEPTION_NUM:
            CurrCntr = &(IoT2GlobalCounter.PendSVCycles);
            iot2ExecMode.currMode = PENDSV_MODE;
            iot2FuncOverhead =  iot2_entr_exit_arr[1][iot2OverheadType];
            break;

        case SYSTICK_EXCEPTION_NUM:
            CurrCntr = &(IoT2GlobalCounter.SysTickCycles);
            iot2ExecMode.currMode = SYSTICK_MODE;
            iot2FuncOverhead =  iot2_entr_exit_arr[1][iot2OverheadType];
            break;

        default:
            CurrCntr = &(IoT2GlobalCounter.ISRCycles);
            iot2ExecMode.currMode = ISR_MODE;
            iot2FuncOverhead =  iot2_entr_exit_arr[0][iot2OverheadType];
    }
}


__attribute__((used)) void iot2StartRecordExceptionMetric(void){
    if(COLLECT_IOT2_METRICS){
        // time stamp the cycle counter with iot2 cntr
        IoT2Cntr->startCntr = DWT->CYCCNT;

        // update iot2overhead type, used to set the fixed iot2 overhead measurement
        iot2OverheadType = ENTRY_OVERHEAD;

        // if there are no preempted exceptions
        if (SCB->ICSR & SCB_ICSR_RETTOBASE_Msk){
            
            // User and hardare exception overhead do not have separate cntrs and we
            // do not track them explicitly. They found by execluding iot2 core metrics.

            // save the value of LR
            iot2LR = origLR;

        }

        // There are prempted exceptions, record the previous one
        else{
            // save the preempted value of LR
            iot2PreemptLR = origLR;
            // end counting exception, note that measurement is not complete yet
            iot2EndPreemptedCoreMetricCntr();
        }

        // update execution mode and CurrCntr
        iot2UpdateExecMode();
        // update metric state to incomplete
        iot2SetCoreMetricIncompleteState();
        // start recording the new core metric
        iot2StartCoreMetricCntr();
    }

}


void iot2UpdatePreemptedExceptionMetric(void){
    
 
    if(ISRCntr->state == MEASUREMENT_INCOMPLETE){
        ISRCntr->startCntr = IoT2Cntr->endCntr;
    }

    if(SVCCntr->state == MEASUREMENT_INCOMPLETE){
        SVCCntr->startCntr = IoT2Cntr->endCntr;
    }

    if (PendSVCntr->state == MEASUREMENT_INCOMPLETE){
        PendSVCntr->startCntr = IoT2Cntr->endCntr;
    }

    if (SysTickCntr->state == MEASUREMENT_INCOMPLETE){
        SysTickCntr->startCntr =IoT2Cntr->endCntr;
    }

}


__attribute__((used)) void iot2EndRecordExceptionMetric(void){
    if (COLLECT_IOT2_METRICS){
        // time stamp the end of the exception
        IoT2Cntr->startCntr = DWT->CYCCNT;

        // update iot2overhead type, used to set the fixed iot2 overhead measurement
        iot2OverheadType = EXIT_OVERHEAD;

        // update execution mode and CurrCntr to ensure we are collecting
        // the correct metric
        iot2UpdateExecMode();
        
        // collect current metric
        iot2EndCoreMetricCntr();
        // set state as finished
        iot2SetCoreMetricFinishState();

        // if there are prempted exceptions, update the start counters
        if(!(SCB->ICSR & SCB_ICSR_RETTOBASE_Msk)){

            // since there is preemption, set origLR to the preempted version
            origLR = iot2PreemptLR;
            
            // Make sure PrevCntr state is incomplete, this is needed in case
            // excpetions of the same type preempt each other (i.e., IRQs)
            iot2SetPreemptedCoreMetricIncompleteState();
            // update for iot2RecordPrivThread
            iot2ExecMode.currMode = USER_MODE;
            // collect IoT2 overhead
            iot2EndIoT2OverheadCntr();
            
            iot2UpdatePreemptedExceptionMetric();

        }else{
            // with no preemption, origLR will be the regular LR
            origLR = iot2LR;
            // update for iot2RecordPrivThread
            iot2ExecMode.currMode = USER_MODE;
            // collect IoT2 overhead
            iot2EndIoT2OverheadCntr();
        }
    }
}


void iot2StartCoreMetricCntr(void){

    // collect iot2 overhead 
    iot2EndIoT2OverheadCntr();
    // set new core metric start counter
    CurrCntr->startCntr = IoT2Cntr->endCntr + iot2FuncOverhead;
}


void iot2EndCoreMetricCntr(void){
    

    CurrCntr->endCntr =IoT2Cntr->startCntr;

    if(CurrCntr->startCntr > CurrCntr->endCntr){
            __asm(
                "bkpt\n");
    }
    CurrCntr->metricCntr += CurrCntr->endCntr - CurrCntr->startCntr;// - DWT->EXCCNT;

    // update ExceptionCntr
    ExceptionCntr += CurrCntr->endCntr - CurrCntr->startCntr;// - DWT->EXCCNT;

    // update exception overhead and reset exception overhead
    ExceptionOverheadCntr += DWT->EXCCNT;
    DWT->EXCCNT = 0;
}


void iot2EndPreemptedCoreMetricCntr(void){
    
    // in preemption, we end measuring the preempted metric once we
    // start recording the higher priority exception
    iot2EndCoreMetricCntr();

}


void iot2EndIoT2OverheadCntr(void){
    // end IoT overhead measurement
    IoT2Cntr->endCntr = DWT->CYCCNT;
    IoT2Cntr->metricCntr += IoT2Cntr->endCntr - IoT2Cntr->startCntr;
    // if there is an overflow, reset DWT cntrs, iot2 start, and end Cntrs
    iot2CheckOverflow();
}


void iot2SetCoreMetricIncompleteState(void){
    CurrCntr->state = MEASUREMENT_INCOMPLETE;
}

void iot2SetCoreMetricFinishState(void){
    CurrCntr->state = MEASUREMENT_FINISHED;
}


void iot2SetPreemptedCoreMetricIncompleteState(void){
    PrevCntr->state = MEASUREMENT_INCOMPLETE;
}


void iot2CheckOverflow(void){
    if (IoT2Cntr->endCntr > IOT2_CYCCNT_OVERFLOW_LIMIT){
        // update runtime cntr and number of overflow
        RuntimeCntr->totalRuntime += IoT2Cntr->endCntr - IOT2_CYCCNT_OVERFLOW_LIMIT;
        RuntimeCntr->numOverflows += 1;
        // reset DWT and IoT2Cntr
        iot2ResetCounters();
        IoT2Cntr->startCntr = 0;
        IoT2Cntr->endCntr = 0;
    }
}


void iot2StartCustomeMetric(IoT2MetricCntr *metric){
    if(COLLECT_IOT2_METRICS){
        int elavate_and_drop_pirvs = __get_CONTROL() & CONTROL_nPRIV_Msk;
        // check if we need to elavate/drop privileges
        if (elavate_and_drop_pirvs){
            // call an SVC to elavate privileges
            iot2CallSVC(IOT2_RESERVED_SVC,IOT2_ELAVATE_PRIV_SVC_NUM);
            // start the metric
            uint32_t time_stamp_cyccnt = DWT->CYCCNT;
            uint64_t total_time_stamp = (RuntimeCntr->numOverflows * IOT2_CYCCNT_OVERFLOW_LIMIT)
                                    + RuntimeCntr->totalRuntime + time_stamp_cyccnt;
            metric->startCntr = total_time_stamp;

            // call an SVC to drop privileges
            iot2CallSVC(IOT2_RESERVED_SVC,IOT2_DE_ELAVATE_PRIV_SVC_NUM);

            // add iot2 overhead
            IoT2Cntr->metricCntr += IOT2_START_CUSTOM_METRIC_OVERHEAD_1;        
        }

        // if at privileged mode
        else{
            // start the metric
            uint32_t time_stamp_cyccnt = DWT->CYCCNT;
            uint64_t total_time_stamp = (RuntimeCntr->numOverflows * IOT2_CYCCNT_OVERFLOW_LIMIT)
                                    + RuntimeCntr->totalRuntime + time_stamp_cyccnt;
            metric->startCntr = total_time_stamp;
            // add iot2 overhead
            IoT2Cntr->metricCntr += IOT2_START_CUSTOM_METRIC_OVERHEAD_2;

        }

    }
}

void iot2EndCustomeMetric(IoT2MetricCntr *metric){
    if(COLLECT_IOT2_METRICS){
        int elavate_and_drop_pirvs = __get_CONTROL() & CONTROL_nPRIV_Msk;
        // check if we need to elavate/drop privileges
        if (elavate_and_drop_pirvs){
            // call an SVC to elavate privileges
            iot2CallSVC(IOT2_RESERVED_SVC,IOT2_ELAVATE_PRIV_SVC_NUM);
        
            // collect the metric
            // start the metric
            uint32_t time_stamp_cyccnt = DWT->CYCCNT;
            uint64_t total_time_stamp = (RuntimeCntr->numOverflows * IOT2_CYCCNT_OVERFLOW_LIMIT)
                                    + RuntimeCntr->totalRuntime + time_stamp_cyccnt;
            metric->endCntr = total_time_stamp;
            metric->metricCntr += metric->endCntr - metric->startCntr;
            // if started from unprivleged, then return to unprivieged
            // call an SVC to elavate privileges
            iot2CallSVC(IOT2_RESERVED_SVC,IOT2_DE_ELAVATE_PRIV_SVC_NUM);

            // add iot2 overhead
            IoT2Cntr->metricCntr += IOT2_END_CUSTOM_METRIC_OVERHEAD_1;        
        }
        else{
            uint32_t time_stamp_cyccnt = DWT->CYCCNT;
            uint64_t total_time_stamp = (RuntimeCntr->numOverflows * IOT2_CYCCNT_OVERFLOW_LIMIT)
                                    + RuntimeCntr->totalRuntime + time_stamp_cyccnt;
            // collect the metric
            metric->endCntr = total_time_stamp;
            metric->metricCntr += metric->endCntr - metric->startCntr;
            // add iot2 overhead
            IoT2Cntr->metricCntr += IOT2_END_CUSTOM_METRIC_OVERHEAD_2;

        }

    }
}

//-----------------------------------------------------------------------------------------------//
//                                 Vector table related functions                                //
//-----------------------------------------------------------------------------------------------//
// number next to an instruciton indicate cycle count. The notation of (subTot) is
// a subtotal per each block of code.


__attribute__((naked,used)) void iot2SVCHandler(void){

    __ASM volatile(
        
        "cpsid i\n"                          // disable interrupts [2]
        "push {r0-r3,r12}\n"                 // save the registers [6]
        "ldr r0,=origLR\n"                   // get origLR addr [2]
        "str lr,[r0]\n"                      // save LR in origLR [2]
        "bl iot2StarsttRecordExceptionMetric\n"// start measurement [1+P = 4]
#if ((IoT2_OS_BENCHMARKS == 0 )
        "mrs r0,msp\n"                       // load sp [2]
        "ldr r0,[r0, #44]\n"                 // load pc [2]
#else
        "mrs r0,psp\n"                       // load sp [2]
        "ldr r0,[r0, #24]\n"                 // load pc [2]
#endif  // IoT2_OS_BENCHMARKS
        "ldrb r0,[r0, #-2]\n"                // get SVC number [2]
        "sub r0,r0,%[immediate]\n"           // sub IoT2SvcHandlerNum from R0 [1]
        "cbnz r0,call_default_svc_handler\n" // use iot svc if 99 [4], subTot = 27
        //---------------------------------------------------------
        // The svc belongs to iot2
        //---------------------------------------------------------
        "call_iot2_svc:\n"      
        "bl iot2ExecSVC\n"                   // use the iot2SVC handler [4]
        "bl iot2EndRecordExceptionMetric\n"  // end measurement [4] + 22 from iot2EndRecord..etc
        "ldr r0,=origLR\n"                   // restore original LR [2]
        "ldr lr,[r0]\n"                      // LR restored [2]
        "pop {r0-r3,r12}\n"                  // restore original state of regs [6]
        "cpsie i\n"                          // re-enable interrupts [2]
        "bx lr\n"                            // return from the svc [4], subTot = 46
        //---------------------------------------------------------
        // The svc does not belong to iot2, so pass it to the
        // original svc by the user/os. Note here that we need
        // to pop R0-R3,R12 and LR to hand the execution transperantly
        // Collecting the metrics is done at the default svc end
        // and not here, this function does not return.
        //---------------------------------------------------------
        "call_default_svc_handler:\n"
        "ldr r0,=OldVTOR\n"                  // load the addr of old VTOR [2]
        "ldr r0,[r0]\n"                      // get the value of old VTOR [2]
        "ldrd r0,[r0, #44]\n"                // get the default svc [2]
        "mov lr, r0, lsr#0\n"                // lr = r0 [1]
        "pop {r0-r3,r12}\n"                  // restore original state of regs [6]
        "push {lr}\n"                        // push the original svc handler [2]
        "ldr lr,=origLR\n"                   // restore original LR [2]
        "ldr lr,[lr]\n"                      // LR restored [2]
        "cpsie i\n"                          // re-enable interrupts [2]
        "pop {pc}\n"                         // call the default svc handler [2+P=5], subTot = 26
        // the measurement is done at the end of the original handler
        :: [immediate] "i" (IoT2SvcHandlerNum)
        );
}


/// This function is called from iot2SVCHandler, it looks up the IOT2_SVC_NUM
/// and calls the correct handler from the IoT2SvcTable table

__attribute__((used)) void iot2ExecSVC(void){
    // check that IoT2_SVC_NUM is within bounds
    if (IOT2_SVC_NUM < IOT2_SVC_TABLE_SIZE){
        void(*func_ptr)(void);
        func_ptr = IoT2SvcTable[IOT2_SVC_NUM];
        // call the handler
        (*func_ptr)();

    }
    // an error...
    else{
        while(1);
    }
}


/// here we assume the register have been saved earlier. This function is
/// supposed to return to the caller which will pop the registers and
/// return from the exception

void iot2SwitchVectorTable(void){
    
    // save the old vector table location, then set VTOR to the iot2 table
    OldVTOR = (uint32_t*)SCB->VTOR;
    SCB->VTOR = (uint32_t) IoT2IsrTable;
    
    // Change the state of iot2VectorTableState to re-target writes to
    // the shifted vector table
    iot2VectorTableState = VECTOR_TABLE_LOCKED;
    // add barrier to update VTOR
    __DSB();
    __ISB();

}



__attribute__((naked,used)) void iot2ISRTrampoline(void){

    __asm volatile(
        "cpsid i\n"                           // disable interrupts
        "push {r0-r3,r12}\n"                  // save regs
        "ldr r0,=origLR\n"                    // get origLR
        "str lr,[r0]\n"                       // store LR in origLR
        "bl iot2GetOriginalExcHandler\n"      // get original handler
        "bl iot2StartRecordExceptionMetric\n" // start cntr----------------
        "ldr lr,=origIRQHandler\n"            // get origISRHandler pointer
        "ldr lr,[lr]\r\n"                     // get original handler function
        "pop {r0-r3,r12}\n"                   // restore regs
        "cpsie i\n"                           // re-enable interrupts
        "blx lr\n"                            // call original handler
        "cpsid i\n"                           // disable interrupts
        "push {r0-r3,r12}\n"                  // save regs
        "bl iot2EndRecordExceptionMetric\n"   // end cntr----------------
        "ldr lr,=origLR\n"                    // restore the original LR
        "ldr lr,[lr]\n"                       // Nor LR is restored
        "pop {r0-r3,r12}\n"                   // restore regs
        "cpsie i\n"                           // re-enable interrupts
        "bx lr\n"                             // exit handler
        );

}


void iot2SetIRQNum(void){

    origExceptionNUM = SCB->ICSR & 0x1FF; // bits[0:8] represent exception number
    
    // check if this a software interrupt from the benchmark, if so use
    // the benchmark handler. If not use the original hardware handler
    if ( (benchHandlerStrct.benchFlag == BENCH_FLAG_ON) && 
        (benchHandlerStrct.benchIRQNum == origExceptionNUM) ){

        // set origHandler to the benchmark specific handler
        origIRQHandler = benchHandlerStrct.benchIRQHandler;
        // make sure to invalidate the bench flag so that other interrupts
        // work correctly
        benchHandlerStrct.benchFlag = BENCH_FLAG_OFF;
    }

    // No interrupt from the benchmark, use the original one
    else{
        // The original IRQ is at offset of NVIC_NUM_VECTORS
        origIRQHandler = (void*)(OldVTOR[origExceptionNUM]);
    }

}


/// This function is a one-way trampoline. It starts the measurement then hands
/// the execution to original handler. It does not automatically collect the 
/// measurement as done in iot2ISRTrampoline. This is used when the original
/// handler require the actual LR value. In order to collect the measurement,
/// the end of the original handler need to be annotated
/// with iot2EndExceptionTrampoline

/*

In order to avoid using any register, we save the following on the stack. The
pc value in the begininng is just a place holder. At the end of the function
we change the saved value on the stack to point to the actual handler, then 
pop it to pc normally.

Stack:
|   pc   |  <---- Changed at the end to the original handler, the pop to pc
----------
|   r0   |
----------
|   r1   |
----------
|   r2   |
----------
|   r3   |
----------
|  r12   |
----------
|   lr   |
----------   <----- Top of stack

*/

__attribute__((naked,used)) void iot2StartExceptionTrampoline(void){
    __ASM volatile(
        "cpsid i\n"                           // disable interrupts
        "push {lr}\n"                        // This is a place holder for pc
        "push {r0-r3,r12,lr}\n"               // save the registers
        "bl iot2GetOriginalExcHandler\n"      // store original handler at origIRQHandler
        "bl iot2StartRecordExceptionMetric\n" // start measurement
        "ldr r0,=origIRQHandler\n"            // str original handler at r0
        "ldr r0,[r0]\n"
        "str r0,[sp,#24]\n"                   // change saved-pc to original handler
        "pop {r0-r3,r12,lr}\n"                // pop register
        "cpsie i\n"                           // re-enable interrupts
        "pop {pc}\n"                          // pop the original handler to pc
        );
}

/// IMPORTANT: this function should be wrapped with CPSID/CPSIE instructions
/// and LR has to be save before to work correctly. Note that exiting the 
// handler is done in the original handler and not in this function.

__attribute__((naked,used)) void iot2EndExceptionTrampoline(void){
    __ASM volatile(
        "push {r0-r3,lr}\n"                 // save lr
        "bl iot2EndRecordExceptionMetric\n" // start measurement
        "pop {r0-r3,lr}\n"                  // pop lr
        "bx lr\n"                           // return to the handler
        );
}

/// This function only calls the original handler. Generally it is used for 
/// fault handlers.

__attribute__((naked,used)) void iot2CallOriginalHandler(void){
    __ASM volatile(
        "push {lr}\n"                        // This is a place holder for pc
        "push {r0-r3,r12,lr}\n"               // save the registers
        "bl iot2GetOriginalExcHandler\n"      // store original handler at origIRQHandler
        "ldr r0,=origIRQHandler\n"            // str original handler at r0
        "ldr r0,[r0]\n"
        "str r0,[sp,#24]\n"                   // change saved-pc to original handler
        "pop {r0-r3,r12,lr}\n"                // pop register
        "pop {pc}\n"                          // pop the original handler to pc
        );
}



__attribute__((used)) void iot2GetOriginalExcHandler(void){
    
    origExceptionNUM = SCB->ICSR & 0x1FF; // bits[0:8] represent exception number

        // check if this a software interrupt from the benchmark, if so use
    // the benchmark handler. If not use the original hardware handler
    if ( (benchHandlerStrct.benchFlag == BENCH_FLAG_ON) && 
        (benchHandlerStrct.benchIRQNum == origExceptionNUM) ){

        // set origHandler to the benchmark specific handler
        origIRQHandler = benchHandlerStrct.benchIRQHandler;
        // make sure to invalidate the bench flag so that other interrupts
        // work correctly
        benchHandlerStrct.benchFlag = BENCH_FLAG_OFF;
    }

    // No interrupt from the benchmark, use the original one
    else{

        // The original IRQ is at offset of NVIC_NUM_VECTORS
        origIRQHandler = (void*)OldVTOR[origExceptionNUM];
        
    }
    
}


//-----------------------------------------------------------------------------------------------//
//                                      MPU related functions                                    //
//-----------------------------------------------------------------------------------------------//


/// @brief: Return the current execution mode
/// @param: none
/// @returns: EXEC_MODE value


void iot2RecordPrivThread(void){
    if(COLLECT_IOT2_METRICS){
        enum EXEC_MODE mode = USER_MODE;
        int call_svc = (iot2ExecMode.currMode == mode) ? 1:0;
        // If execution is unprivileged, check if we need to end measuring
        // privileged thread cycles then return
        if (PrivThreadStatus & CONTROL_nPRIV_Msk){
            if(call_svc){
                // call an SVC to elavate privileges
                iot2CallSVC(IOT2_RESERVED_SVC,IOT2_ELAVATE_REC_PRIV_TH_SVC_NUM);
            }
            uint32_t time_stamp_cyccnt = DWT->CYCCNT;
            if (PrivThreadCntr->state == MEASUREMENT_INCOMPLETE){
                // PrivThread->end - PrivThread->start - (ExceptionCntr betweern start and end)
               uint64_t total_time_stamp = (RuntimeCntr->numOverflows * IOT2_CYCCNT_OVERFLOW_LIMIT)
                                    + RuntimeCntr->totalRuntime + time_stamp_cyccnt;

                PrivThreadCntr->metricCntr += total_time_stamp 
                                              - PrivThreadCntr->startCntr  
                                              -(ExceptionCntr - PrivThreadHelper);

                PrivThreadCntr->state = MEASUREMENT_FINISHED;
            }
            if(call_svc){
                // call an svc to de-elavate privileges
                iot2CallSVC(IOT2_RESERVED_SVC,IOT2_DROP_REC_PRIV_TH_SVC_NUM);
            }
            // add iot2 overhead
            IoT2Cntr->metricCntr += IOT2_PRIV_THREAD_OVERHEAD_1;
        }
        // if execution is privileged, check if we need to start measurement
        else{
            uint32_t time_stamp_cyccnt = DWT->CYCCNT;
            uint64_t total_time_stamp = (RuntimeCntr->numOverflows * IOT2_CYCCNT_OVERFLOW_LIMIT)
                                    + RuntimeCntr->totalRuntime + time_stamp_cyccnt;
            if (PrivThreadCntr->state != MEASUREMENT_INCOMPLETE){

                PrivThreadCntr->startCntr = total_time_stamp;
                PrivThreadHelper = ExceptionCntr; // time stamp exception Cntr
                PrivThreadCntr->state = MEASUREMENT_INCOMPLETE;
            }
            // add iot2 overhead
            IoT2Cntr->metricCntr += IOT2_PRIV_THREAD_OVERHEAD_2;
        }
    }  


}


/// This function is called from iot2EndlAllmeasurements to make sure
/// PrivThread cycles has ended even if execution is still privileged

void iot2HaltPrivThreadCntr(void){
    uint32_t time_stamp_cyccnt = DWT->CYCCNT;
    // record privileged thread cntr if it is currently being measured
    if (PrivThreadCntr->state == MEASUREMENT_INCOMPLETE){
        // PrivThread->end - PrivThread->start - (ExceptionCntr betweern start and end)
        uint64_t total_time_stamp = (RuntimeCntr->numOverflows * IOT2_CYCCNT_OVERFLOW_LIMIT)
                                    + RuntimeCntr->totalRuntime + time_stamp_cyccnt;
        PrivThreadCntr->metricCntr += total_time_stamp 
                                      - PrivThreadCntr->startCntr  
                                      -( (ExceptionCntr) 
                                        - PrivThreadHelper ) 
                                      - IoT2Cntr->metricCntr;
        PrivThreadCntr->state = MEASUREMENT_FINISHED;
    }

}



void iot2ResetPrivThreadcntr(void){
    // reset privileged thread start cntr if it is currently being measured
    if (PrivThreadCntr->state == MEASUREMENT_INCOMPLETE){
        PrivThreadCntr->startCntr = IoT2Cntr->endCntr;
    }
}



void iot2svcElavatePriv(void){
    //__ASM("push {lr}\n");
    uint32_t npriv_val = __get_CONTROL() & (~CONTROL_nPRIV_Msk);
    __set_CONTROL(npriv_val);
    // call ISB to ensure the control register is updated
    __ISB();
    __DSB();
    //__ASM("pop {lr}\n");
}



void iot2svcDeElavatePriv(void){
    //__ASM("push {lr}\n");
    uint32_t npriv_val = __get_CONTROL() | CONTROL_nPRIV_Msk;
    __set_CONTROL(npriv_val);
    // call ISB to ensure the control register is updated
    __ISB();
    __DSB();
    //__ASM("pop {lr}\n");
}


void iot2svcElavateRecordPrivThread(void){
    uint32_t new_control;
    new_control = __get_CONTROL() & (~CONTROL_nPRIV_Msk);
    __ASM volatile ("MSR control, %0" : : "r" (new_control) : "memory");

}


void iot2svcDeElavateRecordPrivThread(void){
    uint32_t new_control = __get_CONTROL() | CONTROL_nPRIV_Msk;
    __ASM volatile ("MSR control, %0" : : "r" (new_control) : "memory");
}


void iot2svcDisableIrq(void){
    __disable_irq();
}


void iot2svcEnableIrq(void){
    __enable_irq();
}

/// Records the privileged code regions and validates DEP is deployed. Add this
/// function whenever the MPU is configured. Note that this function
/// assumes the MPU is enabled already. There are two variants depending
/// on the type of MPU used (e.g., ARM/Kinetis). The type of the MPU is
/// controlled through the IoT2_MPU flag at IoT2_Config.h

#if (IoT2_MPU == 1)


void iot2CheckPrivCodeAndDEP(void){

    // check if the mpu is enabled
    if ( (MPU->CTRL & MPU_CTRL_ENABLE_Msk) && (!IoT2MPU.MPU_ENABLED) ){
        IoT2MPU.MPU_ENABLED = 1;
    }
    // if DEP has been violated before no need to check again
    if (IoT2MPU.DEP){
        uint8_t num_regions = ((MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos);
        uint32_t access_permisions = 0;

        // first check if the beckground region is enabled
        if (MPU->CTRL & MPU_CTRL_PRIVDEFENA_Msk){
            IoT2MPU.BACKGROUND_REGION_ENABLED = 1;
        }
        
        // Check the MPU regions and validate DEP and record 
        // privileged code regions
        for (uint8_t i = 0; i < num_regions; i++){

            // set the region number
            MPU->RNR = i;

            // read the access permissions of the region
            access_permisions = ((MPU->RASR & MPU_RASR_AP_Msk) >> MPU_RASR_AP_Pos);

            // check if the regions is executable
            if ( (MPU->RASR & MPU_RASR_XN_Msk) == 0){
            
                // Check DEP -------------------------------------------------------------------------
                if ( (access_permisions == MPU_ARMV7_PRIV_RW) || 
                    (access_permisions == MPU_ARMV7_PRIV_RW_UNPRIV_RO) ||
                    (access_permisions == MPU_ARMV7_PRIV_RW_UNPRIV_RW)){

                    // DEP is not enabled
                    IoT2MPU.DEP = 0;
                }

            } 

        }
    }

    // add overhead if we are still collecting measurements
    if (COLLECT_IOT2_METRICS){
        IoT2Cntr->metricCntr += IOT2_REC_DEP_OVERHEAD;
    }

}

// if a different mpu
#elif (IoT2_MPU == 2)

void iot2CheckPrivCodeAndDEP(void){

}

// No support for other MPUs currently, through an error
#else

#error "[-] ERROR: invalid option for IoT2_MPU is used. Please add support to your MPU by add the iot2CheckPrivCodeAndDEP function at IoT2Lib.c, or check that you are using the correct option for IoT2MPU in IoT2_Config.h"

#endif
