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


// Check if the target is the evaluation board, otherwise through an error
#if !defined(TARGET_STM32F469NI)
#error "[-] This library is written for the STM32479I-EVAL board. Please add support to your board" \
		" or modify this error handling code if the support has been added already."
#endif	// TARGET_STM32F469NI //

//=========================================== INCLUDES ==========================================//

// Class header file
#include "IoT2_DisplayInterface.h"



//====================================== DEFINES & GLOBALS ======================================//



//========================================= FUNCTIONS ===========================================//
/// Constructor
IoT2_DisplayInterface::IoT2_DisplayInterface(){
    this->xpos = 0;
    this->ypos = 0;
    this->lineCounter = 0;
    this->imgWidth = 320;
    this->imgHight = 240;
    this->colCntr = 0;
}

/// Destructor
IoT2_DisplayInterface::~IoT2_DisplayInterface(){

}

/// This function initializes the display, the default configuration is 
/// landscape mode.
uint8_t IoT2_DisplayInterface::init(void){
    return BSP_LCD_Init();
}

/// This function initializes the display with the given orientation.
uint8_t IoT2_DisplayInterface::initOrientation(LCD_OrientationTypeDef orientation){
    return BSP_LCD_InitEx(orientation);
}

void IoT2_DisplayInterface::reset(void){
    BSP_LCD_Reset();
}

uint8_t IoT2_DisplayInterface::getXSize(void){
    return BSP_LCD_GetXSize();
}

uint8_t IoT2_DisplayInterface::getYSize(void){
    return BSP_LCD_GetYSize();
}

uint8_t IoT2_DisplayInterface::setXSize(void){
    return BSP_LCD_GetXSize();
}

void IoT2_DisplayInterface::setXSize(uint32_t imageWidthPixels){
    BSP_LCD_SetXSize(imageWidthPixels);
}

void IoT2_DisplayInterface::setYSize(uint32_t imageHeightPixels){
    BSP_LCD_SetYSize(imageHeightPixels);
}

void IoT2_DisplayInterface::initDefaultLayer(uint16_t LayerIndex, uint32_t FB_Address){
    BSP_LCD_LayerDefaultInit(LayerIndex, FB_Address);
}

void IoT2_DisplayInterface::setTransparency(uint32_t LayerIndex, uint8_t Transparency){
    BSP_LCD_SetTransparency(LayerIndex, Transparency);
}

void IoT2_DisplayInterface::setLayerAddr(uint32_t LayerIndex, uint32_t Address){
    BSP_LCD_SetLayerAddress(LayerIndex, Address);
}

void IoT2_DisplayInterface::setColorKeying(uint32_t LayerIndex, uint32_t RGBValue){
    BSP_LCD_SetColorKeying(LayerIndex, RGBValue);
}

void IoT2_DisplayInterface::resetColorKeying(uint32_t LayerIndex){
    BSP_LCD_ResetColorKeying(LayerIndex);
}

void IoT2_DisplayInterface::setLayerWindow(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos,
    uint16_t Width, uint16_t Height){

    BSP_LCD_SetLayerWindow(LayerIndex, Xpos, Ypos, Width, Height);
}

void IoT2_DisplayInterface::selectLayer(uint32_t LayerIndex){
    BSP_LCD_SelectLayer(LayerIndex);
}

void IoT2_DisplayInterface::setLayerVisible(uint32_t LayerIndex, FunctionalState State){
    BSP_LCD_SetLayerVisible( LayerIndex, State);
}

void IoT2_DisplayInterface::setTextColor(uint32_t Color){
    BSP_LCD_SetTextColor(Color);
}

uint32_t IoT2_DisplayInterface::getTextColor(void){
    return BSP_LCD_GetTextColor();
}

void IoT2_DisplayInterface::setBackColor(uint32_t Color){
    BSP_LCD_SetBackColor(Color);
}

uint32_t IoT2_DisplayInterface::getBackColor(void){
    return BSP_LCD_GetBackColor();
}

void IoT2_DisplayInterface::setFont(sFONT *fonts){
    BSP_LCD_SetFont(fonts);
}

sFONT* IoT2_DisplayInterface::getFont(void){
    return BSP_LCD_GetFont();
}

uint32_t IoT2_DisplayInterface::readPixel(uint16_t Xpos, uint16_t Ypos){
    return BSP_LCD_ReadPixel( Xpos, Ypos);
}

void IoT2_DisplayInterface::drawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t pixel){
    BSP_LCD_DrawPixel( Xpos, Ypos, pixel);
}

void IoT2_DisplayInterface::clear(uint32_t Color){
    BSP_LCD_Clear(Color);
}

void IoT2_DisplayInterface::clearStrLine(uint32_t Line){
    BSP_LCD_ClearStringLine(Line);
}

void IoT2_DisplayInterface::displayStrAtLine(uint16_t Line, uint8_t *ptr){
    BSP_LCD_DisplayStringAtLine(Line, ptr);
}

void IoT2_DisplayInterface::displayStrAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text,
                                         Text_AlignModeTypdef Mode){
    BSP_LCD_DisplayStringAt(Xpos, Ypos, Text, Mode);
}

void IoT2_DisplayInterface::displayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii){
    BSP_LCD_DisplayChar(Xpos, Ypos, Ascii);
}

void IoT2_DisplayInterface::drawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length){
    BSP_LCD_DrawHLine(Xpos, Ypos, Length);
}

void IoT2_DisplayInterface::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    BSP_LCD_DrawLine(x1, y1, x2, y2);
}

void IoT2_DisplayInterface::drawRect(uint16_t Xpos, uint16_t Ypos,uint16_t Width, uint16_t Height){
    BSP_LCD_DrawRect(Xpos, Ypos, Width, Height);
}

void IoT2_DisplayInterface::drawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius){
    BSP_LCD_DrawCircle(Xpos, Ypos, Radius);
}

void IoT2_DisplayInterface::drawPolygon(pPoint Points, uint16_t PointCount){
    BSP_LCD_DrawPolygon(Points, PointCount);
}

void IoT2_DisplayInterface::fillEllipse(int Xpos, int Ypos, int XRadius, int YRadius){
    BSP_LCD_FillEllipse(Xpos, Ypos, XRadius, YRadius);
}

void IoT2_DisplayInterface::drawBMP(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp){
    BSP_LCD_DrawBitmap(Xpos, Ypos, pbmp);
}

void IoT2_DisplayInterface::turnOff(void){
    BSP_LCD_DisplayOff();
}

void IoT2_DisplayInterface::turnOn(void){
    BSP_LCD_DisplayOn();
}

void IoT2_DisplayInterface::config(const char* msg){
    if (BSP_LCD_Init() != LCD_OK){
        // [iot2-debug]: Error occured in initalizing display, replace later
        // with error handler.
        while(1){;}
    }

    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);   
    BSP_LCD_SelectLayer(0);   

    /* Clear the LCD Background layer */
    BSP_LCD_Clear(LCD_COLOR_WHITE);

    BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

    /* Clear the LCD */
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_Clear(LCD_COLOR_WHITE);

    /* Set the LCD Text Color */
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);

    /* Display LCD messages */
    BSP_LCD_DisplayStringAt(0, 430, (uint8_t *)msg, CENTER_MODE);

    /* Compute centered position to draw on screen the decoded pixels */
    this->xpos = (uint16_t)((BSP_LCD_GetXSize() - this->imgWidth) / 2);
    this->ypos = (uint16_t)((BSP_LCD_GetYSize() - this->imgHight) / 2);
}

void IoT2_DisplayInterface::showBMPImg(uint8_t *rowBuff, uint32_t imgW){
    RGB_typedef *rgbMtx;    // [iot2-debug] rename types, fix rgb<->bgr orders
    rgbMtx = (RGB_typedef*)rowBuff;
    uint32_t dispBuff[imgW];

    for(uint32_t i = 0; i < imgW; i++){
        dispBuff[i] = (uint32_t) (
                      0xFF000000 |
                      (((uint32_t)(rgbMtx[i].B) & 0x000000FF) >> 0) |
                      (((uint32_t)(rgbMtx[i].G) & 0x000000FF) << 8) |
                      (((uint32_t)(rgbMtx[i].R) & 0x000000FF) << 16)
                      );

        this->drawPixel(( i + this->xpos), (colCntr+this->ypos), dispBuff[i]);
    }
    colCntr++;
}