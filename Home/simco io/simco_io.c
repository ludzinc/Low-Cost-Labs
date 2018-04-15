// HOW TO GET cc5x to complie eeprom in MPLAB?? - without having to import hex file...
//
// TO DO
//
// 1.  Have look up table in EEPROM for RELAY ON times
// 2.  Map inputs to outputs in EEPROM
// 3.  Work out serial protocol - rx cmds, chksum and reply!
// 4.  How to break routines out to sep. file so I get main.c alone.....


//#define   PIC16F627A

//#include "16F627A.H"
#include "INT16CXX.H"
#include "SIMCO_IO.H"
//#include "SIMCO_ROUTINES.H"

#pragma origin = 0x0004																	// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???
#pragma config        = 0b0011.1111.0101.1000   	// MCLR Pin is input
//#pragma config        = 0b0011.1111.0111.1000   	// MCLR Pin is input



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// INTERRUPT ROUTINE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma codepage 0
interrupt IntHandler() {
   	int_save_registers
   	FSRTemp = FSR;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TIMING FUNCTIONS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (TMR2IF == 1) {
		T2ms++;
		if (T2ms == 5) {
			T10ms++;
			T2ms = 0;
			debounce(Input0,&InputStates[0],&InputCounters[0]);
			debounce(Input1,&InputStates[1],&InputCounters[1]);
			debounce(Input2,&InputStates[2],&InputCounters[2]);
			debounce(Input3,&InputStates[3],&InputCounters[3]);
			if (T10ms == 10) {
				T100ms++;
				T10ms = 0;
				if (T100ms == 10) {
					T100ms = 0;
					SecondsFlag = 1;
					//GreenLED=!GreenLED;
					//RedLED=0;
					StartupFlag=0;
					for(loop=0;loop<4;loop++){
						temp=RelayTimers[loop];
						if(temp>0){
							temp--;
							RelayTimers[loop]=temp;
						}	
					}
				}
			}
		}
	TMR2IF = 0;
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// UART Rx FUNCTIONS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (RCIF ==1 ) {								// Data in UART Rx Buffer
		RxData =  RxByte();							// read data from UART buffer (also clears UART buffer)
													// check if RxByte is correct (no UART error)
		if( OERR == 1) {							// OVERRUN ERROR (2 Bytes in FIFO, and other byte lost)
			RxData = RCREG;
			RxData = RCREG;							// Read twice to clear FIFO
			CREN = 0;								// Clear Error by re-enabling transmission
			CREN=1;
			RedLED=1;	
			ProtocolState=PROTOCOL_ERROR_UARTOVERRUN;	// packet now broken
		}
		if (FERR == 1) {							// Framing Error.  Stop bit is not = 1.
			RxData = RCREG;
			GreenLED=1;
			ProtocolState=PROTOCOL_ERROR_UARTFRAMING;   // packet broken
		}
		// Otherwise Rx's character is okay and we can proceed	
	
		if(ProtocolState == PROTOCOL_WAIT_START){
			GreenLED=1;
		}		
		if((RxData == COMMAND_SNAP_SYNC)&&(Foo != 0)) {
			RedLED=1;
		}		
	
		if((ProtocolState == PROTOCOL_WAIT_START) && (RxData == COMMAND_SNAP_SYNC)) {	// check if byte = snyc byte, while waiting for snyc
			PacketBytes = 0;						// reset PacketByte (and RxBuffer index) counter
			ProtocolState=PROTOCOL_GATHER_SNAP;		// advance ProtocolState
			RxBuffer[PacketBytes]=RxData;			// place sync byte in index 0 in the RxBuffer
			PacketBytes++;
			//GreenLED=1;
		}

		if(PacketBytes < RxBufferLength){
			if(ProtocolState==PROTOCOL_GATHER_SNAP){
				RxBuffer[PacketBytes] = RxData;
				PacketBytes++;
				if(PacketBytes==PacketLength){
						ProtocolState=PROTOCOL_WAIT_START;
						GreenLED=0;
						RedLED=0;
				}
				if(PacketBytes==3){
					//TO DO!! Calculate packet length
					PacketLength=14;
				}	
				//if(PacketBytes==PacketLength){
				// calculate checksum
				// compare checksum
				// respond with nack if required OR
				// reset with no response		
			}	
		}
		
		if((PacketBytes >= PacketLength)||(ProtocolState == PROTOCOL_ERROR_UARTOVERRUN) || (ProtocolState == PROTOCOL_ERROR_UARTFRAMING)) {
			ProtocolState=PROTOCOL_WAIT_START;
			// replace with PROTOCOL_ERROR_OVERFLOW
		}
	}
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
	
	InitialiseGlobals();
	InitialiseHardware();

	Foo=0;

	//RxBuffer[0]=text;


	while(StartupFlag){
		GreenLED = 1;
		RedLED = 0;
		for(i=0;i<4;i++){
			InputStates[i]=1;
			RelayTimers[i]=0;
		}
	}

	GreenLED=0;
	ConfigSerial();

	while (1) {
		CheckStates();
		SetRelays();
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
			PacketBytes=0;
			loop=0;
		}

		TxByte(ProtocolState);
		
/*
		for(i=0;i<128;i++) {
			WriteEEPROM(i,i+64);
		}	
	*/
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
	
	

	StartupFlag=1;
	
	Relay0 = 0;
	Relay1 = 0;
	Relay2 = 0;
	Relay3 = 0;

}


void InitialiseGlobals(void) {
	T2ms = 0;							// Reset Timers
	T10ms = 0;
	T100ms = 0;
	SecondsFlag=0;
	loop=0;
	RxData=0;
	PacketBytes = 0;
	//ProtocolState=0;
	PacketLength=RxBufferLength;		//initialising to zero is bad here
	//Foo = 0;
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

void CheckStates(){
		// while inputs are active the mapped outputs will also be active, and remain active for their time periods
		// can't be stuffed setting flags to make time periods absolute!
		// decrement (if required) timers in timer interrupt routine
		if(InputStates[0]==INPUT_ACTIVE){
			RelayTimers[0]=DOOR_TIME;					// Close Relay 0 for door0 control			
			RelayTimers[3]=LIGHT_TIME;					// Close Relay 4 for light control
			T100ms=0;
			RedLED=1;
		}
		if(InputStates[1]==INPUT_ACTIVE){
			RelayTimers[1]=DOOR_TIME;					// Close Relay 1 for  door1 control			
			RelayTimers[3]=LIGHT_TIME;					// Close Relay 4 for light control
			T100ms=0;
			RedLED=1;
		}
		if(InputStates[2] == INPUT_ACTIVE){
			RelayTimers[2] = DOOR_TIME;					
			T100ms=0;
			RedLED=1;
		}
		if(InputStates[3] == INPUT_ACTIVE) {	
			RelayTimers[3] = LIGHT_TIME;
			T100ms=0;
			RedLED=1;
		}
}


void SetRelays(){

		if(RelayTimers[0]>0){
			Relay0=1;
		}
		else {
			Relay0=0;
		}
		if(RelayTimers[1]>0){
			Relay1=1;
		}
		else {
			Relay1=0;
		}
		if(RelayTimers[2]>0){
			Relay2=1;
		}
		else {
			Relay2=0;
		}
		if(RelayTimers[3]>0){
			Relay3=1;
		}
		else {
			Relay3=0;
		}
}


void SendString(char *Buffer[RxBufferLength]) {

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
#pragma cdata[] = "SIMCO IO"
#pragma cdata[] =0x0d
#pragma cdata[] = 0x0a
#pragma cdata[] = "Ver 0.00"
#pragma packedCdataStrings 1
#pragma cdata.EEPROM_END
