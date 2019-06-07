//******************************************************************************************************************************
// Query companion microcontroller for Temperature sensor information for display
// now reads from onboard temp sensor- rename to queryTemperature
//******************************************************************************************************************************
void queryTemperature() {

  Serial.println("Requesting Temperature");        // Read temp darta from TCN75A Sensor
  readTemperature();
  currentTemperature = temperatureReading;
}
//******************************************************************************************************************************
// Blink periodic status led (1hz, show system is running)
//******************************************************************************************************************************
void blinkStatus() {
  currentStatusMillis = millis();
  if (currentStatusMillis - previousStatusMillis >= delayStatus) {
    previousStatusMillis = currentStatusMillis;

    blinkCount++;
    if (blinkCount > 1) {
      blinkCount = 0;
    }

    if (WiFiType == 0) {          // Blink Green LED with BLUE LED steady (Running, WiFi connected)
      if (blinkCount == 1) {
        greenLEDFlag = greenLED;
        blueLEDFlag = blueLED;
      } else {
        greenLEDFlag = 0;
        blueLEDFlag = blueLED;
      }
    } else {                      // Blink Blue LED with GREEN LED steady (Running, WiFi Access Point)
      if (blinkCount == 1) {
        greenLEDFlag = greenLED;
        blueLEDFlag = blueLED;
      } else {
        greenLEDFlag = 0;
        blueLEDFlag = blueLED;
      }
    }
  }
}
//******************************************************************************************************************************
// Action timed events (Door relays, light relay etc)
//******************************************************************************************************************************
void timerFunctions() {
  for (int i = 0; i < 4; i++) {
    checkStates(buttons[i], i);
    checkStates(inputs[i], i);
  }
  for (int i = 0; i < 4; i++) {
    checkLights(buttons[i], i);
    checkLights(inputs[i], i);
  }
  for (int i = 0; i < 4; i++) {
    checkTimers(buttons[i], i);
    checkTimers(inputs[i], i);
  }
  for (int i = 0; i < 4; i++) {
    checkOutputs(inputs[i], buttons[i], i);
  }
  lightON = 0;                  // Reset lightON flag (only set in logic checking sections
}
//******************************************************************************************************************************
// Check inputs and set output states as required for IO control
//******************************************************************************************************************************
void checkStates(inputData & input, int i) {
  if (input.flag == 1) {                    // Input flag set, set up timers for that input
    DEBUG_PRINT("Input "); DEBUG_PRINT(i); DEBUG_PRINTLN(" active, setting ms values.");
    input.flag = 0;
    input.previousMillis = millis();
    if (input.mode == 0) {                  // turns on output if door button is pressed
      lightON = 1;                          // Flag to turn on light if a door button has been pressed
      DEBUG_PRINTLN("Door output active, setting LIGHT ON Flag");
    } else if (input.mode == 1) {           // Toggles light if light button has been pressed
      if (input.active == 1) {              // Check if light is already on
        lightON = 2;                        // if already on, set flag to show off
        DEBUG_PRINTLN("Light was already on. Toggling light to OFF.");
      } else {
        lightON = 1;                        // set flag to turn it on (other outputs may be light outputs)
      }
    }
    DEBUG_PRINT("Output "); DEBUG_PRINT(i); DEBUG_PRINT(" set, with delay "); DEBUG_PRINTLN(input.onTime);
    input.active = 1;
  }
}
//******************************************************************************************************************************
// Check and set staus of Light Relays
//******************************************************************************************************************************
void checkLights(inputData & input, int i) {
  if (input.mode == 1) { // light mode
    if (lightON == 1) {
      DEBUG_PRINT("Light "); DEBUG_PRINT(i); DEBUG_PRINTLN(" required, setting ms values.");
      input.active = 1;
      input.flag = 0;
      input.previousMillis = millis();
      DEBUG_PRINT("Light "); DEBUG_PRINT(i); DEBUG_PRINT(" set with delay "); DEBUG_PRINTLN(input.onTime);
    } else if (lightON == 2) {
      input.active = 0;
      DEBUG_PRINTLN("Light was already on. Active now set to 0.");
    }
  }
}
//******************************************************************************************************************************
// Check timers as required for IO control
//******************************************************************************************************************************
void checkTimers(inputData & input, int i) {
  uint8_t relayPin = 0x10;

  if (input.active == 1) {                             // a door open request has occured.
    input.currentMillis = millis();
    if (input.set == 0) {
      Serial.printf("Relay %i Set using mask %i\n", i, (relayPin << i));
      input.set = 1;
    }
    //    greenLEDFlag = greenLED;
    if (input.currentMillis - input.previousMillis >= input.onTime) {            // check if delay is expired
      input.active = 0;
    }
  }
  if (input.active == 0) {
    if (input.set == 1) {
      Serial.printf("Relay %i cleared. with mask %i\n", i, (relayPin << i));
    }
    input.set = 0;
    //    greenLEDFlag = 0;
  }
}
//******************************************************************************************************************************
// Check outputs as required for IO control
//******************************************************************************************************************************
void checkOutputs(inputData & input, inputData & button, int i) {
  uint8_t relayPin = 0x10;
  if ((input.set == 1) || (button.set == 1)) {
    setIOModule(mcp23017PortB, setPins, relayPin << i);// Turn on corresponding relay
  }
  if ((input.set == 0) && (button.set == 0)) {
    setIOModule(mcp23017PortB, clearPins, relayPin << i);// Turn off corresponding relay
  }
}
//******************************************************************************************************************************
// initialise MCP23017 IO Module
//******************************************************************************************************************************
void initI2CModules() {
  Serial.println("Initalising I2C Comms");
  // Set I2C bus
  digitalWrite(mcp23017Reset, 1);
  delay(100);
  //  Wire.begin(14, 2);
  Wire.begin(i2cSDA, i2cSCL);
  // CONFIGURE MCP23017 IO module.  Set I/O pins to outputs
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x00); // IODIRA register
  Wire.write(0xF0); // Set PortA Direction Register (1 = input, 0 = output)
  // PORTA.(0-3) = (OUTPUTS) Status LEDs(0-3)
  // PORTA.(4-7) = ( INPUTS) Door Buttons, active low, need weak pull up
  Wire.endTransmission();
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x01); // IODIRB register
  Wire.write(0x0F); // Set PortB Direction Register (1 = input, 0 = output)
  Wire.endTransmission();
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x0C); // Weak Pull Up register port A
  Wire.write(0xF0); // Set PortA pullups (1 = enabled)
  //  Pullups for Door Buttons
  Wire.endTransmission();
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x0D); // Weak Pull Up register port B
  Wire.write(0x00); // Set PortB pullups (1 = enabled)
  // PORTB.(0-3) = ( INPUTS) External inputs
  // PORTB.(4-7) = (OUTPUTS) Relays
  Wire.endTransmission();
  // CONFIGURE TCN75 temperature sensor (0.5 degree resolution)
  Wire.beginTransmission(addressTCN75A);
  Wire.write(0x01); // CONFIG register
  Wire.write(0x00); // set to 0.5 degree resoltion
  Wire.endTransmission();
}
//******************************************************************************************************************************
// Write to MCP23017 IO Module
//******************************************************************************************************************************
void writeIOModule(int a, int b) {
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x12); // GPIOA
  Wire.write(a); // port A
  Wire.endTransmission();
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x13); // GPIOB
  Wire.write(b); // port B
  Wire.endTransmission();
}
//******************************************************************************************************************************
// Read from MCP23017 IO Module
//******************************************************************************************************************************
void readIOModule() {
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x12); // GPIOA
  Wire.endTransmission();
  Wire.requestFrom(addressMCP23017, 2); // port A and port B
  ioPortA = Wire.read();
  ioPortB = Wire.read();
  Wire.endTransmission();
}
//******************************************************************************************************************************
// Set IO pins on MCP23017 Module
//******************************************************************************************************************************
void setIOModule(char port, char action, char data) {
  readIOModule();                   // port data stored in global variables ioPortA and ioPortB
  if (port == mcp23017PortA) {      // modify portA
    if (action == clearPins ) {     // set pin low
      data = data ^ 0xFF;           // invert bits in the mask
      ioPortA = ioPortA & data;     // set bits in PortA low that are high in data (bitwise AND)
    }
    if (action == setPins ) {       // set pins high
      ioPortA = ioPortA | data;     // set bits in PortA high that are high in data (bitwise OR)
    }
  }
  if (port == mcp23017PortB) {      // modify portB
    if (action == clearPins ) {     // set pin low
      data = data ^ 0xFF;           // invert bits in the mask
      ioPortB = ioPortB & data;     //set bits in PortB low that are high in data
    }
    if (action == setPins ) {       // set pin high
      ioPortB = ioPortB | data;     //set bits in PortB high that are high in data
    }
  }
  //  Serial.printf("Setting IO module PortA: 0x%02x PortB: 0x%02x \n", ioPortA, ioPortB);
  writeIOModule(ioPortA, ioPortB);
}
//******************************************************************************************************************************
// Read temperature from TCN75A
//******************************************************************************************************************************
void readTemperature() {
  Wire.beginTransmission(addressTCN75A);
  Wire.write(0x00); // Temperature reading register
  Wire.endTransmission();
  Wire.requestFrom(addressTCN75A, 2); // read two bytes from temperature register
  temperatureReading = Wire.read();
  temperatureReading = temperatureReading << 8;
  temperatureReading |= Wire.read();
  temperatureReading = temperatureReading / 256;
  Serial.print("Temp C = ");  Serial.println(temperatureReading);
}
//******************************************************************************************************************************
// initialise input arrays
//******************************************************************************************************************************
void initInputs(inputData & input) {
  input.state = 0;          // debounced state, 1 = active
  input.count = 0;          // debounce counter.
  input.current = 0;        // current state of input
  input.lock = 1;           // prevent relays firing at restart
  input.flag = 0;           // active flag
  input.currentMillis = 0;  // counters to compare for timer functions
  input.previousMillis = 0; //
  input.mode = 0;           // Door button mode by default
  input.active = 0;         // Active state.  1 = output is active and is timing
  input.set = 0;            // Marks relay has been set active - to supress excessive serial coms
  input.onTime = 200;       // default 200ms active time for input
}
//******************************************************************************************************************************
// Set Panel LEDS - read global flags and turn LEDS on if required
//******************************************************************************************************************************
void setPanelLeds(void) {
  int newPortA = 0;
  readIOModule();                       // Read current PortA and PortB
  newPortA = ioPortA & 0xF0;               // Clear LED bits
  newPortA = newPortA | redLEDFlag;     // Set LED bits as required
  newPortA = newPortA | yellowLEDFlag;
  newPortA = newPortA | greenLEDFlag;
  newPortA = newPortA | blueLEDFlag;
  //  Serial.printf("Setting Status LEDS with value %i.\n");
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x12); // GPIOA
  Wire.write(newPortA); // port A
  Wire.endTransmission();
}
//******************************************************************************************************************************
// set front panel leds (multiplexed)
// PORTA.(4-7) map to status(3-0)
// status(0-2) used to multiplex front panel LEDS with this mapping:
// STATUS0 STATUS1 STATUS2  LED COLOUR
// 0       0       0        OFF
// 1       0       0        RED and GREEN
// 0       1       0        YELLOW
// 1       1       0        GREEN
// 0       0       1        BLUE
// 1       0       1        RED
// 0       1       1        YELLLOW and BLUE
// 1       1       1        OFF
//******************************************************************************************************************************
//void setPanelStatus(uint8_t panelStatus, uint8_t onFlag) {
//  setIOModule(mcp23017PortA, clearPins, 0xE0);        // Clear staus leds used to mutiplex front panel leds
//  setIOModule(mcp23017PortA, onFlag,   panelStatus); // Set status leds as required to set fron panel leds
//}
