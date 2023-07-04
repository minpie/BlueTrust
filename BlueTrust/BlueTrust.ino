/* 
 *  BlueTrust.ino
 *  
 *  2023.04.12 ~
 *  
 *  V0
*/
// Include:
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdint.h>
#include <LCD.h>
#include <SPI.h>
#include <SD.h>
#include <RtcDS1302.h>
#include <ThreeWire.h>
#include <EEPROM.h>
#include <time.h>


// Pin:
#define PINI2CSDA 0     // I2C: SDA pin
#define PINI2CSCL 0     // I2C: SCL pin
#define PINSPIMOSI 11   // SPI: MOSI pin
#define PINSPIMISO 12   // SPI: MISO pin
#define PINSPISCLK 13   // SPI: SCLK pin
#define PINTDSANALOG A0 // TDS: Analog pin
#define PINBTRX 0       // Bluetooth: Use to RX pin
#define PINBTTX 0       // Bluetooth: Use to TX pin
#define PINJOYVER 0     // Joystick: vertical pin
#define PINJOYHOR 0     // Joystick: horizontal pin
#define PINJOYBTN 0     // Joystick: button pin
#define PINLEDSTRIP 3   // LED Strip: control pin
#define PINFANEN 5      // Motor Driver(Fan): EN pin
#define PINTEMPSEN 2    // Tempreture Sensor Fin
#define PINSDCARDCS 4   // SD Card SPI CS pin
#define PINRTCRST 8
#define PINRTCCLK 6
#define PINRTCDATA 1


/*
EEPROM DATA ADDRESS
[ADDRESS] : [DATA INFO]
-----------------------
0번지~5번지: (컴파일된 연,월,일,시,분,초)

*/
#define ADDR_COMPILE_TIME_YEAR 0
#define ADDR_COMPILE_TIME_MONTH 1
#define ADDR_COMPILE_TIME_DAY 2
#define ADDR_COMPILE_TIME_HOUR 3
#define ADDR_COMPILE_TIME_MINUTE 4
#define ADDR_COMPILE_TIME_SECOND 5
#define EEPROM_ADDRESS_AMOUNT 6 // EEPROM에 저장된 개수


// Define:
#define AMOUNTLEDSTRIP 60 // Amount of LED in LED Strip
#define LEDBRIGHTNESS 250 // LED Strip's brightness, max: 255 min: 0
#define TEMPSENMAX 30     // Tempereture Sensor Max
#define TEMPSENMIN 20     // Tempereture Sensor Min
#define VREF 5.0          // analog reference voltage(Volt) of the ADC
#define SCOUNT 5          // sum of sample point




// Function:
void SetLEDStrip(uint8_t ledNumber, uint8_t r, uint8_t g, uint8_t b); // Set (ledNumber)LED's color
void SetFanStop(void); // Set Fan to stop
void SetFanRunFull(void); // Set Fan to run at full speed
void SetFanWithSpeed(uint8_t spd); // Set Fan with target speed
int GetMedianNum(int bArray[], int iFilterLen);
float GetTDSValue(void); // Get TDS Value
void SetTimeAtCompiled(); // Set RTC Time to Compiled time
void ReadValFromRom(); // Read Value From EEPROM
void GetCompiledTime();
uint32_t GetTimeValue(int year, int month, int day, int hour, int minute, int second);


// Object:
Adafruit_NeoPixel ledStrip(AMOUNTLEDSTRIP, PINLEDSTRIP, NEO_GRB + NEO_KHZ800);
OneWire tempOneWire(PINTEMPSEN);
DallasTemperature tempSensors(&tempOneWire);
File sensorValueLogFile;
ThreeWire rtcWire(PINRTCDATA, PINRTCCLK, PINRTCRST);
RtcDS1302<ThreeWire> objRtc(rtcWire);

// Variable(ROM):
uint8_t isFirstRunAfterCompile = 1; // 컴파일 후 첫 실행 여부
int compileTimes[6] = {0, 0, 0, 0, 0, 0};
int compileTimesInRom[6] = {0, 0, 0, 0, 0, 0};




// TEST:
uint8_t rColor = 0;
uint8_t tdsVal = 0;
uint8_t panSpd = 0;
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 0;


// ### main ###
void setup(void) {
  // pin mode:
  pinMode(PINFANEN, OUTPUT); // motor driver en pin
  pinMode(PINTDSANALOG,INPUT); // TDS analog pin

  // LED Strip:
  ledStrip.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  ledStrip.setBrightness(LEDBRIGHTNESS);
  ledStrip.clear(); // Set all pixel colors to 'off'

  // TEST:
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.begin();    
  Tft.lcd_init(); // init TFT library
  Tft.setRotation(Rotation_90_D);
  SD.begin(PINSDCARDCS); // init SD card
  objRtc.Begin();


  // read values from eeprom:
  ReadValFromRom();
  // 컴파일 후 한번만 실행을 위한 처리:
  GetCompiledTime();
  uint32_t compiledTime = GetTimeValue(compileTimes[0], compileTimes[1], compileTimes[2], compileTimes[3], compileTimes[4], compileTimes[5]);
  uint32_t compiledTimeRom = GetTimeValue(compileTimesInRom[0], compileTimesInRom[1], compileTimesInRom[2], compileTimesInRom[3], compileTimesInRom[4] ,compileTimesInRom[5]);
  if(compiledTime > compiledTimeRom){
    isFirstRunAfterCompile = 1;
  }else{
    isFirstRunAfterCompile = 0;
  }




  // 컴파일 후 한번만 실행할 코드:
  if(isFirstRunAfterCompile){
    SetTimeAtCompiled();
    EEPROM.write(ADDR_COMPILE_TIME_YEAR, compileTimes[0]);
    EEPROM.write(ADDR_COMPILE_TIME_MONTH, compileTimes[1]);
    EEPROM.write(ADDR_COMPILE_TIME_DAY, compileTimes[2]);
    EEPROM.write(ADDR_COMPILE_TIME_HOUR, compileTimes[3]);
    EEPROM.write(ADDR_COMPILE_TIME_MINUTE, compileTimes[4]);
    EEPROM.write(ADDR_COMPILE_TIME_SECOND, compileTimes[5]);
  }

}
void loop(void) {
  // Sensor:
  tempSensors.requestTemperatures();
  temperature = tempSensors.getTempCByIndex(0);
  tdsVal = GetTDSValue();

  // Get RTC Time:
  RtcDateTime dt = objRtc.GetDateTime();


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


  // LED Strip:
  for(uint8_t i=0; i<AMOUNTLEDSTRIP; i++){
    SetLEDStrip(i, rColor, 255 - rColor, 127 - rColor);
    rColor = (rColor + 5) % 256;
    delay(10);
  }

  // Fan Control:
  panSpd = map(constrain(temperature, TEMPSENMIN, TEMPSENMAX), TEMPSENMIN, TEMPSENMAX, 120, 255);
  SetFanWithSpeed(panSpd);


  // For LCD Screen:
  Tft.lcd_clear_screen(WHITE);
  Tft.lcd_display_num(30, 20, (const uint16_t)dt.Year(), 4, FONT_1608, RED);
  Tft.lcd_display_string(60, 20, (const uint8_t *)"/", FONT_1608, RED);
  Tft.lcd_display_num(65, 20, (const uint8_t)dt.Month(), 2, FONT_1608, RED);
  Tft.lcd_display_string(80, 20, (const uint8_t *)"/", FONT_1608, RED);
  Tft.lcd_display_num(90, 20, (const uint8_t)dt.Day(), 2, FONT_1608, RED);
  Tft.lcd_display_num(130, 20, (const uint8_t)dt.Hour(), 2, FONT_1608, RED);
  Tft.lcd_display_string(160, 20, (const uint8_t *)":", FONT_1608, RED);
  Tft.lcd_display_num(170, 20, (const uint8_t)dt.Minute(), 2, FONT_1608, RED);
  Tft.lcd_display_string(200, 20, (const uint8_t *)":", FONT_1608, RED);
  Tft.lcd_display_num(210, 20, (const uint8_t)dt.Second(),2 , FONT_1608, RED);





  Tft.lcd_display_string(30, 50, (const uint8_t *)"TDS Value(ppm):", FONT_1608, RED);
  Tft.lcd_display_num(160, 50, (const uint32_t)tdsVal, 5, FONT_1608, RED);
  Tft.lcd_display_string(30, 80, (const uint8_t *)"Temperature(C):", FONT_1608, RED);
  Tft.lcd_display_num(160, 80, (const uint32_t)temperature, 5, FONT_1608, RED);
  Tft.lcd_display_string(30, 100, (const uint8_t *)"isFirstRunAfterCompile:", FONT_1608, RED);
  Tft.lcd_display_num(200, 100, (const uint32_t)isFirstRunAfterCompile, 5, FONT_1608, RED);


  delay(80);
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
  averageVoltage = GetMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
  tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
  return tdsValue;
}

void SetTimeAtCompiled(){
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if(objRtc.GetIsWriteProtected())
    objRtc.SetIsWriteProtected(false);
  objRtc.SetIsRunning(true);
  objRtc.SetDateTime(compiled);  
}

// Read Value From EEPROM:
void ReadValFromRom(){
  compileTimesInRom[0] = EEPROM.read(ADDR_COMPILE_TIME_YEAR);
  compileTimesInRom[1] = EEPROM.read(ADDR_COMPILE_TIME_MONTH);
  compileTimesInRom[2] = EEPROM.read(ADDR_COMPILE_TIME_DAY);
  compileTimesInRom[3] = EEPROM.read(ADDR_COMPILE_TIME_HOUR);
  compileTimesInRom[4] = EEPROM.read(ADDR_COMPILE_TIME_MINUTE);
  compileTimesInRom[5] = EEPROM.read(ADDR_COMPILE_TIME_SECOND);
}

void GetCompiledTime(){
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

uint32_t GetTimeValue(int year, int month, int day, int hour, int minute, int second){
  uint32_t result = 0;
  result += second;
  result += (60 * minute);
  result += (60 * 60 * hour);
  result += (60 * 60 * 24 * day);
  result += (60 * 60 * 24 * 30 * month);
  result += (60 * 60 * 24 * 30 * 12 * (year - 2000));
  return result;
}