
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


//=========================================== INCLUDES ==========================================//

// mbed files
#include "mbed.h"


//======================================== DEFINES & GLOBALS ====================================//

// modify structs and types to fit your need. API must be maintained to
// be protable to your target.
typedef struct RGB
{
  uint8_t B;
  uint8_t G;
  uint8_t R;
}RGB_typedef;

typedef struct myFont
{
    uint32_t strct_data;
} FONT_TYPE;

typedef enum{
    LANDSCAPE_DISPLAY = 0,
    PORTRAIT_DISPLAY = 1
} DISPLAY_ORIENTATION;


typedef enum{
    DISPLAY_ENABLED = 0,
    DISPLAY_DISABLED = 1
}DISPLAY_STATE;


typedef enum{
    TEXT_CENTER = 0,
    TEXT_RIGHT = 1
}TEXT_ALIGNMENT_TYPE;

typedef struct dispPoint {
    uint16_t x;
    uint16_t y;

} DisplayPoint;

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
    /// @param: (0: Landscape, 1: portrait, INVALID:2)
    /// @retval: 0 IF initialized correctly
    uint8_t initOrientation(DISPLAY_ORIENTATION view);

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

    void setLayerVisible(uint32_t LayerIndex, DISPLAY_STATE State);

    void setTextColor(uint32_t Color);

    uint32_t getTextColor(void);

    void setBackColor(uint32_t Color);

    uint32_t getBackColor(void);

    void setFont(FONT_TYPE *font);

    FONT_TYPE* getFont(void);

    uint32_t readPixel(uint16_t Xpos, uint16_t Ypos);

    void drawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t pixel);

    void clear(uint32_t Color);

    void clearStrLine(uint32_t Line);

    void displayStrAtLine(uint16_t Line, uint8_t *ptr);

    void displayStrAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text,
                                             TEXT_ALIGNMENT_TYPE Mode);

    void displayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii);

    void drawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length);

    void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void drawRect(uint16_t Xpos, uint16_t Ypos,uint16_t Width, uint16_t Height);

    void drawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);

    void drawPolygon(DisplayPoint* Points, uint16_t PointCount);

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