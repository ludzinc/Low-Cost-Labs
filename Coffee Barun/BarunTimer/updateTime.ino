void updateTime() {
  // Format MM:SS for the display
  unitSeconds++;
  if (unitSeconds > 9) {
    unitSeconds = 0;
    if (tensSeconds == -1) {
      tensSeconds = 1;
    } else {
      tensSeconds++;
    }
    if (tensSeconds > 5) {
      tensSeconds = 0;
      if (unitMinutes == -1) {
        unitMinutes = 1;
      } else {
        unitMinutes++;
      }
      if (unitMinutes > 9) {
        unitMinutes = 0;
        if (tensMinutes == -1) {
          tensMinutes = 1;
        } else {
          tensMinutes++;
        }
        if (tensMinutes > 9) {
          tensMinutes = 0;    // clock rolls over to 00:00 - or should we halt?
        }
      }
    }
  }
}

void displayTime() {
  //Set digit
  switch (currentDigit) {
    case 0:
      displayDigit(-1);
      digitalWrite(DIGIT1, HIGH);
      digitalWrite(DIGIT2, HIGH);
      digitalWrite(DIGIT3, HIGH);
      digitalWrite(DIGIT4, LOW);
      displayDigit(unitSeconds);
      currentDigit = 1;
      break;
    case 1:
      displayDigit(-1);
      digitalWrite(DIGIT1, HIGH);
      digitalWrite(DIGIT2, HIGH);
      digitalWrite(DIGIT3, LOW);
      digitalWrite(DIGIT4, HIGH);
      displayDigit(tensSeconds);
      currentDigit = 2;
      break;
    case 2:
      displayDigit(-1);
      digitalWrite(DIGIT1, HIGH);
      digitalWrite(DIGIT2, LOW);
      digitalWrite(DIGIT3, HIGH);
      digitalWrite(DIGIT4, HIGH);
      displayDigit(unitMinutes);
      currentDigit = 3;
      break;
    case 3:
      displayDigit(-1);
      digitalWrite(DIGIT1, LOW);
      digitalWrite(DIGIT2, HIGH);
      digitalWrite(DIGIT3, HIGH);
      digitalWrite(DIGIT4, HIGH);
      displayDigit(tensMinutes);
      currentDigit = 0;
  }
}


