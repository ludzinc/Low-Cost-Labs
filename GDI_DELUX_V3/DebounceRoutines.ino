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
  if ((debounce(input.current, input) == 0) && (input.lock == 0)) {
    input.flag = 1;
    input.lock = 1;
    DEBUG_PRINTLN("input set");
    yellowLEDFlag = yellowLED;
  }
  if ((debounce(input.current, input) == 1) && (input.lock == 1)) {
    input.lock = 0;
    yellowLEDFlag = 0;
    DEBUG_PRINTLN("input cleared");
  }
}
//******************************************************************************************************************************
// Scan and debounce all inputs.  Uses Global Variables ioPortA and ioPortB to store current values
//******************************************************************************************************************************
uint8_t debounceInputs() {
  int i = 0;
  readIOModule();                                       // read current state of ports on IO module
  // Active low buttons
  buttons[0].current = ((ioPortA & 0x20) != 0);
  buttons[1].current = ((ioPortA & 0x40) != 0);
  buttons[2].current = ((ioPortA & 0x80) != 0);
  buttons[3].current = ((ioPortA & 0x10) != 0);
  // Active high Inputs
  inputs[0].current = ((ioPortB & 0x01) == 0);
  inputs[1].current = ((ioPortB & 0x02) == 0);
  inputs[2].current = ((ioPortB & 0x04) == 0);
  inputs[3].current = ((ioPortB & 0x08) == 0);

  for (i = 0; i < 4; i++) {
    processInputs(buttons[i]);
    processInputs(inputs[i]);
  }
}
