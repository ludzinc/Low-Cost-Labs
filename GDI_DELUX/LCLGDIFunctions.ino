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
// Blink LEDs as requested
//******************************************************************************************************************************
void blinkLED(uint8_t LED) {
  digitalWrite(LED, !digitalRead(LED));
  delay(500);
  digitalWrite(LED, !digitalRead(LED));
  delay(500);
}
//******************************************************************************************************************************
// statusRed LED
//******************************************************************************************************************************
void statusRED(uint8_t state) {
  if (state == 1) {                   // Turn on RED led on front panel
    digitalWrite(status0, 1);
    digitalWrite(status1, 0);
    digitalWrite(status2, 1);
  } else {
    digitalWrite(status0, 1);
    digitalWrite(status1, 1);
    digitalWrite(status2, 1);
  }
}
//******************************************************************************************************************************
// statusYellow LED
//******************************************************************************************************************************
void statusYELLOW(uint8_t state) {
  if (state == 1) {                   // Turn on YELLOW led on front panel
    digitalWrite(status0, 1);
    digitalWrite(status1, 1);
    digitalWrite(status2, 0);
  } else {
    digitalWrite(status0, 1);
    digitalWrite(status1, 1);
    digitalWrite(status2, 1);
  }
}
//******************************************************************************************************************************
// statusGREEN LED
//******************************************************************************************************************************
void statusGREEN(uint8_t state) {
  if (state == 1) {                   // Turn on GREEN led on front panel
    digitalWrite(status0, 0);
    digitalWrite(status1, 1);
    digitalWrite(status2, 0);
  } else {
    digitalWrite(status0, 1);
    digitalWrite(status1, 1);
    digitalWrite(status2, 1);
  }
}
//******************************************************************************************************************************
// statusBLUE LED
//******************************************************************************************************************************
void statusBLUE(uint8_t state) {
  if (state == 1) {                   // Turn on BLUE led on front panel
    digitalWrite(status0, 0);
    digitalWrite(status1, 0);
    digitalWrite(status2, 1);
  } else {
    digitalWrite(status0, 1);
    digitalWrite(status1, 1);
    digitalWrite(status2, 1);
  }
}
//******************************************************************************************************************************
// statusOFF LED
//******************************************************************************************************************************
void statusOFF(void) {     // Turn off all front panel charliepled LEDs
  digitalWrite(status0, 0);
  digitalWrite(status1, 0);
  digitalWrite(status2, 0);
}
//******************************************************************************************************************************
// statusGreenBlue LED
//******************************************************************************************************************************
void statusGREENBLUE(void) {     // Turn off all front panel charliepled LEDs
  digitalWrite(status0, 0);
  digitalWrite(status1, 1);
  digitalWrite(status2, 1);
}
//******************************************************************************************************************************
// statusRedYellow LED
//******************************************************************************************************************************
void statusREDYELLOW(void) {     // Turn off all front panel charliepled LEDs
  digitalWrite(status0, 1);
  digitalWrite(status1, 0);
  digitalWrite(status2, 0);
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
    if (blinkCount == 1) {
      writeIOModule(0x00, 0x10);    // turn on I2C module status led
    } else {
      writeIOModule(0x00, 0x00);
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
  lightON = 0;
}
//******************************************************************************************************************************
// Check inputs and set output states as required for IO control
//******************************************************************************************************************************
void checkStates(inputData & input, int i) {
  if (input.flag == 1) {                    // Input flag set, set up timers for that input
    DEBUG_PRINT("Input "); DEBUG_PRINT(i); DEBUG_PRINTLN(" active, setting ms values.");
    input.flag = 0;
    input.previousMillis = millis();
    if (input.mode == 0) {            // turns on output if door button is pressed
      input.onTime = doorDelay;
      lightON = 1;                          // Flag to turn on light if a door button has been pressed
      DEBUG_PRINTLN("Door output active, setting LIGHT ON Flag");
    } else if (input.mode == 1) {     // Toggles light if light button has been pressed
      if (input.active == 1) {              // Check if light is already on
        lightON = 2;                        // if already on, set flag to show off
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
      input.onTime = lightDelay;
      DEBUG_PRINT("Light "); DEBUG_PRINT(i); DEBUG_PRINTLN(" set with delay "); DEBUG_PRINTLN(input.onTime);
    } else if (lightON == 2) {
      input.active = 0;
    }
  }
}
//******************************************************************************************************************************
// Check timers as required for IO control
//******************************************************************************************************************************
void checkTimers(inputData & input, int i) {
  uint8_t relayPin = 1;

  if (input.active == 1) {                             // a door open request has occured.
    input.currentMillis = millis();
    setIOModule(mcp23017PortA, setPins, relayPin << i);// Turn on corresponding door relay
    if (input.set == 0) {
//      Serial.printf("Relay %i Set.\n", i);
      input.set = 1;
    }
    if (input.mode == 0) {                       // Only blink green led if door relay is active
      if (WiFiType == 0) {
        flagGreenLED = 1;
      } else {
        flagGreenLED = 0;
      }
    }
    if (input.currentMillis - input.previousMillis >= input.onTime) {            // check if delay is expired
      input.active = 0;
    }
    if (input.active == 0) {
      setIOModule(mcp23017PortA, clearPins, relayPin << i);
      if (input.set == 1) {
//        Serial.printf("Relay %i cleared.\n", i);
      }
      input.set = 0;
      if (WiFiType == 0) {
        flagGreenLED = 0;
      } else {
        flagGreenLED = 1;
      }
    }
  }
}
//******************************************************************************************************************************
// initialise MCP23017 IO Module
//******************************************************************************************************************************
void initI2CModules() {
  Serial.println("Initalising I2C Comms");
  // Set I2C bus
  Wire.begin(14, 2);                          // Oops! SCL / SDA swapped on schematic
  // CONFIGURE MCP23017 IO module.  Set I/O pins to outputs
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x00); // IODIRA register
  Wire.write(0xF0); // Set PortA Direction Register (1 = input, 0 = output)
  Wire.endTransmission();
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x01); // IODIRB register
  Wire.write(0x8F); // Set PortB Direction Register (1 = input, 0 = output)
  Wire.endTransmission();
  Wire.beginTransmission(addressMCP23017);
  Wire.write(0x0C); // Weak Pull Up register port A
  Wire.write(0xF0); // Set PortA pullups (1 = enabled)
  Wire.write(0x0D); // Weak Pull Up register port B
  Wire.write(0x8F); // Set PortB pullups (1 = enabled)
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
  input.previousMillis = 0;
  input.mode = 0;     // Door button mode by default
  input.active = 0;         // Active state.  1 = output is active and is timing
  input.set = 0;            // Marks relay has been set active - to supress excessive serial coms
  input.onTime = 200;   // default 200ms active time for input
}
//******************************************************************************************************************************
// Set Panel LEDS - read global flags and turn LEDS on if required
//******************************************************************************************************************************
void setPanelLeds(void) {
  loopCounter++;
  switch (loopCounter) {
    case 1:
      statusRED(flagRedLED);
      break;
    case 2:
      statusYELLOW(flagYellowLED);
      break;
    case 3:
      statusGREEN(flagGreenLED);
      break;
    case 4:
      statusBLUE(flagBlueLED);
      break;
    default:
      loopCounter = 0;
  }
}
