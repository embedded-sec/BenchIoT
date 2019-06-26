/**
  *
  *  Portions COPYRIGHT 2017 STMicroelectronics
  *  Copyright (C) 2017, ChaN, all right reserved.
  *
  ******************************************************************************
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

//=================================================================================================
//
// This is a class to be used as an interface for the STM32479I-EVAL display. It mainly serves
// as a generic API as all the implementation is thorugh the HAL library for the board. Other
// boards can change the internals of the functions to fit each board but should maintain
// the SAME API to allow running the benchmarks on multiple targets.
//
//=================================================================================================


#ifndef IOT2_DISPLAYINTERFACE_H
#define IOT2_DISPLAYINTERFACE_H

// Check if the target is the evaluation board, otherwise through an error
#if !defined(TARGET_STM32F469NI)
#error "[-] This library is written for the STM32479I-EVAL board. Please add support to your board" \
		" or modify this error handling code if the support has been added already."
#endif	// TARGET_STM32F469NI //

//=========================================== INCLUDES ==========================================//

// mbed files
#include "mbed.h"

// board specific files
#include "stm32469i_eval_lcd.h"


//======================================== DEFINES & GLOBALS ====================================//

typedef struct RGB
{
  uint8_t B;
  uint8_t G;
  uint8_t R;
}RGB_typedef;


//========================================= FUNCTIONS ===========================================//
    
class IoT2_DisplayInterface
{
public:
	/// Constructor
	IoT2_DisplayInterface();

	/// Destructor
	~IoT2_DisplayInterface();

    /// @brief: Initialize the display
    /// @param: None
    /// @retval: 0 IF initialized correctly
    uint8_t init(void);

    /// @brief: Initialize the display using the given orientation
    /// @param: (0: Portrait, 1: Landscape, INVALID:2)
    /// @retval: 0 IF initialized correctly
    uint8_t initOrientation(LCD_OrientationTypeDef orientation);

    void reset(void);

    uint8_t getXSize(void);

    uint8_t getYSize(void);

    uint8_t setXSize(void);

    void setXSize(uint32_t imageWidthPixels);

    void setYSize(uint32_t imageHeightPixels);

    void initDefaultLayer(uint16_t LayerIndex, uint32_t FB_Address);

    void setTransparency(uint32_t LayerIndex, uint8_t Transparency);

    void setLayerAddr(uint32_t LayerIndex, uint32_t Address);

    void setColorKeying(uint32_t LayerIndex, uint32_t RGBValue);

    void resetColorKeying(uint32_t LayerIndex);

    void setLayerWindow(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos,uint16_t Width,
                         uint16_t Height);

    void selectLayer(uint32_t LayerIndex);

    void setLayerVisible(uint32_t LayerIndex, FunctionalState State);

    void setTextColor(uint32_t Color);

    uint32_t getTextColor(void);

    void setBackColor(uint32_t Color);

    uint32_t getBackColor(void);

    void setFont(sFONT *fonts);

    sFONT* getFont(void);

    uint32_t readPixel(uint16_t Xpos, uint16_t Ypos);

    void drawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t pixel);

    void clear(uint32_t Color);

    void clearStrLine(uint32_t Line);

    void displayStrAtLine(uint16_t Line, uint8_t *ptr);

    void displayStrAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text,
                                             Text_AlignModeTypdef Mode);

    void displayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii);

    void drawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length);

    void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void drawRect(uint16_t Xpos, uint16_t Ypos,uint16_t Width, uint16_t Height);

    void drawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);

    void drawPolygon(pPoint Points, uint16_t PointCoun);

    void fillEllipse(int Xpos, int Ypos, int XRadius, int YRadius);

    void drawBMP(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp);

    void turnOff(void);

    void turnOn(void);

    void config(const char* msg);

    void showBMPImg(uint8_t *rowBuff, uint32_t imgW);

    //------------------------------------------------------------------------//
    //                              data members                              //
    //------------------------------------------------------------------------//

    uint16_t xpos;
    uint16_t ypos;
    uint32_t lineCounter;
    uint16_t imgWidth;
    uint16_t imgHight;
    uint32_t colCntr;

};



#endif	// IOT2_DISPLAYINTERFACE_H //