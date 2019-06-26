//=================================================================================================
//
// This file is used to  configure IoT2 benchmarks and evaluation runtime library.
// Also it configures the  the IRQn numbers on one place. IRQn are different depending
// on target board (both the number and the name can be different). We use a unified
// naming convention for the IoT^2 benchmarks by mapping each IRQn number of the board
// to the defined macro for IoT2. That way to add an additional target only the IoT2
// directives are mapped to new target in order to run the benchmarks.
//
// Note that IoT2_Config.c is an optional file, while the IoT2_Config.h is
// always required. Check IoT2_Config.c to learn how to use it.
//=================================================================================================


#ifndef IOT2_CONFIG_H
#define IOT2_CONFIG_H

#include "IoT2Lib.h"

//-----------------------------------------------------------------------------------------------//
//                                     IoT2 Configuration                                        //
//-----------------------------------------------------------------------------------------------//

/// The below macros are used to configure IoT2_Config.c correctly depending on 
/// macro used for IOT2_CONFIGUATION. The default configuration is
/// the one used for the baseline benchmarks. If 
/// none of the option suit your need then you can add a macro and extend 
/// IoT2_Config.c to work as you wish. If you need to link additional files as 
/// a part of your changes for IoT2_Config.c then you might also 
/// look at IOT2LIB.mk to customize the compilation process as well.

#define IOT2_DEFAULT_CONFIG            0
#define SECURE_DATA_CONFIG             1


/// macro used to configure IoT2_Config.c to customize the metric collector 
/// runtime library. This is supposed to be set automatically through IOT2LIB.mk
//#if (!defined(IOT2_CONFIGUATION))
//#define IOT2_CONFIGUATION       IOT2_DEFAULT_CONFIG
//#endif     


//-----------------------------------------------------------------------------------------------//
//                                        GLOBAL IoT2                                            //
//-----------------------------------------------------------------------------------------------//

/// Prints help messages for debugging using iot2SerialDebug. Set to 0 to turn off
#define IoT2_DEBUG          1

/// This message indicates the begining of the benchmark. The automated serial
/// terminal and tcp driver use this to synchronize the benchmarking process.
#define IoT2_START_BENCHMARKING "[IoT2] benchmark: START"

/// Use this message to signal the end of the benchmark to the
/// benchmark tcp driver
#define IoT2_TCP_DRIVER_END_MSG "[IoT2] END_NETWORK_DRIVER"

/// Used to set the time for the RTC
#define IoT2_RTC_SECONDS    1525055233

/// Macro to specifiy running the benchmarks as bare-metal/with an OS.
/// the default is running the benchmarks with an OS.
/// 1: OS, 0: Bare-metal
///#define IoT2_OS_BENCHMARKS      0 // moved IOT2LIB.mk

/// Network setup for the benchmarks
#define BOARD_IP                "192.168.0.10"    // board ip
#define GATEWAY                 "192.168.0.1"     // gatway for board
#define NETMASK                 "255.255.255.0"           
#define IoT2_REMOTE_IP          "192.168.0.11"    // Remote IP to send benchmark results
#define IoT2_PORT                1337             // port to send benchmark results and metrics

/// Specify the type of MPU to use. Choose 1 for ARM_MPU or 2 for Other MPUs
#define IoT2_MPU    1                             // Options are: ARM_MPU=1, Other MPUs=2

/// this is an empty function to be used as a place holder in case an svc is not
/// used in the IoT2SvcTable
void user_svc(void);

extern void* const IoT2SvcTable[IOT2_SVC_TABLE_SIZE];


#if (IOT2_CONFIGUATION == SECURE_WATCHDOG_CONFIG)

#define WATCHDOG_APPLICATION_ADDR   0x08000000

#elif (IOT2_CONFIGUATION == SECURE_DATA_CONFIG)

#define CONFIG_MPU_SVC_NUM            IOT2_USERSVC1_SVC_NUM 
#define ENABLE_SECURE_DATA_SVC_NUM    IOT2_USERSVC2_SVC_NUM 
#define DISABLE_SECURE_DATA_SVC_NUM   IOT2_USERSVC3_SVC_NUM 

#endif


//-----------------------------------------------------------------------------------------------//
//                                        EVAL_F469NI                                            //
//-----------------------------------------------------------------------------------------------//


#if defined(TARGET_STM32F469NI)
#include "stm32f469xx.h"

/// The initial Stack pointer value according to the hardware
#define IoT2_INITIAL_SP_VALUE   0x20050000U


#define IoT2_PROCESSOR_SPEED    180000000

/// IRQn macros
#define IoT2_USART1_IRQ             USART1_IRQn     // USART interrupt ID in vector table
#define IoT2_USER_BUTTON_IRQ        EXTI15_10_IRQn  // Button interrupt ID in vector table
#define IOT2_MOTION_DETECTOR_IRQ    EXTI4_IRQn      // used for motion detector IRQ, can be any
                                                    // any general GPIO IRQ

/// metrics macros
#define IoT2_GREEN_LED          LED1            // LED used to indicate benchmark success
#define IoT2_ERROR_LED          LED3            // LED used to indicate benchmark failure
#define IoT2_ENERGY_GPIO        PB_14           // GPIO to measure energy
#define IoT2_GENERAL_LED        LED2

//-----------------------------------------------------------------------------------------------//
//                                          K64F                                                 //
//-----------------------------------------------------------------------------------------------//


#elif defined(TARGET_K64F)
#include "MK64F12.h"

#define IoT2_INITIAL_SP_VALUE   0x20030000U

#define IoT2_PROCESSOR_SPEED    120000000

/// IRQn macros
#define IoT2_USART1_IRQ         UART0_RX_TX_IRQn// USART interrupt ID in vector table
#define IoT2_USER_BUTTON_IRQ    PORTC_IRQn      // Button interrupt ID in vector table
#define IOT2_MOTION_DETECTOR_IRQ     PORTC_IRQn      // used for motion detector IRQ, can be any
/// metrics macros
#define IoT2_GREEN_LED          LED_GREEN       // LED used to indicate benchmark success
#define IoT2_ERROR_LED          LED_RED         // LED used to indicate benchmark failure
#define IoT2_ENERGY_GPIO        PTB11           // GPIO to measure energy
#define IoT2_GENERAL_LED        LED3


//-----------------------------------------------------------------------------------------------//
//                                      DISCO_F407VG                                             //
//-----------------------------------------------------------------------------------------------//


#elif defined(TARGET_DISCO_F407VG)
#include "stm32f407xx.h" 

/// The initial Stack pointer value according to the hardware
#define IoT2_INITIAL_SP_VALUE   0x20020000U //[iot2-debug]
#define IoT2_PROCESSOR_SPEED    80000000   //[iot2-debug]
/// metrics macros
#define IoT2_GREEN_LED          LED1
#define IoT2_ERROR_LED          LED3            // LED used to indicate benchmark failure
#define IoT2_ENERGY_GPIO        PB_14
#define IoT2_GENERAL_LED        LED2

//-----------------------------------------------------------------------------------------------//
#else
#error "[-] ERROR: No board was defined. Please add your board to IoT2_Config.h\
 or check that you are using the correct macro"
#endif  // TARGET_*





//========================================= FUNCTIONS ===========================================//

#endif  // IOT2_CONFIG_H //