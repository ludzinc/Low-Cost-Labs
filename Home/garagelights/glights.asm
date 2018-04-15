
; CC8E Version 1.3D, Copyright (c) B Knudsen Data
; C compiler for the PIC18 microcontrollers
; ************   2. Jul 2011  23:29  *************

	processor  PIC18F2321
	radix  DEC

PCLATU      EQU   0xFFB
PCLATH      EQU   0xFFA
TBLPTR      EQU   0xFF6
TABLAT      EQU   0xFF5
PRODH       EQU   0xFF4
PRODL       EQU   0xFF3
INTCON      EQU   0xFF2
INDF0       EQU   0xFEF
POSTINC0    EQU   0xFEE
FSR0        EQU   0xFE9
FSR1        EQU   0xFE1
BSR         EQU   0xFE0
FSR2        EQU   0xFD9
STATUS      EQU   0xFD8
Carry       EQU   0
Zero_       EQU   2
OSCCON      EQU   0xFD3
PR2         EQU   0xFCB
T2CON       EQU   0xFCA
ADCON1      EQU   0xFC1
IPR1        EQU   0xF9F
PIE1        EQU   0xF9D
TRISC       EQU   0xF94
TRISB       EQU   0xF93
TRISA       EQU   0xF92
PORTC       EQU   0xF82
PORTB       EQU   0xF81
PORTA       EQU   0xF80
TMR2IF      EQU   1
GREEN_LED   EQU   0
RED_LED     EQU   2
DOOR1       EQU   2
DOOR2       EQU   3
LIGHT       EQU   0
T1ms        EQU   0x18
T10ms       EQU   0x19
T100ms      EQU   0x1A
TIMERS      EQU   0x1E
OUTPUTS     EQU   0x2E
LoopCounter EQU   0x36
sv_FSR0     EQU   0x00
sv_FSR1     EQU   0x02
sv_FSR2     EQU   0x04
sv_PCLATH   EQU   0x06
sv_PCLATU   EQU   0x07
sv_PRODL    EQU   0x08
sv_PRODH    EQU   0x09
sv_TBLPTR   EQU   0x0A
sv_TABLAT   EQU   0x0D
svrSTATUS   EQU   0x0E
svrBSR      EQU   0x0F
svrWREG     EQU   0x10
i           EQU   0x11
temp        EQU   0x12
time        EQU   0x13
i_2         EQU   0x14
test        EQU   0x14
state       EQU   0x15
counter     EQU   0x16

	GOTO main

  ; FILE GLIGHTS.C
			;#include "INT18XXX.H"
			;
			;#pragma origin = 0x0004																	// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???
	ORG 0x0004
			;
			;#pragma config1  = 0b0000.1011			//  Internal Osciallator block, Clockouot RA6\
			;#pragma config2l = 0b0000.0000
			;#pragma config2h = 0b0000.0000			// not watchdog
			;#pragma config3h = 0b1000.0000			// mclr enabled
			;#pragma config4l = 0b1000.0000
			;#pragma config5l = 0b0000.0011
			;#pragma config5h = 0b1100.0000
			;#pragma config6l = 0b0000.0011
			;#pragma config6h = 0b1110.0000
			;#pragma config7l = 0b0000.0011
			;#pragma config7h = 0b0100.0000
			;
			;
			;
			;
			;
			;#pragma bit GREEN_LED @ LATB.0
			;#pragma bit RED_LED   @ LATB.2
			;
			;#pragma bit DOOR1	@ 	LATC.2
			;#pragma bit DOOR2	@	LATC.3
			;#pragma bit LIGHT	@	LATC.0
			;
			;#define MAX_DEBOUNCE_COUNT 	10			// How many times do we need the changed state 
			;#define DOOR1_RUNTIME		5			// 500ms
			;#define DOOR2_RUNTIME		5			// 500ms
			;#define LIGHTS_RUNTIME		50			// 5 seconds
			;
			;// Declare Function Prototypes
			;void InitialiseHardware(void);
			;void InitialiseGlobals(void);
			;void debounce(char test, char *state, char *counter);
			;
			;char FSRTemp;
			;char T1ms;								// 1 millisecond counter
			;char T10ms;								// 10's millisecond counter
			;char T100ms;							// 100's millisecond counter
			;
			;char StartButtonState;
			;char StartButtonPrev;
			;char StartButtonCounter;
			;
			;// DECALRE GOLBALS
			;
			;uns16	TIMERS[4];
			;char	INPUTS_RAW[4];
			;char	INPUTS_STATE[4];
			;char	OUTPUTS[4];
			;char	DebounceCounters[4];
			;
			;char 	LoopCounter;
			;
			;#pragma origin 0x18
	ORG 0x0018
			;interrupt lowPriorityIntServer(void)
			;{
lowPriorityIntServer
			;    // W, STATUS and BSR are saved by the next macro.
			;uns16 sv_FSR0 = FSR0;
	MOVFF FSR0,sv_FSR0
	MOVFF FSR0+1,sv_FSR0+1
			;uns16 sv_FSR1 = FSR1;
	MOVFF FSR1,sv_FSR1
	MOVFF FSR1+1,sv_FSR1+1
			;uns16 sv_FSR2 = FSR2;
	MOVFF FSR2,sv_FSR2
	MOVFF FSR2+1,sv_FSR2+1
			;uns8 sv_PCLATH = PCLATH;
	MOVFF PCLATH,sv_PCLATH
			;uns8 sv_PCLATU = PCLATU;
	MOVFF PCLATU,sv_PCLATU
			;uns8 sv_PRODL = PRODL;
	MOVFF PRODL,sv_PRODL
			;uns8 sv_PRODH = PRODH;
	MOVFF PRODH,sv_PRODH
			;uns24 sv_TBLPTR = TBLPTR;
	MOVFF TBLPTR,sv_TBLPTR
	MOVFF TBLPTR+1,sv_TBLPTR+1
	MOVFF TBLPTR+2,sv_TBLPTR+2
			;uns8 sv_TABLAT = TABLAT;
	MOVFF TABLAT,sv_TABLAT
			;    int_save_registers
	MOVFF STATUS,svrSTATUS
	MOVFF BSR,svrBSR
	MOVWF svrWREG,0
			;    
			;	if (TMR2IF == 1) {
	BTFSS 0xF9E,TMR2IF,0
	BRA   m013
			;		T1ms++;
	INCF  T1ms,1,0
			;		if(T1ms == 10){
	MOVLW 10
	CPFSEQ T1ms,0
	BRA   m012
			;			T10ms++;
	INCF  T10ms,1,0
			;			T1ms=0;
	CLRF  T1ms,0
			;			if (T10ms == 10) {								
	MOVLW 10
	CPFSEQ T10ms,0
	BRA   m012
			;				T100ms++;
	INCF  T100ms,1,0
			;				T10ms = 0;
	CLRF  T10ms,0
			;				GREEN_LED=!GREEN_LED;
	BTG   0xF8A,GREEN_LED,0
			;				for(LoopCounter=0;LoopCounter<3;LoopCounter++){
	CLRF  LoopCounter,0
m001	MOVLW 3
	CPFSLT LoopCounter,0
	BRA   m011
			;					if(OUTPUTS[LoopCounter]>0){			// IF output flag has been set
	CLRF  FSR0+1,0
	MOVLW 46
	ADDWF LoopCounter,W,0
	MOVWF FSR0,0
	MOVF  INDF0,W,0
	BTFSC 0xFD8,Zero_,0
	BRA   m002
			;						OUTPUTS[LoopCounter]--;
	CLRF  FSR0+1,0
	MOVLW 46
	ADDWF LoopCounter,W,0
	MOVWF FSR0,0
	DECF  INDF0,1,0
			;					}
			;					if(OUTPUTS[LoopCounter]>0){
m002	CLRF  FSR0+1,0
	MOVLW 46
	ADDWF LoopCounter,W,0
	MOVWF FSR0,0
	MOVF  INDF0,W,0
	BTFSC 0xFD8,Zero_,0
	BRA   m006
			;						switch(LoopCounter){
	MOVF  LoopCounter,W,0
	BTFSC 0xFD8,Zero_,0
	BRA   m003
	XORLW 1
	BTFSC 0xFD8,Zero_,0
	BRA   m004
	XORLW 3
	BTFSC 0xFD8,Zero_,0
	BRA   m005
	BRA   m010
			;							case 0:
			;								LIGHT=1;
m003	BSF   0xF8B,LIGHT,0
			;								break;
	BRA   m010
			;							case 1:
			;								DOOR1=1;
m004	BSF   0xF8B,DOOR1,0
			;								OUTPUTS[0]=LIGHTS_RUNTIME;	
	MOVLW 50
	MOVWF OUTPUTS,0
			;								break;
	BRA   m010
			;							case 2:
			;								DOOR2=1;
m005	BSF   0xF8B,DOOR2,0
			;								OUTPUTS[0]=LIGHTS_RUNTIME;
	MOVLW 50
	MOVWF OUTPUTS,0
			;								break;
	BRA   m010
			;							default:
			;							 	break;	
			;							}
			;					}			 
			;					else {
			;						switch(LoopCounter){
m006	MOVF  LoopCounter,W,0
	BTFSC 0xFD8,Zero_,0
	BRA   m007
	XORLW 1
	BTFSC 0xFD8,Zero_,0
	BRA   m008
	XORLW 3
	BTFSC 0xFD8,Zero_,0
	BRA   m009
	BRA   m010
			;							case 0:
			;								LIGHT=0;
m007	BCF   0xF8B,LIGHT,0
			;								break;
	BRA   m010
			;							case 1:
			;								DOOR1=0;
m008	BCF   0xF8B,DOOR1,0
			;								break;
	BRA   m010
			;							case 2:
			;								DOOR2=0;
m009	BCF   0xF8B,DOOR2,0
			;								break;
			;							default:
			;							 	break;								
			;							 }									
			;					}		
			;				}
m010	INCF  LoopCounter,1,0
	BRA   m001
			;				if (T100ms == 10) {
m011	MOVLW 10
	CPFSEQ T100ms,0
	BRA   m012
			;					T100ms = 0;
	CLRF  T100ms,0
			;				}
			;			}
			;		}	
			;		TMR2IF = 0;
m012	BCF   0xF9E,TMR2IF,0
			;	}
			;FSR0 = sv_FSR0;
m013	MOVFF sv_FSR0,FSR0
	MOVFF sv_FSR0+1,FSR0+1
			;FSR1 = sv_FSR1;
	MOVFF sv_FSR1,FSR1
	MOVFF sv_FSR1+1,FSR1+1
			;FSR2 = sv_FSR2;
	MOVFF sv_FSR2,FSR2
	MOVFF sv_FSR2+1,FSR2+1
			;PCLATH = sv_PCLATH;
	MOVFF sv_PCLATH,PCLATH
			;PCLATU = sv_PCLATU;
	MOVFF sv_PCLATU,PCLATU
			;PRODL = sv_PRODL;
	MOVFF sv_PRODL,PRODL
			;PRODH = sv_PRODH;
	MOVFF sv_PRODH,PRODH
			;TBLPTR = sv_TBLPTR;
	MOVFF sv_TBLPTR,TBLPTR
	MOVFF sv_TBLPTR+1,TBLPTR+1
	MOVFF sv_TBLPTR+2,TBLPTR+2
			;TABLAT = sv_TABLAT;
	MOVFF sv_TABLAT,TABLAT
			;   	int_restore_registers
	MOVF  svrWREG,W,0
	MOVFF svrBSR,BSR
	MOVFF svrSTATUS,STATUS
			;}
	RETFIE
			;
			;
			;
			;void main(void) {
main
			;
			;int i = 0;
	CLRF  i,0
			;int temp = 0;
	CLRF  temp,0
			;int time = 0; 
	CLRF  time,0
			;
			;	InitialiseHardware();
	RCALL InitialiseHardware
			;	InitialiseGlobals();
	RCALL InitialiseGlobals
			;
			;	while (1) {
			;	temp = PORTA;	
m014	MOVFF PORTA,temp
			;	for(i=0;i<3;i++){
	CLRF  i,0
m015	BTFSC i,7,0
	BRA   m016
	MOVLW 3
	CPFSLT i,0
	BRA   m014
			;		INPUTS_RAW[i]=temp&0x01;
m016	CLRF  FSR0+1,0
	MOVLW 38
	ADDWF i,W,0
	MOVWF FSR0,0
	MOVLW 1
	ANDWF temp,W,0
	MOVWF INDF0,0
			;		if(INPUTS_RAW[i]==0){
	CLRF  FSR0+1,0
	MOVLW 38
	ADDWF i,W,0
	MOVWF FSR0,0
	MOVF  INDF0,W,0
	BTFSS 0xFD8,Zero_,0
	BRA   m017
			;			RED_LED = 1;
	BSF   0xF8A,RED_LED,0
			;			} else {
	BRA   m018
			;			RED_LED= 0;
m017	BCF   0xF8A,RED_LED,0
			;		}			 
			;		temp = temp >> 1;
m018	BSF   0xFD8,Carry,0
	BTFSS temp,7,0
	BCF   0xFD8,Carry,0
	RRCF  temp,1,0
			;		debounce(INPUTS_RAW[i], &INPUTS_STATE[i], &DebounceCounters[i]);
	CLRF  FSR0+1,0
	MOVLW 38
	ADDWF i,W,0
	MOVWF FSR0,0
	MOVFF INDF0,test
	MOVLW 42
	ADDWF i,W,0
	MOVWF state,0
	MOVLW 50
	ADDWF i,W,0
	MOVWF counter,0
	RCALL debounce
			;		if(INPUTS_STATE[i]==0){
	CLRF  FSR0+1,0
	MOVLW 42
	ADDWF i,W,0
	MOVWF FSR0,0
	MOVF  INDF0,W,0
	BTFSS 0xFD8,Zero_,0
	BRA   m019
			;				time=TIMERS[i]; 								// SET OUTPUT FLAG HIGH IF INPUTS ARE DEBOUNCED
	CLRF  FSR0+1,0
	BCF   0xFD8,Carry,0
	RLCF  i,W,0
	ADDLW 30
	MOVWF FSR0,0
	MOVFF POSTINC0,time
			;				OUTPUTS[i]=time;
	CLRF  FSR0+1,0
	MOVLW 46
	ADDWF i,W,0
	MOVWF FSR0,0
	MOVFF time,INDF0
			;			}	
			;		}
m019	INCF  i,1,0
	BRA   m015
			;	}
			;}// end main()
			;
			;
			;void InitialiseHardware(void) {
InitialiseHardware
			;  	
			;  	OSCCON = 0b0111.0000;
	MOVLW 112
	MOVWF OSCCON,0
			;  	
			;  	TRISA  = 0x07; 						// PORTA (0 = OUTPUT)
	MOVLW 7
	MOVWF TRISA,0
			;  	TRISB  = 0x00; 						// PORTB (0 = OUTPUT)
	CLRF  TRISB,0
			;  	TRISC  = 0x00; 						// PORTC (0 = OUTPUT)
	CLRF  TRISC,0
			;	PORTA  = 0x00;				// Initialise PORTA
	CLRF  PORTA,0
			;	PORTB  = 0x00;				// Initialise PORTB
	CLRF  PORTB,0
			;	PORTC  = 0X00;
	CLRF  PORTC,0
			;	ADCON1 = 0x0F;						// All portA inpouts are digital
	MOVLW 15
	MOVWF ADCON1,0
			;	INTCON = 0b11000000;
	MOVLW 192
	MOVWF INTCON,0
			;	T2CON  = 0b00001101;				// TMR2 on, prescale = 1:4, and postscale = 1:2  (4us ticks with 8MHz oscillator)
	MOVLW 13
	MOVWF T2CON,0
			;	PIE1   = 0x02;
	MOVLW 2
	MOVWF PIE1,0
			;	IPR1  = 0x00;
	CLRF  IPR1,0
			;	PR2	   = 250;						// 250 * 4us = 1ms interrupts 
	MOVLW 250
	MOVWF PR2,0
			;
			;}
	RETURN
			;
			;void InitialiseGlobals(void) {
InitialiseGlobals
			;	int i = 0;
	CLRF  i_2,0
			;	
			;	T1ms = 0;							// Reset Timers
	CLRF  T1ms,0
			;	T10ms = 0;
	CLRF  T10ms,0
			;	T100ms = 0;
	CLRF  T100ms,0
			;//	SecondsFlag=0;
			;	for(i=0;i<4;i++){
	CLRF  i_2,0
m020	BTFSC i_2,7,0
	BRA   m021
	MOVLW 4
	CPFSLT i_2,0
	BRA   m022
			;		INPUTS_RAW[i]=0;
m021	CLRF  FSR0+1,0
	MOVLW 38
	ADDWF i_2,W,0
	MOVWF FSR0,0
	CLRF  INDF0,0
			;		INPUTS_STATE[i]=1;
	CLRF  FSR0+1,0
	MOVLW 42
	ADDWF i_2,W,0
	MOVWF FSR0,0
	MOVLW 1
	MOVWF INDF0,0
			;		OUTPUTS[i]=0;
	CLRF  FSR0+1,0
	MOVLW 46
	ADDWF i_2,W,0
	MOVWF FSR0,0
	CLRF  INDF0,0
			;	}
	INCF  i_2,1,0
	BRA   m020
			;	LoopCounter=0;
m022	CLRF  LoopCounter,0
			;	TIMERS[0] = LIGHTS_RUNTIME;
	MOVLW 50
	MOVWF TIMERS,0
	CLRF  TIMERS+1,0
			;	TIMERS[1] = DOOR1_RUNTIME;
	MOVLW 5
	MOVWF TIMERS+2,0
	CLRF  TIMERS+3,0
			;	TIMERS[2] = DOOR2_RUNTIME;
	MOVLW 5
	MOVWF TIMERS+4,0
	CLRF  TIMERS+5,0
			;	
			;}
	RETURN
			;
			;
			;void debounce(char test, char *state, char *counter) {
debounce
			;	if ((*state) != test) {
	MOVFF state,FSR0
	CLRF  FSR0+1,0
	MOVF  INDF0,W,0
	XORWF test,W,0
	BTFSC 0xFD8,Zero_,0
	BRA   m023
			;   	(*counter)++;
	MOVFF counter,FSR0
	CLRF  FSR0+1,0
	INCF  INDF0,1,0
			;		if ((*counter) >= MAX_DEBOUNCE_COUNT) {
	MOVFF counter,FSR0
	CLRF  FSR0+1,0
	MOVLW 9
	CPFSGT INDF0,0
	BRA   m024
			;			(*counter) = 0;
	MOVFF counter,FSR0
	CLRF  FSR0+1,0
	CLRF  INDF0,0
			;			(*state) = test;
	MOVFF state,FSR0
	CLRF  FSR0+1,0
	MOVFF test,INDF0
			;		}
			;	} else {
	BRA   m024
			;		(*counter) = 0;
m023	MOVFF counter,FSR0
	CLRF  FSR0+1,0
	CLRF  INDF0,0
			;	}
			;}
m024	RETURN

	END


; *** KEY INFO ***

; 0x0001C6   21 word(s)  0 % : InitialiseHardware
; 0x0001F0   39 word(s)  0 % : InitialiseGlobals
; 0x00023E   32 word(s)  0 % : debounce
; 0x000018  146 word(s)  3 % : lowPriorityIntServer
; 0x00013C   69 word(s)  1 % : main

; RAM usage: 55 bytes (23 local), 457 bytes free
; Maximum call level: 1 (+1 for interrupt)
; Total of 309 code words (7 %)
