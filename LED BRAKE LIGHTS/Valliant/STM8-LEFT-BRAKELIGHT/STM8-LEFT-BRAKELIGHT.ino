// STM8-PCA9685 example
// Change delays to non-blocking

#include <Wire.h>

#define addr 0x40             // Address of PCA9685 PWM driver
#define park    64
//#define brake   128
#define brake   4095
#define fulloff 4096
#define numLEDs 11
#define parkIn  1
#define brakeIn 0
#define LED     12
#define modeOFF 0
#define modePARK 1
#define modeBRAKE 2
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Globals
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//uint8_t pinMap[] = {6, 5, 4, 3, 2, 1, 0, 10, 11, 12, 13, 14, 15}; //Right side pinmap
//uint8_t pinMap[] = {6, 5, 4, 3, 2, 1, 0, 13, 12, 11, 10}; // Left side pinmap
uint8_t pinMap[] = {0, 1, 2, 3, 4, 5, 6, 10, 11, 12, 13}; // Left side pinmap
uint8_t mode = modeOFF;
uint8_t ledState = LOW;
unsigned long previousMillis = 0;
const long interval = 1000 / numLEDs;
//const long interval = 100;U
uint8_t index = 0;
bool sequence = false;
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
void setLEDS(uint16_t pwmValue, uint8_t i) {
  //  for (uint8_t i = 0; i < numLEDs; i++) {
  setPWM(pinMap[i], 0, pwmValue);
  //  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Setup
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup() {
  Wire_begin();               // join i2c bus (address optional for master)
  initPCA9685();
  //  setPWM(6, 4096, 0);        // Status LED - Full Brightness
  pinMode(2, OUTPUT);
  pinMode(12, OUTPUT);
  //  digitalWrite(2, HIGH);
  for (uint8_t i = 0; i < numLEDs; i++) {
    setPWM(pinMap[i], 0, fulloff);
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Loop
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
  unsigned long currentMillis = millis();
  //  index = 0;
  //  mode = modeOFF;
  if ((digitalRead(parkIn) == 1) && (digitalRead(brakeIn) == 1)) {
    mode = modeOFF;
    sequence = false;
    setLEDS(fulloff, index);
    index++;
  }
  if ((digitalRead(parkIn) == 0) && (digitalRead(brakeIn) == 1)) {
    if (mode != modePARK) {
      mode = modePARK;
      sequence = true;
      index = 0;
    }
    setLEDS(park, index);
    //    index++;
  }
  if ((digitalRead(brakeIn) == 0)) {
    if (mode != modeBRAKE) {
      mode = modeBRAKE;
      sequence = false;
      index = 0;
    }
    setLEDS(brake, index);
    index++;
  }
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    if (sequence == true) {
      index++;
      digitalWrite(LED, ledState);
    }
  }
  if (index >= numLEDs) {
    index = 0;
  }
}
