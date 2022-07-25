/* Low Cost Labs Project Timer
  Program button selects one of 4 programs to time
  Select will start / stop timers
  LEDS indicate which program is timing
  Will time to 99HRS 59Mins then overflows
  - acutal time will be availble on serial output
  - program led will blink for each time it's overflowed *to do
*/

//#include "SevSeg.h"
//SevSeg sevseg; //Instantiate a seven segment controller object
#include <EEPROM.h>
#include <stdio.h>
#include <Arduino.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 10
#define DIO 11
#define LED0 9
#define LED1 6
#define LED2 5
#define LED3 3
#define buttonProgram 7
#define buttonSelect 8
#define programLED 12
#define selectLED 13
#define buttonProgramBrightness 63
#define buttonSelectBrightness 63

int buttonPins[] = {buttonProgram, buttonSelect};
int ledPins[] = {LED0, LED1, LED2, LED3};
int hours[5] = {0, 0, 0, 0, 0};
int minutes[5] = {0, 0, 0, 0, 0};
int seconds[5] = {0, 0, 0, 0, 0};
int counting[5] = {0, 0, 0, 0, 0};
int overflow[5] = {0, 0, 0, 0};
uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };
uint8_t LEDBrightness[] = {63, 63, 10, 63};

long displayTime = 0;
unsigned long debounceDelay = 50;
unsigned long longPressDelay = 2000;

typedef struct inputButton {
  uint8_t state;
  uint8_t longState;
  uint8_t lastState;
  uint32_t previousMillis;
};

inputButton buttons[2];

int currentProject = 4;
int setCurrentProject = 0;
int run = 0;
int toggleRun = 0;

char stringBuffer[100];

TM1637Display display(CLK, DIO);

void setup() {
  /** the current address in the EEPROM (i.e. which byte we're going to write to next) **/
  //  int addr = 0;
  //  for (int i = 0; i < 4; i++) {
  //    EEPROM.write((addr + i), (i * 10 + i));
  //    EEPROM.write((addr + i + 4), (i * 10 + i));
  //    EEPROM.write((addr + i + 8), 0);
  //    EEPROM.write((addr + i + 12), 0);
  //  }
  //  Serial.println("Initial EEPROM Values written.");
  //
  for (int i = 0; i < 2; i++) {
    buttons[i].state = 1;
    buttons[i].longState = 1;
    buttons[i].lastState = 1;
  }
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    analogWrite(ledPins[i], LEDBrightness[i]);
  }

  pinMode(buttonProgram, INPUT_PULLUP);
  pinMode(buttonSelect, INPUT_PULLUP);
  pinMode(programLED, OUTPUT);
  pinMode(selectLED, OUTPUT);
  analogWrite(programLED, buttonProgramBrightness);
  analogWrite(selectLED, buttonSelectBrightness);

  Serial.println("Reading EEPROM Values.");
  readTimers();
  for (int i = 0; i < 4; i++) {
    sprintf(stringBuffer, "Program %i values are: %d:%d:%d - %d", i, hours[i], minutes[i], seconds[i], overflow[i]);
    Serial.println(stringBuffer);
  }
  display.setBrightness(0x01);
  display.setSegments(data);
  Serial.println("LOW COST LABS Project Timer Started.");
  delay(1000);
}

void loop() {
  static unsigned long timer = millis();
  static int deciSeconds = 0;
  displayTime = hours[currentProject] * 100 + minutes[currentProject];
  debounceButtons();
  checkProgram();

  if (millis() - timer >= 100) {
    timer += 100;
    if (run == 1) {
      deciSeconds++; // 100 milliSeconds is equal to 1 deciSecond
    }
    if (deciSeconds == 5) { // Reset to 0 after counting for 1000 seconds.
      display.showNumberDecEx(displayTime, 0b01000000, true, 4, 0);
    }
    if (deciSeconds == 10) { // Reset to 0 after counting for 1000 seconds.
      deciSeconds = 0;
      display.showNumberDecEx(displayTime, 0b00000000, true, 4, 0); // No Colon
      seconds[currentProject]++;
      if (seconds[currentProject] == 60 ) {
        seconds[currentProject] = 0;
        minutes[currentProject]++;
        if (minutes[currentProject] == 60) {
          minutes[currentProject] = 0;
          hours[currentProject]++;
        }
      }
      storeTimers();
      sprintf(stringBuffer, "Current Time for Poject %i is %d:%d:%d with overflow %d", currentProject, hours[currentProject], minutes[currentProject], seconds[currentProject], overflow[currentProject]);
      Serial.println(stringBuffer);
    }
  }
  if (run == 0) {
    display.showNumberDecEx(displayTime, 0b01000000, true, 4, 0);
  }
}

void storeTimers() {
  /** the current address in the EEPROM (i.e. which byte we're going to write to next) **/
  int addr = 0;
  for (int i = 0; i < 4; i++) {
    EEPROM.write((addr + i), hours[i]);
    EEPROM.write((addr + i + 4), minutes[i]);
    EEPROM.write((addr + i + 8), seconds[i]);
    EEPROM.write((addr + i + 12), overflow[i]);
  }
  //  Serial.println("EEPROM Values written.");
}

void readTimers() {
  /** the current address in the EEPROM (i.e. which byte we're going to write to next) **/
  int addr = 0;
  for (int i = 0; i < 4; i++) {
    hours[i] = EEPROM.read(addr + i);
    minutes[i] = EEPROM.read(addr + i + 4);
    seconds[i] = EEPROM.read(addr + i + 8);
    overflow[i] = EEPROM.read(addr + i + 12);
    if (hours[i] > 99) {
      hours[i] = 0;
    }
    if (minutes[i] > 99) {
      minutes[i] = i + 1;
    }
    if (seconds[i] > 99) {
      seconds[i] = 0;
    }
    if (overflow[i] == 255) {
      overflow[i] = 0;
    }
  }
  hours[4] = 88;
  minutes[4] = 88;
  //  Serial.println("EEPROM Values read.");
}

void setLEDS() {
  //  Serial.print("Setting LEDs. Current Project is : "); Serial.print(currentProject);
  //  Serial.print(" LED"); Serial.print(ledPins[currentProject]);
  //  Serial.print(" Brightness : "); Serial.println( LEDBrightness[currentProject]);
  if (currentProject < 4) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(ledPins[i], LOW);
    }
    analogWrite(ledPins[currentProject], LEDBrightness[currentProject]);
  } else {
    for (int i = 0; i < 4; i++) {
      analogWrite(ledPins[currentProject], LEDBrightness[currentProject]);
    }
  }
}

void debounceButtons() {
  for (int i = 0; i < 2; i++) {
    // read the state of the switch into a local variable:
    int reading = digitalRead(buttonPins[i]);
    if (reading != buttons[i].lastState) {
      // reset the debouncing timer
      //      Serial.println("Button state change detected");
      buttons[i].previousMillis = millis();
    }
    if ((millis() - buttons[i].previousMillis) > debounceDelay) {
      if (reading != buttons[i].state) {
        buttons[i].state = reading;                           // button is debounced, short press
        //        Serial.println("Button debounced");
      }
    }
    if ((millis() - buttons[i].previousMillis) > longPressDelay) {
      if (reading != buttons[i].longState) {
        buttons[i].longState = reading;                           // button is debounced, short press
        //        Serial.println("Button Long Press Detected");
      }
    }
    if (reading == 1) {
      buttons[i].state = 1;
      buttons[i].longState = 1;
    }
    buttons[i].lastState = reading;
  }
}

void checkProgram () {
  if ((setCurrentProject == 0) && (buttons[0].state == 0)) {
    setCurrentProject = 1;
    run = 0;                          // pause all programmes while cycling through
    currentProject++;
    if (currentProject > 3) {
      currentProject = 0;
    }
    displayTime = hours[currentProject] * 100 + minutes[currentProject];
    Serial.print("Current program is :"); Serial.println(currentProject);
    setLEDS();
  }
  if (buttons[0].state == 1) {
    setCurrentProject = 0;
  }
  if ((toggleRun == 0) && (buttons[1].state == 0) && (currentProject < 4)) {
    toggleRun = 1;
    if (run == 0) {
      run = 1;
      counting[currentProject] = 1;
    } else {
      run = 0;
    }
  }
  if (buttons[1].state == 1) {
    toggleRun = 0;
  }
  if ((buttons[1].longState == 0) && (counting[currentProject] == 1)) {
    Serial.print("Resetting Project "); Serial.println(currentProject);
    hours[currentProject] = 0;
    minutes[currentProject] = 0;
    counting[currentProject] = 0;
    run = 0;
    buttons[1].longState = 1;
    storeTimers();
  }
}
/// END ///
