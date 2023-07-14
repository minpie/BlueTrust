/* 
 *  BlueTrust.ino
 *  
 *  2023.07.14 ~
 *  
 *  V1
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
#include "related_JSON.h"


// Define:
#define AMOUNTLEDSTRIP 60 // Amount of LED in LED Strip
#define LEDBRIGHTNESS 250 // LED Strip's brightness, max: 255 min: 0
#define TEMPSENMAX 35     // Tempereture Sensor Max
#define TEMPSENMIN 15     // Tempereture Sensor Min
#define VREF 5.0          // analog reference voltage(Volt) of the ADC
#define SCOUNT 5          // sum of sample point
#define SIZESCREENWIDTH 240
#define SIZESCREENHEIGHT 320


// Function:
void SetLEDStrip(uint8_t ledNumber, uint8_t r, uint8_t g, uint8_t b); // Set (ledNumber)LED's color
void SetFanStop(void); // Set Fan to stop
void SetFanRunFull(void); // Set Fan to run at full speed
void SetFanWithSpeed(uint8_t spd); // Set Fan with target speed
int GetMedianNum(int bArray[], int iFilterLen);
float GetTDSValue(void); // Get TDS Value
void SetTimeAtCompiled(void); // Set RTC Time to Compiled time
void ReadValFromRom(void); // Read Value From EEPROM
void GetCompiledTime(void);
double GetTimeValueDOUBLE(int year, int month, int day, int hour, int minute, int second);
unsigned long long int GetTimeValueUINT(int year, int month, int day, int hour, int minute, int second);
void Callback_LEDStripBtn(void);
void OnLedStrip(void);
void OffLedStrip(void);
bool convertToJson(const String& t, JsonVariant variant);
void InterpretJsonToUi(char * text_json);

// Object:
Adafruit_NeoPixel ledStrip(AMOUNTLEDSTRIP, PINLEDSTRIP, NEO_GRB + NEO_KHZ800);
OneWire tempOneWire(PINTEMPSEN);
DallasTemperature tempSensors(&tempOneWire);
File sensorValueLogFile;
ThreeWire rtcWire(PINRTCDATA, PINRTCCLK, PINRTCRST);
RtcDS1302<ThreeWire> objRtc(rtcWire);
RtcDateTime dt;
Adafruit_ST7789 tft = Adafruit_ST7789(PINDISPLAYCS, PINDISPLAYDC, PINDISPLAYRST); // display object
related_Display Scn = related_Display(&tft, SIZESCREENWIDTH, SIZESCREENHEIGHT);

// Variable(ROM):
uint8_t isFirstRunAfterCompile = 1; // 컴파일 후 첫 실행 여부
int compileTimes[6] = {0, 0, 0, 0, 0, 0};
int compileTimesInRom[6] = {0, 0, 0, 0, 0, 0};


// Variable(about JSON):
char * TESTJSON = "{\"TemperatureBarBox\":{\"type\":\"box\",\"x1\":\"5\",\"y1\":\"70\",\"width\":\"235\",\"height\":\"10\",\"color\":\"f800\"},\"TdsBarBox\":{\"type\":\"box\",\"x1\":\"5\",\"y1\":\"100\",\"width\":\"235\",\"height\":\"10\",\"color\":\"f800\"}}";
const unsigned int capacity = 200;
DynamicJsonDocument JsonDoc(capacity);




// Variable:
uint8_t tdsVal = 0;
uint8_t panSpd = 0;
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 0;
uint8_t isUserStoppedLedStrip = 0;
double previousLedOnTimeValue = 0;
double currentTimeValue = 0;
unsigned long long int ledStripOnPeriod = (8) * (60 * 60); // LED 켤 시간(단위: 시간)
uint8_t flagLed = 1;
uint8_t waterGrade = 0;
uint8_t tdsBar;
uint8_t tempBar;
uint16_t waterGradeColor;
uint16_t BarColor;
char * waterQualityString[5] = {"clean", "good", "ordinary", "bad", "worst"};


// ### main ###
void setup(void) {
  // pin mode:
  pinMode(PINFANEN, OUTPUT); // motor driver en pin
  pinMode(PINTDSANALOG,INPUT); // TDS analog pin
  pinMode(PINLEDSWITCH, INPUT_PULLUP); // LED Strp on/off button

  // LED Strip:
  ledStrip.begin(); // INITIALIZE NeoPixel strip object (REQUIST77XX_RED)
  ledStrip.setBrightness(LEDBRIGHTNESS);
  ledStrip.clear(); // Set all pixel colors to 'off'

  // TEST:
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.begin(); 
  tft.init(SIZESCREENWIDTH, SIZESCREENHEIGHT); 
  tft.setRotation(0); // rotates the screen
  tft.fillScreen(ST77XX_WHITE); // fills the screen with black colour
  SD.begin(PINSDCARDCS); // init SD card
  objRtc.Begin();
  attachInterrupt(digitalPinToInterrupt(PINLEDSWITCH), Callback_LEDStripBtn ,FALLING); // Setting Interrupt


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

  // read values from eeprom:
  ReadValFromRom();
  // 컴파일 후 한번만 실행을 위한 처리:
  GetCompiledTime();
  unsigned long long int compiledTime = GetTimeValueUINT(compileTimes[0], compileTimes[1], compileTimes[2], compileTimes[3], compileTimes[4], compileTimes[5]);
  unsigned long long int compiledTimeRom = GetTimeValueUINT(compileTimesInRom[0], compileTimesInRom[1], compileTimesInRom[2], compileTimesInRom[3], compileTimesInRom[4] ,compileTimesInRom[5]);
  if(compiledTime > compiledTimeRom){
    isFirstRunAfterCompile = 1;
  }else{
    isFirstRunAfterCompile = 0;
  }


  // 컴파일 후 한번만 실행할 코드:
  if(isFirstRunAfterCompile){
    SetTimeAtCompiled();
    WriteRom(ADDR_COMPILE_TIME_YEAR, compileTimes[0]);
    WriteRom(ADDR_COMPILE_TIME_MONTH, compileTimes[1]);
    WriteRom(ADDR_COMPILE_TIME_DAY, compileTimes[2]);
    WriteRom(ADDR_COMPILE_TIME_HOUR, compileTimes[3]);
    WriteRom(ADDR_COMPILE_TIME_MINUTE, compileTimes[4]);
    WriteRom(ADDR_COMPILE_TIME_SECOND, compileTimes[5]);
  }



  // UI 그리기:
  InterpretJsonToUi(TESTJSON);
}
void loop(void) {
  // Sensor:
  tempSensors.requestTemperatures();
  temperature = tempSensors.getTempCByIndex(0);
  tdsVal = GetTDSValue();

  // Get RTC Time:
  dt = objRtc.GetDateTime();

  // Logging Sensor Value:
  sensorValueLogFile = SD.open("test.txt", FILE_WRITE);
  sensorValueLogFile.print(dt.Year());
  sensorValueLogFile.print("/");
  sensorValueLogFile.print(dt.Month());
  sensorValueLogFile.print("/");
  sensorValueLogFile.print(dt.Day());
  sensorValueLogFile.print(" ");
  sensorValueLogFile.print(dt.Hour());
  sensorValueLogFile.print(":");
  sensorValueLogFile.print(dt.Minute());
  sensorValueLogFile.print(":");
  sensorValueLogFile.print(dt.Second());
  sensorValueLogFile.print(" | TDS Value: ");
  sensorValueLogFile.print(tdsVal);
  sensorValueLogFile.print(" ppm | Temp: ");
  sensorValueLogFile.print(temperature);
  sensorValueLogFile.println(" C");
  sensorValueLogFile.close();



  

  // Fan Control:
  panSpd = map(constrain(temperature, TEMPSENMIN, TEMPSENMAX), TEMPSENMIN, TEMPSENMAX, 120, 255);
  SetFanWithSpeed(panSpd);


  // For LCD Screen:
  //tft.fillScreen(ST77XX_WHITE);

  /*
  ********************************
  *             시계
  * 온도(바 형태)
  * 수질(바 형태)
  ********************************
  */
  // 시계(날짜):
  Scn.DisplayFilledRect(70, 20, 70, 20, ST77XX_WHITE);
  Scn.DisplayNum(70, 20, (const uint16_t)dt.Year(), FONT_1, ST77XX_RED);
  Scn.DisplayString(96, 20, "/", FONT_1, ST77XX_RED);
  Scn.DisplayNum(104, 20, (const uint8_t)dt.Month(), FONT_1, ST77XX_RED);
  Scn.DisplayString(118, 20, "/", FONT_1, ST77XX_RED);
  Scn.DisplayNum(126, 20, (const uint8_t)dt.Day(), FONT_1, ST77XX_RED);
  // 시계(시간):
  Scn.DisplayNum(70, 32, (const uint8_t)dt.Hour(), FONT_1, ST77XX_RED);
  Scn.DisplayString(88, 32, ":", FONT_1, ST77XX_RED);
  Scn.DisplayNum(98, 32, (const uint8_t)dt.Minute(), FONT_1, ST77XX_RED);
  Scn.DisplayString(116, 32, ":", FONT_1, ST77XX_RED);
  Scn.DisplayNum(126, 32, (const uint8_t)dt.Second(), FONT_1, ST77XX_RED);


  
  // 온도:
  tempBar = map(constrain(temperature, TEMPSENMIN, TEMPSENMAX), TEMPSENMIN, TEMPSENMAX, 5, 180);
  char tempStr[25];
  sprintf(tempStr, "Water Temperature: %3d'C", (int)temperature);
  Scn.DisplayFilledRect(5, 50, 160, 8, ST77XX_WHITE);
  Scn.DisplayString(5, 50, tempStr, FONT_1, ST77XX_RED);

  
  // 수질:
  tdsBar = map(constrain(tdsVal, 0, 750), 0, 750, 5, 225);
  char tdsStr[18];
  char waterqualityStr[30];
  waterGrade = 0;
  if(tdsVal < 50){
    waterGrade = 0;
  }else if(tdsVal < 180){
    waterGrade = 1;
  }else if(tdsVal < 230){
    waterGrade = 2;
  }else if(tdsVal < 300){
    waterGrade = 3;
  }else{
    waterGrade = 4;
  }

  sprintf(tdsStr, "TDS Value: %4dppm", tdsVal);
  sprintf(waterqualityStr, "Water Quality: %s", waterQualityString[waterGrade]);

  Scn.DisplayFilledRect(5, 80, 120, 8, ST77XX_WHITE);
  Scn.DisplayString(5, 80, tdsStr, FONT_1, ST77XX_RED);


  // 수질 그림:
  switch(waterGrade){
    case 0:
      waterGradeColor = ST77XX_WHITE;
      break;
    case 1:
      waterGradeColor = ST77XX_BLUE;
      break;
    case 2:
      waterGradeColor = ST77XX_GREEN;
      break;
    case 3:
      waterGradeColor = ST77XX_YELLOW;
      break;
    case 4:
      waterGradeColor = ST77XX_RED;
      break;
  }
  Scn.SetElement(0, Scn._Elem[0].x1, Scn._Elem[0].y1, 0, 0, Scn._Elem[0].radius, 1, 0, waterGradeColor, 0);
  Scn.SetElement(1, Scn._Elem[1].x1, Scn._Elem[1].y1, tempBar, Scn._Elem[1].height, 0, 1, 0, Scn._Elem[1].color, 0);
  Scn.SetElement(2, Scn._Elem[2].x1, Scn._Elem[2].y1, tdsBar, Scn._Elem[2].height, 0, 1, 0, Scn._Elem[2].color, 0);
  Scn.SetElement(2, Scn._Elem[5].x1, Scn._Elem[5].y1, 0,0, 0, 1, (unsigned int*)&tdsVal, Scn._Elem[5].color, 0);


 // LED Strip 일정 시간만 점등:
 currentTimeValue = GetTimeValueDOUBLE(dt.Year(), dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());
 if(!isUserStoppedLedStrip){
  // 사용자가 LED를 끄지 않았으면:
  if((currentTimeValue - previousLedOnTimeValue) > (ledStripOnPeriod)){
    // 지정된 시간이 지났으면:
    // 끄고, 다음 시간에 켜기:
    OffLedStrip();
    previousLedOnTimeValue = previousLedOnTimeValue + (24 * 60 * 60);
    flagLed = 1;
  }else if(((currentTimeValue - previousLedOnTimeValue) > 0) && flagLed){
    // 지정된 시간이 지나지 않았으면:
      OnLedStrip();
      previousLedOnTimeValue = GetTimeValueDOUBLE(dt.Year(), dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());
      flagLed = 0;
  }
 }


 Scn.RunDisplayElements();

  delay(10);
}
// ### main ###


// Function:
void SetLEDStrip(uint8_t ledNumber, uint8_t r, uint8_t g, uint8_t b){
  // Set (ledNumber)LED's color
  ledStrip.setPixelColor(ledNumber, ledStrip.Color(r, g, b));
  ledStrip.show();   // Send the updated pixel colors to the hardware.
}
void SetFanStop(void){
  // Set Fan to stop
  analogWrite(PINFANEN, 0);
}
void SetFanRunFull(void){
  // Set Fan to run at full speed
  analogWrite(PINFANEN, 255);
}
void SetFanWithSpeed(uint8_t spd){
  // Set Fan with target speed
  analogWrite(PINFANEN, spd);
}


int GetMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
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

float GetTDSValue(void){
  analogBuffer[analogBufferIndex] = analogRead(PINTDSANALOG);    //read the analog value and store into the buffer
  analogBufferIndex++;
  if(analogBufferIndex == SCOUNT) 
    analogBufferIndex = 0;
  for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
    analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
  averageVoltage = GetMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; 
  // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float compensationCoefficient=1.0+0.02*(temperature-25.0);
  //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
  tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5;
  //convert voltage value to tds value
  return tdsValue;
}

void SetTimeAtCompiled(void){
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if(objRtc.GetIsWriteProtected())
    objRtc.SetIsWriteProtected(false);
  objRtc.SetIsRunning(true);
  objRtc.SetDateTime(compiled);  
}

// Read Value From EEPROM:
void ReadValFromRom(void){
  compileTimesInRom[0] = ReadRom(ADDR_COMPILE_TIME_YEAR);
  compileTimesInRom[1] = ReadRom(ADDR_COMPILE_TIME_MONTH);
  compileTimesInRom[2] = ReadRom(ADDR_COMPILE_TIME_DAY);
  compileTimesInRom[3] = ReadRom(ADDR_COMPILE_TIME_HOUR);
  compileTimesInRom[4] = ReadRom(ADDR_COMPILE_TIME_MINUTE);
  compileTimesInRom[5] = ReadRom(ADDR_COMPILE_TIME_SECOND);
}

void GetCompiledTime(void){
  char buff[11];
  int month, day, year, hour, minute, second;
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  sscanf(__DATE__, "%s %d %d", buff, &day, &year);
  sscanf(__TIME__, "%02d:%02d:%02d", &hour, &minute, &second);
  month = (strstr(month_names, buff)-month_names)/3+1;

  compileTimes[0] = year;
  compileTimes[1] = month;
  compileTimes[2] = day;
  compileTimes[3] = hour;
  compileTimes[4] = minute;
  compileTimes[5] = second;
}

double GetTimeValueDOUBLE(int year, int month, int day, int hour, int minute, int second){
  double result = 0;
  result += second;
  result += (60 * minute);
  result += (60 * 60 * hour);
  result += (60 * 60 * 24 * day);
  result += (60 * 60 * 24 * 30 * month);
  result += (60 * 60 * 24 * 30 * 12 * (year - 2000));
  return result;
}

unsigned long long int GetTimeValueUINT(int year, int month, int day, int hour, int minute, int second){
  unsigned long long int result = 0;
  result += second;
  result += (60 * minute);
  result += (60 * 60 * hour);
  result += (60 * 60 * 24 * day);
  result += (60 * 60 * 24 * 30 * month);
  result += (60 * 60 * 24 * 30 * 12 * (year - 2000));
  return result;
}

void Callback_LEDStripBtn(void){
  // LED Strip:
  if(isUserStoppedLedStrip){
    // LED 꺼짐 -> 켜짐
    dt = objRtc.GetDateTime();
    previousLedOnTimeValue = GetTimeValueDOUBLE(dt.Year(), dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());
    
    OnLedStrip();
    isUserStoppedLedStrip = 0;
  }else{
    // LED 켜짐 -> 꺼짐
    OffLedStrip();
    isUserStoppedLedStrip = 1;
  }
}

void OnLedStrip(void){
  for(uint8_t i=0; i<AMOUNTLEDSTRIP; i++){
    ledStrip.setPixelColor(i, ledStrip.Color(255, 255, 255));
  }
  ledStrip.show();
}
void OffLedStrip(void){
  for(uint8_t i=0; i<AMOUNTLEDSTRIP; i++){
    ledStrip.setPixelColor(i, ledStrip.Color(0, 0, 0));
  }
  ledStrip.show();
}


void InterpretJsonToUi(char * text_json){
  InterpretJson(&JsonDoc, text_json);
  JsonObject jsonObj = JsonDoc.as<JsonObject>();
  for(JsonPair jsonKV : jsonObj) {
    // 1 객체:
    JsonObject jsonElem = jsonKV.value().as<JsonObject>();


    String uiElemTypeTemp = jsonElem["type"]; // UI객체 타입
    char uiElemType[10];
    uiElemTypeTemp.toCharArray(uiElemType, 10);
    uint16_t x1, y1, width, height, radius, clr, fnt, update_flag;
    x1 = jsonElem.containsKey("x1") ? jsonElem["x1"] : 0;
    y1 = jsonElem.containsKey("y1") ? jsonElem["y1"] : 0;
    width = jsonElem.containsKey("width") ? jsonElem["width"] : 0;
    height = jsonElem.containsKey("height") ? jsonElem["height"] : 0;
    radius = jsonElem.containsKey("radius") ? jsonElem["radius"] : 0;
    clr = jsonElem.containsKey("color") ? jsonElem["color"] : 0;
    BarColor = jsonElem.containsKey("color-bar") ? jsonElem["color-bar"] : 0;
    fnt = jsonElem.containsKey("font") ? jsonElem["font"] : 0;
    update_flag = jsonElem.containsKey("refresh") ? jsonElem["refresh"] : 0;


    if(!strcmp(uiElemType, "1")){
      // 일반 box 그리기:
      Scn.AddElement(1, x1, y1, width, height, 0, update_flag, 0, clr, fnt);
    }else if(!strcmp(uiElemType, "100")){
      // 특수
      Scn.AddElement(100, x1, y1, 0, 0, radius, 1, 0, waterGradeColor, fnt);
    }else if(!strcmp(uiElemType, "101")){
      // 특수
      Scn.AddElement(101, x1, y1, tempBar, height, 0, 1, 0, BarColor, fnt);
    }else if(!strcmp(uiElemType, "102")){
      // 특수
      Scn.AddElement(102, x1, y1, tdsBar, height, 0, 1, 0, BarColor, fnt);
    }else if(!strcmp(uiElemType, "103")){
      // 특수
      Scn.AddElement(103, x1, y1, 0, 0, 0, 1, (unsigned int *)&temperature, clr, fnt);
    }else if(!strcmp(uiElemType, "104")){
      // 특수
      Scn.AddElement(104, x1, y1, 0, 0, 0, 1, (unsigned int *)&tdsVal, clr, fnt);
    }else if(!strcmp(uiElemType, "105")){
      // 특수
      Scn.AddElement(105, x1, y1, 0, 0, 0, 1, (unsigned int *)&tdsVal, clr, fnt);
    }else if(!strcmp(uiElemType, "106")){
      // 특수
      Scn.AddElement(106, x1, y1, 0, 0, 0, 1, (unsigned int *)(waterQualityString[waterGrade]), clr, fnt);
    }
  }
}