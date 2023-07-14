/*
related_Display.h

디스플레이 관련
*/
#ifndef _DEFINED_ABOUT_DISPLAY_
#define _DEFINED_ABOUT_DISPLAY_
#define MAX_UI_ELEMENTS 64
#define BACKGROUND_COLOR ST77XX_WHITE
#define FONT_1 1
#define FONT_2 2
#define FONT_3 3
#include <SPI.h>
#include <Adafruit_GFX.h> // graphics library
#include <Adafruit_ST7789.h> // library for this display

typedef struct UiElementType{
    uint8_t type;
    /*
    type = 
    1: 속이 빈 Box
    100: 특수, 속이 채워진 원, 수질 표시용
    101: 특수, 속이 채워진 사각형, 온도 바
    102: 특수, 속이 채워진 사각형, TDS 바
    103: 특수, 문자열, 온도 수치
    104: 특수, 문자열, TDS 수치
    105: 특수, 문자열, TDS 수치(대형)
    10: 특수, 문자열, 수질 안내문
    */
    uint8_t is_need_continuously_update; // 지속적으로 새로 그려져야 하는지 여부
    uint16_t x1;
    uint16_t y1;
    uint16_t width;
    uint16_t height;
    uint8_t radius;
    uint16_t color;
    uint8_t font;
    unsigned int * context_addr; // 출력할 내용물(있으면) 주소
}UiElement;


class related_Display{
    public:
    // Member Variables:
    Adafruit_ST7789* _lcd;
    uint16_t _screen_width;
    uint16_t _screen_height;
    UiElement _Elem[MAX_UI_ELEMENTS];
    uint16_t _usage_Elem;
    uint8_t _once_draw;
    // Methods:
    related_Display(Adafruit_ST7789 * lcd, uint16_t screen_width, uint16_t screen_height);
    void related_Display::DisplayNum(uint16_t x, uint16_t y, uint32_t num, uint16_t font, uint16_t clr);
    void related_Display::DisplayString(uint16_t x, uint16_t y, char * str, uint16_t font, uint16_t clr);
    void related_Display::DisplayRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t clr);
    void related_Display::DisplayCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t clr);
    void related_Display::DisplayFilledRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t clr);
    void related_Display::DisplayFilledCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t clr);
    void related_Display::DisplayHLine(uint16_t x, uint16_t y, uint16_t width, uint16_t clr);
    void related_Display::DisplayVLine(uint16_t x, uint16_t y, uint16_t height, uint16_t clr);
    uint16_t related_Display::AddElement(uint8_t typ, uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, uint16_t radi, uint8_t flag_update, unsigned int * ctext_addr, uint16_t clr, uint8_t fnt);
    void related_Display::SetElement(uint16_t elem_addr, uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, uint16_t radi, uint8_t flag_update, unsigned int * ctext_addr, uint16_t clr, uint8_t fnt);
    void related_Display::RunDisplayElements(void);
};
#endif