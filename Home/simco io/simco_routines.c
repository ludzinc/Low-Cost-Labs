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
}

char RxByte(void) {					// read Rx data from UART

	char text = 0;
	char i = 0;
	
		if( OERR ==0 && FERR ==0 ) {
			text = RCREG;
			RedLED=0;
			GreenLED=0;
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
		text = RxBuffer[i];				// Put ito RAM to speed things up (why kep reading the Array?)  Does this do speed improvement?
		if(text != 0){	
			RxCtrl = !RxActive;			// Disable Rs485 Rx
			TxByte(text);				// Transmit buffer contents
			RxBuffer[i]=0;
		}
		while(TRMT ==0);				// Wait for Byte to finsih being sent
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

#define EEPROM_START 0x2100
#pragma cdata[EEPROM_START]
//Serial No, device type and firmware version
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

#pragma cdata.EEPROM_END
