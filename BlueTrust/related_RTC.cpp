/*
related_RTC.cpp

RTC 관련
*/
#include "related_RTC.h"
// Functions:
related_RTC::related_RTC(uint8_t pin_data, uint8_t pin_clk, uint8_t pin_rst)
{
    _pin_data = pin_data;
    _pin_clk = pin_clk;
    _pin_rst = pin_rst;
    _rtc_wire = &ThreeWire(_pin_data, _pin_clk, _pin_rst);
    _rtc_obj = &RtcDS1302<ThreeWire>(*_rtc_wire);
}
void related_RTC::RTCInit(void)
{
    (*_rtc_obj).Begin();
}
void related_RTC::RTCSetTimeAtCompiled(void)
{
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    if ((*_rtc_obj).GetIsWriteProtected())
        (*_rtc_obj).SetIsWriteProtected(false);
    (*_rtc_obj).SetIsRunning(true);
    (*_rtc_obj).SetDateTime(compiled);
}
void related_RTC::RTCSetTime(int year, int month, int day, int hour, int minute, int second)
{
    char dat[12];
    char tim[9];

    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char buff[4] = "";
    strncpy(buff, month_names[(month - 1) * 3], 3);
    sprintf(dat, "%s %d %d", buff, day, year);
    sprintf(tim, "%02d:%02d:%02d", hour, minute, second);

    RtcDateTime target = RtcDateTime(__DATE__, "12:34:56");
    if ((*_rtc_obj).GetIsWriteProtected())
        (*_rtc_obj).SetIsWriteProtected(false);
    (*_rtc_obj).SetIsRunning(true);
    (*_rtc_obj).SetDateTime(target);
}
void related_RTC::RTCGetTime(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
    _dt = (*_rtc_obj).GetDateTime();
    *year = (_dt.Year());
    *month = (_dt.Month());
    *day = (_dt.Day());
    *hour = (_dt.Hour());
    *minute = (_dt.Minute());
    *second = (_dt.Second());
}