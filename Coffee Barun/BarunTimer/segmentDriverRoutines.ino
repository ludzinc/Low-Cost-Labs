// Set Segments to drive digits
void displayDigit(int character) {

  switch (character) {
    case 0:
      digitalWrite(SEGA, HIGH);
      digitalWrite(SEGB, HIGH);
      digitalWrite(SEGC, HIGH);
      digitalWrite(SEGD, HIGH);
      digitalWrite(SEGE, HIGH);
      digitalWrite(SEGF, HIGH);
      digitalWrite(SEGG, LOW);
      break;
    case 1:
      digitalWrite(SEGA, LOW);
      digitalWrite(SEGB, HIGH);
      digitalWrite(SEGC, HIGH);
      digitalWrite(SEGD, LOW);
      digitalWrite(SEGE, LOW);
      digitalWrite(SEGF, LOW);
      digitalWrite(SEGG, LOW);
      break;
    case 2:
      digitalWrite(SEGA, HIGH);
      digitalWrite(SEGB, HIGH);
      digitalWrite(SEGC, LOW);
      digitalWrite(SEGD, HIGH);
      digitalWrite(SEGE, HIGH);
      digitalWrite(SEGF, LOW);
      digitalWrite(SEGG, HIGH);
      break;
    case 3:
      digitalWrite(SEGA, HIGH);
      digitalWrite(SEGB, HIGH);
      digitalWrite(SEGC, HIGH);
      digitalWrite(SEGD, HIGH);
      digitalWrite(SEGE, LOW);
      digitalWrite(SEGF, LOW);
      digitalWrite(SEGG, HIGH);
      break;
    case 4:
      digitalWrite(SEGA, LOW);
      digitalWrite(SEGB, HIGH);
      digitalWrite(SEGC, HIGH);
      digitalWrite(SEGD, LOW);
      digitalWrite(SEGE, LOW);
      digitalWrite(SEGF, HIGH);
      digitalWrite(SEGG, HIGH);
      break;
    case 5:
      digitalWrite(SEGA, HIGH);
      digitalWrite(SEGB, LOW);
      digitalWrite(SEGC, HIGH);
      digitalWrite(SEGD, HIGH);
      digitalWrite(SEGE, LOW);
      digitalWrite(SEGF, HIGH);
      digitalWrite(SEGG, HIGH);
      break;
    case 6:
      digitalWrite(SEGA, HIGH);
      digitalWrite(SEGB, LOW);
      digitalWrite(SEGC, HIGH);
      digitalWrite(SEGD, HIGH);
      digitalWrite(SEGE, HIGH);
      digitalWrite(SEGF, HIGH);
      digitalWrite(SEGG, HIGH);
      break;
    case 7:
      digitalWrite(SEGA, HIGH);
      digitalWrite(SEGB, HIGH);
      digitalWrite(SEGC, HIGH);
      digitalWrite(SEGD, LOW);
      digitalWrite(SEGE, LOW);
      digitalWrite(SEGF, LOW);
      digitalWrite(SEGG, LOW);
      break;
    case 8:
      digitalWrite(SEGA, HIGH);
      digitalWrite(SEGB, HIGH);
      digitalWrite(SEGC, HIGH);
      digitalWrite(SEGD, HIGH);
      digitalWrite(SEGE, HIGH);
      digitalWrite(SEGF, HIGH);
      digitalWrite(SEGG, HIGH);
      break;
    case 9:
      digitalWrite(SEGA, HIGH);
      digitalWrite(SEGB, HIGH);
      digitalWrite(SEGC, HIGH);
      digitalWrite(SEGD, HIGH);
      digitalWrite(SEGE, LOW);
      digitalWrite(SEGF, HIGH);
      digitalWrite(SEGG, HIGH);
      break;
    default:                    // Set a digit to -1 to blank it
      digitalWrite(SEGA, LOW);
      digitalWrite(SEGB, LOW);
      digitalWrite(SEGC, LOW);
      digitalWrite(SEGD, LOW);
      digitalWrite(SEGE, LOW);
      digitalWrite(SEGF, LOW);
      digitalWrite(SEGG, LOW);
      break;
  }
}
