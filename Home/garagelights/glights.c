#include "INT18XXX.H"

#pragma origin = 0x0004																	// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???

#pragma config1  = 0b0000.1011			//  Internal Osciallator block, Clockouot RA6\
#pragma config2l = 0b0000.0000
#pragma config2h = 0b0000.0000			// not watchdog
#pragma config3h = 0b1000.0000			// mclr enabled
#pragma config4l = 0b1000.0000
#pragma config5l = 0b0000.0011
#pragma config5h = 0b1100.0000
#pragma config6l = 0b0000.0011
#pragma config6h = 0b1110.0000
#pragma config7l = 0b0000.0011
#pragma config7h = 0b0100.0000





#pragma bit GREEN_LED @ LATB.0
#pragma bit RED_LED   @ LATB.2

#pragma bit DOOR1	@ 	LATC.2
#pragma bit DOOR2	@	LATC.3
#pragma bit LIGHT	@	LATC.0

#define MAX_DEBOUNCE_COUNT 	10			// How many times do we need the changed state 
#define DOOR1_RUNTIME		5			// 500ms
#define DOOR2_RUNTIME		5			// 500ms
#define LIGHTS_RUNTIME		50			// 5 seconds

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
				GREEN_LED=!GREEN_LED;
				for(LoopCounter=0;LoopCounter<3;LoopCounter++){
					if(OUTPUTS[LoopCounter]>0){			// IF output flag has been set
						OUTPUTS[LoopCounter]--;
					}
					if(OUTPUTS[LoopCounter]>0){
						switch(LoopCounter){
							case 0:
								LIGHT=1;
								break;
							case 1:
								DOOR1=1;
								OUTPUTS[0]=LIGHTS_RUNTIME;	
								break;
							case 2:
								DOOR2=1;
								OUTPUTS[0]=LIGHTS_RUNTIME;
								break;
							default:
							 	break;	
							}
					}			 
					else {
						switch(LoopCounter){
							case 0:
								LIGHT=0;
								break;
							case 1:
								DOOR1=0;
								break;
							case 2:
								DOOR2=0;
								break;
							default:
							 	break;								
							 }									
					}		
				}
				if (T100ms == 10) {
					T100ms = 0;
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
int time = 0; 

	InitialiseHardware();
	InitialiseGlobals();

	while (1) {
	temp = PORTA;	
	for(i=0;i<3;i++){
		INPUTS_RAW[i]=temp&0x01;
		if(INPUTS_RAW[i]==0){
			RED_LED = 1;
			} else {
			RED_LED= 0;
		}			 
		temp = temp >> 1;
		debounce(INPUTS_RAW[i], &INPUTS_STATE[i], &DebounceCounters[i]);
		if(INPUTS_STATE[i]==0){
				time=TIMERS[i]; 								// SET OUTPUT FLAG HIGH IF INPUTS ARE DEBOUNCED
				OUTPUTS[i]=time;
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
	T2CON  = 0b00001101;				// TMR2 on, prescale = 1:4, and postscale = 1:2  (4us ticks with 8MHz oscillator)
	PIE1   = 0x02;
	IPR1  = 0x00;
	PR2	   = 250;						// 250 * 4us = 1ms interrupts 

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
	TIMERS[0] = LIGHTS_RUNTIME;
	TIMERS[1] = DOOR1_RUNTIME;
	TIMERS[2] = DOOR2_RUNTIME;
	
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


