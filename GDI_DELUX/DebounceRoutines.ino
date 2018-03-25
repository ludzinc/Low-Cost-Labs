//******************************************************************************************************************************
// Debounce indivdual intput, return state
//******************************************************************************************************************************
uint8_t debounce(uint8_t current, inputData &input) {
  if (input.state != current) {
    input.count = input.count + 1;
    if (input.count > debounceCount) {
      input.count = 0;
      input.state = current;
    }
  } else {
    input.count = 0;
  }
  return input.state;
}
//******************************************************************************************************************************
// Debounce inputs and set flags
//******************************************************************************************************************************
void processInputs(inputData &input) {
  //Serial.printf("Processing Inputs ioPORTA 0x%02x ioPORTB 0x%02x\n", ioPortA, ioPortB);
  if ((debounce(input.current, input) == 0) && (input.lock == 0)) {
    input.flag = 1;
    input.lock = 1;
    DEBUG_PRINTLN("input set");
    flagYellowLED = 1;
  }
  if ((debounce(input.current, input) == 1) && (input.lock == 1)) {
    input.lock = 0;
    flagYellowLED = 0;
    DEBUG_PRINTLN("input cleared");
  }
}
//******************************************************************************************************************************
// Scan and debounce all inputs.  Uses Global Variables ioPortA and ioPortB to store current values
//******************************************************************************************************************************
uint8_t debounceInputs() {
  int i = 0;
  readIOModule();                                       // read current state of ports on IO module
  // Apply bitmask and set current states.  PORTA - inputs are GPA4 - GPA7  (Front Panel Door Buttons 1 - 4, active low)
  // Active Low Inputs
  buttons[0].current = ((ioPortA & 0x80) != 0);
  buttons[1].current = ((ioPortA & 0x40) != 0);
  buttons[2].current = ((ioPortA & 0x20) != 0);
  buttons[3].current = ((ioPortA & 0x10) != 0);
  // Active High Inputs
  inputs[0].current = ((ioPortB & 0x01) == 0);
  inputs[1].current = ((ioPortB & 0x02) == 0);
  inputs[2].current = ((ioPortB & 0x04) == 0);
  inputs[3].current = ((ioPortB & 0x08) == 0);

  for (i = 0; i < 4; i++) {
    processInputs(buttons[i]);
    processInputs(inputs[i]);
  }
}

