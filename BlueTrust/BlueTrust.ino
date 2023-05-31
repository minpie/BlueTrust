/* 
 *  BlueTrust.ino
 *  
 *  2023.04.12 ~
 *  
 *  V0
*/
// Include:
#include <Adafruit_NeoPixel.h>


// Pin:
#define PINI2CSDA 0 // I2C: SDA pin
#define PINI2CSCL 0 // I2C: SCL pin
#define PINSPIMOSI 0 // SPI: MOSI pin
#define PINSPIMISO 0 // SPI: MISO pin
#define PINSPISCLK 0 // SPI: SCLK pin
#define PINTDSANALOG 0 // TDS: Analog pin
#define PINBTRX 0 // Bluetooth: RX pin
#define PINBTTX 0 // Bluetooth: TX pin
#define PINJOYVER 0 // Joystick: vertical pin
#define PINJOYHOR 0 // Joystick: horizontal pin
#define PINJOYBTN 0 // Joystick: button pin
#define PINLEDSTRIP 8 // LED Strip: control pin


// Define:
#define AMOUNTLEDSTRIP 60 // Amount of LED in LED Strip
#define LEDBRIGHTNESS 240 // LED Strip's brightness, max: 255 min: 0


// Function:
void SetLEDStrip(uint8_t ledNumber, uint8_t r, uint8_t g, uint8_t b); // Set (ledNumber)LED's color


// Object:
Adafruit_NeoPixel ledStrip(AMOUNTLEDSTRIP, PINLEDSTRIP, NEO_GRB + NEO_KHZ800);



// TEST:
uint8_t rColor = 0;


/* ### main ### */
void setup(void) {
  // TEST:
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // LED Strip:
  ledStrip.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  ledStrip.setBrightness(LEDBRIGHTNESS);
  ledStrip.clear(); // Set all pixel colors to 'off'
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
