/* 
 *  BlueTrust.ino
 *  
 *  2023.04.12 ~
 *  
 *  V0
*/
// Include:
#include <Adafruit_NeoPixel.h>




/*


TEST
TEST
TEST
TEST

*/

// Pin:
#define PINI2CSDA 24 // I2C: SDA pin
#define PINI2CSCL 25 // I2C: SCL pin
#define PINSPIMOSI 19 // SPI: MOSI pin
#define PINSPIMISO 20 // SPI: MISO pin
#define PINSPISCLK 18 // SPI: SCLK pin
#define PINTDSANALOG 27 // TDS: Analog pin
#define PINBTRX 10 // Bluetooth: Use to RX pin
#define PINBTTX 9 // Bluetooth: Use to TX pin
#define PINJOYVER 28 // Joystick: vertical pin
#define PINJOYHOR 29 // Joystick: horizontal pin
#define PINJOYBTN 11 // Joystick: button pin
#define PINLEDSTRIP 6 // LED Strip: control pin
#define PINFANEN 12 // Motor Driver(Fan): EN pin


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
  SetFanRunFull();
}
void loop(void) {
  // TEST:
  for(uint8_t i=0; i<AMOUNTLEDSTRIP; i++){
    SetLEDStrip(i, rColor, 255 - rColor, 127 - rColor);
    rColor = (rColor + 5) % 256;
    delay(30);
  }
  

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
