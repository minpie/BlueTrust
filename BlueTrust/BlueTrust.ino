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


// Pin:
#define PINI2CSDA 0 // I2C: SDA pin
#define PINI2CSCL 0 // I2C: SCL pin
#define PINSPIMOSI 0 // SPI: MOSI pin
#define PINSPIMISO 0 // SPI: MISO pin
#define PINSPISCLK 0 // SPI: SCLK pin
#define PINTDSANALOG A0 // TDS: Analog pin
#define PINBTRX 0 // Bluetooth: Use to RX pin
#define PINBTTX 0 // Bluetooth: Use to TX pin
#define PINJOYVER 0 // Joystick: vertical pin
#define PINJOYHOR 0 // Joystick: horizontal pin
#define PINJOYBTN 0 // Joystick: button pin
#define PINLEDSTRIP 3 // LED Strip: control pin
#define PINFANEN 5 // Motor Driver(Fan): EN pin
#define PINTEMPSEN 2 // Tempreture Sensor Fin


// Define:
#define AMOUNTLEDSTRIP 60 // Amount of LED in LED Strip
#define LEDBRIGHTNESS 240 // LED Strip's brightness, max: 255 min: 0
#define TEMPSENMAX 30 // Tempereture Sensor Max
#define TEMPSENMIN 20 // Tempereture Sensor Min




#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  3           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;


// Function:
void SetLEDStrip(uint8_t ledNumber, uint8_t r, uint8_t g, uint8_t b); // Set (ledNumber)LED's color
void SetFanStop(void); // Set Fan to stop
void SetFanRunFull(void); // Set Fan to run at full speed
void SetFanWithSpeed(uint8_t spd); // Set Fan with target speed
int getMedianNum(int bArray[], int iFilterLen);


// Object:
Adafruit_NeoPixel ledStrip(AMOUNTLEDSTRIP, PINLEDSTRIP, NEO_GRB + NEO_KHZ800);
OneWire tempOneWire(PINTEMPSEN);
DallasTemperature tempSensors(&tempOneWire);



// TEST:
uint8_t rColor = 0;


// ### main ###
void setup(void) {
  // TEST:
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // pin mode:
  pinMode(PINFANEN, OUTPUT); // motor driver en pin

  // LED Strip:
  ledStrip.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  ledStrip.setBrightness(LEDBRIGHTNESS);
  ledStrip.clear(); // Set all pixel colors to 'off'

  // TEST:
  //SetFanRunFull();
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.begin();

    
  Tft.lcd_init();                                      // init TFT library
  Tft.setRotation(Rotation_90_D);
  //Tft.lcd_display_string(60, 120, (const uint8_t *)"Hello, world !", FONT_1608, RED);
  //Tft.lcd_display_string(30, 152, (const uint8_t *)"2.8' TFT Touch Shield", FONT_1608, RED);


  //Serial.begin(115200);
  pinMode(PINTDSANALOG,INPUT);
}
void loop(void) {
  // TEST:
  for(uint8_t i=0; i<AMOUNTLEDSTRIP; i++){
    SetLEDStrip(i, rColor, 255 - rColor, 127 - rColor);
    rColor = (rColor + 5) % 256;
    delay(30);
  }
  tempSensors.requestTemperatures();
  float temp = tempSensors.getTempCByIndex(0);
  temperature = temp;
  uint8_t panSpd = map(constrain(temp, TEMPSENMIN, TEMPSENMAX), TEMPSENMIN, TEMPSENMAX, 120, 255);
  SetFanWithSpeed(panSpd);
  

  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){
    //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(PINTDSANALOG);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT) 
      analogBufferIndex = 0;
  }   
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
      analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
    tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value


    Tft.lcd_clear_screen(WHITE);
    Tft.lcd_display_string(60, 120, (const uint8_t *)"TDS Value(ppm):", FONT_1608, RED);
    Tft.lcd_display_num(30, 152, (const uint32_t)tdsValue, 5, FONT_1608, RED);
  }

  delay(200);
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


int getMedianNum(int bArray[], int iFilterLen){
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