/*
related_Display.cpp

디스플레이 관련
*/
#include "related_Display.h"
// Functions:

related_Display::related_Display(Adafruit_ST7789 * lcd, uint16_t screen_width, uint16_t screen_height)
{
  _lcd = lcd;
  _screen_width = screen_width;
  _screen_height = screen_height;
}
void related_Display::DisplayInit()
{
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.begin();
  (*_lcd).init(_screen_width, _screen_height);
  (*_lcd).setRotation(0);               // rotates the screen
  (*_lcd).fillScreen(BACKGROUND_COLOR); // fills the screen with black colour
}
void related_Display::DisplayNum(uint16_t x, uint16_t y, uint32_t num, uint16_t font, uint16_t clr)
{
  (*_lcd).setCursor(x, y);   // starts to write text at y10 x10
  (*_lcd).setTextColor(clr); // text colour to white you can use hex codes like 0xDAB420 too
  (*_lcd).setTextSize(font); // sets font size
  (*_lcd).setTextWrap(true);
  (*_lcd).print(num);
}
void related_Display::DisplayString(uint16_t x, uint16_t y, char *str, uint16_t font, uint16_t clr)
{
  (*_lcd).setCursor(x, y);   // starts to write text at y10 x10
  (*_lcd).setTextColor(clr); // text colour to white you can use hex codes like 0xDAB420 too
  (*_lcd).setTextSize(font); // sets font size
  (*_lcd).setTextWrap(true);
  (*_lcd).print(str);
}
void related_Display::DisplayRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t clr)
{
  (*_lcd).drawRect(x, y, width, height, clr);
}
void related_Display::DisplayCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t clr)
{
  (*_lcd).drawCircle(x, y, radius, clr);
}
void related_Display::DisplayFilledRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t clr)
{
  (*_lcd).fillRect(x, y, width, height, clr);
}
void related_Display::DisplayFilledCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t clr)
{
  (*_lcd).fillCircle(x, y, radius, clr);
}
void related_Display::DisplayHLine(uint16_t x, uint16_t y, uint16_t width, uint16_t clr)
{
  (*_lcd).drawFastHLine(x, y, width, clr);
}
void related_Display::DisplayVLine(uint16_t x, uint16_t y, uint16_t height, uint16_t clr)
{
  (*_lcd).drawFastVLine(x, y, height, clr);
}