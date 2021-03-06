/* mbed Microcontroller Library
 * Copyright (c) 2017, STMicroelectronics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "cmsis.h"

#if defined(TARGET_STM32F469NI)

#include "stm32f469xx.h"
#include "stm32469i_eval.h"
#include "stm32469i_eval_io.h"
#include "stm32469i_eval_camera.h"
void EXTI9_5_IRQHandler(void);

#endif




// This function is called after RAM initialization and before main.
void mbed_sdk_init()
{
    // Update the SystemCoreClock variable.
    SystemCoreClockUpdate();
    HAL_Init();

#if TARGET_STM32F7
    // Enable CPU L1-Cache
    SCB_EnableICache();
    SCB_EnableDCache();
#endif /* TARGET_STM32F7 */

    /* Configure the System clock source, PLL Multiplier and Divider factors,
       AHB/APBx prescalers and Flash settings */
    SetSysClock();
    SystemCoreClockUpdate();

    //__ASM volatile ("SVC 1\n\t");
// if we are using the evaluation board
#if defined(TARGET_STM32F469NI)
    //BSP_IO_Init();//[debug-mbed]
    //BSP_CAMERA_PwrDown(); // camera must be powered down to use the SD card 
    //BSP_IO_ConfigPin(MII_INT_PIN, IO_MODE_IT_FALLING_EDGE);
#endif 
}

#if defined(TARGET_STM32F469NI)


/**
  * @brief  This function handles External line 8 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(MFX_IRQOUT_PIN);
}
#endif 
