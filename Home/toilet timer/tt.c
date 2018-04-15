//#define   PIC16F627A

//#include "16F627A.H"
#include "INT16CXX.H"

#pragma origin = 0x0004																	// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???
#pragma config        = 0b0011.1111.0101.1000   	// MCLR Pin is input
//#pragma config        = 0b0011.1111.0111.1000   	// MCLR Pin is input

#define PIR				RB0		// Input 	~ PIR (active high)
#define Pushbutton      RB1     // Input   	~ On Board Pushbutton (active low)
#define ValveOFF		RB2		// Output	~ Low side drive for H-Bridge
#define ValveON 	 	RB3		// Output	~ High side drive for H-Bridge
#define	DIPBank0		RB4		// Input / Output 	~ Active low to select DIP Switch 0 
#define DIPBank1		RB5		// Input / Output	~ Active low to select DIP Switch 1
#define Status0         RB6     // Output   ~ Green LED
#define Status1         RB7     // Output   ~ Red LED

#define BANK0			0b11101111		// Mask to set read line for DIP0 low
#define BANK1			0b11011111		// Mask to set read line for DIP1 low
#define BANK0Mask		0b11101111		// Mask to set TRISB to set read line for DIP0 to output.
#define BANK1Mask		0b11011111		// Mask to set TRISB to set read line for DIP1 to output.

#define Low				0
#define High			1

#define MAX_DEBOUNCE_COUNT 	10			// How many times do we need the changed state = 100 ms
#define SolenoidDelay		250			// pulse on / off time for solenoid in milliseconds
#define LEDDelay			1000		// time to flash test leds at start up

char FSRTemp;
char T1ms;								// millisecond counter
char T10ms;								// 10's millisecond counter
char T100ms;							// 100's millisecond counter

#pragma codepage 0
interrupt IntHandler() {
   int_save_registers
   FSRTemp = FSR;
   
	if (TMR2IF == 1) {
		T1ms++;
			if(T1ms == 10) {
				T10ms++;
				T1ms = 0;
				if (T10ms == 10) {
					T100ms++;
					T10ms = 0;
//					if(T100ms == 10) {
//						T100ms =0;			// Handled in main code.  Probably dumb to do this.
//					}
				}
			}
		TMR2IF = 0;
		}

   FSR = FSRTemp;
   int_restore_registers
}

// Declare Function Prototypes
void InitialiseHardware(void);
void InitialiseGlobals(void);
char CheckSeconds(void);
char ReadDIP(char DIPBank, char BANKMask);	// Read required DIP switch
void ValveControl(char ValveState); 
void debounce(char test, char *state, char *counter);
void delay(long delayTime);

void main(void) {

char Delay;			// 0 - 255 second delay time
char Run;			// 0 - 255 second on time (may restirct to 4 bits later, to allow option switches)
char State;			// 0 = Waiting for trigger, 1 = Delay Count Down, 2 = Run Count Down
char NewSecond;		// 1 = New second, process event in loop
char Ticks;			// 10ms Counter


char PIRState;       				// The current debounced state of the input
char PIRCounter;   					// How many times have we seen the opposite state
char PIRTest;           			// The current undebouced test input

char PushbuttonState;       		// The current debounced state of the input
char PushbuttonCounter;   			// How many times have we seen the opposite state
char PushbuttonTest;        		// The current undebouced test input

InitialiseHardware();
InitialiseGlobals();

	// Initialise variables

	NewSecond = 0;

	Delay = 0;						// read DIP0 for Delay time
	Run = 0;						// Read DIP1 for Run Time
	State = 0;

	PIRState = 0;					// Reset coutners and input states
	PIRCounter = 0;
	PIRTest = 0;

	PushbuttonState = 0;
	PushbuttonCounter = 0;
	PushbuttonTest = 0;

	Ticks = 0;
	
	Status0 = 1;
	Status1 = 1;
	delay(LEDDelay);				// Display LEDs at startup
   	Status0 = 0;
  	Status1 = 0;

	while (1) {
		NewSecond = CheckSeconds();

		if(Ticks != T10ms) {			// 10ms has expired
			Ticks = T10ms;
			debounce(Pushbutton, &PushbuttonState, &PushbuttonCounter);		// Check inputs every 10mS
			debounce(PIR, &PIRState, &PIRCounter);
		}
	
		if(!PushbuttonState) {				// Input is Active Low.
ValveON = 1;
			State = 0;
			Status0 = 1;
			Status1 = 1;					// Blink LED to show reinitalise has happened.	
			delay(150);
			Status0 = 0;
			Status1 = 0;
ValveON = 0;
		}

		switch (State) {
  			case 0:							// Waiting for input trigger (State 0)
	   			if(PIRState) {				// Check PIR input
					State = 1;				// If active move to next state					
					Delay = ReadDIP(BANK0, BANK0Mask);		// Get Delay Time from DIP0
					Run = ReadDIP(BANK1, BANK1Mask);		// Get Run time from DIP1
					T100ms = 0;				// Reset 'edge' of seconds counter
					}
				break;
  			case 1:							// Delay and then pulse solenoid on (State 1)
				if(NewSecond) {				// A new second has expired
					if(Delay == 0) {		// Delay Time has now expired
						ValveON = 1;
						Status1 = 1;
						delay(SolenoidDelay);
						ValveON = 0;
						Status1 = 0;						
						State = 2;
					}
					Delay--;				// Decrement Delay counter
					NewSecond = 0;		
				}
				break;
  			case 2:							
				if(NewSecond) {				// A new second has expired
					NewSecond = 0;		
					if(Run == 0) {			// Delay Time has now expired
						ValveOFF = 1;
						Status1 = 1;
						delay(SolenoidDelay);
						ValveOFF = 0;
						Status1 = 0;
						State = 3;
					}
					Run--;					// Decrement Delay counter
				}
				break;
			default:
    			State=0;					// Reset and start again
			   	break;
		}// end switch (State)
	} //end While(1)
}// end main()


void InitialiseHardware(void) {
  
	TRISA  = 0b11111111; 				// PORTA All HIGHZ (Input)
	TRISB  = 0b00110011;      			// PORTB (1:0) = INPUT
	PORTB  = 0b00000000;				// Set all outputs low
	OPTION = 0b10001000;   				// No weak pull ups, prescaler assigned to WDT
   	INTCON = 0b11000000;				// TMR2 used to provide 1ms ticks.
	CMCON  = 0b00000111;				// Enable RA0:3 as Digital Inputs
	PCON   = 0b00001000;				// Set internal osciallator to 4MHz
	T2CON  = 0b00000101;				// TMR2 on, prescale = 1:4 (4us ticks with 4MHz oscillator), and postscale = 0
	PIE1   = 0b00000010;				// Bit 1 enables TMR2 = PR2 interface
	PIR1   = 0b00000000;				// Read this to see if TMR2 = PR2 flag is set
	PR2 = 250;							// TMR2 match value, for 1ms ticks
}

void InitialiseGlobals(void) {

	T1ms = 0;							// Reset Timers
	T10ms = 0;
	T100ms = 0;
}

char CheckSeconds(void) {

char NewSecond;

NewSecond = 0;
		if (T100ms > 9) {				
			T100ms = 0;
			NewSecond=1;
			Status0=!Status0;			// Keep this in the final code so you can see device is running.
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

//void ValveControl(char ValveState) {

//	ValveState = 1;
//	Status1=1;
//	delay(SolenoidDelay);
//	Status1=0;
//	ValveState = 0;

//}// End			


void debounce(char test, char *state, char *counter) {
	if ((*state) != test) {
   	(*counter)++;
		if ((*counter) >= MAX_DEBOUNCE_COUNT) {
			(*counter) = 0;
			(*state) = test;
		}
	} else {
		(*counter) = 0;
	}
}

void delay(long delayTime) {				// delay for 'delayTime' in milliseconds, up to 65535!

	char mscheck;
	mscheck = 0;

	while (delayTime >0 ){
		mscheck=T1ms;
		while(mscheck == T1ms) {
			};								// wait for TMR2 to overflow, not interrupt driven
		delayTime--;						// decrement dealy, exit if = 0
	} 
}




