
//=================================================================================================
//
// This is a class to be used as an interface for display. It mainly serves
// as a generic API as all the implementation is thorugh the HAL library for the board.
// Boards can change the internals of the functions to fit each board but should maintain
// the SAME API to allow running the benchmarks on multiple targets.
//
//=================================================================================================



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
    return 0;
}

/// This function initializes the display with the given orientation.
uint8_t IoT2_DisplayInterface::initOrientation(DISPLAY_ORIENTATION view){
    return 0;
}

void IoT2_DisplayInterface::reset(void){
    
}

uint8_t IoT2_DisplayInterface::getXSize(void){
    return 0;
}

uint8_t IoT2_DisplayInterface::getYSize(void){
    return 0;
}

uint8_t IoT2_DisplayInterface::setXSize(void){
    return 0;
}

void IoT2_DisplayInterface::setXSize(uint32_t imageWidthPixels){
    
}

void IoT2_DisplayInterface::setYSize(uint32_t imageHeightPixels){
    
}

void IoT2_DisplayInterface::initDefaultLayer(uint16_t LayerIndex, uint32_t FB_Address){
    
}

void IoT2_DisplayInterface::setTransparency(uint32_t LayerIndex, uint8_t Transparency){
    
}

void IoT2_DisplayInterface::setLayerAddr(uint32_t LayerIndex, uint32_t Address){
    
}

void IoT2_DisplayInterface::setColorKeying(uint32_t LayerIndex, uint32_t RGBValue){
    
}

void IoT2_DisplayInterface::resetColorKeying(uint32_t LayerIndex){
    
}

void IoT2_DisplayInterface::setLayerWindow(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos,
    uint16_t Width, uint16_t Height){

}

void IoT2_DisplayInterface::selectLayer(uint32_t LayerIndex){
    
}

void IoT2_DisplayInterface::setLayerVisible(uint32_t LayerIndex, DISPLAY_STATE State){
    
}

void IoT2_DisplayInterface::setTextColor(uint32_t Color){
   
}

uint32_t IoT2_DisplayInterface::getTextColor(void){
    return 0;
}

void IoT2_DisplayInterface::setBackColor(uint32_t Color){
    
}

uint32_t IoT2_DisplayInterface::getBackColor(void){
    return 0;
}

void IoT2_DisplayInterface::setFont(FONT_TYPE *font){
    
}

FONT_TYPE* IoT2_DisplayInterface::getFont(void){
    FONT_TYPE font = {0};
    return &font;
}

uint32_t IoT2_DisplayInterface::readPixel(uint16_t Xpos, uint16_t Ypos){
    return 0;
}

void IoT2_DisplayInterface::drawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t pixel){
    
}

void IoT2_DisplayInterface::clear(uint32_t Color){
    
}

void IoT2_DisplayInterface::clearStrLine(uint32_t Line){
    
}

void IoT2_DisplayInterface::displayStrAtLine(uint16_t Line, uint8_t *ptr){
    
}

void IoT2_DisplayInterface::displayStrAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text,
                                         TEXT_ALIGNMENT_TYPE Mode){
    
}

void IoT2_DisplayInterface::displayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii){
    
}

void IoT2_DisplayInterface::drawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length){
    
}

void IoT2_DisplayInterface::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    
}

void IoT2_DisplayInterface::drawRect(uint16_t Xpos, uint16_t Ypos,uint16_t Width, uint16_t Height){
    
}

void IoT2_DisplayInterface::drawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius){
    
}

void IoT2_DisplayInterface::drawPolygon(DisplayPoint* Points, uint16_t PointCount){
    
}

void IoT2_DisplayInterface::fillEllipse(int Xpos, int Ypos, int XRadius, int YRadius){
    
}

void IoT2_DisplayInterface::drawBMP(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp){
    
}

void IoT2_DisplayInterface::turnOff(void){
    
}

void IoT2_DisplayInterface::turnOn(void){
    
}

void IoT2_DisplayInterface::config(const char* msg){
    
}

void IoT2_DisplayInterface::showBMPImg(uint8_t *rowBuff, uint32_t imgW){

}