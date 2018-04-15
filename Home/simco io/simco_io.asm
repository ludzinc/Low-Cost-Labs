
; CC5X Version 3.2, Copyright (c) B Knudsen Data
; C compiler for the PICmicro family
; ************  22. Dec 2010  17:16  *************

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
GIE         EQU   7
OPTION_REG  EQU   0x81
PIR1        EQU   0x0C
T2CON       EQU   0x12
TXREG       EQU   0x19
RCREG       EQU   0x1A
CMCON       EQU   0x1F
PIE1        EQU   0x8C
PCON        EQU   0x8E
PR2         EQU   0x92
TXSTA       EQU   0x98
SPBRG       EQU   0x99
EEDATA      EQU   0x9A
EEADR       EQU   0x9B
EECON2      EQU   0x9D
RA0         EQU   0
RA1         EQU   1
RA2         EQU   2
RA3         EQU   3
RA6         EQU   6
RA7         EQU   7
RB0         EQU   0
RB3         EQU   3
RB4         EQU   4
RB5         EQU   5
RB6         EQU   6
RB7         EQU   7
TMR2IF      EQU   1
TXIF        EQU   4
RCIF        EQU   5
OERR        EQU   1
FERR        EQU   2
CREN        EQU   4
SPEN        EQU   7
RCIE        EQU   5
TRMT        EQU   1
TXEN        EQU   5
RD          EQU   0
WR          EQU   1
WREN        EQU   2
FSRTemp     EQU   0x38
T2ms        EQU   0x39
T10ms       EQU   0x3A
T100ms      EQU   0x3B
SecondsFlag EQU   0x3C
StartupFlag EQU   0x3D
RelayTimers EQU   0x3E
loop        EQU   0x46
temp        EQU   0x47
PacketLength EQU   0x57
PacketBytes EQU   0x58
RxData      EQU   0x59
ProtocolState EQU   0x5A
Foo         EQU   0x5B
InputStates EQU   0x60
svrWREG     EQU   0x70
svrSTATUS   EQU   0x20
i           EQU   0x24
StartFlag   EQU   0x25
text        EQU   0x26
input       EQU   0x21
state       EQU   0x22
counter     EQU   0x23
data        EQU   0x37
text_2      EQU   0x21
i_2         EQU   0x22
Buffer      EQU   0x27
i_3         EQU   0x35
text_3      EQU   0x36
address     EQU   0x7F
data_2      EQU   0x7F
address_2   EQU   0x7F
data_3      EQU   0x7F

	GOTO main

  ; FILE SIMCO_IO.C
			;// HOW TO GET cc5x to complie eeprom in MPLAB?? - without having to import hex file...
			;//
			;// TO DO
			;//
			;// 1.  Have look up table in EEPROM for RELAY ON times
			;// 2.  Map inputs to outputs in EEPROM
			;// 3.  Work out serial protocol - rx cmds, chksum and reply!
			;// 4.  How to break routines out to sep. file so I get main.c alone.....
			;
			;
			;//#define   PIC16F627A
			;
			;//#include "16F627A.H"
			;#include "INT16CXX.H"
			;#include "SIMCO_IO.H"
			;//#include "SIMCO_ROUTINES.H"
			;
			;#pragma origin = 0x0004																	// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???
	ORG 0x0004
			;#pragma config        = 0b0011.1111.0101.1000   	// MCLR Pin is input
			;//#pragma config        = 0b0011.1111.0111.1000   	// MCLR Pin is input
			;
			;
			;
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;// INTERRUPT ROUTINE
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;#pragma codepage 0
			;interrupt IntHandler() {
IntHandler
			;   	int_save_registers
	MOVWF svrWREG
	SWAPF STATUS,W
	BCF   0x03,RP0
	BCF   0x03,RP1
	MOVWF svrSTATUS
			;   	FSRTemp = FSR;
	MOVF  FSR,W
	MOVWF FSRTemp
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;// TIMING FUNCTIONS
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;	if (TMR2IF == 1) {
	BTFSS 0x0C,TMR2IF
	GOTO  m004
			;		T2ms++;
	INCF  T2ms,1
			;		if (T2ms == 5) {
	MOVF  T2ms,W
	XORLW .5
	BTFSS 0x03,Zero_
	GOTO  m003
			;			T10ms++;
	INCF  T10ms,1
			;			T2ms = 0;
	CLRF  T2ms
			;			debounce(Input0,&InputStates[0],&InputCounters[0]);
	CLRF  input
	BTFSC 0x05,RA0
	INCF  input,1
	MOVLW .96
	MOVWF state
	MOVLW .100
	MOVWF counter
	CALL  debounce
			;			debounce(Input1,&InputStates[1],&InputCounters[1]);
	CLRF  input
	BTFSC 0x05,RA1
	INCF  input,1
	MOVLW .97
	MOVWF state
	MOVLW .101
	MOVWF counter
	CALL  debounce
			;			debounce(Input2,&InputStates[2],&InputCounters[2]);
	CLRF  input
	BTFSC 0x05,RA2
	INCF  input,1
	MOVLW .98
	MOVWF state
	MOVLW .102
	MOVWF counter
	CALL  debounce
			;			debounce(Input3,&InputStates[3],&InputCounters[3]);
	CLRF  input
	BTFSC 0x05,RA3
	INCF  input,1
	MOVLW .99
	MOVWF state
	MOVLW .103
	MOVWF counter
	CALL  debounce
			;			if (T10ms == 10) {
	MOVF  T10ms,W
	XORLW .10
	BTFSS 0x03,Zero_
	GOTO  m003
			;				T100ms++;
	INCF  T100ms,1
			;				T10ms = 0;
	CLRF  T10ms
			;				if (T100ms == 10) {
	MOVF  T100ms,W
	XORLW .10
	BTFSS 0x03,Zero_
	GOTO  m003
			;					T100ms = 0;
	CLRF  T100ms
			;					SecondsFlag = 1;
	MOVLW .1
	MOVWF SecondsFlag
			;					//GreenLED=!GreenLED;
			;					//RedLED=0;
			;					StartupFlag=0;
	CLRF  StartupFlag
			;					for(loop=0;loop<4;loop++){
	CLRF  loop
m001	MOVLW .4
	SUBWF loop,W
	BTFSC 0x03,Carry
	GOTO  m003
			;						temp=RelayTimers[loop];
	BCF   0x03,Carry
	RLF   loop,W
	ADDLW .62
	MOVWF FSR
	BCF   0x03,IRP
	MOVF  INDF,W
	MOVWF temp
	INCF  FSR,1
	MOVF  INDF,W
	MOVWF temp+1
			;						if(temp>0){
	MOVF  temp,W
	IORWF temp+1,W
	BTFSC 0x03,Zero_
	GOTO  m002
			;							temp--;
	DECF  temp,1
	INCF  temp,W
	BTFSC 0x03,Zero_
	DECF  temp+1,1
			;							RelayTimers[loop]=temp;
	BCF   0x03,Carry
	RLF   loop,W
	ADDLW .62
	MOVWF FSR
	BCF   0x03,IRP
	MOVF  temp,W
	MOVWF INDF
	INCF  FSR,1
	MOVF  temp+1,W
	MOVWF INDF
			;						}	
			;					}
m002	INCF  loop,1
	GOTO  m001
			;				}
			;			}
			;		}
			;	TMR2IF = 0;
m003	BCF   0x0C,TMR2IF
			;	}
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;// UART Rx FUNCTIONS
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;	if (RCIF ==1 ) {								// Data in UART Rx Buffer
m004	BTFSS 0x0C,RCIF
	GOTO  m012
			;		RxData =  RxByte();							// read data from UART buffer (also clears UART buffer)
	CALL  RxByte
	MOVWF RxData
			;													// check if RxByte is correct (no UART error)
			;		if( OERR == 1) {							// OVERRUN ERROR (2 Bytes in FIFO, and other byte lost)
	BTFSS 0x18,OERR
	GOTO  m005
			;			RxData = RCREG;
	MOVF  RCREG,W
	MOVWF RxData
			;			RxData = RCREG;							// Read twice to clear FIFO
	MOVF  RCREG,W
	MOVWF RxData
			;			CREN = 0;								// Clear Error by re-enabling transmission
	BCF   0x18,CREN
			;			CREN=1;
	BSF   0x18,CREN
			;			RedLED=1;	
	BSF   0x06,RB6
			;			ProtocolState=PROTOCOL_ERROR_UARTOVERRUN;	// packet now broken
	MOVLW .254
	MOVWF ProtocolState
			;		}
			;		if (FERR == 1) {							// Framing Error.  Stop bit is not = 1.
m005	BTFSS 0x18,FERR
	GOTO  m006
			;			RxData = RCREG;
	MOVF  RCREG,W
	MOVWF RxData
			;			GreenLED=1;
	BSF   0x06,RB7
			;			ProtocolState=PROTOCOL_ERROR_UARTFRAMING;   // packet broken
	MOVLW .255
	MOVWF ProtocolState
			;		}
			;		// Otherwise Rx's character is okay and we can proceed	
			;	
			;		if(ProtocolState == PROTOCOL_WAIT_START){
m006	MOVF  ProtocolState,1
	BTFSC 0x03,Zero_
			;			GreenLED=1;
	BSF   0x06,RB7
			;		}		
			;		if((RxData == COMMAND_SNAP_SYNC)&&(Foo != 0)) {
	MOVF  RxData,W
	XORLW .84
	BTFSS 0x03,Zero_
	GOTO  m007
	MOVF  Foo,1
	BTFSS 0x03,Zero_
			;			RedLED=1;
	BSF   0x06,RB6
			;		}		
			;	
			;		if((ProtocolState == PROTOCOL_WAIT_START) && (RxData == COMMAND_SNAP_SYNC)) {	// check if byte = snyc byte, while waiting for snyc
m007	MOVF  ProtocolState,1
	BTFSS 0x03,Zero_
	GOTO  m008
	MOVF  RxData,W
	XORLW .84
	BTFSS 0x03,Zero_
	GOTO  m008
			;			PacketBytes = 0;						// reset PacketByte (and RxBuffer index) counter
	CLRF  PacketBytes
			;			ProtocolState=PROTOCOL_GATHER_SNAP;		// advance ProtocolState
	MOVLW .1
	MOVWF ProtocolState
			;			RxBuffer[PacketBytes]=RxData;			// place sync byte in index 0 in the RxBuffer
	MOVLW .73
	ADDWF PacketBytes,W
	MOVWF FSR
	BCF   0x03,IRP
	MOVF  RxData,W
	MOVWF INDF
			;			PacketBytes++;
	INCF  PacketBytes,1
			;			//GreenLED=1;
			;		}
			;
			;		if(PacketBytes < RxBufferLength){
m008	MOVLW .14
	SUBWF PacketBytes,W
	BTFSC 0x03,Carry
	GOTO  m010
			;			if(ProtocolState==PROTOCOL_GATHER_SNAP){
	DECFSZ ProtocolState,W
	GOTO  m010
			;				RxBuffer[PacketBytes] = RxData;
	MOVLW .73
	ADDWF PacketBytes,W
	MOVWF FSR
	BCF   0x03,IRP
	MOVF  RxData,W
	MOVWF INDF
			;				PacketBytes++;
	INCF  PacketBytes,1
			;				if(PacketBytes==PacketLength){
	MOVF  PacketBytes,W
	XORWF PacketLength,W
	BTFSS 0x03,Zero_
	GOTO  m009
			;						ProtocolState=PROTOCOL_WAIT_START;
	CLRF  ProtocolState
			;						GreenLED=0;
	BCF   0x06,RB7
			;						RedLED=0;
	BCF   0x06,RB6
			;				}
			;				if(PacketBytes==3){
m009	MOVF  PacketBytes,W
	XORLW .3
	BTFSS 0x03,Zero_
	GOTO  m010
			;					//TO DO!! Calculate packet length
			;					PacketLength=14;
	MOVLW .14
	MOVWF PacketLength
			;				}	
			;				//if(PacketBytes==PacketLength){
			;				// calculate checksum
			;				// compare checksum
			;				// respond with nack if required OR
			;				// reset with no response		
			;			}	
			;		}
			;		
			;		if((PacketBytes >= PacketLength)||(ProtocolState == PROTOCOL_ERROR_UARTOVERRUN) || (ProtocolState == PROTOCOL_ERROR_UARTFRAMING)) {
m010	MOVF  PacketLength,W
	SUBWF PacketBytes,W
	BTFSC 0x03,Carry
	GOTO  m011
	MOVF  ProtocolState,W
	XORLW .254
	BTFSC 0x03,Zero_
	GOTO  m011
	INCF  ProtocolState,W
	BTFSC 0x03,Zero_
			;			ProtocolState=PROTOCOL_WAIT_START;
m011	CLRF  ProtocolState
			;			// replace with PROTOCOL_ERROR_OVERFLOW
			;		}
			;	}
			;   	FSR = FSRTemp;
m012	MOVF  FSRTemp,W
	MOVWF FSR
			;   	int_restore_registers
	SWAPF svrSTATUS,W
	MOVWF STATUS
	SWAPF svrWREG,1
	SWAPF svrWREG,W
			;}
	RETFIE
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;// MAIN ROUTINE
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;void main(void) {
main
			;
			;char i =0;
	BCF   0x03,RP0
	BCF   0x03,RP1
	CLRF  i
			;char StartFlag =0;
	CLRF  StartFlag
			;char text =0;
	CLRF  text
			;	
			;	InitialiseGlobals();
	CALL  InitialiseGlobals
			;	InitialiseHardware();
	BSF   0x03,RP0
	CALL  InitialiseHardware
			;
			;	Foo=0;
	CLRF  Foo
			;
			;	//RxBuffer[0]=text;
			;
			;
			;	while(StartupFlag){
m013	MOVF  StartupFlag,1
	BTFSC 0x03,Zero_
	GOTO  m015
			;		GreenLED = 1;
	BSF   0x06,RB7
			;		RedLED = 0;
	BCF   0x06,RB6
			;		for(i=0;i<4;i++){
	CLRF  i
m014	MOVLW .4
	SUBWF i,W
	BTFSC 0x03,Carry
	GOTO  m013
			;			InputStates[i]=1;
	MOVLW .96
	ADDWF i,W
	MOVWF FSR
	BCF   0x03,IRP
	MOVLW .1
	MOVWF INDF
			;			RelayTimers[i]=0;
	BCF   0x03,Carry
	RLF   i,W
	ADDLW .62
	MOVWF FSR
	BCF   0x03,IRP
	CLRF  INDF
	INCF  FSR,1
	CLRF  INDF
			;		}
	INCF  i,1
	GOTO  m014
			;	}
			;
			;	GreenLED=0;
m015	BCF   0x06,RB7
			;	ConfigSerial();
	BSF   0x03,RP0
	CALL  ConfigSerial
			;
			;	while (1) {
			;		CheckStates();
m016	CALL  CheckStates
			;		SetRelays();
	CALL  SetRelays
			;/*		
			;		if(StartFlag==0){
			;			for(i=0;i<20;i++){			
			;				text=ReadEEPROM(i);
			;				RxBuffer[i]=text;
			;			}
			;			StartFlag=1;
			;			PacketBytes=RxBufferLength;
			;		}
			;*/
			;		if(PacketBytes == RxBufferLength){
	MOVF  PacketBytes,W
	XORLW .14
	BTFSS 0x03,Zero_
	GOTO  m017
			;			SendString(&RxBuffer[RxBufferLength]);	
	MOVLW .87
	MOVWF Buffer
	CALL  SendString
			;			PacketBytes=0;
	CLRF  PacketBytes
			;			loop=0;
	CLRF  loop
			;		}
			;
			;		TxByte(ProtocolState);
m017	MOVF  ProtocolState,W
	CALL  TxByte
			;		
			;/*
			;		for(i=0;i<128;i++) {
			;			WriteEEPROM(i,i+64);
			;		}	
			;	*/
			;	}	
	GOTO  m016
			;}// end main()
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;
			;void InitialiseHardware(void) {
InitialiseHardware
			;  	TRISA  = 0b00111111; 				// PORTA (0 = OUTPUT)
	MOVLW .63
	MOVWF TRISA
			;	PORTA  = 0b00000000;				// Initialise PORTA
	BCF   0x03,RP0
	CLRF  PORTA
			;	TRISB  = 0b00000000;      			// PORTB (0 = OUTPUT), RB1, RB2 = Serial coms, set as inputs.  Coms hardware asserts outputs as required
	BSF   0x03,RP0
	CLRF  TRISB
			;	PORTB  = 0b00000000;				// Initialise PORTB
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
			;	T2CON  = 0b00001101;				// TMR2 on, prescale = 1:4, and postscale = 1:2  (8us ticks with 4MHz oscillator)
	MOVLW .13
	BCF   0x03,RP0
	MOVWF T2CON
			;	PIE1   = 0b00000010;				// Bit 1 enables TMR2 = PR2 interface
	MOVLW .2
	BSF   0x03,RP0
	MOVWF PIE1
			;	RCIE = 1;							// enable UART Rx Interrupts
	BSF   0x8C,RCIE
			;	
			;	PIR1   = 0b00000000;				// Read this to see if TMR2 = PR2 flag is set
	BCF   0x03,RP0
	CLRF  PIR1
			;	PR2 = 250;							// TMR2 match value, for 2ms ticks
	MOVLW .250
	BSF   0x03,RP0
	MOVWF PR2
			;	
			;	
			;
			;	StartupFlag=1;
	MOVLW .1
	BCF   0x03,RP0
	MOVWF StartupFlag
			;	
			;	Relay0 = 0;
	BCF   0x06,RB5
			;	Relay1 = 0;
	BCF   0x06,RB4
			;	Relay2 = 0;
	BCF   0x05,RA7
			;	Relay3 = 0;
	BCF   0x05,RA6
			;
			;}
	RETURN
			;
			;
			;void InitialiseGlobals(void) {
InitialiseGlobals
			;	T2ms = 0;							// Reset Timers
	CLRF  T2ms
			;	T10ms = 0;
	CLRF  T10ms
			;	T100ms = 0;
	CLRF  T100ms
			;	SecondsFlag=0;
	CLRF  SecondsFlag
			;	loop=0;
	CLRF  loop
			;	RxData=0;
	CLRF  RxData
			;	PacketBytes = 0;
	CLRF  PacketBytes
			;	//ProtocolState=0;
			;	PacketLength=RxBufferLength;		//initialising to zero is bad here
	MOVLW .14
	MOVWF PacketLength
			;	//Foo = 0;
			;}
	RETURN
			;
			;
			;
			;
			;void debounce(char input, char *state, char *counter) {
debounce
			;	if ((*state) != input) {
	BCF   0x03,IRP
	MOVF  state,W
	MOVWF FSR
	MOVF  INDF,W
	XORWF input,W
	BTFSC 0x03,Zero_
	GOTO  m018
			;   		(*counter)++;
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
	GOTO  m019
			;			(*counter) = 0;
	BCF   0x03,IRP
	MOVF  counter,W
	MOVWF FSR
	CLRF  INDF
			;			(*state) = input;
	BCF   0x03,IRP
	MOVF  state,W
	MOVWF FSR
	MOVF  input,W
	MOVWF INDF
			;		}
			;	} else {
	GOTO  m019
			;		(*counter) = 0;
m018	BCF   0x03,IRP
	MOVF  counter,W
	MOVWF FSR
	CLRF  INDF
			;	}
			;}
m019	RETURN
			;
			;void ConfigSerial(){
ConfigSerial
			;	
			;	TRISB = TRISB|0b0000.0110;		// RB1, RB2 inputs - UART sets outputs as needed
	MOVLW .6
	IORWF TRISB,1
			;	SPBRG = 0x19;					// 9600 baud at 4MHz (9615 baud exactly) - HIGH SPEED BAUD RATE GENERATOR
	MOVLW .25
	MOVWF SPBRG
			;	TXSTA = 0b0010.0100;			// Enable asynch Tx mode
	MOVLW .36
	MOVWF TXSTA
			;
			;	SPEN = 1;						// Turn UART on
	BCF   0x03,RP0
	BSF   0x18,SPEN
			;	CREN = 1;						// Enable Reception
	BSF   0x18,CREN
			;	TXEN = 1;						// Enable Transmission
	BSF   0x03,RP0
	BSF   0x98,TXEN
			;
			;	TxCtrl = TxActive;				// Turn on 485 
	BCF   0x03,RP0
	BSF   0x06,RB3
			;} 
	RETURN
			;	
			;void TxByte(char data){
TxByte
	MOVWF data
			;
			;		TXREG = data;				// stick byte into buffer
	MOVWF TXREG
			;		while(TRMT ==0);			// Wait for Byte to finsih being sent
	BSF   0x03,RP0
m020	BTFSS 0x98,TRMT
	GOTO  m020
			;}
	BCF   0x03,RP0
	RETURN
			;
			;char RxByte(void) {					// read Rx data from UART
RxByte
			;
			;	char text = 0;
	CLRF  text_2
			;	char i = 0;
	CLRF  i_2
			;	
			;		if( OERR ==0 && FERR ==0 ) {
	BTFSC 0x18,OERR
	GOTO  m021
	BTFSC 0x18,FERR
	GOTO  m021
			;			text = RCREG;
	MOVF  RCREG,W
	MOVWF text_2
			;			//RedLED=0;
			;			//GreenLED=0;
			;		}
			;	return text;
m021	MOVF  text_2,W
	RETURN
			;}	 
			;
			;void CheckStates(){
CheckStates
			;		// while inputs are active the mapped outputs will also be active, and remain active for their time periods
			;		// can't be stuffed setting flags to make time periods absolute!
			;		// decrement (if required) timers in timer interrupt routine
			;		if(InputStates[0]==INPUT_ACTIVE){
	MOVF  InputStates,1
	BTFSS 0x03,Zero_
	GOTO  m022
			;			RelayTimers[0]=DOOR_TIME;					// Close Relay 0 for door0 control			
	MOVLW .1
	MOVWF RelayTimers
	CLRF  RelayTimers+1
			;			RelayTimers[3]=LIGHT_TIME;					// Close Relay 4 for light control
	MOVLW .44
	MOVWF RelayTimers+6
	MOVLW .1
	MOVWF RelayTimers+7
			;			T100ms=0;
	CLRF  T100ms
			;			RedLED=1;
	BSF   0x06,RB6
			;		}
			;		if(InputStates[1]==INPUT_ACTIVE){
m022	MOVF  InputStates+1,1
	BTFSS 0x03,Zero_
	GOTO  m023
			;			RelayTimers[1]=DOOR_TIME;					// Close Relay 1 for  door1 control			
	MOVLW .1
	MOVWF RelayTimers+2
	CLRF  RelayTimers+3
			;			RelayTimers[3]=LIGHT_TIME;					// Close Relay 4 for light control
	MOVLW .44
	MOVWF RelayTimers+6
	MOVLW .1
	MOVWF RelayTimers+7
			;			T100ms=0;
	CLRF  T100ms
			;			RedLED=1;
	BSF   0x06,RB6
			;		}
			;		if(InputStates[2] == INPUT_ACTIVE){
m023	MOVF  InputStates+2,1
	BTFSS 0x03,Zero_
	GOTO  m024
			;			RelayTimers[2] = DOOR_TIME;					
	MOVLW .1
	MOVWF RelayTimers+4
	CLRF  RelayTimers+5
			;			T100ms=0;
	CLRF  T100ms
			;			RedLED=1;
	BSF   0x06,RB6
			;		}
			;		if(InputStates[3] == INPUT_ACTIVE) {	
m024	MOVF  InputStates+3,1
	BTFSS 0x03,Zero_
	GOTO  m025
			;			RelayTimers[3] = LIGHT_TIME;
	MOVLW .44
	MOVWF RelayTimers+6
	MOVLW .1
	MOVWF RelayTimers+7
			;			T100ms=0;
	CLRF  T100ms
			;			RedLED=1;
	BSF   0x06,RB6
			;		}
			;}
m025	RETURN
			;
			;
			;void SetRelays(){
SetRelays
			;
			;		if(RelayTimers[0]>0){
	MOVF  RelayTimers,W
	IORWF RelayTimers+1,W
	BTFSC 0x03,Zero_
	GOTO  m026
			;			Relay0=1;
	BSF   0x06,RB5
			;		}
			;		else {
	GOTO  m027
			;			Relay0=0;
m026	BCF   0x06,RB5
			;		}
			;		if(RelayTimers[1]>0){
m027	MOVF  RelayTimers+2,W
	IORWF RelayTimers+3,W
	BTFSC 0x03,Zero_
	GOTO  m028
			;			Relay1=1;
	BSF   0x06,RB4
			;		}
			;		else {
	GOTO  m029
			;			Relay1=0;
m028	BCF   0x06,RB4
			;		}
			;		if(RelayTimers[2]>0){
m029	MOVF  RelayTimers+4,W
	IORWF RelayTimers+5,W
	BTFSC 0x03,Zero_
	GOTO  m030
			;			Relay2=1;
	BSF   0x05,RA7
			;		}
			;		else {
	GOTO  m031
			;			Relay2=0;
m030	BCF   0x05,RA7
			;		}
			;		if(RelayTimers[3]>0){
m031	MOVF  RelayTimers+6,W
	IORWF RelayTimers+7,W
	BTFSC 0x03,Zero_
	GOTO  m032
			;			Relay3=1;
	BSF   0x05,RA6
			;		}
			;		else {
	GOTO  m033
			;			Relay3=0;
m032	BCF   0x05,RA6
			;		}
			;}
m033	RETURN
			;
			;
			;void SendString(char *Buffer[RxBufferLength]) {
SendString
			;
			;char i =0;
	CLRF  i_3
			;char text =0;	
	CLRF  text_3
			;	
			;	for(i=0;i<RxBufferLength;i++){		// prepare to send data packet
	CLRF  i_3
m034	MOVLW .14
	SUBWF i_3,W
	BTFSC 0x03,Carry
	GOTO  m037
			;		while(TXIF == 0);				// Wait for Tx buffer to be empty
m035	BTFSS 0x0C,TXIF
	GOTO  m035
			;		text = RxBuffer[i];				// Put ito RAM to speed things up (why keep reading the Array?)  Does this do speed improvement?
	MOVLW .73
	ADDWF i_3,W
	MOVWF FSR
	BCF   0x03,IRP
	MOVF  INDF,W
	MOVWF text_3
			;		if(text != 0){	
	MOVF  text_3,1
	BTFSC 0x03,Zero_
	GOTO  m036
			;			RxCtrl = !RxActive;			// Disable Rs485 Rx
	BSF   0x06,RB0
			;			TxByte(text);				// Transmit buffer contents
	CALL  TxByte
			;			RxBuffer[i]=0;
	MOVLW .73
	ADDWF i_3,W
	MOVWF FSR
	BCF   0x03,IRP
	CLRF  INDF
			;		}
			;		//while(TRMT ==0);				// Wait for Byte to finsih being sent
			;		RxCtrl = RxActive;				// Enable RS485 Rx
m036	BCF   0x06,RB0
			;	}	
	INCF  i_3,1
	GOTO  m034
			;}
m037	RETURN
			;
			;// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;// EEPROM data access (PIC16F627A Version)
			;// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;char ReadEEPROM(char address) {
ReadEEPROM
	MOVWF address
			;   	char data = 0;
	CLRF  data_2
			;   
			;	EEADR = address;        // 1. Write address to EEADR
	MOVF  address,W
	BSF   0x03,RP0
	BCF   0x03,RP1
	MOVWF EEADR
			;//   	EEPGD = 0;              // 2. Clear EEPGD bit to point to EEPROM data (not Needed 16F627A)
			;   	RD = 1;                 // 3. Set the RD bit to start the read
	BSF   0x9C,RD
			;   	data = EEDATA;          // 4. Read the data from EEDATA
	MOVF  EEDATA,W
	MOVWF data_2
			;   	return data;
	RETURN
			;}
			;
			;void WriteEEPROM(char address, char data) {
WriteEEPROM
	MOVWF data_3
			;
			;	EEADR = address;
	MOVF  address_2,W
	BSF   0x03,RP0
	BCF   0x03,RP1
	MOVWF EEADR
			;	EEDATA = data;
	MOVF  data_3,W
	MOVWF EEDATA
			;	
			;	GIE = 0;				// Disable interrupts
	BCF   0x0B,GIE
			;	WREN = 1;				// Enable EEPROM write
	BSF   0x9C,WREN
			;	EECON2 = 0x55;
	MOVLW .85
	MOVWF EECON2
			;	EECON2 = 0xAA;			
	MOVLW .170
	MOVWF EECON2
			;	WR = 1;					// EEPROM write sequence initiated
	BSF   0x9C,WR
			;	while(WR == 1);			// Wait for write to complete
m038	BTFSC 0x9C,WR
	GOTO  m038
			;	GIE = 1;				// Re-enable interrupts
	BSF   0x0B,GIE
			;	
			;}
	RETURN

	ORG 0x2100
	DATA 000DH
	DATA 000AH
	DATA 0053H
	DATA 0049H
	DATA 004DH
	DATA 0043H
	DATA 004FH
	DATA 0020H
	DATA 0049H
	DATA 004FH
	DATA 000DH
	DATA 000AH
	DATA 0056H
	DATA 0065H
	DATA 0072H
	DATA 0020H
	DATA 0030H
	DATA 002EH
	DATA 0030H
	DATA 0030H
	ORG 0x2007
	DATA 3F58H
	END


; *** KEY INFO ***

; 0x00F7   39 word(s)  3 % : InitialiseHardware
; 0x011E   10 word(s)  0 % : InitialiseGlobals
; 0x0128   33 word(s)  3 % : debounce
; 0x0149   14 word(s)  1 % : ConfigSerial
; 0x0157    7 word(s)  0 % : TxByte
; 0x015E   10 word(s)  0 % : RxByte
; 0x0168   42 word(s)  4 % : CheckStates
; 0x0192   29 word(s)  2 % : SetRelays
; 0x01AF   29 word(s)  2 % : SendString
; 0x01CC   10 word(s)  0 % : ReadEEPROM
; 0x01D6   18 word(s)  1 % : WriteEEPROM
; 0x0004  191 word(s) 18 % : IntHandler
; 0x00C3   52 word(s)  5 % : main

; RAM usage: 73 bytes (25 local), 151 bytes free
; Maximum call level: 2 (+2 for interrupt)
; Total of 485 code words (47 %)
