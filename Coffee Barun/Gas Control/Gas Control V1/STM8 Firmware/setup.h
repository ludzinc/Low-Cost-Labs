#define segA    PB_ODR_ODR5
#define segB    PC_ODR_ODR5
#define segC    PD_ODR_ODR2
#define segD    PC_ODR_ODR7
#define segE    PC_ODR_ODR6
#define segF    PB_ODR_ODR4
#define segG    PD_ODR_ODR3
#define segDP   PD_ODR_ODR1

#define digitON 1
#define digitOFF 0
#define segON   0
#define segOFF  1    

#define digitHunds PC_ODR_ODR3
#define digitTens  PC_ODR_ODR4
#define digitUnits PA_ODR_ODR3

int tickCounter = 0;           //Timer 2 interrupts every 500us (2kHz)
char ms5Counter = 0;
int seconds = 0;
char Units = 0;
char Tens = 0;
char Hundreds = 0;

char ADCRead = 0;
