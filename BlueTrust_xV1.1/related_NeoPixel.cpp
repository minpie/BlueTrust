/*
related_NeoPixel.cpp

NeoPixel 관련
*/
#include "related_NeoPixel.h"
// Functions:
related_NeoPixel::related_NeoPixel(uint8_t pin_neopixel, uint8_t amount_neopixel, uint8_t brightness)
{
    _pin_neopixel = pin_neopixel;
    _amount_neopixel = amount_neopixel;
    _brightness = brightness;
    _neopixel = &Adafruit_NeoPixel(_amount_neopixel, _pin_neopixel, NEO_GRB + NEO_KHZ800);
}
void related_NeoPixel::StripInit(void)
{
    (*_neopixel).begin();
    (*_neopixel).setBrightness(_brightness);
    (*_neopixel).clear(); // Set all pixel colors to 'off'
}
void related_NeoPixel::StripSetLED(uint8_t ledNumber, uint8_t r, uint8_t g, uint8_t b)
{
    // Set (ledNumber)LED's color
    (*_neopixel).setPixelColor(ledNumber, (*_neopixel).Color(r, g, b));
    (*_neopixel).show(); // Send the updated pixel colors to the hardware.
}
void related_NeoPixel::StripOnAll(void)
{
    for (uint8_t i = 0; i < _amount_neopixel; i++)
    {
        (*_neopixel).setPixelColor(i, (*_neopixel).Color(255, 255, 255));
    }
    (*_neopixel).show();
}
void related_NeoPixel::StripOffAll(void)
{
    for (uint8_t i = 0; i < _amount_neopixel; i++)
    {
        (*_neopixel).setPixelColor(i, (*_neopixel).Color(0, 0, 0));
    }
    (*_neopixel).show();
}