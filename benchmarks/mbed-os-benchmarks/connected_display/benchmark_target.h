//=================================================================================================
//
// This file is used to include any board specific file or macro. It is included in each benchmark
// header file. This way the benchmark files remain the same (or any needed changes are minimized)
// when using different targets.
//
//=================================================================================================


#ifndef BENCHMARK_TARGET_H
#define BENCHMARK_TARGET_H


//=========================================== INCLUDES ==========================================//

// board specific files
#if defined(TARGET_STM32F469NI)

#include "stm32f4xx_hal.h"
#include "stm32469i_eval.h"
#include "stm32469i_eval_io.h"
#include "stm32469i_eval_sdram.h"
#include "stm32469i_eval_camera.h"

#define COLOR_WHITE     LCD_COLOR_WHITE
#define DISPLAY_XPOS    0
#define DISPLAY_YPOS    430
#define DISPLAY_MODE    CENTER_MODE

#elif defined(TARGET_K64F)
#include "MK64F12.h"

#define COLOR_WHITE     0
#define DISPLAY_XPOS    0
#define DISPLAY_YPOS    0
#define DISPLAY_MODE    TEXT_CENTER


#else
#error "[-] ERROR: No board was defined. Please add your board to benchmark_target.h\
 or check that you are using the correct macro"


#endif  // TARGET_*


//======================================== DEFINES & GLOBALS ====================================//



//========================================= FUNCTIONS ===========================================//

#endif  // BENCHMARK_TARGET_H //