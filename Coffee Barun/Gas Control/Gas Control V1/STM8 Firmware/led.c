#include "setup.h"

void setupHardware(void){
  
   
  //Set all pins as outputs
  PA_DDR = 0xFF;    
  PB_DDR = 0xFF;
  PC_DDR = 0xFF;  
  PD_DDR = 0xFF;  
   
  //Set Push Pull ouputs (CR1) for Digit Drivers and 2MHz speed (CR2)
  //Common Anode 100's digit (PC3)
  PC_CR1_C13 = 1; //Push Pull Output
  PC_CR2_C23 = 0; //2MHz speed output
  //Common Anode 10's digit (PC4) - this isn't working, why....
  PC_CR1_C14 = 1; //Push Pull Output
  PC_CR2_C24 = 0; //2MHz speed output
  //Common Anode 1's digit (PA3)
  PA_CR1_C13 = 1; //Push Pull Output
  PA_CR2_C23 = 0; //2MHz speed output
  
  //Turn off all pins
  PA_ODR = 0; 
  PB_ODR = 0; 
  PC_ODR = 0; 
  PD_ODR = 0; 
  /*
  digitHunds = digitON;
  digitTens = digitON;
  digitUnits = digitON;
  */
  segA = segOFF;
  segB = segOFF;
  segC = segOFF;
  segD = segOFF;
  segE = segOFF;
  segF = segOFF;
  segG = segOFF;
  segDP = segON;
   
}

void displayDigit(char digit){
// take in two parameters - the digit and the value.  digit is 100's or 10's or 1's, value = 0 - 9
  
  // Character 0 - 9 in lower 7 bits of digit.  MSB of digit sets decimal point
  
  /* Character Map
  *           0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
  * A:  PB5   0  1  0  0  1  0  0  0  0  0  0  1  0  1  0  0
  * B:  PC5   0  0  0  0  0  1  1  0  0  0  0  1  1  0  1  1
  * C:  PD2   0  0  1  0  0  0  0  0  0  0  0  0  1  0  1  1
  * D:  PC7   0  1  0  0  1  0  0  1  0  0  1  0  0  0  0  1
  * E:  PC6   0  1  0  1  1  1  0  1  0  1  0  0  0  0  0  0
  * F:  PB4   0  1  1  1  0  0  0  1  0  0  0  0  0  1  0  0
  * G:  PD3   1  1  0  0  0  0  0  1  0  0  0  0  1  0  0  0
  */
   switch (digit & 0x7f)
  {
  case 0:  // 0
    segA = segON;
    segB = segON;
    segC = segON;
    segD = segON;
    segE = segON;
    segF = segON;
    segG = segOFF;
    break;
  case 1:  // 1
    segA = segOFF;
    segB = segON;
    segC = segON;
    segD = segOFF;
    segE = segOFF;
    segF = segOFF;
    segG = segOFF;
    break;
  case 2:  // 2
    segA = segON;
    segB = segON;
    segC = segOFF;
    segD = segON;
    segE = segON;
    segF = segOFF;
    segG = segON;
    break;
  case 3:  // 2
    segA = segON;
    segB = segON;
    segC = segON;
    segD = segON;
    segE = segOFF;
    segF = segOFF;
    segG = segON;
    break;
  case 4:  // 2
    segA = segOFF;
    segB = segON;
    segC = segON;
    segD = segOFF;
    segE = segOFF;
    segF = segON;
    segG = segON;
    break;
  case 5:  // 2
    segA = segON;
    segB = segOFF;
    segC = segON;
    segD = segON;
    segE = segOFF;
    segF = segON;
    segG = segON;
    break;
  case 6:  // 2
    segA = segON;
    segB = segOFF;
    segC = segON;
    segD = segON;
    segE = segON;
    segF = segON;
    segG = segON;
    break;
  case 7:  // 2
    segA = segON;
    segB = segON;
    segC = segON;
    segD = segOFF;
    segE = segOFF;
    segF = segOFF;
    segG = segOFF;
    break;
  case 8:  // 2
    segA = segON;
    segB = segON;
    segC = segON;
    segD = segON;
    segE = segON;
    segF = segON;
    segG = segON;
    break;
  case 9:  // 2
    segA = segON;
    segB = segON;
    segC = segON;
    segD = segON;
    segE = segOFF;
    segF = segON;
    segG = segON;
    break;
  default: // Blank
    segA = segOFF;
    segB = segOFF;
    segC = segOFF;
    segD = segOFF;
    segE = segOFF;
    segF = segOFF;
    segG = segOFF;
  }
  if (digit & 0x80) // DP
  {
    segDP = segON;
  }
  else
  {
    segDP = segOFF;
  }
  
}
//
// Populate the display 
//
void populateDisplay(int value){
  
  Units = value % 10;
  Tens = value / 10;
  Tens = Tens % 10;
  Hundreds = value / 100;
  
  if(tickCounter < 3){
      digitUnits = digitON;
      digitTens = digitOFF;
      digitHunds = digitOFF;
      displayDigit(Units);
      
    }
    else if(tickCounter < 6){
      digitUnits = digitOFF;
      digitTens = digitON;
      digitHunds = digitOFF;
      displayDigit(Tens);
    }
    else if(tickCounter < 9){
      digitUnits = digitOFF;
      digitTens = digitOFF;
      digitHunds = digitON;
      displayDigit(Hundreds);
    }
}

