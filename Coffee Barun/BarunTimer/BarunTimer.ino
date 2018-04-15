// MODES OF OPERATION
// COUNT - Timer increments each second.  Started by "START" button.
// PAUSED - Timer stops counting, shows current time.
//          "START" button toggles between PAUSED and COUNT
//          TIME blinks in sync with colon when PAUSED
// STOPPED - Timer stops counting.  Mode entered by "STOP" button.
//          Pressing "STOP" again resets the display
//          COLON stopps blinking when timer is STOPPED (but visible)
// On Power On Reset:
//          Diplays shows 00:00
// When timer starts
//          Display clears leading Zeros, flashes colon at 1HZ i.e. "  : 1" for 1 second
#define SEGA   3
#define SEGB   4
#define SEGC   5
#define SEGD   6
#define SEGE   7
#define SEGF   8
#define SEGG   9
#define COLON  A0
// Upper colon tied to Digit3
// Lower colon tied to Digit 4
#define DIGIT1 10     // display tensMinutes
#define DIGIT2 11     // display unitMinutes
#define DIGIT3 12     // display tensSeconds
#define DIGIT4 13     // display unitMinutes  // also Built In LED
#define StartButton A3// Start Button input 
#define StopButton  A2// Stop Button input
#define debounceCount 200

unsigned long previousMillis = 0;
const long interval = 500;        // Flash COLON at 1 Hz
int unitSeconds = 0;
int tensSeconds = 0;
int unitMinutes = 0;
int tensMinutes = 0;
int halfSecond = 0;
int currentDigit = 0;
// Debounce Setup
int buttonStartState; // Active High Input
int buttonStartCounter;

// Modes of operation
enum mode {
  stopped,
  pause,
  count
} currentMode;

void setup() {
  Serial.begin(115200);
  Serial.println("Barun Awesome Timer is Awesome.");
  currentMode = stopped;
  // Initialise LED driver pins as outputs
  pinMode(SEGA, OUTPUT);
  pinMode(SEGB, OUTPUT);
  pinMode(SEGC, OUTPUT);
  pinMode(SEGD, OUTPUT);
  pinMode(SEGE, OUTPUT);
  pinMode(SEGF, OUTPUT);
  pinMode(SEGG, OUTPUT);
  pinMode(COLON, OUTPUT);
  pinMode(DIGIT1, OUTPUT);
  pinMode(DIGIT2, OUTPUT);
  pinMode(DIGIT3, OUTPUT);
  pinMode(DIGIT4, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(StartButton, INPUT);
  pinMode(StopButton, INPUT);
  // Turn all LEDs off (Digit Cathodes have to be set high)
  digitalWrite(SEGA, LOW);
  digitalWrite(SEGB, LOW);
  digitalWrite(SEGC, LOW);
  digitalWrite(SEGD, LOW);
  digitalWrite(SEGE, LOW);
  digitalWrite(SEGF, LOW);
  digitalWrite(SEGG, LOW);
  digitalWrite(COLON, HIGH);    // Show Colon at Power On Reset
  digitalWrite(DIGIT1, HIGH);
  digitalWrite(DIGIT2, HIGH);
  digitalWrite(DIGIT3, HIGH);
  digitalWrite(DIGIT4, HIGH);
  buttonStartState = 0; // Active High Input
  buttonStartCounter = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();
  // debouce inputs here
  int currentInput = 0;
  currentInput = digitalRead(StartButton);
  //  debounce(currentInput, &buttonStartState, &buttonStartCounter);
  //  debounce(digitalRead(StartButton), &buttonStartState, &buttonStartCounter);
  Serial.print("Debouncing currentState: ");
  Serial.print(buttonStartState); Serial.print(" :");
  Serial.print(currentInput); Serial.print(" :");
  Serial.println(buttonStartCounter);

  if (buttonStartState != currentInput) {
    buttonStartCounter++;
    if (buttonStartCounter >= debounceCount) {
      buttonStartCounter = 0;
      buttonStartState = currentInput; // WHY DOESNT THIS WORK??!!
      Serial.print("Current State is now "); Serial.println(buttonStartState);
      Serial.println("Debounced");
      delay(5000);
    }
  } else {
    buttonStartCounter = 0;
  }
  //Serial.print("buttonStartState is :"); Serial.print(buttonStartState); Serial.println(buttonStartCounter);
  Serial.print("Current State is now top be compared "); Serial.println(buttonStartState);
  if (buttonStartState = 1) {
    if (currentMode == count) {
      currentMode = pause;
    } else {
      currentMode = count;
    }
  }
  //  if (debounceStop.read() == 1) {
  //currentMode = stopped;
  //}
  if (currentMillis - previousMillis >= interval) {                 // Every Half Second
    previousMillis = currentMillis;

    if (currentMode == count) { // count = 2
      if (halfSecond == 0) {
        updateTime();
        //        digitalWrite(COLON, !digitalRead(COLON));
        digitalWrite(COLON, HIGH);
        halfSecond = 1;
      } else {
        digitalWrite(COLON, LOW);
        halfSecond = 0;
      }
    }
  }
  displayTime();
}

