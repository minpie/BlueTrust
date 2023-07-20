/*
related_Display.cpp

디스플레이 관련
*/
#include "related_Display.h"
// Functions:

related_Display::related_Display(Adafruit_ST7789 * lcd,uint16_t screen_width, uint16_t screen_height){
  _lcd = lcd;
  _screen_width = screen_width;
  _screen_height = screen_height;
  _usage_Elem = 0;
  _once_draw = 1;

}
void related_Display::DisplayNum(uint16_t x, uint16_t y, uint32_t num, uint16_t font, uint16_t clr){
  (*_lcd).setCursor(x, y); // starts to write text at y10 x10
  (*_lcd).setTextColor(clr); // text colour to white you can use hex codes like 0xDAB420 too
  (*_lcd).setTextSize(font); // sets font size
  (*_lcd).setTextWrap(true);
  (*_lcd).print(num);
}
void related_Display::DisplayString(uint16_t x, uint16_t y, char * str, uint16_t font, uint16_t clr){
  (*_lcd).setCursor(x, y); // starts to write text at y10 x10
  (*_lcd).setTextColor(clr); // text colour to white you can use hex codes like 0xDAB420 too
  (*_lcd).setTextSize(font); // sets font size
  (*_lcd).setTextWrap(true);
  (*_lcd).print(str);
}
void related_Display::DisplayRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t clr){
  (*_lcd).drawRect(x, y, width, height, clr);
}
void related_Display::DisplayCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t clr){
  (*_lcd).drawCircle(x, y, radius, clr);
}
void related_Display::DisplayFilledRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t clr){
  (*_lcd).fillRect(x, y, width, height, clr);
}
void related_Display::DisplayFilledCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t clr){
  (*_lcd).fillCircle(x, y, radius, clr);
}
void related_Display::DisplayHLine(uint16_t x, uint16_t y, uint16_t width, uint16_t clr){
  (*_lcd).drawFastHLine(x, y, width, clr);
}
void related_Display::DisplayVLine(uint16_t x, uint16_t y, uint16_t height, uint16_t clr){
  (*_lcd).drawFastVLine(x, y, height, clr);
}
uint16_t related_Display::AddElement(uint8_t typ, uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, uint16_t radi, uint8_t flag_update, char *ctext_addr, uint16_t clr, uint8_t fnt){
  _Elem[_usage_Elem].is_need_continuously_update = flag_update;
  _Elem[_usage_Elem].context_addr = ctext_addr;
  _Elem[_usage_Elem].x1 = x;
  _Elem[_usage_Elem].y1 = y;
  _Elem[_usage_Elem].width = wid;
  _Elem[_usage_Elem].height = hei;
  _Elem[_usage_Elem].radius = radi;
  _Elem[_usage_Elem].type = typ;
  _Elem[_usage_Elem].color = clr;
  _Elem[_usage_Elem].font = fnt;
  _usage_Elem++;
  return (_usage_Elem-1);
}
void related_Display::SetElement(uint16_t elem_addr, uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, uint16_t radi, uint8_t flag_update, char * ctext_addr, uint16_t clr, uint8_t fnt){
  _Elem[elem_addr].is_need_continuously_update = flag_update;
  _Elem[elem_addr].context_addr = ctext_addr;
  _Elem[elem_addr].x1 = x;
  _Elem[elem_addr].y1 = y;
  _Elem[elem_addr].width = wid;
  _Elem[elem_addr].height = hei;
  _Elem[elem_addr].radius = radi;
  _Elem[elem_addr].color = clr;
  _Elem[elem_addr].font = fnt;
}
void related_Display::RunDisplayElements(void){
  for(uint16_t i=0; i<_usage_Elem; i++){
    if((_once_draw) || (_Elem[i].is_need_continuously_update)){
      // 계속 그려야 하는 요소 발견 또는 첫 출력인경우:
      switch(_Elem[i].type){
        case 1:
          DisplayRect(_Elem[i].x1, _Elem[i].y1, _Elem[i].width, _Elem[i].height, _Elem[i].color);
          break;
        case 2:
          DisplayString(_Elem[i].x1, _Elem[i].y1, (_Elem[i].context_addr), _Elem[i].font, _Elem[i].color);
          break;
        case 100:
          DisplayFilledCircle(_Elem[i].x1, _Elem[i].y1, _Elem[i].radius, _Elem[i].color);
          break;
        case 101:
        case 102:
          DisplayFilledRect(_Elem[i].x1, _Elem[i].y1, _Elem[i].width, _Elem[i].height, _Elem[i].color);
          break;
        case 103:
        case 104:
        case 105:
          DisplayNum(_Elem[i].x1, _Elem[i].y1, (_Elem[i].context_addr), _Elem[i].font, _Elem[i].color);
          break;
        case 106:
        case 107:
        case 108:
          DisplayString(_Elem[i].x1, _Elem[i].y1, (_Elem[i].context_addr), _Elem[i].font, _Elem[i].color);
          break;
      }
    }
  }
  if(_once_draw){
    _once_draw = 0;
  }
}
void related_Display::ClearDisplayElements(void){
  for(uint16_t i=0; i<_usage_Elem; i++){
    if(_Elem[i].is_need_continuously_update){
      switch(_Elem[i].type){
        case 1:
          DisplayRect(_Elem[i].x1, _Elem[i].y1, _Elem[i].width, _Elem[i].height, BACKGROUND_COLOR);
          break;
        case 2:
          DisplayString(_Elem[i].x1, _Elem[i].y1, (_Elem[i].context_addr), _Elem[i].font, BACKGROUND_COLOR);
          break;
        case 100:
          DisplayFilledCircle(_Elem[i].x1, _Elem[i].y1, _Elem[i].radius, BACKGROUND_COLOR);
          break;
        case 101:
        case 102:
          DisplayFilledRect(_Elem[i].x1, _Elem[i].y1, _Elem[i].width, _Elem[i].height, BACKGROUND_COLOR);
          break;
        case 103:
        case 104:
        case 105:
          DisplayNum(_Elem[i].x1, _Elem[i].y1, (_Elem[i].context_addr), _Elem[i].font, BACKGROUND_COLOR);
          break;
        case 106:
        case 107:
        case 108:
          DisplayString(_Elem[i].x1, _Elem[i].y1, (_Elem[i].context_addr), _Elem[i].font, BACKGROUND_COLOR);
          break;
      }
    }
  }
}