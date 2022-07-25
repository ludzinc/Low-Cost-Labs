// STM8-PCA9685 example

#include <Wire.h>

#define addr 0x40             // Address of PCA9685 PWM driver
#define park    64
#define brake   128
#define fulloff 4096
#define numLEDs 13
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Globals
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t pinMap[] = {6, 5, 4, 3, 2, 1, 0, 10, 11, 12, 13, 14, 15};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void initPCA9685(void) {
  // SW reset to clear any stuck patterns
  //  write8(PCA9685_MODE1, MODE1_RESTART);
  Wire_beginTransmission(0x00); // General Call Address
  Wire_write(0x06);             // SWRST Byte
  Wire_endTransmission();
  // Need to set up auto increment mode
  Wire_beginTransmission(addr); // default addres PCA985
  Wire_write(0x00);             // address MODE1 register
  Wire_write(0x21);             // Enable Auto increment, ALLCALL
  Wire_endTransmission();
}

void setPWM(uint8_t pinnum, uint16_t ontime, uint16_t offtime) {
  Wire_beginTransmission(addr); // default addres PCA985
  Wire_write(0x06 + 4 * pinnum );
  Wire_write(ontime);
  Wire_write(ontime >> 8);
  Wire_write(offtime);
  Wire_write(offtime >> 8);
  Wire_endTransmission();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Setup
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup() {
  Wire_begin();               // join i2c bus (address optional for master)
  initPCA9685();
  setPWM(6, 4096, 0);        // Status LED - Full Brightness
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
}

byte val = 0;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Loop
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {

  uint8_t i = 0;
  for (uint8_t i = 0; i < numLEDs; i++) {
    setPWM(pinMap[i], 0, park);
    delay(1000 / numLEDs);
  }
  delay(500);
  for (uint8_t i = 0; i < numLEDs; i++) {
    setPWM(pinMap[i], 0, brake);
    delay(1000 / numLEDs);
  }
  delay(500);
  for (uint8_t i = 0; i < numLEDs; i++) {
    setPWM(pinMap[i], 0, fulloff);
    delay(1000 / numLEDs);
  }
  delay(500);
}
