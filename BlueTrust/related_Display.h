/*
related_Display.h

디스플레이 관련
*/
#ifndef _DEFINED_ABOUT_DISPLAY_
#define _DEFINED_ABOUT_DISPLAY_
#define BACKGROUND_COLOR ST77XX_WHITE
#define FONT_1 1
#define FONT_2 2
#define FONT_3 3
#include <SPI.h>
#include <Adafruit_GFX.h>    // graphics library
#include <Adafruit_ST7789.h> // library for this display

class related_Display
{
public:
    // Member Variables:
    Adafruit_ST7789 * _lcd;
    uint16_t _screen_width;
    uint16_t _screen_height;
    // Methods:
    related_Display(Adafruit_ST7789 * lcd, uint16_t screen_width, uint16_t screen_height);
    void related_Display::DisplayInit();
    void related_Display::DisplayNum(uint16_t x, uint16_t y, uint32_t num, uint16_t font, uint16_t clr);
    void related_Display::DisplayString(uint16_t x, uint16_t y, char *str, uint16_t font, uint16_t clr);
    void related_Display::DisplayRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t clr);
    void related_Display::DisplayCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t clr);
    void related_Display::DisplayFilledRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t clr);
    void related_Display::DisplayFilledCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t clr);
    void related_Display::DisplayHLine(uint16_t x, uint16_t y, uint16_t width, uint16_t clr);
    void related_Display::DisplayVLine(uint16_t x, uint16_t y, uint16_t height, uint16_t clr);
};
#endif