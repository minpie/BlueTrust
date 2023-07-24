/*
related_NeoPixel.h

NeoPixel 관련
*/
#ifndef _DEFINED_ABOUT_NEOPIXEL_
#define _DEFINED_ABOUT_NEOPIXEL_
#include <Adafruit_NeoPixel.h>

class related_NeoPixel
{
public:
    // Methods:
    related_NeoPixel(uint8_t pin_neopixel, uint8_t amount_neopixel, uint8_t brightness);
    void related_NeoPixel::StripInit(void);
    void related_NeoPixel::StripSetLED(uint8_t ledNumber, uint8_t r, uint8_t g, uint8_t b);
    void related_NeoPixel::StripOnAll(void);
    void related_NeoPixel::StripOffAll(void);

private:
    // Member Variables:
    Adafruit_NeoPixel *_neopixel;
    uint8_t _pin_neopixel;
    uint8_t _amount_neopixel;
    uint8_t _brightness;
};
#endif