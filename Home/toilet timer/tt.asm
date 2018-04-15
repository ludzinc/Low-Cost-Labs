
; CC5X Version 3.2, Copyright (c) B Knudsen Data
; C compiler for the PICmicro family
; ************   8. Mar 2010   0:00  *************

	processor  16F627A
	radix  DEC

INDF        EQU   0x00
STATUS      EQU   0x03
FSR         EQU   0x04
PORTA       EQU   0x05
TRISA       EQU   0x85
PORTB       EQU   0x06
TRISB       EQU   0x86
INTCON      EQU   0x0B
Carry       EQU   0
Zero_       EQU   2
RP0         EQU   5
RP1         EQU   6
IRP         EQU   7
OPTION_REG  EQU   0x81
PIR1        EQU   0x0C
T2CON       EQU   0x12
CMCON       EQU   0x1F
PIE1        EQU   0x8C
PCON        EQU   0x8E
PR2         EQU   0x92
RB0         EQU   0
RB1         EQU   1
RB2         EQU   2
RB3         EQU   3
RB6         EQU   6
RB7         EQU   7
TMR2IF      EQU   1
FSRTemp     EQU   0x2F
T1ms        EQU   0x30
T10ms       EQU   0x31
T100ms      EQU   0x32
svrWREG     EQU   0x70
svrSTATUS   EQU   0x20
Delay       EQU   0x21
Run         EQU   0x22
State       EQU   0x23
NewSecond   EQU   0x24
Ticks       EQU   0x25
PIRState    EQU   0x26
PIRCounter  EQU   0x27
PIRTest     EQU   0x28
PushbuttonState EQU   0x29
PushbuttonCounter EQU   0x2A
PushbuttonTest EQU   0x2B
NewSecond_2 EQU   0x2C
DIPBank     EQU   0x2C
BANKMask    EQU   0x2D
Dipswitch   EQU   0x2E
test        EQU   0x2C
state       EQU   0x2D
counter     EQU   0x2E
delayTime   EQU   0x2C
mscheck     EQU   0x2E

	GOTO main

  ; FILE TT.c
			;//#define   PIC16F627A
			;
			;//#include "16F627A.H"
			;#include "INT16CXX.H"
			;
			;#pragma origin = 0x0004																	// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???
	ORG 0x0004
			;#pragma config        = 0b0011.1111.0101.1000   	// MCLR Pin is input
			;//#pragma config        = 0b0011.1111.0111.1000   	// MCLR Pin is input
			;
			;#define PIR				RB0		// Input 	~ PIR (active high)
			;#define Pushbutton      RB1     // Input   	~ On Board Pushbutton (active low)
			;#define ValveOFF		RB2		// Output	~ Low side drive for H-Bridge
			;#define ValveON 	 	RB3		// Output	~ High side drive for H-Bridge
			;#define	DIPBank0		RB4		// Input / Output 	~ Active low to select DIP Switch 0 
			;#define DIPBank1		RB5		// Input / Output	~ Active low to select DIP Switch 1
			;#define Status0         RB6     // Output   ~ Green LED
			;#define Status1         RB7     // Output   ~ Red LED
			;
			;#define BANK0			0b11101111		// Mask to set read line for DIP0 low
			;#define BANK1			0b11011111		// Mask to set read line for DIP1 low
			;#define BANK0Mask		0b11101111		// Mask to set TRISB to set read line for DIP0 to output.
			;#define BANK1Mask		0b11011111		// Mask to set TRISB to set read line for DIP1 to output.
			;
			;#define Low				0
			;#define High			1
			;
			;#define MAX_DEBOUNCE_COUNT 	10			// How many times do we need the changed state = 100 ms
			;#define SolenoidDelay		250			// pulse on / off time for solenoid in milliseconds
			;#define LEDDelay			1000		// time to flash test leds at start up
			;
			;char FSRTemp;
			;char T1ms;								// millisecond counter
			;char T10ms;								// 10's millisecond counter
			;char T100ms;							// 100's millisecond counter
			;
			;#pragma codepage 0
			;interrupt IntHandler() {
IntHandler
			;   int_save_registers
	MOVWF svrWREG
	SWAPF STATUS,W
	BCF   0x03,RP0
	BCF   0x03,RP1
	MOVWF svrSTATUS
			;   FSRTemp = FSR;
	MOVF  FSR,W
	MOVWF FSRTemp
			;   
			;	if (TMR2IF == 1) {
	BTFSS 0x0C,TMR2IF
	GOTO  m002
			;		T1ms++;
	INCF  T1ms,1
			;			if(T1ms == 10) {
	MOVF  T1ms,W
	XORLW .10
	BTFSS 0x03,Zero_
	GOTO  m001
			;				T10ms++;
	INCF  T10ms,1
			;				T1ms = 0;
	CLRF  T1ms
			;				if (T10ms == 10) {
	MOVF  T10ms,W
	XORLW .10
	BTFSS 0x03,Zero_
	GOTO  m001
			;					T100ms++;
	INCF  T100ms,1
			;					T10ms = 0;
	CLRF  T10ms
			;//					if(T100ms == 10) {
			;//						T100ms =0;			// Handled in main code.  Probably dumb to do this.
			;//					}
			;				}
			;			}
			;		TMR2IF = 0;
m001	BCF   0x0C,TMR2IF
			;		}
			;
			;   FSR = FSRTemp;
m002	MOVF  FSRTemp,W
	MOVWF FSR
			;   int_restore_registers
	SWAPF svrSTATUS,W
	MOVWF STATUS
	SWAPF svrWREG,1
	SWAPF svrWREG,W
			;}
	RETFIE
			;
			;// Declare Function Prototypes
			;void InitialiseHardware(void);
			;void InitialiseGlobals(void);
			;char CheckSeconds(void);
			;char ReadDIP(char DIPBank, char BANKMask);	// Read required DIP switch
			;void ValveControl(char ValveState); 
			;void debounce(char test, char *state, char *counter);
			;void delay(long delayTime);
			;
			;void main(void) {
main
			;
			;char Delay;			// 0 - 255 second delay time
			;char Run;			// 0 - 255 second on time (may restirct to 4 bits later, to allow option switches)
			;char State;			// 0 = Waiting for trigger, 1 = Delay Count Down, 2 = Run Count Down
			;char NewSecond;		// 1 = New second, process event in loop
			;char Ticks;			// 10ms Counter
			;
			;
			;char PIRState;       				// The current debounced state of the input
			;char PIRCounter;   					// How many times have we seen the opposite state
			;char PIRTest;           			// The current undebouced test input
			;
			;char PushbuttonState;       		// The current debounced state of the input
			;char PushbuttonCounter;   			// How many times have we seen the opposite state
			;char PushbuttonTest;        		// The current undebouced test input
			;
			;InitialiseHardware();
	BSF   0x03,RP0
	BCF   0x03,RP1
	CALL  InitialiseHardware
			;InitialiseGlobals();
	BCF   0x03,RP0
	CALL  InitialiseGlobals
			;
			;	// Initialise variables
			;
			;	NewSecond = 0;
	CLRF  NewSecond
			;
			;	Delay = 0;						// read DIP0 for Delay time
	CLRF  Delay
			;	Run = 0;						// Read DIP1 for Run Time
	CLRF  Run
			;	State = 0;
	CLRF  State
			;
			;	PIRState = 0;					// Reset coutners and input states
	CLRF  PIRState
			;	PIRCounter = 0;
	CLRF  PIRCounter
			;	PIRTest = 0;
	CLRF  PIRTest
			;
			;	PushbuttonState = 0;
	CLRF  PushbuttonState
			;	PushbuttonCounter = 0;
	CLRF  PushbuttonCounter
			;	PushbuttonTest = 0;
	CLRF  PushbuttonTest
			;
			;	Ticks = 0;
	CLRF  Ticks
			;	
			;	Status0 = 1;
	BSF   0x06,RB6
			;	Status1 = 1;
	BSF   0x06,RB7
			;	delay(LEDDelay);				// Display LEDs at startup
	MOVLW .232
	MOVWF delayTime
	MOVLW .3
	MOVWF delayTime+1
	CALL  delay
			;   	Status0 = 0;
	BCF   0x06,RB6
			;  	Status1 = 0;
	BCF   0x06,RB7
			;
			;	while (1) {
			;		NewSecond = CheckSeconds();
m003	CALL  CheckSeconds
	MOVWF NewSecond
			;
			;		if(Ticks != T10ms) {			// 10ms has expired
	MOVF  Ticks,W
	XORWF T10ms,W
	BTFSC 0x03,Zero_
	GOTO  m004
			;			Ticks = T10ms;
	MOVF  T10ms,W
	MOVWF Ticks
			;			debounce(Pushbutton, &PushbuttonState, &PushbuttonCounter);		// Check inputs every 10mS
	CLRF  test
	BTFSC 0x06,RB1
	INCF  test,1
	MOVLW .41
	MOVWF state
	MOVLW .42
	MOVWF counter
	CALL  debounce
			;			debounce(PIR, &PIRState, &PIRCounter);
	CLRF  test
	BTFSC 0x06,RB0
	INCF  test,1
	MOVLW .38
	MOVWF state
	MOVLW .39
	MOVWF counter
	CALL  debounce
			;		}
			;	
			;		if(!PushbuttonState) {				// Input is Active Low.
m004	MOVF  PushbuttonState,1
	BTFSS 0x03,Zero_
	GOTO  m005
			;ValveON = 1;
	BSF   0x06,RB3
			;			State = 0;
	CLRF  State
			;			Status0 = 1;
	BSF   0x06,RB6
			;			Status1 = 1;					// Blink LED to show reinitalise has happened.	
	BSF   0x06,RB7
			;			delay(150);
	MOVLW .150
	MOVWF delayTime
	CLRF  delayTime+1
	CALL  delay
			;			Status0 = 0;
	BCF   0x06,RB6
			;			Status1 = 0;
	BCF   0x06,RB7
			;ValveON = 0;
	BCF   0x06,RB3
			;		}
			;
			;		switch (State) {
m005	MOVF  State,W
	BTFSC 0x03,Zero_
	GOTO  m006
	XORLW .1
	BTFSC 0x03,Zero_
	GOTO  m007
	XORLW .3
	BTFSC 0x03,Zero_
	GOTO  m009
	GOTO  m011
			;  			case 0:							// Waiting for input trigger (State 0)
			;	   			if(PIRState) {				// Check PIR input
m006	MOVF  PIRState,1
	BTFSC 0x03,Zero_
	GOTO  m003
			;					State = 1;				// If active move to next state					
	MOVLW .1
	MOVWF State
			;					Delay = ReadDIP(BANK0, BANK0Mask);		// Get Delay Time from DIP0
	MOVLW .239
	MOVWF DIPBank
	CALL  ReadDIP
	MOVWF Delay
			;					Run = ReadDIP(BANK1, BANK1Mask);		// Get Run time from DIP1
	MOVLW .223
	MOVWF DIPBank
	CALL  ReadDIP
	MOVWF Run
			;					T100ms = 0;				// Reset 'edge' of seconds counter
	CLRF  T100ms
			;					}
			;				break;
	GOTO  m003
			;  			case 1:							// Delay and then pulse solenoid on (State 1)
			;				if(NewSecond) {				// A new second has expired
m007	MOVF  NewSecond,1
	BTFSC 0x03,Zero_
	GOTO  m003
			;					if(Delay == 0) {		// Delay Time has now expired
	MOVF  Delay,1
	BTFSS 0x03,Zero_
	GOTO  m008
			;						ValveON = 1;
	BSF   0x06,RB3
			;						Status1 = 1;
	BSF   0x06,RB7
			;						delay(SolenoidDelay);
	MOVLW .250
	MOVWF delayTime
	CLRF  delayTime+1
	CALL  delay
			;						ValveON = 0;
	BCF   0x06,RB3
			;						Status1 = 0;						
	BCF   0x06,RB7
			;						State = 2;
	MOVLW .2
	MOVWF State
			;					}
			;					Delay--;				// Decrement Delay counter
m008	DECF  Delay,1
			;					NewSecond = 0;		
	CLRF  NewSecond
			;				}
			;				break;
	GOTO  m003
			;  			case 2:							
			;				if(NewSecond) {				// A new second has expired
m009	MOVF  NewSecond,1
	BTFSC 0x03,Zero_
	GOTO  m003
			;					NewSecond = 0;		
	CLRF  NewSecond
			;					if(Run == 0) {			// Delay Time has now expired
	MOVF  Run,1
	BTFSS 0x03,Zero_
	GOTO  m010
			;						ValveOFF = 1;
	BSF   0x06,RB2
			;						Status1 = 1;
	BSF   0x06,RB7
			;						delay(SolenoidDelay);
	MOVLW .250
	MOVWF delayTime
	CLRF  delayTime+1
	CALL  delay
			;						ValveOFF = 0;
	BCF   0x06,RB2
			;						Status1 = 0;
	BCF   0x06,RB7
			;						State = 3;
	MOVLW .3
	MOVWF State
			;					}
			;					Run--;					// Decrement Delay counter
m010	DECF  Run,1
			;				}
			;				break;
	GOTO  m003
			;			default:
			;    			State=0;					// Reset and start again
m011	CLRF  State
			;			   	break;
	GOTO  m003
			;		}// end switch (State)
			;	} //end While(1)
			;}// end main()
			;
			;
			;void InitialiseHardware(void) {
InitialiseHardware
			;  
			;	TRISA  = 0b11111111; 				// PORTA All HIGHZ (Input)
	MOVLW .255
	MOVWF TRISA
			;	TRISB  = 0b00110011;      			// PORTB (1:0) = INPUT
	MOVLW .51
	MOVWF TRISB
			;	PORTB  = 0b00000000;				// Set all outputs low
	BCF   0x03,RP0
	CLRF  PORTB
			;	OPTION = 0b10001000;   				// No weak pull ups, prescaler assigned to WDT
	MOVLW .136
	BSF   0x03,RP0
	MOVWF OPTION_REG
			;   	INTCON = 0b11000000;				// TMR2 used to provide 1ms ticks.
	MOVLW .192
	MOVWF INTCON
			;	CMCON  = 0b00000111;				// Enable RA0:3 as Digital Inputs
	MOVLW .7
	BCF   0x03,RP0
	MOVWF CMCON
			;	PCON   = 0b00001000;				// Set internal osciallator to 4MHz
	MOVLW .8
	BSF   0x03,RP0
	MOVWF PCON
			;	T2CON  = 0b00000101;				// TMR2 on, prescale = 1:4 (4us ticks with 4MHz oscillator), and postscale = 0
	MOVLW .5
	BCF   0x03,RP0
	MOVWF T2CON
			;	PIE1   = 0b00000010;				// Bit 1 enables TMR2 = PR2 interface
	MOVLW .2
	BSF   0x03,RP0
	MOVWF PIE1
			;	PIR1   = 0b00000000;				// Read this to see if TMR2 = PR2 flag is set
	BCF   0x03,RP0
	CLRF  PIR1
			;	PR2 = 250;							// TMR2 match value, for 1ms ticks
	MOVLW .250
	BSF   0x03,RP0
	MOVWF PR2
			;}
	RETURN
			;
			;void InitialiseGlobals(void) {
InitialiseGlobals
			;
			;	T1ms = 0;							// Reset Timers
	CLRF  T1ms
			;	T10ms = 0;
	CLRF  T10ms
			;	T100ms = 0;
	CLRF  T100ms
			;}
	RETURN
			;
			;char CheckSeconds(void) {
CheckSeconds
			;
			;char NewSecond;
			;
			;NewSecond = 0;
	CLRF  NewSecond_2
			;		if (T100ms > 9) {				
	MOVLW .10
	SUBWF T100ms,W
	BTFSS 0x03,Carry
	GOTO  m012
			;			T100ms = 0;
	CLRF  T100ms
			;			NewSecond=1;
	MOVLW .1
	MOVWF NewSecond_2
			;			Status0=!Status0;			// Keep this in the final code so you can see device is running.
	MOVLW .64
	XORWF PORTB,1
			;		}
			;
			;	return NewSecond;
m012	MOVF  NewSecond_2,W
	RETURN
			;}	
			;
			;char ReadDIP(char DIPBank, char BANKMask) {
ReadDIP
	MOVWF BANKMask
			;
			;	char Dipswitch;
			;	Dipswitch = 0;	
	CLRF  Dipswitch
			;
			;	TRISB = TRISB | 0b00110000;		// Set Both DIP control Lines as Inputs, leaving other as set previously
	MOVLW .48
	BSF   0x03,RP0
	IORWF TRISB,1
			;	TRISB = TRISB & BANKMask;		// Set RB4 or RB5 as OUTPUT, as per mask
	BCF   0x03,RP0
	MOVF  BANKMask,W
	BSF   0x03,RP0
	ANDWF TRISB,1
			;	PORTB = PORTB & DIPBank;		// Set RB4 or RB5 pin low, to enable read of that bank
	BCF   0x03,RP0
	MOVF  DIPBank,W
	ANDWF PORTB,1
			;	Dipswitch = ~PORTA;				// Read PORTA = DIPx Setting, but invert bits as DIP switches are active low
	COMF  PORTA,W
	MOVWF Dipswitch
			;	TRISB = TRISB | 0b00110000;		// Exit ensuring both DIP control lines are left as inputs (High Z)
	MOVLW .48
	BSF   0x03,RP0
	IORWF TRISB,1
			;
			;	return Dipswitch;
	BCF   0x03,RP0
	MOVF  Dipswitch,W
	RETURN
			;}
			;
			;//void ValveControl(char ValveState) {
			;
			;//	ValveState = 1;
			;//	Status1=1;
			;//	delay(SolenoidDelay);
			;//	Status1=0;
			;//	ValveState = 0;
			;
			;//}// End			
			;
			;
			;void debounce(char test, char *state, char *counter) {
debounce
			;	if ((*state) != test) {
	BCF   0x03,IRP
	MOVF  state,W
	MOVWF FSR
	MOVF  INDF,W
	XORWF test,W
	BTFSC 0x03,Zero_
	GOTO  m013
			;   	(*counter)++;
	BCF   0x03,IRP
	MOVF  counter,W
	MOVWF FSR
	INCF  INDF,1
			;		if ((*counter) >= MAX_DEBOUNCE_COUNT) {
	BCF   0x03,IRP
	MOVF  counter,W
	MOVWF FSR
	MOVLW .10
	SUBWF INDF,W
	BTFSS 0x03,Carry
	GOTO  m014
			;			(*counter) = 0;
	BCF   0x03,IRP
	MOVF  counter,W
	MOVWF FSR
	CLRF  INDF
			;			(*state) = test;
	BCF   0x03,IRP
	MOVF  state,W
	MOVWF FSR
	MOVF  test,W
	MOVWF INDF
			;		}
			;	} else {
	GOTO  m014
			;		(*counter) = 0;
m013	BCF   0x03,IRP
	MOVF  counter,W
	MOVWF FSR
	CLRF  INDF
			;	}
			;}
m014	RETURN
			;
			;void delay(long delayTime) {				// delay for 'delayTime' in milliseconds, up to 65535!
delay
			;
			;	char mscheck;
			;	mscheck = 0;
	CLRF  mscheck
			;
			;	while (delayTime >0 ){
m015	BTFSC delayTime+1,7
	GOTO  m017
	MOVF  delayTime,W
	IORWF delayTime+1,W
	BTFSC 0x03,Zero_
	GOTO  m017
			;		mscheck=T1ms;
	MOVF  T1ms,W
	MOVWF mscheck
			;		while(mscheck == T1ms) {
m016	MOVF  mscheck,W
	XORWF T1ms,W
	BTFSC 0x03,Zero_
			;			};								// wait for TMR2 to overflow, not interrupt driven
	GOTO  m016
			;		delayTime--;						// decrement dealy, exit if = 0
	DECF  delayTime,1
	INCF  delayTime,W
	BTFSC 0x03,Zero_
	DECF  delayTime+1,1
			;	} 
	GOTO  m015
			;}
m017	RETURN

	ORG 0x2007
	DATA 3F58H
	END


; *** KEY INFO ***

; 0x0004   30 word(s)  2 % : IntHandler
; 0x00A2   29 word(s)  2 % : InitialiseHardware
; 0x00BF    4 word(s)  0 % : InitialiseGlobals
; 0x00C3   12 word(s)  1 % : CheckSeconds
; 0x00CF   20 word(s)  1 % : ReadDIP
; 0x00E3   33 word(s)  3 % : debounce
; 0x0104   19 word(s)  1 % : delay
; 0x0022  128 word(s) 12 % : main

; RAM usage: 20 bytes (16 local), 204 bytes free
; Maximum call level: 1 (+1 for interrupt)
; Total of 276 code words (26 %)
