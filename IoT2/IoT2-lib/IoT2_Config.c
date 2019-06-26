//=================================================================================================
//
// This file is used to customize the configuratoin of the IoT2 runtime metric
// collector library in addition to the IoT2_Config.h file.
//
// Some defenses require customization/annotations/modifications to enable the
// metric collector library. To enable this with out modifying the IoT2Lib files
// we use this file. The default configuration is to compile this as an empty
// file (as done with the baseline for the benchmarks). If a specific 
// configuration is needed you can use or add to the available configurations.
//=================================================================================================

// IoT2 configuration and special interface files
#include "IoT2_Config.h"

#if defined(TARGET_STM32F469NI)
#include "stm32f469xx.h"
#include "stm32469i_eval.h"
#include "stm32469i_eval_io.h"
#include "stm32469i_eval_camera.h"
#endif // defined(TARGET_STM32F469NI)



//-----------------------------------------------------------------------------------------------//
//                                        SECURE_DATA                                            //
//-----------------------------------------------------------------------------------------------//

#if (IOT2_CONFIGUATION == SECURE_DATA_CONFIG)



void config_mpu(void);
void enable_secure_data_mpu(void);
void disable_secure_data_mpu(void);

#if (IoT2_OS_BENCHMARKS == 1)

//-----------------------------------------------
// OS vector table
//-----------------------------------------------

// The default configuration maps the mbed-os global structure for SVCs
/// this is the vector table used by iot2, alligned at 128 to ensure
/// bits [0-6] are zeroed, this is important to correctly update
/// SCB->VTOR since bits [0-6] are reserved
 __attribute__ ((aligned(128))) const uint32_t IoT2IsrTable[IOT2_NUM_VECTORS+1] = {
    //--------------------------------------------------------------------------
    // these are fault handlers and we do not measure these, so we call the
    // original handler
    //--------------------------------------------------------------------------
    (uint32_t)IoT2_INITIAL_SP_VALUE,          // Initial stack pointer
    (uint32_t)&iot2ISRTrampoline,       // Reset
    (uint32_t)&iot2ISRTrampoline,       // NMI
    (uint32_t)&iot2ISRTrampoline,       // Hard Fault
    (uint32_t)&iot2ISRTrampoline,       // Memmory management fault
    (uint32_t)&iot2ISRTrampoline,       // bus fault
    (uint32_t)&iot2ISRTrampoline,       // usage fault
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    //--------------------------------------------------------------------------
    (uint32_t)&iot2SVCHandler,                // SVC
    //--------------------------------------------------------------------------
    (uint32_t)&iot2ISRTrampoline,             // debug handler
    (uint32_t)&iot2ISRTrampoline,       // reserved
    //--------------------------------------------------------------------------
    // for pendSV and Systick we only start the measurement and then call
    // the original handler. The measurement is then collected by another call 
    // at the end of the handler
    (uint32_t)&iot2StartExceptionTrampoline,  // PendSv
    (uint32_t)&iot2StartExceptionTrampoline,  //Systick
    //--------------------------------------------------------------------------
    // For IRQs, use the trampoline to start/end meaurements
    [NUM_CORTEX_M_CORE_VECTORES ... IOT2_NUM_VECTORS-1] =
                                        (uint32_t) &iot2ISRTrampoline,
    //--------------------------------------------------------------------------
    // The last handler is always resever for IoT2
    (uint32_t)&iot2EndExceptionTrampoline     // Reserved for IoT2
};




#else

//-----------------------------------------------
// Baremetal vector table
//-----------------------------------------------

// The default configuration maps the mbed-os global structure for SVCs
/// this is the vector table used by iot2, alligned at 128 to ensure
/// bits [0-6] are zeroed, this is important to correctly update
/// SCB->VTOR since bits [0-6] are reserved
 __attribute__ ((aligned(128))) const uint32_t IoT2IsrTable[IOT2_NUM_VECTORS+1] = {
    //--------------------------------------------------------------------------
    // these are fault handlers and we do not measure these, so we call the
    // original handler
    //--------------------------------------------------------------------------
    (uint32_t)IoT2_INITIAL_SP_VALUE,          // Initial stack pointer
    (uint32_t)&iot2ISRTrampoline,       // Reset
    (uint32_t)&iot2ISRTrampoline,       // NMI
    (uint32_t)&iot2ISRTrampoline,       // Hard Fault
    (uint32_t)&iot2ISRTrampoline,       // Memmory management fault
    (uint32_t)&iot2ISRTrampoline,       // bus fault
    (uint32_t)&iot2ISRTrampoline,       // usage fault
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    //--------------------------------------------------------------------------
    (uint32_t)&iot2SVCHandler,                // SVC
    //--------------------------------------------------------------------------
    (uint32_t)&iot2ISRTrampoline,             // debug handler
    (uint32_t)&iot2ISRTrampoline,       // reserved
    //--------------------------------------------------------------------------
    // for pendSV and Systick we only start the measurement and then call
    // the original handler. The measurement is then collected by another call 
    // at the end of the handler
    (uint32_t)&iot2ISRTrampoline,  // PendSv
    (uint32_t)&iot2ISRTrampoline,  //Systick
    //--------------------------------------------------------------------------
    // For IRQs, use the trampoline to start/end meaurements
    [NUM_CORTEX_M_CORE_VECTORES ... IOT2_NUM_VECTORS-1] =
                                        (uint32_t) &iot2ISRTrampoline,
    //--------------------------------------------------------------------------
    // The last handler is always resever for IoT2
    (uint32_t)&iot2EndExceptionTrampoline     // Reserved for IoT2
};



#endif // #if (IoT2_OS_BENCHMARKS == 1)


void* const IoT2SvcTable[IOT2_SVC_TABLE_SIZE] = {
                                 (void *)iot2SwitchVectorTable, //IOT2_SVC_NUM=0
                                 (void *)iot2HWsetup,     //IOT2_SVC_NUM=1
                                 (void *)iot2EndAllMeasurements,//IOT2_SVC_NUM=2
                                 (void *)iot2svcElavatePriv,    //IOT2_SVC_NUM=3
                                 (void *)iot2svcDeElavatePriv,  //IOT2_SVC_NUM=4
                                 (void *)iot2svcDisableIrq,     //IOT2_SVC_NUM=5
                                 (void *)iot2svcEnableIrq,      //IOT2_SVC_NUM=6
                                 (void *)iot2svcElavateRecordPrivThread,//IOT2_SVC_NUM=7
                                 (void *)iot2svcDeElavateRecordPrivThread,//IOT2_SVC_NUM=8
                                 (void *)config_mpu,            //IOT2_SVC_NUM=9
                                 (void *)enable_secure_data_mpu,//IOT2_SVC_NUM=10
                                 (void *)disable_secure_data_mpu,//IOT2_SVC_NUM=12
                                 (void *)user_svc };            //IOT2_SVC_NUM=13





void config_mpu(void){
    MPU_Region_InitTypeDef MPU_InitStruct;
    // diable mpu
    __DMB();
    // disable faults (mem, usage, and bus)
    SCB->SHCSR &= ~(SCB_SHCSR_MEMFAULTENA_Msk| 
                  SCB_SHCSR_USGFAULTENA_Msk| 
                  SCB_SHCSR_BUSFAULTENA_Msk); 

    // disable MPU control register
    MPU->CTRL = 0;


    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x00000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;  
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;  
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    
    // RAM
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = (0x20000000UL);//RAM_ADDRESS_START;
    MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS ;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER1;//RAM_REGION_NUMBER;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  
    HAL_MPU_ConfigRegion(&MPU_InitStruct);


    // FLASH
    MPU_InitStruct.BaseAddress = (0x08000000);//FLASH_ADDRESS_START;
    MPU_InitStruct.Size = MPU_REGION_SIZE_2MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO_URO ;
    MPU_InitStruct.Number = MPU_REGION_NUMBER2;//FLASH_REGION_NUMBER;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // Peripherals
    MPU_InitStruct.BaseAddress = (0x40000000);//PERIPH_ADDRESS_START;
    MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;//PERIPH_SIZE;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.Number = MPU_REGION_NUMBER3;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  
    HAL_MPU_ConfigRegion(&MPU_InitStruct);


    // enable the MPU, will enable the background region to maintain
    // correct functionality for Mbed
    MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk| 
                  SCB_SHCSR_USGFAULTENA_Msk| 
                  SCB_SHCSR_BUSFAULTENA_Msk;
    __DSB();
    __ISB();
    // check if we need to record PrivThread cycles
    iot2RecordPrivThread();
    iot2CheckPrivCodeAndDEP();
}


void enable_secure_data_mpu(void){
    MPU_Region_InitTypeDef MPU_InitStruct;
    // diable mpu
    __DMB();
    // disable faults (mem, usage, and bus)
    SCB->SHCSR &= ~(SCB_SHCSR_MEMFAULTENA_Msk| 
                  SCB_SHCSR_USGFAULTENA_Msk| 
                  SCB_SHCSR_BUSFAULTENA_Msk); 

    // disable MPU control register
    MPU->CTRL = 0;

    // Secure data region
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = (0x10000000);//CCRAM_ADDRESS_START;
    MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
    MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RW ;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER4;//RAM_REGION_NUMBER;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);


    // enable the MPU, will enable the background region to maintain
    // correct functionality for Mbed
    MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk| 
                  SCB_SHCSR_USGFAULTENA_Msk| 
                  SCB_SHCSR_BUSFAULTENA_Msk;
    __DSB();
    __ISB();
    // check if we need to record PrivThread cycles
    iot2RecordPrivThread();
    iot2CheckPrivCodeAndDEP();

}
void disable_secure_data_mpu(void){
        MPU_Region_InitTypeDef MPU_InitStruct;
    // diable mpu
    __DMB();
    // disable faults (mem, usage, and bus)
    SCB->SHCSR &= ~(SCB_SHCSR_MEMFAULTENA_Msk| 
                  SCB_SHCSR_USGFAULTENA_Msk| 
                  SCB_SHCSR_BUSFAULTENA_Msk); 

    // disable MPU control register
    MPU->CTRL = 0;

    // Secure data region
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = (0x10000000);//CCRAM_ADDRESS_START;
    MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS ;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER4;//CCRAM_REGION_NUMBER;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);


    // enable the MPU, will enable the background region to maintain
    // correct functionality for Mbed
    MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk| 
                  SCB_SHCSR_USGFAULTENA_Msk| 
                  SCB_SHCSR_BUSFAULTENA_Msk;
    __DSB();
    __ISB();
    // check if we need to record PrivThread cycles
    iot2RecordPrivThread();
    iot2CheckPrivCodeAndDEP();
}


void user_svc(void){

}


//-----------------------------------------------------------------------------------------------//
//                                     IOT2_DEFAULT_CONFIG                                       //
//-----------------------------------------------------------------------------------------------//

#else

void user_svc(void){

}



#if (IoT2_OS_BENCHMARKS == 1)

//-----------------------------------------------
// OS vector table
//-----------------------------------------------

// The default configuration maps the mbed-os global structure for SVCs
/// this is the vector table used by iot2, alligned at 128 to ensure
/// bits [0-6] are zeroed, this is important to correctly update
/// SCB->VTOR since bits [0-6] are reserved
 __attribute__ ((aligned(128))) const uint32_t IoT2IsrTable[IOT2_NUM_VECTORS+1] = {
    //--------------------------------------------------------------------------
    // these are fault handlers and we do not measure these, so we call the
    // original handler
    //--------------------------------------------------------------------------
    (uint32_t)IoT2_INITIAL_SP_VALUE,          // Initial stack pointer
    (uint32_t)&iot2ISRTrampoline,       // Reset
    (uint32_t)&iot2ISRTrampoline,       // NMI
    (uint32_t)&iot2ISRTrampoline,       // Hard Fault
    (uint32_t)&iot2ISRTrampoline,       // Memmory management fault
    (uint32_t)&iot2ISRTrampoline,       // bus fault
    (uint32_t)&iot2ISRTrampoline,       // usage fault
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    //--------------------------------------------------------------------------
    (uint32_t)&iot2SVCHandler,                // SVC
    //--------------------------------------------------------------------------
    (uint32_t)&iot2ISRTrampoline,             // debug handler
    (uint32_t)&iot2ISRTrampoline,       // reserved
    //--------------------------------------------------------------------------
    // for pendSV and Systick we only start the measurement and then call
    // the original handler. The measurement is then collected by another call 
    // at the end of the handler
    (uint32_t)&iot2StartExceptionTrampoline,  // PendSv
    (uint32_t)&iot2StartExceptionTrampoline,  //Systick
    //--------------------------------------------------------------------------
    // For IRQs, use the trampoline to start/end meaurements
    [NUM_CORTEX_M_CORE_VECTORES ... IOT2_NUM_VECTORS-1] =
                                        (uint32_t) &iot2ISRTrampoline,
    //--------------------------------------------------------------------------
    // The last handler is always resever for IoT2
    (uint32_t)&iot2EndExceptionTrampoline     // Reserved for IoT2
};




#else

//-----------------------------------------------
// Baremetal vector table
//-----------------------------------------------

// The default configuration maps the mbed-os global structure for SVCs
/// this is the vector table used by iot2, alligned at 128 to ensure
/// bits [0-6] are zeroed, this is important to correctly update
/// SCB->VTOR since bits [0-6] are reserved
 __attribute__ ((aligned(128))) const uint32_t IoT2IsrTable[IOT2_NUM_VECTORS+1] = {
    //--------------------------------------------------------------------------
    // these are fault handlers and we do not measure these, so we call the
    // original handler
    //--------------------------------------------------------------------------
    (uint32_t)IoT2_INITIAL_SP_VALUE,          // Initial stack pointer
    (uint32_t)&iot2ISRTrampoline,       // Reset
    (uint32_t)&iot2ISRTrampoline,       // NMI
    (uint32_t)&iot2ISRTrampoline,       // Hard Fault
    (uint32_t)&iot2ISRTrampoline,       // Memmory management fault
    (uint32_t)&iot2ISRTrampoline,       // bus fault
    (uint32_t)&iot2ISRTrampoline,       // usage fault
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    (uint32_t)&iot2ISRTrampoline,       // reserved
    //--------------------------------------------------------------------------
    (uint32_t)&iot2SVCHandler,                // SVC
    //--------------------------------------------------------------------------
    (uint32_t)&iot2ISRTrampoline,             // debug handler
    (uint32_t)&iot2ISRTrampoline,       // reserved
    //--------------------------------------------------------------------------
    // for pendSV and Systick we only start the measurement and then call
    // the original handler. The measurement is then collected by another call 
    // at the end of the handler
    (uint32_t)&iot2ISRTrampoline,  // PendSv
    (uint32_t)&iot2ISRTrampoline,  //Systick
    //--------------------------------------------------------------------------
    // For IRQs, use the trampoline to start/end meaurements
    [NUM_CORTEX_M_CORE_VECTORES ... IOT2_NUM_VECTORS-1] =
                                        (uint32_t) &iot2ISRTrampoline,
    //--------------------------------------------------------------------------
    // The last handler is always resever for IoT2
    (uint32_t)&iot2EndExceptionTrampoline     // Reserved for IoT2
};



#endif // #if (IoT2_OS_BENCHMARKS == 1)





void* const IoT2SvcTable[IOT2_SVC_TABLE_SIZE] = {
                                 (void *)iot2SwitchVectorTable, //IOT2_SVC_NUM=0
                                 (void *)iot2HWsetup,     //IOT2_SVC_NUM=1
                                 (void *)iot2EndAllMeasurements,//IOT2_SVC_NUM=2
                                 (void *)iot2svcElavatePriv,    //IOT2_SVC_NUM=3
                                 (void *)iot2svcDeElavatePriv,  //IOT2_SVC_NUM=4
                                 (void *)iot2svcDisableIrq,     //IOT2_SVC_NUM=5
                                 (void *)iot2svcEnableIrq,      //IOT2_SVC_NUM=6
                                 (void *)iot2svcElavateRecordPrivThread,//IOT2_SVC_NUM=7
                                 (void *)iot2svcDeElavateRecordPrivThread,//IOT2_SVC_NUM=8
                                 (void *)user_svc,              //IOT2_SVC_NUM=9
                                 (void *)user_svc,              //IOT2_SVC_NUM=10
                                 (void *)user_svc,              //IOT2_SVC_NUM=11
                                 (void *)user_svc };            //IOT2_SVC_NUM=12



#endif  // IOT2_CONFIG_H //