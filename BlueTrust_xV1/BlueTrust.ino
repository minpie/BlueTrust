/* 
 *  BlueTrust.ino
 *  
 *  2023.07.14 ~ 2023.07.19
 *  
 *  xV1
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

char TESTJSON[] = "{\"DateTimeWhiteBox\":{\"type\":102,\"x1\":70,\"y1\":20,\"width\":90,\"height\":20,\"color\":65535,\"refresh\":1},\"DateStr\":{\"type\":107,\"x1\":70,\"y1\":20,\"color\":63488,\"font\":1,\"text\":\"????/??/??\",\"refresh\":1},\"TimeStr\":{\"type\":108,\"x1\":70,\"y1\":32,\"color\":63488,\"font\":1,\"text\":\"??:??:??\",\"refresh\":1},\"WaterQualityFilledCircle\":{\"type\":100,\"x1\":120,\"y1\":230,\"radius\":80,\"color\":63488,\"refresh\":0},\"TemperatureBar\":{\"type\":101,\"x1\":7,\"y1\":71,\"height\":8,\"color-bar\":31,\"refresh\":0},\"TdsBar\":{\"type\":102,\"x1\":7,\"y1\":101,\"height\":8,\"color-bar\":31,\"refresh\":0},\"TemperatureValue\":{\"type\":103,\"x1\":120,\"y1\":50,\"color\":63488,\"font\":1,\"refresh\":0},\"TdsValue\":{\"type\":104,\"x1\":100,\"y1\":80,\"color\":63488,\"font\":1,\"refresh\":0},\"TdsValueBig\":{\"type\":105,\"x1\":95,\"y1\":207,\"color\":63488,\"font\":3,\"refresh\":0},\"WaterQualityString\":{\"type\":106,\"x1\":100,\"y1\":110,\"color\":63488,\"font\":3,\"refresh\":0},\"TemperatureBarBox\":{\"type\":1,\"x1\":5,\"y1\":70,\"width\":235,\"height\":10,\"color\":27469,\"refresh\":0},\"TdsBarBox\":{\"type\":1,\"x1\":5,\"y1\":100,\"width\":235,\"height\":10,\"color\":27469,\"refresh\":0},\"Str1\":{\"type\":2,\"x1\":5,\"y1\":80,\"color\":63488,\"font\":1,\"text\":\"TDSValue(ppm):\",\"refresh\":0},\"Str2\":{\"type\":2,\"x1\":5,\"y1\":50,\"color\":63488,\"font\":1,\"text\":\"WaterTemperature('C):\",\"refresh\":0},\"Str3\":{\"type\":2,\"x1\":5,\"y1\":110,\"color\":63488,\"font\":1,\"text\":\"WaterQuality:\",\"refresh\":0}}";
const unsigned int capacity = 1600;
DynamicJsonDocument JsonDoc(capacity);
char jsonFromSd[1600];




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
char * waterQualityString[10] = {"clean", "good", "ordinary", "bad", "worst"};
char DateStr[12];
char TimeStr[10];
uint8_t isCanUseSD = true;


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
  isCanUseSD = SD.begin(PINSDCARDCS); // init SD card
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

  // SD카드에서 UI 정보 읽기:
  File UiSd;
  int fsize = 0;
  if(isCanUseSD){
    UiSd = SD.open("ui.txt");
    fsize = UiSd.available();
    UiSd.read(jsonFromSd, fsize);
    UiSd.close();
    InterpretJsonToUi(jsonFromSd);
  }else{
    InterpretJsonToUi(TESTJSON);
  }
}
void loop(void) {
  Scn.ClearDisplayElements();


  // Sensor:
  tempSensors.requestTemperatures();
  temperature = tempSensors.getTempCByIndex(0);
  tdsVal = GetTDSValue();

  // Get RTC Time:
  dt = objRtc.GetDateTime();

  // Logging Sensor Value:
  if(isCanUseSD){
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
  }




  

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
  sprintf(DateStr, "%04d/%02d/%02d", dt.Year(), dt.Month(), dt.Day());
  sprintf(TimeStr, "%02d/%02d/%02d", dt.Hour(), dt.Minute(), dt.Second());  
  // 온도:
  tempBar = map(constrain(temperature, TEMPSENMIN, TEMPSENMAX), TEMPSENMIN, TEMPSENMAX, 5, 180);  
  // 수질:
  tdsBar = map(constrain(tdsVal, 0, 750), 0, 750, 5, 225);
  char tdsStr[30];
  char waterqualityStr[50];
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
  char temperatureStr[5], tdsValStr[6];
  sprintf(temperatureStr, "%d", (int)temperature);
  sprintf(tdsValStr, "%d", (int)tdsVal);



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

  
  Scn.SetElement(0, Scn._Elem[0].x1, Scn._Elem[0].y1, Scn._Elem[0].width, Scn._Elem[0].height, 0, 1, 0, Scn._Elem[0].color, 0);
  Scn.SetElement(1, Scn._Elem[1].x1, Scn._Elem[1].y1, 0, 0, 0, 1, DateStr, Scn._Elem[1].color, FONT_1);
  Scn.SetElement(2, Scn._Elem[2].x1, Scn._Elem[2].y1, 0, 0, 0, 1, TimeStr, Scn._Elem[2].color, FONT_1);
  Scn.SetElement(3, Scn._Elem[3].x1, Scn._Elem[3].y1, 0, 0, Scn._Elem[0].radius, 1, 0, waterGradeColor, 0);
  Scn.SetElement(4, Scn._Elem[4].x1, Scn._Elem[4].y1, tempBar, Scn._Elem[4].height, 0, 1, 0, Scn._Elem[4].color, 0);
  Scn.SetElement(5, Scn._Elem[5].x1, Scn._Elem[5].y1, tdsBar, Scn._Elem[5].height, 0, 1, 0, Scn._Elem[5].color, 0);
  Scn.SetElement(6, Scn._Elem[6].x1, Scn._Elem[6].y1, 0, 0, 0, 1, temperatureStr, Scn._Elem[6].color, FONT_1);
  Scn.SetElement(7, Scn._Elem[7].x1, Scn._Elem[7].y1, 0, 0, 0, 1, tdsValStr, Scn._Elem[7].color, FONT_1);
  Scn.SetElement(8, Scn._Elem[8].x1, Scn._Elem[8].y1, 0, 0, 0, 1, tdsValStr, Scn._Elem[8].color, FONT_2);
  Scn.SetElement(9, Scn._Elem[9].x1, Scn._Elem[9].y1, 0, 0, 0, 1, (waterQualityString[waterGrade]), Scn._Elem[9].color, FONT_1);
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
    char contxt_str[80];
    uint16_t x1, y1, width, height, radius, clr, fnt, update_flag;
    String contxt_str_tmp = jsonElem.containsKey("text") ? jsonElem["text"] : (String)"";
    contxt_str_tmp.toCharArray(contxt_str, 80);
    x1 = jsonElem.containsKey("x1") ? jsonElem["x1"] : 0;
    y1 = jsonElem.containsKey("y1") ? jsonElem["y1"] : 0;
    width = jsonElem.containsKey("width") ? jsonElem["width"] : 0;
    height = jsonElem.containsKey("height") ? jsonElem["height"] : 0;
    radius = jsonElem.containsKey("radius") ? jsonElem["radius"] : 0;
    clr = jsonElem.containsKey("color") ? jsonElem["color"] : 0;
    BarColor = jsonElem.containsKey("color-bar") ? jsonElem["color-bar"] : 0;
    fnt = jsonElem.containsKey("font") ? jsonElem["font"] : 0;
    update_flag = jsonElem.containsKey("refresh") ? jsonElem["refresh"] : 0;
    char tempstr[6];

    if(!strcmp(uiElemType, "1")){
      // 일반 box 그리기:
      Scn.AddElement(1, x1, y1, width, height, 0, update_flag, 0, clr, fnt);
    }else if(!strcmp(uiElemType, "2")){
      // 일반 문자열 그리기:
      Scn.AddElement(2, x1, y1, 0, 0, 0, update_flag, contxt_str, clr, fnt);
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
      sprintf(tempstr, "%.0f", temperature);
      Scn.AddElement(103, x1, y1, 0, 0, 0, 1, tempstr, clr, fnt);
    }else if(!strcmp(uiElemType, "104")){
      // 특수
      sprintf(tempstr, "%d", tdsVal);
      Scn.AddElement(104, x1, y1, 0, 0, 0, 1, tempstr, clr, fnt);
    }else if(!strcmp(uiElemType, "105")){
      // 특수
      sprintf(tempstr, "%d", tdsVal);
      Scn.AddElement(105, x1, y1, 0, 0, 0, 1, tempstr, clr, fnt);
    }else if(!strcmp(uiElemType, "106")){
      // 특수
      Scn.AddElement(106, x1, y1, 0, 0, 0, 1, (waterQualityString[waterGrade]), clr, fnt);
    }else if(!strcmp(uiElemType, "107")){
      // 특수
      Scn.AddElement(107, x1, y1, 0, 0, 0, 1, (DateStr), clr, fnt);
    }else if(!strcmp(uiElemType, "108")){
      // 특수
      Scn.AddElement(108, x1, y1, 0, 0, 0, 1, (TimeStr), clr, fnt);
    }
  }
}
