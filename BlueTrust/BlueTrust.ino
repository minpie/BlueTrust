/*
 *  BlueTrust.ino
 *
 *  2023.07.19
 *
 *  Rev.1
 */
// Include:
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdint.h>
#include <SD.h>
#include <RtcDS1302.h>
#include <ThreeWire.h>
#include "related_Pin.h"
#include "related_EEPROM.h"
#include "related_Display.h"
#include "images.h"

// Define:
#define AMOUNTLEDSTRIP 60 // Amount of LED in LED Strip
#define LEDBRIGHTNESS 250 // LED Strip's brightness, max: 255 min: 0
#define TEMPSENMAX 35     // Tempereture Sensor Max
#define TEMPSENMIN 15     // Tempereture Sensor Min
#define VREF 5.0          // analog reference voltage(Volt) of the ADC
#define SCOUNT 5          // sum of sample point
#define SIZESCREENWIDTH 240
#define SIZESCREENHEIGHT 320
#define SIZESCREENREFRESHRATE 4 // Display Re-draw period per minute

// Function:
void SetLEDStrip(uint8_t ledNumber, uint8_t r, uint8_t g, uint8_t b); // Set (ledNumber)LED's color
void SetFanStop(void);                                                // Set Fan to stop
void SetFanRunFull(void);                                             // Set Fan to run at full speed
void SetFanWithSpeed(uint8_t spd);                                    // Set Fan with target speed
int GetMedianNum(int bArray[], int iFilterLen);
float GetTDSValue(void);      // Get TDS Value
void SetTimeAtCompiled(void); // Set RTC Time to Compiled time
void ReadValFromRom(void);    // Read Value From EEPROM
void GetCompiledTime(void);
double GetTimeValue(int year, int month, int day, int hour, int minute, int second);
unsigned long long int GetTimeValue2(int year, int month, int day, int hour, int minute, int second);
void Callback_LEDStripBtn(void);
void OnLedStrip(void);
void OffLedStrip(void);

// Object:
Adafruit_NeoPixel ledStrip(AMOUNTLEDSTRIP, PINLEDSTRIP, NEO_GRB + NEO_KHZ800);
OneWire tempOneWire(PINTEMPSEN);
DallasTemperature tempSensors(&tempOneWire);
File sensorValueLogFile;
ThreeWire rtcWire(PINRTCDATA, PINRTCCLK, PINRTCRST);
RtcDS1302<ThreeWire> objRtc(rtcWire);
RtcDateTime dt;
Adafruit_ST7789 tft = Adafruit_ST7789(PINDISPLAYCS, PINDISPLAYDC, PINDISPLAYRST); // display object
related_Display Scn = related_Display(&tft, SIZESCREENWIDTH, SIZESCREENHEIGHT);   // display object

// Variable(ROM):
uint8_t isFirstRunAfterCompile = 1; // 컴파일 후 첫 실행 여부
int compileTimes[6] = {0, 0, 0, 0, 0, 0};
int compileTimesInRom[6] = {0, 0, 0, 0, 0, 0};

// TEST:
uint8_t tdsVal = 0;
uint8_t panSpd = 0;
int analogBuffer[SCOUNT]; // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 0;
uint8_t isUserStoppedLedStrip = 0;
double previousLedOnTimeValue = 0;
double currentTimeValue = 0;
unsigned long long int ledStripOnPeriod = (8) * (60 * 60); // LED 켤 시간(단위: 시간)
uint8_t flagLed = 1;
uint8_t waterGrade = 0;
uint8_t tmpcnt = 0;
uint8_t isCanUseSD = true;
unsigned long long int screenRefreshRate = ((60.0 / SIZESCREENREFRESHRATE) * 1000);

// Variables:
int now_year = 0;
int now_month = 0;
int now_day = 0;
int now_hour = 0;
int now_minute = 0;
int now_second = 0;
unsigned long long int curtime = 0;
unsigned long long int curtime2 = 0;

// ### main ###
void setup(void)
{
  // pin mode:
  pinMode(PINFANEN, OUTPUT);           // motor driver en pin
  pinMode(PINTDSANALOG, INPUT);        // TDS analog pin
  pinMode(PINLEDSWITCH, INPUT_PULLUP); // LED Strip on/off button

  // Display:
  Scn.DisplayInit();

  // SD Card:
  isCanUseSD = SD.begin(PINSDCARDCS); // init SD card

  // RTC:
  objRtc.Begin();

  /*
  // RTC 강제 시간지정 코드
  WriteRom(ADDR_COMPILE_TIME_YEAR, 2023);
  WriteRom(ADDR_COMPILE_TIME_MONTH, 7);
  WriteRom(ADDR_COMPILE_TIME_DAY, 3);
  WriteRom(ADDR_COMPILE_TIME_HOUR, 12);
  WriteRom(ADDR_COMPILE_TIME_MINUTE, 34);
  WriteRom(ADDR_COMPILE_TIME_SECOND, 56);
  RtcDateTime compiled = RtcDateTime("Jul 4 2023", __TIME__);
  if(objRtc.GetIsWriteProtected())
    objRtc.SetIsWriteProtected(false);
  objRtc.SetIsRunning(true);
  objRtc.SetDateTime(compiled);
  */

  // LED Strip:
  ledStrip.begin(); // INITIALIZE NeoPixel strip object (REQUIST77XX_BLACK)
  ledStrip.setBrightness(LEDBRIGHTNESS);
  ledStrip.clear(); // Set all pixel colors to 'off'
  attachInterrupt(digitalPinToInterrupt(PINLEDSWITCH), Callback_LEDStripBtn, FALLING);

  // read values from eeprom:
  ReadValFromRom();
  // 컴파일 후 한번만 실행을 위한 처리:
  GetCompiledTime();
  unsigned long long int compiledTime = GetTimeValue2(compileTimes[0], compileTimes[1], compileTimes[2], compileTimes[3], compileTimes[4], compileTimes[5]);
  unsigned long long int compiledTimeRom = GetTimeValue2(compileTimesInRom[0], compileTimesInRom[1], compileTimesInRom[2], compileTimesInRom[3], compileTimesInRom[4], compileTimesInRom[5]);
  if (compiledTime > compiledTimeRom)
  {
    isFirstRunAfterCompile = 1;
  }
  else
  {
    isFirstRunAfterCompile = 0;
  }

  // 컴파일 후 한번만 실행할 코드:
  if (isFirstRunAfterCompile)
  {
    SetTimeAtCompiled();
    WriteRom(ADDR_COMPILE_TIME_YEAR, compileTimes[0] - 2000);
    WriteRom(ADDR_COMPILE_TIME_MONTH, compileTimes[1]);
    WriteRom(ADDR_COMPILE_TIME_DAY, compileTimes[2]);
    WriteRom(ADDR_COMPILE_TIME_HOUR, compileTimes[3]);
    WriteRom(ADDR_COMPILE_TIME_MINUTE, compileTimes[4]);
    WriteRom(ADDR_COMPILE_TIME_SECOND, compileTimes[5]);
  }

  // UI 그리기:
  // 온도:
  Scn.DisplayHLine(5, 70, 235, ST77XX_BLACK);
  Scn.DisplayHLine(5, 80, 235, ST77XX_BLACK);
  Scn.DisplayVLine(5, 70, 10, ST77XX_BLACK);
  Scn.DisplayVLine(235, 70, 10, ST77XX_BLACK);
  // 수질:
  Scn.DisplayHLine(5, 100, 235, ST77XX_BLACK);
  Scn.DisplayHLine(5, 110, 235, ST77XX_BLACK);
  Scn.DisplayVLine(5, 100, 10, ST77XX_BLACK);
  Scn.DisplayVLine(235, 100, 10, ST77XX_BLACK);
}
void loop(void)
{
  // #Time:
  dt = objRtc.GetDateTime(); // Get RTC Time
  now_year = dt.Year();
  now_month = dt.Month();
  now_day = dt.Day();
  now_hour = dt.Hour();
  now_minute = dt.Minute();
  now_second = dt.Second();

  // #Sensor:
  tempSensors.requestTemperatures();
  temperature = tempSensors.getTempCByIndex(0);
  tdsVal = GetTDSValue();

  // #Calc(s):
  panSpd = map(constrain(temperature, TEMPSENMIN, TEMPSENMAX), TEMPSENMIN, TEMPSENMAX, 120, 255);                    // Calc pan speed
  uint8_t temperatureBarWidth = map(constrain(temperature, TEMPSENMIN, TEMPSENMAX), TEMPSENMIN, TEMPSENMAX, 5, 180); // Calc Temp Bar UI width
  uint8_t tdsBarWidth = map(constrain(tdsVal, 0, 750), 0, 750, 5, 225);                                              // Calc TDS Bar UI width

  // #Other operations:
  char temperatureStr[25];
  char tdsStr[18];
  char waterqualityStr[30];
  char *waterQualityString[5] = {"clean", "good", "ordinary", "bad", "worst"};

  waterGrade = 0;
  if (tdsVal < 50)
  {
    waterGrade = 0;
  }
  else if (tdsVal < 180)
  {
    waterGrade = 1;
  }
  else if (tdsVal < 230)
  {
    waterGrade = 2;
  }
  else if (tdsVal < 300)
  {
    waterGrade = 3;
  }
  else
  {
    waterGrade = 4;
  }

  sprintf(temperatureStr, "Water Temperature: %3d'C", (int)temperature);
  sprintf(tdsStr, "TDS Value: %4dppm", tdsVal);
  sprintf(waterqualityStr, "Water Quality: %s", waterQualityString[waterGrade]);

  // #Logging Sensor Value:
  if (isCanUseSD)
  {
    sensorValueLogFile = SD.open("test.txt", FILE_WRITE);
    sensorValueLogFile.print(now_year);
    sensorValueLogFile.print("/");
    sensorValueLogFile.print(now_month);
    sensorValueLogFile.print("/");
    sensorValueLogFile.print(now_day);
    sensorValueLogFile.print(" ");
    sensorValueLogFile.print(now_hour);
    sensorValueLogFile.print(":");
    sensorValueLogFile.print(now_minute);
    sensorValueLogFile.print(":");
    sensorValueLogFile.print(now_second);
    sensorValueLogFile.print(" | TDS Value: ");
    sensorValueLogFile.print(tdsVal);
    sensorValueLogFile.print(" ppm | Temp: ");
    sensorValueLogFile.print(temperature);
    sensorValueLogFile.println(" C");
    sensorValueLogFile.close();
  }

  // #Fan Control:
  SetFanWithSpeed(panSpd);

  // #LED Strip:
  // LED Strip 일정 시간만 점등:
  currentTimeValue = GetTimeValue(now_year, now_month, now_day, now_hour, now_minute, now_second);
  if (!isUserStoppedLedStrip)
  {
    // 사용자가 LED를 끄지 않았으면:
    if ((currentTimeValue - previousLedOnTimeValue) > (ledStripOnPeriod))
    {
      // 지정된 시간이 지났으면:
      // 끄고, 다음 시간에 켜기:
      OffLedStrip();
      previousLedOnTimeValue = previousLedOnTimeValue + (24 * 60 * 60);
      flagLed = 1;
    }
    else if (((currentTimeValue - previousLedOnTimeValue) > 0) && flagLed)
    {
      // 지정된 시간이 지나지 않았으면:
      OnLedStrip();
      previousLedOnTimeValue = GetTimeValue(now_year, now_month, now_day, now_hour, now_minute, now_second);
      flagLed = 0;
    }
  }

  // # 일정 주기마다 할 일:
  curtime2 = millis();
  if ((tmpcnt < 5) || ((curtime2 - curtime) > screenRefreshRate))
  {
    curtime = curtime2;
    tmpcnt++;
    if (tmpcnt > 6)
    {
      tmpcnt = 6;
    }
    // 일정 주기마다 할일:

    
    // #Display:

    // 시계(날짜):
    Scn.DisplayFilledRect(70, 20, 70, 20, ST77XX_WHITE);
    Scn.DisplayNum(70, 20, (const uint16_t)now_year, FONT_1, ST77XX_BLACK);
    Scn.DisplayString(96, 20, "/", FONT_1, ST77XX_BLACK);
    Scn.DisplayNum(104, 20, (const uint8_t)now_month, FONT_1, ST77XX_BLACK);
    Scn.DisplayString(118, 20, "/", FONT_1, ST77XX_BLACK);
    Scn.DisplayNum(126, 20, (const uint8_t)now_day, FONT_1, ST77XX_BLACK);
    // 시계(시간):
    Scn.DisplayNum(70, 32, (const uint8_t)now_hour, FONT_1, ST77XX_BLACK);
    Scn.DisplayString(88, 32, ":", FONT_1, ST77XX_BLACK);
    Scn.DisplayNum(98, 32, (const uint8_t)now_minute, FONT_1, ST77XX_BLACK);
    Scn.DisplayString(116, 32, ":", FONT_1, ST77XX_BLACK);
    Scn.DisplayNum(126, 32, (const uint8_t)now_second, FONT_1, ST77XX_BLACK);

    // 온도:
    Scn.DisplayFilledRect(5, 52, 160, 8, ST77XX_WHITE);

    Scn.DisplayString(5, 52, temperatureStr, FONT_1, ST77XX_BLACK);
    Scn.DisplayFilledRect(7, 71, temperatureBarWidth, 8, ST77XX_WHITE);
    Scn.DisplayFilledRect(7, 71, temperatureBarWidth, 8, ST77XX_BLUE);

    // 수질:

    Scn.DisplayFilledRect(7, 101, tdsBarWidth, 8, ST77XX_WHITE);
    Scn.DisplayFilledRect(5, 82, 120, 8, ST77XX_WHITE);
    Scn.DisplayFilledRect(5, 112, 160, 8, ST77XX_WHITE);
    Scn.DisplayFilledRect(7, 101, tdsBarWidth, 8, ST77XX_BLUE);
    Scn.DisplayString(5, 82, tdsStr, FONT_1, ST77XX_BLACK);
    Scn.DisplayString(5, 112, waterqualityStr, FONT_1, ST77XX_BLACK);

    // 수질 그림:
    switch (waterGrade)
    {
    case 0:
      tft.drawBitmap(40, 150, emo5, 160, 160, ST77XX_WHITE, ST77XX_BLACK);
      // tft.fillCircle(120, 230, 80, ST77XX_WHITE);
      break;
    case 1:
      tft.drawBitmap(40, 150, emo4, 160, 160, ST77XX_WHITE, ST77XX_BLUE);
      // tft.fillCircle(120, 230, 80, ST77XX_BLUE);
      break;
    case 2:
      tft.drawBitmap(40, 150, emo3, 160, 160, ST77XX_WHITE, ST77XX_GREEN);
      // tft.fillCircle(120, 230, 80, ST77XX_GREEN);
      break;
    case 3:
      tft.drawBitmap(40, 150, emo2, 160, 160, ST77XX_WHITE, ST77XX_YELLOW);
      // tft.fillCircle(120, 230, 80, ST77XX_YELLOW);
      break;
    case 4:
      tft.drawBitmap(40, 150, emo1, 160, 160, ST77XX_WHITE, ST77XX_RED);
      // tft.fillCircle(120, 230, 80, ST77XX_BLACK);
      break;
    }
    // lcd_display_num(tft, 95, 207, (const uint32_t)tdsVal, FONT_3, ST77XX_RED);
  }
  delay(10);
}
// ### main ###

// Function:
void SetLEDStrip(uint8_t ledNumber, uint8_t r, uint8_t g, uint8_t b)
{
  // Set (ledNumber)LED's color
  ledStrip.setPixelColor(ledNumber, ledStrip.Color(r, g, b));
  ledStrip.show(); // Send the updated pixel colors to the hardware.
}
void SetFanStop(void)
{
  // Set Fan to stop
  analogWrite(PINFANEN, 0);
}
void SetFanRunFull(void)
{
  // Set Fan to run at full speed
  analogWrite(PINFANEN, 255);
}
void SetFanWithSpeed(uint8_t spd)
{
  // Set Fan with target speed
  analogWrite(PINFANEN, spd);
}

int GetMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

float GetTDSValue(void)
{
  analogBuffer[analogBufferIndex] = analogRead(PINTDSANALOG); // read the analog value and store into the buffer
  analogBufferIndex++;
  if (analogBufferIndex == SCOUNT)
    analogBufferIndex = 0;
  for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
    analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
  averageVoltage = GetMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;                                                                                                  // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);                                                                                                               // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVolatge = averageVoltage / compensationCoefficient;                                                                                                            // temperature compensation
  tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; // convert voltage value to tds value
  return tdsValue;
}

void SetTimeAtCompiled(void)
{
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (objRtc.GetIsWriteProtected())
    objRtc.SetIsWriteProtected(false);
  objRtc.SetIsRunning(true);
  objRtc.SetDateTime(compiled);
}

// Read Value From EEPROM:
void ReadValFromRom(void)
{
  compileTimesInRom[0] = ReadRom(ADDR_COMPILE_TIME_YEAR);
  compileTimesInRom[1] = ReadRom(ADDR_COMPILE_TIME_MONTH);
  compileTimesInRom[2] = ReadRom(ADDR_COMPILE_TIME_DAY);
  compileTimesInRom[3] = ReadRom(ADDR_COMPILE_TIME_HOUR);
  compileTimesInRom[4] = ReadRom(ADDR_COMPILE_TIME_MINUTE);
  compileTimesInRom[5] = ReadRom(ADDR_COMPILE_TIME_SECOND);
}

void GetCompiledTime(void)
{
  char buff[11];
  int month, day, year, hour, minute, second;
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  sscanf(__DATE__, "%s %d %d", buff, &day, &year);
  sscanf(__TIME__, "%02d:%02d:%02d", &hour, &minute, &second);
  month = (strstr(month_names, buff) - month_names) / 3 + 1;

  compileTimes[0] = year;
  compileTimes[1] = month;
  compileTimes[2] = day;
  compileTimes[3] = hour;
  compileTimes[4] = minute;
  compileTimes[5] = second;
}

double GetTimeValue(int year, int month, int day, int hour, int minute, int second)
{
  double result = 0;
  result += second;
  result += (60 * minute);
  result += (60 * 60 * hour);
  result += (60 * 60 * 24 * day);
  result += (60 * 60 * 24 * 30 * month);
  result += (60 * 60 * 24 * 30 * 12 * (year - 2000));
  return result;
}

unsigned long long int GetTimeValue2(int year, int month, int day, int hour, int minute, int second)
{
  unsigned long long int result = 0;
  result += second;
  result += (60 * minute);
  result += (60 * 60 * hour);
  result += (60 * 60 * 24 * day);
  result += (60 * 60 * 24 * 30 * month);
  result += (60 * 60 * 24 * 30 * 12 * (year - 2000));
  return result;
}

void Callback_LEDStripBtn(void)
{
  // LED Strip:
  if (isUserStoppedLedStrip)
  {
    // LED 꺼짐 -> 켜짐
    dt = objRtc.GetDateTime();
    previousLedOnTimeValue = GetTimeValue(dt.Year(), dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());

    OnLedStrip();
    isUserStoppedLedStrip = 0;
  }
  else
  {
    // LED 켜짐 -> 꺼짐
    OffLedStrip();
    isUserStoppedLedStrip = 1;
  }
}

void OnLedStrip(void)
{
  for (uint8_t i = 0; i < AMOUNTLEDSTRIP; i++)
  {
    ledStrip.setPixelColor(i, ledStrip.Color(255, 255, 255));
  }
  ledStrip.show();
}
void OffLedStrip(void)
{
  for (uint8_t i = 0; i < AMOUNTLEDSTRIP; i++)
  {
    ledStrip.setPixelColor(i, ledStrip.Color(0, 0, 0));
  }
  ledStrip.show();
}