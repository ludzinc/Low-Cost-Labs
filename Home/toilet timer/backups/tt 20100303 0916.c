//#define   PIC16F627A

//#include "16F627A.H"
#include "INT16CXX.H"

#pragma origin = 0x0004


#define PIR				RB0		// Input 	~ PIR (active high)
#define Pushbutton      RB1     // Input   	~ On Board Pushbutton (active low)
#define LatchON			RB2		// Output	~ High side drive for H-Bridge
#define LatchOFF		RB3		// Output	~ Low side drive for H-Bridge
#define	DIPBank0		RB4		// Input / Output 	~ Active low to select DIP Switch 0 
#define DIPBank1		RB5		// Input / Output	~ Active low to select DIP Switch 1
#define Status0         RB6     // Output   ~ Green LED
#define Status1         RB7     // Output   ~ Red LED

#pragma config                 = 0b0010.0001.0011.0000   

char FSRTemp;
char T10ms;

#pragma codepage 0
interrupt IntHandler() {
   int_save_registers
   FSRTemp = FSR;
   
   if (T0IF == 1) {	// If TMR0 Overflow interrupt is set
	T10ms++;		// Update 10ms counter and then	
      T0IF = 0;		// Clear TMR0 Overflow flag
   }
   
   FSR = FSRTemp;
   int_restore_registers
}

// Declare Function Prototypes
void InitializeHardware(void);
char CheckSeconds(void);
char DebouncePIR(char ticks);			//Pass in no. ticks for debouncing
char DebouncePushbutton(char ticks);	//Pass in no. ticks for debouncing

void main(void) {

char Delay;			// 0 - 255 second delay time
char Run;			// 0 - 255 second on time (may restirct to 4 bits later, to allow option switches)
char State;			// 0 = Waiting for trigger, 1 = Delay Count Down, 2 = Run Count Down
char NewSecond;		// 1 = New second, process event in loop
char StatePushbutton;
char TicksPushbutton;
char StatePIR;
char TicksPIR;

InitializeHardware();

   	Status0 = 0;
   	Status1 = 0;
	T10ms = 0;
	NewSecond = 0;

	Delay = 5;		// read DIP0 for Delay time
	Run = 4;		// Read DIP1 for Run Time
	State = 0;

	TicksPushbutton = 0;
	TicksPIR = 0;
	
	while (1) {
		NewSecond = CheckSeconds();
		
		TicksPushbutton = DebouncePushbutton(TicksPushbutton);
		TicksPIR = DebouncePIR(TicksPIR);
		
		if(TicksPIR == 100) {
			StatePIR=1;					// Question - do we flush after first PIR trigger, or reset wait timer on each trigger
		} else {						// I'm going to do the former until told differently.
			StatePIR = 0;				// A sleep time between triggers needed?
		}	

		if(TicksPushbutton == 100) {
			StatePushbutton=1;
		} else {
			StatePushbutton = 0;
		}	


	
		if (StatePIR && (State == 0)) {	// Active PIR Input and was in 'wait state')
			State = 1;
			Status1 = 1;
			T10ms = 0;					// Reset Second Counter to prevent partial second timing
		}
	
		if (State == 1) {				// replace this hack with CASE statement!!
			if (NewSecond) {
				Delay--;
				NewSecond = 0;
				if(Delay == 0) {
					State = 0;		// Ready to run solenoid
					Status1=0;
					Delay = 4;
				}
			}		
		}	
		if (State == 2) {
		// Assert ON control, start timing run time, release ON control
		}
		if (State == 3) {
		// wait for run time to expire
		}
		if (State ==4) {
		// Assert OFF control, release OFF control, reset all timers etc to wait for next PIR event
		}



	
      } //end While(1)
}// end main()


void InitializeHardware(void) {
   
	TRISA = 0b11111111;      // PORTA All HIGHZ (Input)
	TRISB = 0b00000011;      // PORTB (1:0) = INPUT
	OPTION = 0b11010111;   
//  TIMR0 clocked internally
//  Prescaler (256) assigned to TIMR0
//  With 4MHz oscillator TIMR0 will overflow every 64uS
   	INTCON = 0b10100000;
//  ONLY TMR0 interrupts enabled
}

char CheckSeconds(void) {

char NewSecond;

NewSecond = 0;
		if (T10ms > 10) {				// 1 Second has expired
			T10ms = 0;
			NewSecond=1;
			Status0=!Status0;
		}
	return NewSecond;
}	

char DebouncePIR(char ticks) {
	if(PIR) {
		ticks--;
		if(ticks == 101) {				// If high for 100 loops (time =?, who cares?)
			ticks = 100;				// consider input debounced, prevent overflow
			}
	}
	if(!PIR) {
		ticks = 0;						// if input not active, reset counter.  Harsh, but works.				
	}	
	return ticks;		
}

char DebouncePushbutton(char ticks) {
	
	if(!Pushbutton) {
		ticks--;
		if(ticks == 101) {				// If high for 100 loops (time =?, who cares?)
			ticks = 100;				// consider input debounced, prevent overflow
			}
	}
	if(Pushbutton) {
		ticks = 0;						// if input not active, reset counter.  Harsh, but works.				
	}	
	return ticks;		
}