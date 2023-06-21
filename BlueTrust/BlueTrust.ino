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


// Pin:
#define PINI2CSDA 0 // I2C: SDA pin
#define PINI2CSCL 0 // I2C: SCL pin
#define PINSPIMOSI 0 // SPI: MOSI pin
#define PINSPIMISO 0 // SPI: MISO pin
#define PINSPISCLK 0 // SPI: SCLK pin
#define PINTDSANALOG 0 // TDS: Analog pin
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


// Function:
void SetLEDStrip(uint8_t ledNumber, uint8_t r, uint8_t g, uint8_t b); // Set (ledNumber)LED's color
void SetFanStop(void); // Set Fan to stop
void SetFanRunFull(void); // Set Fan to run at full speed
void SetFanWithSpeed(uint8_t spd); // Set Fan with target speed


// Object:
Adafruit_NeoPixel ledStrip(AMOUNTLEDSTRIP, PINLEDSTRIP, NEO_GRB + NEO_KHZ800);
OneWire tempOneWire(PINTEMPSEN);
DallasTemperature tempSensors(&tempOneWire);



// TEST:
uint8_t rColor = 0;


/* ### main ### */
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
  uint8_t panSpd = map(constrain(temp, 27, 30), 27, 30, 0, 255);
  SetFanWithSpeed(panSpd);
  delay(200);
}
/* ### main ### */


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
