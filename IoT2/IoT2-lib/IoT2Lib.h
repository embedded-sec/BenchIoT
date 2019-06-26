//=================================================================================================
//
// IoT2-lib: A library for functions to be generally used accross all benchmarks. Generally are 
// architecture independent (i.e. only depenends on cmsis lib) and do not use a specific board
// feature as much as possible.
//
//=================================================================================================

#ifndef IOT2LIB_H
#define IOT2LIB_H

//========================================== INCLUDES ===========================================//


#if defined(TARGET_STM32F469NI)
#include "stm32f469xx.h"

#elif defined(TARGET_K64F)
#include "MK64F12.h"

#elif defined(TARGET_DISCO_F407VG)
#include "stm32f407xx.h" // [debug-mbed]: right now the board is added manually, will be automated in the future

#else
#error "[-] ERROR: No board was defined. Please add your board to IoTLib.h or check\
 that you are using the correct macro"
#endif  // TARGET_*

#include "core_cm4.h"
#include "cmsis_gcc.h"
#include "cmsis.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif


//====================================== DEFINES & GLOBALS ======================================//


/// Macros to help transperant control the IoT2 vector table
/// IOT2_NUM_VECTORS enable a fixed size for the iot2 vector table regardless of the
/// underlying hardware. The default size is 256 to over-approximate the size
/// the vector table size, but is customizable in case a larger size is needed.
/// Note that you need to set the INITIAL_SP_VALUE macro in IoT2_Config.h
/// since this value is different depending on the underlying hardware and
/// it occupies the first index of the vector table. IMPORTANT: The last
/// index of the vector table is reserved to a special IoT2 handler.
#define IOT2_NUM_VECTORS            255
#define NUM_CORTEX_M_CORE_VECTORES  16
#define SVCall_EXCEPTION_NUM        11
#define PendSV_EXCEPTION_NUM        14
#define SYSTICK_EXCEPTION_NUM       15

/// This is the vector table iot2 uses. The setup is found in IoT2_Config.c
/// since this table is configured according to the defense.
extern const uint32_t IoT2IsrTable[IOT2_NUM_VECTORS+1];


/// Re-defining NVIC_setVector to control writes to vector table
/// the default setting to enable these unless working with mbed-uvisor (or any
/// defense that requires control of these calls).
#if !(defined(IOT2_UVISOR))
#ifdef NVIC_SetVector
#undef NVIC_SetVector
#endif
#define NVIC_SetVector __iot2_NVIC_SetVector

#ifdef NVIC_GetVector
#undef NVIC_GetVector
#endif
#define NVIC_GetVector __iot2_NVIC_GetVector

#endif // !(defined(IOT2_UVISOR))

/// In order to measure sleep cycles iot2 remaps macros using sleep instructions
#ifdef __WFI
#undef __WFI
#endif

#define __WFI()                             __iot2WFI()       // remap WFI to count sleep cycles //

#ifdef __WFE
#undef __WFE
#endif

#define __WFE()                             __iot2WFE()       // remap WFE to count sleep cycles //


//[iot2-debug]
/// We wrap the macro to use an svc then execute the cmsis function as they 
/// access privilege resources such as the SCB
/*#ifdef NVIC_GetPriorityGrouping
#undef NVIC_GetPriorityGrouping
#endif
#define NVIC_GetPriorityGrouping __iot2_NVIC_GetPriorityGrouping


#ifdef NVIC_SetPriority
#undef NVIC_SetPriority
#endif
#define NVIC_SetPriority __iot2_NVIC_SetPriority
*/
#define iot2NVIC_GetPriorityGrouping __iot2_NVIC_GetPriorityGrouping
//

/// The value below is equal to HIGHEST_CYCCNT_VALUE - OVERFLOW_SAFEGUARD_CYCLES
/// the global counter (i.e., DWT->CYCCNT) is a 32 bit counter, and will overflow
/// to 0 after that. To ensure CYCCNT does not overflow without catching it, a 
/// safe gaurd of cycles is used (i.e. OVERFLOW_SAFEGUARD_CYCLES) and we will reset
/// CYCCNT when it reaches a value higher than (HIGHEST_CYCCNT_VALUE - OVERFLOW_SAFEGUARD_CYCLES)
/// To be save, OVERFLOW_SAFEGUARD_CYCLES is set to 1e6
#define HIGHEST_CYCCNT_VALUE            4294967296      // 2^(32)
#define OVERFLOW_SAFEGUARD_CYCLES       214748364       // round(0.05 * HIGHEST_CYCCNT_VALUE)
#define IOT2_CYCCNT_OVERFLOW_LIMIT      (HIGHEST_CYCCNT_VALUE - OVERFLOW_SAFEGUARD_CYCLES)

/// Fixed overheads for iot2 functions, in cycles
#define IOT2_PRIV_THREAD_OVERHEAD_1               1874
#define IOT2_PRIV_THREAD_OVERHEAD_2               135
#define IOT2_REC_DEP_OVERHEAD                     461
#define IOT2_START_CUSTOM_METRIC_OVERHEAD_1       2433
#define IOT2_START_CUSTOM_METRIC_OVERHEAD_2       112
#define IOT2_END_CUSTOM_METRIC_OVERHEAD_1         2472
#define IOT2_END_CUSTOM_METRIC_OVERHEAD_2         98


//------------------------------------------------------------------------------
// IoT2SvcTable fixed svc indices
//------------------------------------------------------------------------------

///  This macro defines the size of IoT2SvcTable
#define IOT2_SVC_TABLE_SIZE             13

/// Reserve the defined value to be used exclusivley by IoT2
#define IOT2_RESERVED_SVC               99

/// The macros below indicate the index in the IoT2SvcTable for SVCs used by
/// IoT2Lib. The used can configure the table to their own needs but must make
/// make sure the call in the table matches the index configured below
#define IOT2_SWITCH_VEC_TABLE_SVC_NUM       0
#define IOT2_DWT_REST_SETUP_SVC_NUM         1
#define IOT2_END_MEASUREMENT_SVC_NUM        2
#define IOT2_ELAVATE_PRIV_SVC_NUM           3
#define IOT2_DE_ELAVATE_PRIV_SVC_NUM        4
#define IOT2_DISABLE_IRQ_SVC_NUM            5
#define IOT2_ENABLE_IRQ_SVC_NUM             6
#define IOT2_ELAVATE_REC_PRIV_TH_SVC_NUM    7
#define IOT2_DROP_REC_PRIV_TH_SVC_NUM       8
#define IOT2_USERSVC1_SVC_NUM               9
#define IOT2_USERSVC2_SVC_NUM               10
#define IOT2_USERSVC3_SVC_NUM               11
#define IOT2_USERSVC4_SVC_NUM               12

/// The svc handler is invoked by calling svc 99, we then use IoT2SvcNum as an 
/// index in the IoT2SvcTable to call the desired handler
extern const uint8_t IoT2SvcHandlerNum;

/// This variable is used to setup calling the correct SVC handler for IoT2 from
/// IOT2_SVC_TABLE
extern uint8_t volatile IOT2_SVC_NUM;

/// This flag is used to indicate wether the called SVC should be handled by 
/// the default handler or by IoT2, 1 = handle by IoT2, 0 = Default handler
extern volatile int IOT2_SVC_FLAG;

extern uint32_t* OldVTOR;

extern uint32_t PrivThreadStatus;

//-----------------------------------------------------------------------------------------------//

/// enum to indicate if the measurement for a specific metric has finished
enum IoT2_METRIC_STATE{
    MEASUREMENT_NOT_INITIALIZED = 0,
    MEASUREMENT_INCOMPLETE = 1,
    MEASUREMENT_FINISHED = 2
};

/// A struct to keep track of each individual metric
typedef volatile struct
{
    enum IoT2_METRIC_STATE volatile state;
    uint64_t startCntr;
    uint64_t endCntr;
    uint64_t metricCntr;
}IoT2MetricCntr;


/// A struct to track the total runtime. The number of overflows indicate
/// the captured overflows in the DWT cycle counter. The total runtime
/// is (totalRuntime + (numOverflows * 2^(32)))
typedef volatile struct 
{
    uint32_t numOverflows;
    uint64_t totalRuntime;
}IoT2RuntimeCntr;


/// This is the global structure encapsulating measurements collected by IoT2. It can be extended
/// to allow custome measurements as defined by a developer.
typedef volatile struct
{
	// Total runtime
    IoT2RuntimeCntr RuntimeCycles;
    // Core metrics
    IoT2MetricCntr ISRCycles;
    IoT2MetricCntr SVCCycles;
    IoT2MetricCntr PendSVCycles;
    IoT2MetricCntr SysTickCycles;
    IoT2MetricCntr InitCycles;
    IoT2MetricCntr IoT2Cycles;          // counter for time stamping and iot2 overhead
    // Security
    IoT2MetricCntr PrivCycles;
    IoT2MetricCntr PrivThreadCycles;
    IoT2MetricCntr UnprivCycles;
	// Performance 
    IoT2MetricCntr IOBoundCycles;
    IoT2MetricCntr CPUCycles;
    IoT2MetricCntr SleepCycles;
	IoT2MetricCntr DebugCycles;        // Additional counter for debugging
    // Throughput
    IoT2MetricCntr IoT2ThroughputCycles;   // measure the time it takes to send data
    uint32_t IoT2throughputBytes;          // throughput bytes
    float IoT2throughput;                  // final throughput

} IoT2CntrStruct;


// IoT2 global counter and metric specific counter
extern IoT2CntrStruct IoT2GlobalCounter;
extern IoT2RuntimeCntr *volatile RuntimeCntr;
extern IoT2MetricCntr *volatile ISRCntr;
extern IoT2MetricCntr *volatile SVCCntr;
extern IoT2MetricCntr *volatile PendSVCntr;
extern IoT2MetricCntr *volatile SysTickCntr;
extern IoT2MetricCntr *volatile InitCntr;
extern IoT2MetricCntr *volatile SleepCntr;
extern IoT2MetricCntr *volatile IoT2Cntr;
extern IoT2MetricCntr *volatile PrivThreadCntr;
extern IoT2MetricCntr *volatile IOBoundCntr;
extern IoT2MetricCntr *volatile CurrCntr;       // Points to current counter
extern IoT2MetricCntr *volatile PrevCntr;       // Points to previous (Preempted) counter
extern IoT2MetricCntr *volatile DebugCntr;

/// A global structure to obatain the current state of execution. There are 5 possible modes:
///     1) User Mode: Encapsulates both privileged and unperivileged thread mode
///     2) ISR: Executing any IRQ.
///     3) SVC
///     4) PendSV
///     5) SysTick
enum EXEC_MODE {
    USER_MODE,
    ISR_MODE ,
    SVC_MODE,
    PENDSV_MODE ,
    SYSTICK_MODE

};

typedef volatile struct{
    enum EXEC_MODE prevMode;
    enum EXEC_MODE currMode;
}IoT2ExecModeStrct;

extern IoT2ExecModeStrct volatile iot2ExecMode;

// used to check which fixed overhead is used in used in measurement (i.e., the
// the overhead of entry and exit instructions for iot2 not captured by 
// instrumenstation)
enum IOT2_OVERHEAD_TYPE
{
    ENTRY_OVERHEAD = 0,
    EXIT_OVERHEAD = 1
};

extern enum IOT2_OVERHEAD_TYPE iot2OverheadType;

extern uint8_t iot2_entr_exit_arr[3][2];

/// Global structure to handle shiftin the vector table
enum IoT2_VECTOR_TABLE_STATE
{
    VECTOR_TABLE_WRITABLE = 0,
    VECTOR_TABLE_LOCKED = 1
};

extern enum IoT2_VECTOR_TABLE_STATE volatile iot2VectorTableState;


enum IoT2_PROCESSOR_STATE
{
    IoT2_SLEEP = 0,
    IoT2_RUN = 1
};

extern enum IoT2_PROCESSOR_STATE volatile iot2ProcessorState;

/// The original IRQ is at offset of NVIC_NUM_VECTORS

extern void (*origIRQHandler)(void);            // Pointes to the IRQ handler to call
extern uint32_t volatile origExceptionNUM;      // holds the exception number
extern int volatile COLLECT_IOT2_METRICS;      
extern uint64_t volatile PrivThreadHelper;
extern uint64_t volatile ExceptionCntr;
extern uint32_t volatile ExceptionOverheadCntr;

//-----------------------------------------------------------------------------------------------//


enum  BENCH_IRQ_FLAG{
    BENCH_FLAG_OFF = 0,
    BENCH_FLAG_ON = 1
};

extern enum BENCH_IRQ_FLAG benchFlag;

typedef volatile struct{
    void (*benchIRQHandler)(void);              // Pointes to benchmark IRQ handler
    enum BENCH_IRQ_FLAG benchFlag;              
    uint32_t benchIRQNum;
}IoT2BenchHandlerStrct;

extern IoT2BenchHandlerStrct benchHandlerStrct;


//-----------------------------------------------------------------------------------------------//

#define IoT2_MAX_PRIV_CODE_REGS         12

// MPU Directives
#define MPU_ARMV7_XN                    (0x01UL << MPU_RASR_XN_Pos) 
#define MPU_ARMV7_PRIV_RW               (0x01UL)// << MPU_RASR_AP_Pos) 
#define MPU_ARMV7_PRIV_RW_UNPRIV_RO     (0x02UL)// << MPU_RASR_AP_Pos) 
#define MPU_ARMV7_PRIV_RW_UNPRIV_RW     (0x03UL)// << MPU_RASR_AP_Pos) 
#define MPU_ARMV7_PRIV_RO               (0x05UL)// << MPU_RASR_AP_Pos) 
#define MPU_ARMV7_PRIV_RO_UNPRIV_RO     (0x06UL)// << MPU_RASR_AP_Pos) 
#define MPU_ARMV7_PRIV_RO_UNPRIV_RO_2   (0x07UL)// << MPU_RASR_AP_Pos) 

typedef struct{
    uint32_t base_addr;
    uint32_t size;
    uint32_t access_permissions;
} IoT2MPURegion;


typedef struct{
    uint8_t MPU_ENABLED;
    uint8_t BACKGROUND_REGION_ENABLED;
    uint8_t DEP;
    IoT2MPURegion PrivRegions[IoT2_MAX_PRIV_CODE_REGS];
} IoT2MPUMetrics;

extern IoT2MPUMetrics IoT2MPU;

extern uint32_t iot2PrivState;



//========================================== FUNCTIONS ==========================================//

void iot2Setup(void);

void iot2Init(void);

///
/// 1- Set the USERSETMPEND bit in CCR so that ISR can be triggered in unprivileged mode
/// 2- Enable any needed IRQs for the current application
///
void iot2HWsetup(void);

void iot2EnableSWI(IRQn_Type IRQn);

/// @brief: Triggers an ISR from software
/// @param: Number of IRQ
void iot2TriggerSWI(IRQn_Type IRQn);

/// @brief Higher level API to trigger software interrupt for a benchmark
void iot2TriggerBenchSoftIRQ(void(*benchIrq)(), IRQn_Type benchIrqNum);


//-----------------------------------------------------------------------------------------------//
//                                      DWT related functions                                    //
//-----------------------------------------------------------------------------------------------//

/// @brief: Reset all DWT counters
/// @param: None
void iot2ResetCounters(void);

/// @brief: Enable DWT counters
/// @param: None
void iot2EnableCounters(void);

/// @brief: Setup the DWT unit counter
///
void iot2SetupCounters(void);

void iot2DisableCounters(void);

/// @brief: Collect results from the global IoT2 counter
/// @param: None
void iot2CollectResults();

void iot2EndAllMeasurements(void);

/// @brief: Function to act as ending point of all benchmarks
/// @param: None
/// @return: None
__attribute__((used)) void iot2EndBenchmark(void);

//-----------------------------------------------------------------------------------------------//
//                               IoT2 runtime measurement functions                              //
//-----------------------------------------------------------------------------------------------//

void iot2UpdateProcessorState(void);

void iot2UpdateExecMode(void);

void iot2StartRecordExceptionMetric(void);

void iot2UpdatePreemptedExceptionMetric(void);

void iot2EndRecordExceptionMetric(void);

void iot2StartCoreMetricCntr(void);

void iot2EndCoreMetricCntr(void);

void iot2EndPreemptedCoreMetricCntr(void);

void iot2EndIoT2OverheadCntr(void);

void iot2SetCoreMetricIncompleteState(void);

void iot2SetCoreMetricFinishState(void);

void iot2SetPreemptedCoreMetricIncompleteState(void);

void iot2CheckOverflow(void);

void iot2StartCustomeMetric(IoT2MetricCntr *metric);

void iot2EndCustomeMetric(IoT2MetricCntr *metric);
//-----------------------------------------------------------------------------------------------//
//                             Vector table related functions                                    //
//-----------------------------------------------------------------------------------------------//

void iot2SVCHandler(void);


void iot2ExecSVC(void);

/// @brief: Sets the vector table to the iot2 vector table 
///         configured in IoT2IsrTable
/// @param: None
void iot2SwitchVectorTable(void);


/// @brief: Trampoline ISR to record begining and end of ISR. It extracts the actual handler
///         and hands execution to it.
/// @param: None
void iot2ISRTrampoline(void);

void iot2SetIRQNum(void);


/// This function only starts recoding the expection and hands the execution to
/// the original ISR, unlike iot2ISRTrampoline, it does not traps the end
/// of the exception. It is needed for some systems that require using
/// LR without any change, thus iot2ISRTrampoline does not work since it
/// modifies LR.
void iot2StartExceptionTrampoline(void);

/// This functio only collects the measurement for the exception handler, it 
/// then returns the execution to the calling handler
void iot2EndExceptionTrampoline(void);

/// this function does nothing but call the original handler before
/// changing SCB->VTOR, it does not make any measurement.
void iot2CallOriginalHandler(void);

/// Gets the original handler and stores it at origIRQHandler
void iot2GetOriginalExcHandler(void);


/// @brief: Calls an SVC given the SVC number
/// @param: SVC number
__attribute__((always_inline)) __STATIC_INLINE void iot2CallSVC(uint8_t svc_num, uint8_t iot2SVC){
    // setup iot2 svc number
    IOT2_SVC_NUM = iot2SVC;

    __ASM volatile ("svc %[immediate]" ::[immediate] "I" (svc_num) );

}


__STATIC_INLINE void __iot2_NVIC_SetVector(IRQn_Type IRQn, uint32_t vector)
{
    
    uint32_t *vectors = (uint32_t *)SCB->VTOR;
    // If the vector table is locked, then write the shifted
    // vector table to avoid overwritting the iot2ISRTrampoline
    if (iot2VectorTableState == VECTOR_TABLE_LOCKED){
        OldVTOR[(int32_t)IRQn + NUM_CORTEX_M_CORE_VECTORES] = vector;
    }
    else{
        vectors[(int32_t)IRQn + NUM_CORTEX_M_CORE_VECTORES] = vector;
    }
    
}


__STATIC_INLINE uint32_t __iot2_NVIC_GetVector(IRQn_Type IRQn)
{
    
    uint32_t *vectors = (uint32_t *)SCB->VTOR;
    // If the vector table is locked, then read the shifted
    // vector table to avoid copying iot2ISRTrampoline
    if (iot2VectorTableState == VECTOR_TABLE_LOCKED){
        return OldVTOR[(int32_t)IRQn + NVIC_USER_IRQ_OFFSET+NVIC_NUM_VECTORS];
    }
    
    return vectors[(int32_t)IRQn + NVIC_USER_IRQ_OFFSET];
    
}


__STATIC_INLINE uint32_t __iot2_NVIC_GetPriorityGrouping(void){
    uint32_t ret = 0;
    // call an SVC to elavate privileges
    iot2CallSVC(IOT2_RESERVED_SVC,IOT2_ELAVATE_PRIV_SVC_NUM);
    ret = NVIC_GetPriorityGrouping();
    // call an svc to de-elavate privileges
    iot2CallSVC(IOT2_RESERVED_SVC,IOT2_DE_ELAVATE_PRIV_SVC_NUM);
    return ret;

}

//-----------------------------------------------------------------------------------------------//
//                                  Sleep mode related functions                                 //
//-----------------------------------------------------------------------------------------------//

// WFI re-implementation
__STATIC_INLINE void __iot2WFI(){

    iot2StartCustomeMetric(SleepCntr);
    __ASM volatile ("wfi");
    // end initialization cycles metric
    iot2EndCustomeMetric(SleepCntr);

}


// WFE re-implementation
__STATIC_INLINE void __iot2WFE(){
    iot2StartCustomeMetric(SleepCntr);
    __ASM volatile ("wfe");
    // end initialization cycles metric
    iot2EndCustomeMetric(SleepCntr);
}

//-----------------------------------------------------------------------------------------------//
//                                     MPU related functions                                     //
//-----------------------------------------------------------------------------------------------//


/// @brief: Return the current execution mode
/// @param: none
/// @returns: EXEC_MODE value
//enum EXEC_MODE iot2GetExecMode(void);

void iot2RecordPrivThread(void);

void iot2HaltPrivThreadCntr(void);

void iot2ResetPrivThreadcntr(void);

void iot2svcElavatePriv(void);

void iot2svcDeElavatePriv(void);

void iot2svcElavateRecordPrivThread(void);

void iot2svcDeElavateRecordPrivThread(void);

void iot2svcDisableIrq(void);

void iot2svcEnableIrq(void);

void iot2CheckPrivCodeAndDEP(void);




#ifdef __cplusplus
}
#endif

#endif /* IOT2LIB_H */
