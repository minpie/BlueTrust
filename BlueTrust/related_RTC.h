/*
related_RTC.h

RTC 관련
*/
#ifndef _DEFINED_ABOUT_RTC_
#define _DEFINED_ABOUT_RTC_
#include <RtcDS1302.h>
#include <ThreeWire.h>

class related_RTC
{
public:
    // Methods:
    related_RTC(uint8_t pin_data, uint8_t pin_clk, uint8_t pin_rst);
    void related_RTC::RTCInit(void);
    void related_RTC::RTCSetTimeAtCompiled(void);
    void related_RTC::RTCGetTime(int *year, int *month, int *day, int *hour, int *minute, int *second);
    void related_RTC::RTCSetTime(int year, int month, int day, int hour, int minute, int second);

private:
    // Member Variables:
    uint8_t _pin_data;
    uint8_t _pin_clk;
    uint8_t _pin_rst;
    ThreeWire *_rtc_wire;
    RtcDS1302<ThreeWire> *_rtc_obj;
    RtcDateTime _dt;
};
#endif