// TO DO
//
// 1. why does making RxBufferLength larger break things??
// 2. checksum on / off testing
// 3. error reporting??
// 4. return message if checksum matches
// 5. build customised return messages

//#define   PIC16F627A
//#include "16F627A.H"
#include "INT16CXX.H"
#include "SNAP.H"

#pragma origin 			= 0x0004					// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???
#pragma config        	= 0b0011.1111.0101.1000   	// MCLR Pin is input
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// INTERRUPT ROUTINE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma codepage 0
interrupt IntHandler() {
   	int_save_registers
   	FSRTemp = FSR;
   	int i = 0;
   	
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TIMING FUNCTIONS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (TMR2IF == 1) {
		T2ms++;
		if (T2ms == 5) {
			T10ms++;
			T2ms = 0;											// add debounce routines here if needed
			if (T10ms == 10) {
				T100ms++;
				T10ms = 0;
				if (T100ms == 10) {
					T100ms = 0;
					SecondsFlag = 1;
					//GreenLED=!GreenLED;
					//RedLED=0;
				}
			}
		}
	TMR2IF = 0;
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// UART Rx FUNCTIONS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (RCIF ==1 ) {									// Data in UART Rx Buffer
		RxData =  RxByte();								// read data from UART buffer (also clears UART buffer)
		RedLED=1;										// check if RxByte is correct (no UART error)
		if( OERR == 1) {								// OVERRUN ERROR (2 Bytes in FIFO, and other byte lost)
			RxData = RCREG;
			RxData = RCREG;								// Read twice to clear FIFO
			CREN = 0;									// Clear Error by re-enabling transmission
			CREN=1;
			ProtocolState=PROTOCOL_ERROR_UARTOVERRUN;	// packet now broken
		}
		if (FERR == 1) {								// Framing Error.  Stop bit is not = 1.
			RxData = RCREG;
			ProtocolState=PROTOCOL_ERROR_UARTFRAMING;   // packet broken
		}
		// Otherwise Rx's character is okay and we can proceed	
		if((ProtocolState == PROTOCOL_WAIT_START) && (RxData == COMMAND_SNAP_SYNC)) {	// check if byte = sync byte, while waiting for sync
			PacketBytes = 0;							// reset PacketByte (and RxBuffer index) counter
			ProtocolState=PROTOCOL_GATHER_SNAP;			// advance ProtocolState
			GreenLED=1;
		}
		if((PacketBytes < RxBufferLength)&& (ProtocolState == PROTOCOL_GATHER_SNAP)){
			RxBuffer[PacketBytes] = RxData;
			PacketBytes++;
			if(PacketBytes==PacketLength){
					ProtocolState=PROTOCOL_WAIT_START;
					GreenLED=0;
					SNAP_CHECKSUM = 0;
			}
			if(PacketBytes==3){
				//TO DO!! Calculate packet length
				HDB2=RxBuffer[1];
				HDB1=RxBuffer[2];
				
				SNAP_ACK_NACK=HDB2&0x03;
				HDB2=HDB2>>2;
				SNAP_PFB=HDB2&0x03;
				HDB2=HDB2>>2;
				SNAP_SAB=HDB2&0x03;
				HDB2=HDB2>>2;
				SNAP_DAB=HDB2&0x03;
				
				SNAP_NDB=HDB1&0x0F;					//If SNAP_NDB < 8, then = HDBA&0b0000.1111
				if(SNAP_NDB > 8) {
					SNAP_NDB = 8*(2^(HDB1&0x07));	//Else apply SNAP rules
				}	
				
				RxBuffer[0]=SNAP_DAB+48;
				RxBuffer[1]=SNAP_SAB+48;
				RxBuffer[2]=SNAP_NDB+48;
				
				SNAP_EDM = (HDB1 & 0x70) >> 4;
				
				PacketLength=3 + SNAP_DAB + SNAP_SAB + SNAP_NDB;

				switch (SNAP_EDM) {
					case 0:
						break;
					case 1:
						break;
					case 2:
						PacketLength = PacketLength +1;
						break;
					case 3:
						PacketLength = PacketLength +1;
						break;
					case 4:
						PacketLength = PacketLength +2;
					case 5:
						PacketLength = PacketLength +4;
					case 6:
					case 7:
					default:
						break;
				}
			}		
			if(PacketBytes == PacketLength) {
				switch (SNAP_EDM) {
					case 0:							// do nothing - no checksum 
						break;						// No change in PacketLength!
					case 1:							// do nothing - x3 transmission mode (check x3 buffers here later)
						break;						// No change in PacketLength!
					case 2:							// 8 bit sum checksum
						for(i=0;i<PacketLength-1;i++) {
							SNAP_CHECKSUM = SNAP_CHECKSUM + RxBuffer[i];
						}
						//PacketLength = PacketLength + 1;
						RxBuffer[PacketLength-1]=SNAP_CHECKSUM;
						break;	
					case 3:
						//PacketLength = PacketLength + 1;
						break;						// do 8 bit crc here
					case 4:
						//PacketLength = PacketLength + 2;
						break;						// do 16 bit crc here
					case 5:
						//PacketLength = PacketLength + 4;
						break;						// do 32 bit crc here
					default:
						break;
				}	
			} 		
			if((PacketBytes > PacketLength)||(ProtocolState == PROTOCOL_ERROR_UARTOVERRUN) || (ProtocolState == PROTOCOL_ERROR_UARTFRAMING)) {
				ProtocolState=PROTOCOL_WAIT_START;
				// report error here before resetting to WAIT_STATE
			}
		}
	}
	RedLED=0;
   	FSR = FSRTemp;
   	int_restore_registers  	
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MAIN ROUTINE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void main(void) {

	char i =0;
	char StartFlag =0;
	char text =0;
	
	printstate=0;
		
	InitialiseGlobals();
	InitialiseHardware();

	GreenLED=0;
	ConfigSerial();

	while (1) {
/*		
		if(StartFlag==0){
			for(i=0;i<20;i++){			
				text=ReadEEPROM(i);
				RxBuffer[i]=text;
			}
			StartFlag=1;
			PacketBytes=RxBufferLength;
		}
*/
		if(PacketBytes == RxBufferLength){
			SendString(&RxBuffer[RxBufferLength]);	
			//PacketBytes=0;
		}
	}	
}// end main()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void InitialiseHardware(void) {
  	TRISA  = 0b00111111; 				// PORTA (0 = OUTPUT)
	PORTA  = 0b00000000;				// Initialise PORTA
	TRISB  = 0b00000000;      			// PORTB (0 = OUTPUT), RB1, RB2 = Serial coms, set as inputs.  Coms hardware asserts outputs as required
	PORTB  = 0b00000000;				// Initialise PORTB
	OPTION = 0b10001000;   				// No weak pull ups, prescaler assigned to WDT
   	INTCON = 0b11000000;				// TMR2 used to provide 1ms ticks.
	CMCON  = 0b00000111;				// Enable RA0:3 as Digital Inputs
	PCON   = 0b00001000;				// Set internal osciallator to 4MHz
	T2CON  = 0b00001101;				// TMR2 on, prescale = 1:4, and postscale = 1:2  (8us ticks with 4MHz oscillator)
	PIE1   = 0b00000010;				// Bit 1 enables TMR2 = PR2 interface
	RCIE = 1;							// enable UART Rx Interrupts
	
	PIR1   = 0b00000000;				// Read this to see if TMR2 = PR2 flag is set
	PR2 = 250;							// TMR2 match value, for 2ms ticks
}


void InitialiseGlobals(void) {
	T2ms = 0;							// Reset Timers
	T10ms = 0;
	T100ms = 0;
	SecondsFlag=0;
	RxData=0;
	PacketBytes = 0;
	//ProtocolState=0;
	PacketLength=RxBufferLength;		//initialising to zero is bad here
	HDB1 = 0;
	HDB2 = 0;
	SNAP_CHECKSUM = 0;
}




void debounce(char input, char *state, char *counter) {
	if ((*state) != input) {
   		(*counter)++;
		if ((*counter) >= MAX_DEBOUNCE_COUNT) {
			(*counter) = 0;
			(*state) = input;
		}
	} else {
		(*counter) = 0;
	}
}

void ConfigSerial(){
	
	TRISB = TRISB|0b0000.0110;		// RB1, RB2 inputs - UART sets outputs as needed
	SPBRG = 0x19;					// 9600 baud at 4MHz (9615 baud exactly) - HIGH SPEED BAUD RATE GENERATOR
	TXSTA = 0b0010.0100;			// Enable asynch Tx mode

	SPEN = 1;						// Turn UART on
	CREN = 1;						// Enable Reception
	TXEN = 1;						// Enable Transmission

	TxCtrl = TxActive;				// Turn on 485 
} 
	
void TxByte(char data){

		TXREG = data;				// stick byte into buffer
		while(TRMT ==0);			// Wait for Byte to finsih being sent
}

char RxByte(void) {					// read Rx data from UART

	char text = 0;
	char i = 0;
	
		if( OERR ==0 && FERR ==0 ) {
			text = RCREG;
			//RedLED=0;
			//GreenLED=0;
		}
	return text;
}	 
void SendString(char *RxBuffer[RxBufferLength]) {

char i =0;
char text =0;	
	
	for(i=0;i<RxBufferLength;i++){		// prepare to send data packet
		while(TXIF == 0);				// Wait for Tx buffer to be empty
		text = RxBuffer[i];				// Put ito RAM to speed things up (why keep reading the Array?)  Does this do speed improvement?
		if(text != 0){	
			RxCtrl = !RxActive;			// Disable Rs485 Rx
			TxByte(text);				// Transmit buffer contents
			RxBuffer[i]=0;
		}
		//while(TRMT ==0);				// Wait for Byte to finsih being sent
		RxCtrl = RxActive;				// Enable RS485 Rx
	}	
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EEPROM data access (PIC16F627A Version)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
char ReadEEPROM(char address) {
   	char data = 0;
   
	EEADR = address;        // 1. Write address to EEADR
//   	EEPGD = 0;              // 2. Clear EEPGD bit to point to EEPROM data (not Needed 16F627A)
   	RD = 1;                 // 3. Set the RD bit to start the read
   	data = EEDATA;          // 4. Read the data from EEDATA
   	return data;
}

void WriteEEPROM(char address, char data) {

	EEADR = address;
	EEDATA = data;
	
	GIE = 0;				// Disable interrupts
	WREN = 1;				// Enable EEPROM write
	EECON2 = 0x55;
	EECON2 = 0xAA;			
	WR = 1;					// EEPROM write sequence initiated
	while(WR == 1);			// Wait for write to complete
	GIE = 1;				// Re-enable interrupts
	
}

#define EEPROM_START 0x2100
#pragma cdata[EEPROM_START]
//Serial No, device type and firmware version
/*
#pragma cdata[] = 0x0A				//  00 - Serial No. 0
#pragma cdata[] = 0x0D				//  01 - Serial No. 1
#pragma cdata[] = 'B'				//  02 - Serial No. 2
#pragma cdata[] = 'y'				//  03 - Serial No. 3
#pragma cdata[] = 't'				//  04 - Serial No. 4
#pragma cdata[] = 'e'				//  05 - Serial No. 5
#pragma cdata[] = ' '				//  05 - Serial No. 6
#pragma cdata[] = 'm'				//  07 - Serial No. 7
#pragma cdata[] = 'e'				//  08 - Device Type 0 
#pragma cdata[] = '*'				//  08 - Device Type 1
#pragma cdata[] = 'm'				//  09 - Device Type 2
#pragma cdata[] = 'o'				//  10 - Firmware Version 0
#pragma cdata[] = 'n'				//  11 - 
#pragma cdata[] = ' '				//  12 - 
#pragma cdata[] = 0x0A				//  13 - 
#pragma cdata[] = 0x0D				//  14 - 
#pragma cdata[] = 0x01				//  15 - 
#pragma cdata[] = 0x02				//  16 - 
#pragma cdata[] = 0x01				//  17 - 
#pragma cdata[] = 0x02				//  18 - 
#pragma cdata[] = 0x01				//  19 - 
*/


//#pragma cdata[] = 'a','s','c','i','i'
#pragma packedCdataStrings 0
#pragma cdata[] =0x0d
#pragma cdata[] = 0x0a
#pragma cdata[] = "SNAP "
#pragma cdata[] =0x0d
#pragma cdata[] = 0x0a
#pragma cdata[] = "Ver 0.00"
#pragma packedCdataStrings 1
#pragma cdata.EEPROM_END
