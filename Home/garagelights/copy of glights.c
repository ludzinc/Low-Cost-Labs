#include "INT18XXX.H"

#pragma origin = 0x0004																	// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???
/*
#pragma bit RB0 @ 6.0
#pragma bit RB2 @ 6.2

#define GREEN_LED			RB0			// OUTPUT, BIT0 BCD WORD
#define RED_LED      		RB2   		// OUTPUT, BIT1 BCD WORD  
*/

#pragma bit GREEN_LED @ LATB.0
#pragma bit RED_LED   @ LATB.2

#define MAX_DEBOUNCE_COUNT 	10			// How many times do we need the changed state = 100 ms


// Declare Function Prototypes
void InitialiseHardware(void);
void InitialiseGlobals(void);
void debounce(char test, char *state, char *counter);

char FSRTemp;
char T1ms;								// 1 millisecond counter
char T10ms;								// 10's millisecond counter
char T100ms;							// 100's millisecond counter

char StartButtonState;
char StartButtonPrev;
char StartButtonCounter;

// DECALRE GOLBALS

uns16	TIMERS[4];
//char 	INPUTS[4];
char	INPUTS_RAW[4];
char	INPUTS_STATE[4];
char	OUTPUTS[4];
char	DebounceCounters[4];

char 	LoopCounter;

#pragma origin 0x18
interrupt lowPriorityIntServer(void)
{
    // W, STATUS and BSR are saved by the next macro.
uns16 sv_FSR0 = FSR0;
uns16 sv_FSR1 = FSR1;
uns16 sv_FSR2 = FSR2;
uns8 sv_PCLATH = PCLATH;
uns8 sv_PCLATU = PCLATU;
uns8 sv_PRODL = PRODL;
uns8 sv_PRODH = PRODH;
uns24 sv_TBLPTR = TBLPTR;
uns8 sv_TABLAT = TABLAT;
    int_save_registers
    
	if (TMR2IF == 1) {
		T1ms++;
		if(T1ms == 10){
			T10ms++;
			T1ms=0;
			if (T10ms == 10) {
				T100ms++;
				T10ms = 0;
				if (T100ms == /*10*/ 5) {
					T100ms = 0;
					GREEN_LED = !GREEN_LED;
	//				SecondsFlag = 1;
//	decrement coutners
					for(LoopCounter=0;LoopCounter<3;LoopCounter++){
						if(OUTPUTS[LoopCounter]==1){				// IF output flag has been set
							TIMERS[LoopCounter]--;
							if(TIMERS[LoopCounter]==0){		// decrement the timer
								OUTPUTS[LoopCounter]=0;		// if timer = 0 reset output flag
							}
						}
					}					
				RED_LED = OUTPUTS[0];				// turn on outputs if flags are high, turn off otherwise
				}
			}
		}	
		TMR2IF = 0;
	}
FSR0 = sv_FSR0;
FSR1 = sv_FSR1;
FSR2 = sv_FSR2;
PCLATH = sv_PCLATH;
PCLATU = sv_PCLATU;
PRODL = sv_PRODL;
PRODH = sv_PRODH;
TBLPTR = sv_TBLPTR;
TABLAT = sv_TABLAT;
   	int_restore_registers
}



void main(void) {

int i = 0;
int temp = 0;

	InitialiseHardware();
	InitialiseGlobals();


//	GREEN_LED = 1;

	while (1) {
/*	
	INPUTS_RAW[0]=PORTA.0;
	INPUTS_RAW[1]=PORTA.1;
	INPUTS_RAW[2]=PORTA.3;

	temp = PORTA;
	for(i=0;i<3;i++){
		INPUTS_RAW[i]=temp&0x01;
		temp = temp >> 1;
	}	
*/	
	temp = PORTA;	
	for(i=0;i<4;i++){
		INPUTS_RAW[i]=temp&0x01;
		temp = temp >> 1;
		debounce(INPUTS_RAW[i], &INPUTS_STATE[i], &DebounceCounters[i]);
		if(INPUTS_STATE[i]==0){
				OUTPUTS[i]=1;			// SET OUTPUT FLAG HIGH IF INPUTS ARE DEBOUNCED
				TIMERS[i]=5;
			}		
		}	
	}
}// end main()


void InitialiseHardware(void) {
  	
  	OSCCON = 0b0111.0000;
  	
  	TRISA  = 0x07; 						// PORTA (0 = OUTPUT)
  	TRISB  = 0x00; 						// PORTB (0 = OUTPUT)
  	TRISC  = 0x00; 						// PORTC (0 = OUTPUT)
	PORTA  = 0x00;				// Initialise PORTA
	PORTB  = 0x00;				// Initialise PORTB
	PORTC  = 0X00;
	ADCON1 = 0x0F;						// All portA inpouts are digital
	INTCON = 0b11000000;
//	CMCON  = 0b00000111;				// Enable RA0:3 as Digital Inputs
//	PCON   = 0b00001000;				// Set internal osciallator to 4MHz
	T2CON  = 0b00010101;				// TMR2 on, prescale = 1:4, and postscale = 1:1  (2us ticks with 8MHz oscillator)
	PIE1   = 0x02;
	IPR1  = 0x00;
	PR2	   = 250;						// 50 * 2us = 10ms interrupts

}

void InitialiseGlobals(void) {
	int i = 0;
	
	T1ms = 0;							// Reset Timers
	T10ms = 0;
	T100ms = 0;
//	SecondsFlag=0;
	for(i=0;i<4;i++){
		INPUTS_RAW[i]=0;
		INPUTS_STATE[i]=1;
		OUTPUTS[i]=0;
	}
	LoopCounter=0;

/*
	StartButtonState = StartButton;
	StartButtonPrev = StartButtonState;
	StartButtonCounter = 0;
	StopButtonState =0;
	StopButtonPrev =0;
	StopButtonCounter =0;
	State1 = STATE_STOPPED;
*/
}


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

