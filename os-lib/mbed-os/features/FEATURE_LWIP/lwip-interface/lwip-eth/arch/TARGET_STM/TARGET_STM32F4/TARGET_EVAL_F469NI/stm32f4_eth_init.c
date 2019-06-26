/**
  ******************************************************************************
  * @author  MCD Application Team
  * @brief   This file implements Ethernet network interface drivers for lwIP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */


/**
  *=============================================================================
  *
  * This file uses the setup from ST and incorporates it to follow the mbed-os
  * ethernet configuration for the STM32F469I-Eval board.(e.g., interrupt mode 
  * instead of polling).
  *
  *=============================================================================
  */


#include "stm32f4xx_hal.h"

/**
 * Override HAL Eth Init function
 */
void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    if (heth->Instance == ETH) {

        /* Enable GPIOs clocks */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOE_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOI_CLK_ENABLE();

        /** ETH GPIO Configuration
          ETH_MDIO -------------------------> PA2
          ETH_MDC --------------------------> PC1
          ETH_PPS_OUT ----------------------> PB5
          ETH_MII_RXD2 ---------------------> PH6
          ETH_MII_RXD3 ---------------------> PH7
          ETH_MII_TX_CLK -------------------> PC3
          ETH_MII_TXD2 ---------------------> PC2
          ETH_MII_TXD3 ---------------------> PE2
          ETH_MII_RX_CLK -------------------> PA1
          ETH_MII_RX_DV --------------------> PA7
          ETH_MII_RXD0 ---------------------> PC4
          ETH_MII_RXD1 ---------------------> PC5
          ETH_MII_TX_EN --------------------> PG11
          ETH_MII_TXD0 ---------------------> PG13
          ETH_MII_TXD1 ---------------------> PG14
          ETH_MII_RX_ER --------------------> PI10 (not configured)        
          ETH_MII_CRS ----------------------> PA0  (not configured)
          ETH_MII_COL ----------------------> PH3  (not configured)
         */
        
        /* Configure PA1, PA2 and PA7 */
        /* Note : on MB1165 ETH_MDIO is connected to PA2 by default (SB40 is closed) */
        GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStructure.Pull = GPIO_NOPULL; 
        GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
        GPIO_InitStructure.Pin = GPIO_PIN_0 |GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        
        /* Configure PB5 */
        GPIO_InitStructure.Pin = GPIO_PIN_5;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* Configure PE2 */
        GPIO_InitStructure.Pin = GPIO_PIN_2;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

        /* Configure PC1, PC2, PC3, PC4 and PC5 */
        /* Note : on MB1165 ETH pins PC1..5 are connected by default (bridges are closed): */
        /* PC1 (sb31), PC2 (r233), PC3 (sb54) PC4 (sb53), PC5 (sb73) */
        GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);


        /* Configure PG11, PG14 and PG13 */
        GPIO_InitStructure.Pin =  GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

        /* Configure PH6, PH7 */
        GPIO_InitStructure.Pin =  GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_7;
        HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);

        /* Enable the Ethernet global Interrupt */
        HAL_NVIC_SetPriority(ETH_IRQn, 0x7, 0);
        HAL_NVIC_EnableIRQ(ETH_IRQn);
        
        /* Enable ETHERNET clock  */
        __HAL_RCC_ETH_CLK_ENABLE();
    }
}

/**
 * Override HAL Eth DeInit function
 */
void HAL_ETH_MspDeInit(ETH_HandleTypeDef* heth)
{
    if (heth->Instance == ETH) {
        /* Peripheral clock disable */
        __HAL_RCC_ETH_CLK_DISABLE();

        /** ETH GPIO Configuration
          ETH_MDIO -------------------------> PA2
          ETH_MDC --------------------------> PC1
          ETH_PPS_OUT ----------------------> PB5
          ETH_MII_RXD2 ---------------------> PH6
          ETH_MII_RXD3 ---------------------> PH7
          ETH_MII_TX_CLK -------------------> PC3
          ETH_MII_TXD2 ---------------------> PC2
          ETH_MII_TXD3 ---------------------> PE2
          ETH_MII_RX_CLK -------------------> PA1
          ETH_MII_RX_DV --------------------> PA7
          ETH_MII_RXD0 ---------------------> PC4
          ETH_MII_RXD1 ---------------------> PC5
          ETH_MII_TX_EN --------------------> PG11
          ETH_MII_TXD0 ---------------------> PG13
          ETH_MII_TXD1 ---------------------> PG14
          ETH_MII_RX_ER --------------------> PI10 (not configured)        
          ETH_MII_CRS ----------------------> PA0  (not configured)
          ETH_MII_COL ----------------------> PH3  (not configured)
         */

        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5);
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
        HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2);
        HAL_GPIO_DeInit(GPIOG, GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14);
        HAL_GPIO_DeInit(GPIOH, GPIO_PIN_6 | GPIO_PIN_7);

        /* Disable the Ethernet global Interrupt */
        NVIC_DisableIRQ(ETH_IRQn);
    }
}