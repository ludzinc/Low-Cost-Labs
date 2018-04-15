//#define   PIC16F627A

//#include "16F627A.H"
#include "INT16CXX.H"

#pragma origin = 0x0004																	// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???
#pragma config        = 0b0010.0001.0001.0000   	// MCLR Pin is input

#define PIR				RB0		// Input 	~ PIR (active high)
#define Pushbutton      RB1     // Input   	~ On Board Pushbutton (active low)
#define ValveON			RB2		// Output	~ High side drive for H-Bridge
#define ValveOFF 	 	RB3		// Output	~ Low side drive for H-Bridge
#define	DIPBank0		RB4		// Input / Output 	~ Active low to select DIP Switch 0 
#define DIPBank1		RB5		// Input / Output	~ Active low to select DIP Switch 1
#define Status0         RB6     // Output   ~ Green LED
#define Status1         RB7     // Output   ~ Red LED

#define BANK0			0b11101111		// Mask to set read line for DIP0 low
#define BANK1			0b11011111		// Mask to set read line for DIP1 low
#define BANK0Mask		0b11101111		// Mask to set TRISB to set read line for DIP0 to output.
#define BANK1Mask		0b11011111		// Mask to set TRISB to set read line for DIP1 to output.


char FSRTemp;
char T10ms;

#pragma codepage 0
interrupt IntHandler() {
   int_save_registers
   FSRTemp = FSR;
   
	if (T0IF == 1) {	// If TMR0 Overflow interrupt is set
		T10ms++;		// Update 10ms counter and then	
      	T0IF = 0;		// Clear TMR0 Overflow flag
		TMR0 = 215;		// will give 40 'ticks' before overflow.  40 x 256us = 10.024ms with 4MHz oscillator
   }
   
   FSR = FSRTemp;
   int_restore_registers
}

// Declare Function Prototypes
void InitializeHardware(void);
char CheckSeconds(void);
char ReadDIP(char DIPBank, char BANKMask);	// Read required DIP switch
void ValveControl(char ValveState); 

void main(void) {

char Delay;			// 0 - 255 second delay time
char Run;			// 0 - 255 second on time (may restirct to 4 bits later, to allow option switches)
char State;			// 0 = Waiting for trigger, 1 = Delay Count Down, 2 = Run Count Down
char NewSecond;		// 1 = New second, process event in loop
unsigned int Debounce;		// counter for debouncing inputs

InitializeHardware();

   	Status0 = 0;
   	Status1 = 0;
	T10ms = 0;
	NewSecond = 0;

	Delay = 0;		// read DIP0 for Delay time
	Run = 0;		// Read DIP1 for Run Time
	State = 0;

	while (1) {
		NewSecond = CheckSeconds();

		if(!Pushbutton) {
			Debounce = 65535; // max unsigned int.  do better later.
			while(Debounce>0){
				Debounce--;
			}
			if(!Pushbutton) {	
				State = 0;
				Status0 = 1;
				Status1 = 1;
				T10ms = 0;
				Debounce = 65535; // max unsigned int.  do better later.
				while(Debounce>0){
					Debounce--;
				}
				Status0 = 0;
				Status1 = 0;
			}
		}

		switch (State) {
  			case 0:							// Waiting for input trigger (State 0)
	   			if(PIR) {					// Check PIR input
					Debounce = 65535;
					while(Debounce>0){
						Debounce--;
					}
					if(PIR) {
						State = 1;				// If active move to next state				
						Delay = ReadDIP(BANK0, BANK0Mask);		// Get Delay Time from DIP0
						Run = ReadDIP(BANK1, BANK1Mask);		// Get Run time from DIP1
						T10ms = 0;				// Reset 'edge' of seconds counter
					}
				}
				break;
  			case 1:							// Delay and then pulse solenoid on (State 1)
				if(NewSecond) {				// A new second has expired
					Delay--;				// Decrement Delay counter
					NewSecond = 0;		
					if(Delay == 0) {		// Delay Time has now expired
						ValveControl(ValveON);
						State = 2;
					}
				}
				break;
  			case 2:							
				if(NewSecond) {				// A new second has expired
					Run--;					// Decrement Delay counter
					NewSecond = 0;		
					if(Run == 0) {			// Delay Time has now expired
						ValveControl(ValveOFF);
						State = 3;
					}
				}
				break;
			default:
    			State=0;					// Reset and start again
			   	break;
		}// end switch (State)
	} //end While(1)
}// end main()


void InitializeHardware(void) {

unsigned int delay;
unsigned int delay1;

delay = 65535;
delay1 = 65535;
   
	TRISA = 0b11111111; 				// PORTA All HIGHZ (Input)
	TRISB = 0b00110011;      			// PORTB (1:0) = INPUT
	OPTION = 0b11010111;   				
//  TIMR0 clocked internally
//  Prescaler (256) assigned to TMR0
//  With 4MHz oscillator (1MHz to TMR0) TMR0 will overflow every 256uS
   	INTCON = 0b10100000;
	CMCON  = 0b00000111;				// Enable RA0:3 as Digital Inputs, motherfucker!
//  ONLY TMR0 interrupts enabled

	Status0 = 1;
	Status1 = 1;
	while(delay > 0) {
		delay--;
		while(delay1 > 0) {
			delay1--;
		}
	}
	Status0 = 0;
	Status1 = 0;
}

char CheckSeconds(void) {

char NewSecond;

NewSecond = 0;
		if (T10ms > 99) {				// 99 * 10.024 ms = 0.992376s, for 0.7624% error
			T10ms = 0;
			NewSecond=1;
			Status0=!Status0;
		}
	return NewSecond;
}	

char ReadDIP(char DIPBank, char BANKMask) {

	char Dipswitch;
	Dipswitch = 0;	

	TRISB = TRISB | 0b00110000;		// Set Both DIP control Lines as Inputs, leaving other as set previously
	TRISB = TRISB & BANKMask;		// Set RB4 or RB5 as OUTPUT, as per mask
	PORTB = PORTB & DIPBank;		// Set RB4 or RB5 pin low, to enable read of that bank
	Dipswitch = ~PORTA;				// Read PORTA = DIPx Setting, but invert bits as DIP switches are active low
	TRISB = TRISB | 0b00110000;		// Exit ensuring both DIP control lines are left as inputs (High Z)

	return Dipswitch;
}

void ValveControl(char ValveState) {

unsigned long delay;

delay = 0;

	ValveState = 1;
	Status1=1;	
	delay = 65535;
	while(delay>0){
		delay--;
	}
	ValveState = 0;
	Status1=0;		
}// End			
