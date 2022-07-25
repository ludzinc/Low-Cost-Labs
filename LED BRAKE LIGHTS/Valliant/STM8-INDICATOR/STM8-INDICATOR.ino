// STM8-PCA9685 example

#include <Wire.h>

#define addr 0x40             // Address of PCA9685 PWM driver
#define park    64
#define brake   4095
#define fulloff 4096
#define numLEDs 13
#define parkIn  0aa/ 
#define brakeIn 1
#define LED     12
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Globals
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Pin Map for Right Indicator
uint8_t pinMap[] = {6, 5, 4, 3, 2, 1, 0, 10, 11, 12, 13, 14, 15};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void initPCA9685(void) {
  // SW reset to clear any stuck patterns
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
  //  setPWM(6, 4096, 0);        // Status LED - Full Brightness
  pinMode(LED, OUTPUT);
  pinMode(parkIn, INPUT);
  pinMode(brakeIn, INPUT);
  for (uint8_t i = 0; i < numLEDs; i++) {
    setPWM(pinMap[i], 0, fulloff);
  }
}

// .
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Loop
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {

  if ((digitalRead(brakeIn) == 0)) {
    //    digitalWrite(LED, HIGH);
    for (uint8_t i = 0; i < numLEDs; i++) {
      setPWM(pinMap[i], 0, brake);
      delay(1000 / numLEDs);
    }
  } else {
    //    digitalWrite(LED, LOW);
    for (uint8_t i = 0; i < numLEDs; i++) {
      setPWM(pinMap[i], 0, fulloff);
    }
  }
}
