
; CC5X Version 3.2, Copyright (c) B Knudsen Data
; C compiler for the PICmicro family
; ************  29. Dec 2010   0:27  *************

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
RB0         EQU   0
RB3         EQU   3
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
RxBuffer    EQU   0x3D
PacketLength EQU   0x4B
PacketBytes EQU   0x4C
RxData      EQU   0x4D
ProtocolState EQU   0x4E
printstate  EQU   0x4F
HDB1        EQU   0x50
HDB2        EQU   0x51
SNAP_DAB    EQU   0x52
SNAP_SAB    EQU   0x53
SNAP_PFB    EQU   0x54
SNAP_ACK_NACK EQU   0x55
SNAP_EDM    EQU   0x57
SNAP_NDB    EQU   0x58
SNAP_CHECKSUM EQU   0x59
svrWREG     EQU   0x70
svrSTATUS   EQU   0x20
i           EQU   0x21
i_2         EQU   0x24
StartFlag   EQU   0x25
text        EQU   0x26
input       EQU   0x7F
state       EQU   0x7F
counter     EQU   0x7F
data        EQU   0x37
text_2      EQU   0x22
i_3         EQU   0x23
Buffer      EQU   0x27
i_4         EQU   0x35
text_3      EQU   0x36
address     EQU   0x7F
data_2      EQU   0x7F
address_2   EQU   0x7F
data_3      EQU   0x7F

	GOTO main

  ; FILE SNAP.C
			;// TO DO
			;//
			;// 1. why does making RxBufferLength larger break things??
			;// 2. checksum on / off testing
			;// 3. error reporting??
			;// 4. return message if checksum matches
			;// 5. build customised return messages
			;
			;//#define   PIC16F627A
			;//#include "16F627A.H"
			;#include "INT16CXX.H"
			;#include "SNAP.H"
			;
			;#pragma origin 			= 0x0004					// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???
	ORG 0x0004
			;#pragma config        	= 0b0011.1111.0101.1000   	// MCLR Pin is input
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
			;   	int i = 0;
	CLRF  i
			;   	
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;// TIMING FUNCTIONS
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;	if (TMR2IF == 1) {
	BTFSS 0x0C,TMR2IF
	GOTO  m002
			;		T2ms++;
	INCF  T2ms,1
			;		if (T2ms == 5) {
	MOVF  T2ms,W
	XORLW .5
	BTFSS 0x03,Zero_
	GOTO  m001
			;			T10ms++;
	INCF  T10ms,1
			;			T2ms = 0;											// add debounce routines here if needed
	CLRF  T2ms
			;			if (T10ms == 10) {
	MOVF  T10ms,W
	XORLW .10
	BTFSS 0x03,Zero_
	GOTO  m001
			;				T100ms++;
	INCF  T100ms,1
			;				T10ms = 0;
	CLRF  T10ms
			;				if (T100ms == 10) {
	MOVF  T100ms,W
	XORLW .10
	BTFSS 0x03,Zero_
	GOTO  m001
			;					T100ms = 0;
	CLRF  T100ms
			;					SecondsFlag = 1;
	MOVLW .1
	MOVWF SecondsFlag
			;					//GreenLED=!GreenLED;
			;					//RedLED=0;
			;				}
			;			}
			;		}
			;	TMR2IF = 0;
m001	BCF   0x0C,TMR2IF
			;	}
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;// UART Rx FUNCTIONS
			;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			;	if (RCIF ==1 ) {									// Data in UART Rx Buffer
m002	BTFSS 0x0C,RCIF
	GOTO  m018
			;		RxData =  RxByte();								// read data from UART buffer (also clears UART buffer)
	CALL  RxByte
	MOVWF RxData
			;		RedLED=1;										// check if RxByte is correct (no UART error)
	BSF   0x06,RB6
			;		if( OERR == 1) {								// OVERRUN ERROR (2 Bytes in FIFO, and other byte lost)
	BTFSS 0x18,OERR
	GOTO  m003
			;			RxData = RCREG;
	MOVF  RCREG,W
	MOVWF RxData
			;			RxData = RCREG;								// Read twice to clear FIFO
	MOVF  RCREG,W
	MOVWF RxData
			;			CREN = 0;									// Clear Error by re-enabling transmission
	BCF   0x18,CREN
			;			CREN=1;
	BSF   0x18,CREN
			;			ProtocolState=PROTOCOL_ERROR_UARTOVERRUN;	// packet now broken
	MOVLW .254
	MOVWF ProtocolState
			;		}
			;		if (FERR == 1) {								// Framing Error.  Stop bit is not = 1.
m003	BTFSS 0x18,FERR
	GOTO  m004
			;			RxData = RCREG;
	MOVF  RCREG,W
	MOVWF RxData
			;			ProtocolState=PROTOCOL_ERROR_UARTFRAMING;   // packet broken
	MOVLW .255
	MOVWF ProtocolState
			;		}
			;		// Otherwise Rx's character is okay and we can proceed	
			;		if((ProtocolState == PROTOCOL_WAIT_START) && (RxData == COMMAND_SNAP_SYNC)) {	// check if byte = sync byte, while waiting for sync
m004	MOVF  ProtocolState,1
	BTFSS 0x03,Zero_
	GOTO  m005
	MOVF  RxData,W
	XORLW .84
	BTFSS 0x03,Zero_
	GOTO  m005
			;			PacketBytes = 0;							// reset PacketByte (and RxBuffer index) counter
	CLRF  PacketBytes
			;			ProtocolState=PROTOCOL_GATHER_SNAP;			// advance ProtocolState
	MOVLW .1
	MOVWF ProtocolState
			;			GreenLED=1;
	BSF   0x06,RB7
			;		}
			;		if((PacketBytes < RxBufferLength)&& (ProtocolState == PROTOCOL_GATHER_SNAP)){
m005	MOVLW .14
	SUBWF PacketBytes,W
	BTFSC 0x03,Carry
	GOTO  m018
	DECFSZ ProtocolState,W
	GOTO  m018
			;			RxBuffer[PacketBytes] = RxData;
	MOVLW .61
	ADDWF PacketBytes,W
	MOVWF FSR
	BCF   0x03,IRP
	MOVF  RxData,W
	MOVWF INDF
			;			PacketBytes++;
	INCF  PacketBytes,1
			;			if(PacketBytes==PacketLength){
	MOVF  PacketBytes,W
	XORWF PacketLength,W
	BTFSS 0x03,Zero_
	GOTO  m006
			;					ProtocolState=PROTOCOL_WAIT_START;
	CLRF  ProtocolState
			;					GreenLED=0;
	BCF   0x06,RB7
			;					SNAP_CHECKSUM = 0;
	CLRF  SNAP_CHECKSUM
			;			}
			;			if(PacketBytes==3){
m006	MOVF  PacketBytes,W
	XORLW .3
	BTFSS 0x03,Zero_
	GOTO  m012
			;				//TO DO!! Calculate packet length
			;				HDB2=RxBuffer[1];
	MOVF  RxBuffer+1,W
	MOVWF HDB2
			;				HDB1=RxBuffer[2];
	MOVF  RxBuffer+2,W
	MOVWF HDB1
			;				
			;				SNAP_ACK_NACK=HDB2&0x03;
	MOVLW .3
	ANDWF HDB2,W
	MOVWF SNAP_ACK_NACK
			;				HDB2=HDB2>>2;
	BCF   0x03,Carry
	RRF   HDB2,1
	BCF   0x03,Carry
	RRF   HDB2,1
			;				SNAP_PFB=HDB2&0x03;
	MOVLW .3
	ANDWF HDB2,W
	MOVWF SNAP_PFB
			;				HDB2=HDB2>>2;
	BCF   0x03,Carry
	RRF   HDB2,1
	BCF   0x03,Carry
	RRF   HDB2,1
			;				SNAP_SAB=HDB2&0x03;
	MOVLW .3
	ANDWF HDB2,W
	MOVWF SNAP_SAB
			;				HDB2=HDB2>>2;
	BCF   0x03,Carry
	RRF   HDB2,1
	BCF   0x03,Carry
	RRF   HDB2,1
			;				SNAP_DAB=HDB2&0x03;
	MOVLW .3
	ANDWF HDB2,W
	MOVWF SNAP_DAB
			;				
			;				SNAP_NDB=HDB1&0x0F;					//If SNAP_NDB < 8, then = HDBA&0b0000.1111
	MOVLW .15
	ANDWF HDB1,W
	MOVWF SNAP_NDB
			;				if(SNAP_NDB > 8) {
	MOVLW .9
	SUBWF SNAP_NDB,W
	BTFSS 0x03,Carry
	GOTO  m007
			;					SNAP_NDB = 8*(2^(HDB1&0x07));	//Else apply SNAP rules
	MOVLW .7
	ANDWF HDB1,W
	XORLW .2
	MOVWF SNAP_NDB
	BCF   0x03,Carry
	RLF   SNAP_NDB,1
	BCF   0x03,Carry
	RLF   SNAP_NDB,1
	BCF   0x03,Carry
	RLF   SNAP_NDB,1
			;				}	
			;				
			;				RxBuffer[0]=SNAP_DAB+48;
m007	MOVLW .48
	ADDWF SNAP_DAB,W
	MOVWF RxBuffer
			;				RxBuffer[1]=SNAP_SAB+48;
	MOVLW .48
	ADDWF SNAP_SAB,W
	MOVWF RxBuffer+1
			;				RxBuffer[2]=SNAP_NDB+48;
	MOVLW .48
	ADDWF SNAP_NDB,W
	MOVWF RxBuffer+2
			;				
			;				SNAP_EDM = (HDB1 & 0x70) >> 4;
	MOVLW .112
	ANDWF HDB1,W
	MOVWF SNAP_EDM
	SWAPF SNAP_EDM,W
	ANDLW .15
	MOVWF SNAP_EDM
			;				
			;				PacketLength=3 + SNAP_DAB + SNAP_SAB + SNAP_NDB;
	MOVLW .3
	ADDWF SNAP_DAB,W
	ADDWF SNAP_SAB,W
	ADDWF SNAP_NDB,W
	MOVWF PacketLength
			;
			;				switch (SNAP_EDM) {
	MOVF  SNAP_EDM,W
	BTFSC 0x03,Zero_
	GOTO  m012
	XORLW .1
	BTFSC 0x03,Zero_
	GOTO  m012
	XORLW .3
	BTFSC 0x03,Zero_
	GOTO  m008
	XORLW .1
	BTFSC 0x03,Zero_
	GOTO  m009
	XORLW .7
	BTFSC 0x03,Zero_
	GOTO  m010
	XORLW .1
	BTFSC 0x03,Zero_
	GOTO  m011
	XORLW .3
	BTFSC 0x03,Zero_
	GOTO  m012
	XORLW .1
	BTFSC 0x03,Zero_
	GOTO  m012
	GOTO  m012
			;					case 0:
			;						break;
			;					case 1:
			;						break;
			;					case 2:
			;						PacketLength = PacketLength +1;
m008	INCF  PacketLength,1
			;						break;
	GOTO  m012
			;					case 3:
			;						PacketLength = PacketLength +1;
m009	INCF  PacketLength,1
			;						break;
	GOTO  m012
			;					case 4:
			;						PacketLength = PacketLength +2;
m010	MOVLW .2
	ADDWF PacketLength,1
			;					case 5:
			;						PacketLength = PacketLength +4;
m011	MOVLW .4
	ADDWF PacketLength,1
			;					case 6:
			;					case 7:
			;					default:
			;						break;
			;				}
			;			}		
			;			if(PacketBytes == PacketLength) {
m012	MOVF  PacketBytes,W
	XORWF PacketLength,W
	BTFSS 0x03,Zero_
	GOTO  m016
			;				switch (SNAP_EDM) {
	MOVF  SNAP_EDM,W
	BTFSC 0x03,Zero_
	GOTO  m016
	XORLW .1
	BTFSC 0x03,Zero_
	GOTO  m016
	XORLW .3
	BTFSC 0x03,Zero_
	GOTO  m013
	XORLW .1
	BTFSC 0x03,Zero_
	GOTO  m016
	XORLW .7
	BTFSC 0x03,Zero_
	GOTO  m016
	XORLW .1
	BTFSC 0x03,Zero_
	GOTO  m016
	GOTO  m016
			;					case 0:							// do nothing - no checksum 
			;						break;						// No change in PacketLength!
			;					case 1:							// do nothing - x3 transmission mode (check x3 buffers here later)
			;						break;						// No change in PacketLength!
			;					case 2:							// 8 bit sum checksum
			;						for(i=0;i<PacketLength-1;i++) {
m013	CLRF  i
m014	DECF  PacketLength,W
	SUBWF i,W
	BTFSC 0x03,Carry
	GOTO  m015
			;							SNAP_CHECKSUM = SNAP_CHECKSUM + RxBuffer[i];
	MOVLW .61
	ADDWF i,W
	MOVWF FSR
	BCF   0x03,IRP
	MOVF  INDF,W
	ADDWF SNAP_CHECKSUM,1
			;						}
	INCF  i,1
	GOTO  m014
			;						//PacketLength = PacketLength + 1;
			;						RxBuffer[PacketLength-1]=SNAP_CHECKSUM;
m015	MOVLW .60
	ADDWF PacketLength,W
	MOVWF FSR
	BCF   0x03,IRP
	MOVF  SNAP_CHECKSUM,W
	MOVWF INDF
			;						break;	
			;					case 3:
			;						//PacketLength = PacketLength + 1;
			;						break;						// do 8 bit crc here
			;					case 4:
			;						//PacketLength = PacketLength + 2;
			;						break;						// do 16 bit crc here
			;					case 5:
			;						//PacketLength = PacketLength + 4;
			;						break;						// do 32 bit crc here
			;					default:
			;						break;
			;				}	
			;			} 		
			;			if((PacketBytes > PacketLength)||(ProtocolState == PROTOCOL_ERROR_UARTOVERRUN) || (ProtocolState == PROTOCOL_ERROR_UARTFRAMING)) {
m016	MOVF  PacketBytes,W
	SUBWF PacketLength,W
	BTFSS 0x03,Carry
	GOTO  m017
	MOVF  ProtocolState,W
	XORLW .254
	BTFSC 0x03,Zero_
	GOTO  m017
	INCF  ProtocolState,W
	BTFSC 0x03,Zero_
			;				ProtocolState=PROTOCOL_WAIT_START;
m017	CLRF  ProtocolState
			;				// report error here before resetting to WAIT_STATE
			;			}
			;		}
			;	}
			;	RedLED=0;
m018	BCF   0x06,RB6
			;   	FSR = FSRTemp;
	MOVF  FSRTemp,W
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
			;	char i =0;
	BCF   0x03,RP0
	BCF   0x03,RP1
	CLRF  i_2
			;	char StartFlag =0;
	CLRF  StartFlag
			;	char text =0;
	CLRF  text
			;	
			;	printstate=0;
	CLRF  printstate
			;		
			;	InitialiseGlobals();
	CALL  InitialiseGlobals
			;	InitialiseHardware();
	BSF   0x03,RP0
	CALL  InitialiseHardware
			;
			;	GreenLED=0;
	BCF   0x03,RP0
	BCF   0x06,RB7
			;	ConfigSerial();
	BSF   0x03,RP0
	CALL  ConfigSerial
			;
			;	while (1) {
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
m019	MOVF  PacketBytes,W
	XORLW .14
	BTFSS 0x03,Zero_
	GOTO  m019
			;			SendString(&RxBuffer[RxBufferLength]);	
	MOVLW .75
	MOVWF Buffer
	CALL  SendString
			;			//PacketBytes=0;
			;		}
			;	}	
	GOTO  m019
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
			;	RxData=0;
	CLRF  RxData
			;	PacketBytes = 0;
	CLRF  PacketBytes
			;	//ProtocolState=0;
			;	PacketLength=RxBufferLength;		//initialising to zero is bad here
	MOVLW .14
	MOVWF PacketLength
			;	HDB1 = 0;
	CLRF  HDB1
			;	HDB2 = 0;
	CLRF  HDB2
			;	SNAP_CHECKSUM = 0;
	CLRF  SNAP_CHECKSUM
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
	GOTO  m020
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
	GOTO  m021
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
	GOTO  m021
			;		(*counter) = 0;
m020	BCF   0x03,IRP
	MOVF  counter,W
	MOVWF FSR
	CLRF  INDF
			;	}
			;}
m021	RETURN
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
m022	BTFSS 0x98,TRMT
	GOTO  m022
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
	CLRF  i_3
			;	
			;		if( OERR ==0 && FERR ==0 ) {
	BTFSC 0x18,OERR
	GOTO  m023
	BTFSC 0x18,FERR
	GOTO  m023
			;			text = RCREG;
	MOVF  RCREG,W
	MOVWF text_2
			;			//RedLED=0;
			;			//GreenLED=0;
			;		}
			;	return text;
m023	MOVF  text_2,W
	RETURN
			;}	 
			;void SendString(char *Buffer[RxBufferLength]) {
SendString
			;
			;char i =0;
	CLRF  i_4
			;char text =0;	
	CLRF  text_3
			;	
			;	for(i=0;i<RxBufferLength;i++){		// prepare to send data packet
	CLRF  i_4
m024	MOVLW .14
	SUBWF i_4,W
	BTFSC 0x03,Carry
	GOTO  m027
			;		while(TXIF == 0);				// Wait for Tx buffer to be empty
m025	BTFSS 0x0C,TXIF
	GOTO  m025
			;		text = RxBuffer[i];				// Put ito RAM to speed things up (why keep reading the Array?)  Does this do speed improvement?
	MOVLW .61
	ADDWF i_4,W
	MOVWF FSR
	BCF   0x03,IRP
	MOVF  INDF,W
	MOVWF text_3
			;		if(text != 0){	
	MOVF  text_3,1
	BTFSC 0x03,Zero_
	GOTO  m026
			;			RxCtrl = !RxActive;			// Disable Rs485 Rx
	BSF   0x06,RB0
			;			TxByte(text);				// Transmit buffer contents
	CALL  TxByte
			;			RxBuffer[i]=0;
	MOVLW .61
	ADDWF i_4,W
	MOVWF FSR
	BCF   0x03,IRP
	CLRF  INDF
			;		}
			;		//while(TRMT ==0);				// Wait for Byte to finsih being sent
			;		RxCtrl = RxActive;				// Enable RS485 Rx
m026	BCF   0x06,RB0
			;	}	
	INCF  i_4,1
	GOTO  m024
			;}
m027	RETURN
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
m028	BTFSC 0x9C,WR
	GOTO  m028
			;	GIE = 1;				// Re-enable interrupts
	BSF   0x0B,GIE
			;	
			;}
	RETURN

	ORG 0x2100
	DATA 000DH
	DATA 000AH
	DATA 0053H
	DATA 004EH
	DATA 0041H
	DATA 0050H
	DATA 0020H
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

; 0x010F   32 word(s)  3 % : InitialiseHardware
; 0x012F   12 word(s)  1 % : InitialiseGlobals
; 0x013B   33 word(s)  3 % : debounce
; 0x015C   14 word(s)  1 % : ConfigSerial
; 0x016A    7 word(s)  0 % : TxByte
; 0x0171   10 word(s)  0 % : RxByte
; 0x017B   29 word(s)  2 % : SendString
; 0x0198   10 word(s)  0 % : ReadEEPROM
; 0x01A2   18 word(s)  1 % : WriteEEPROM
; 0x0004  246 word(s) 24 % : IntHandler
; 0x00FA   21 word(s)  2 % : main

; RAM usage: 59 bytes (25 local), 165 bytes free
; Maximum call level: 2 (+2 for interrupt)
; Total of 433 code words (42 %)
