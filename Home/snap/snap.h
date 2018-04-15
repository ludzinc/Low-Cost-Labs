// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HARDWARE SETUP
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define RedLED			RB6		
#define GreenLED      	RB7   	

#define Input0			RA0
#define Input1			RA1
#define	Input2			RA2
#define Input3			RA3

#define Relay0			RB5
#define Relay1			RB4
#define Relay2			RA7
#define Relay3			RA6

#define TxCtrl	    	RB3
#define TxActive		1
#define RxCtrl			RB0
#define RxActive		0

#define MAX_DEBOUNCE_COUNT 				10			// How many times do we need the changed state = 100 ms
#define INPUT_ACTIVE					0
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SERIAL COMS SETUPS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define RxBufferLength	14
#define RxBufferLength	14
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EEPROM LOCATIONS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define EEPROM_SERNO_0	0x00
#define EEPROM_SERNO_1	0x01
#define EEPROM_SERNO_2	0x02
#define EEPROM_SERNO_3	0x03
#define EEPROM_SERNO_4	0x04
#define EEPROM_SERNO_5	0x05
#define EEPROM_SERNO_6	0x06
#define EEPROM_SERNO_7	0x07
#define EEPROM_DEV_0	0x08
#define EEPROM_DEV_1	0x09
#define EEPROM_FWVER	0x0A
#define EEPROM_CHIP		0x0B
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GLOBAL VARIABLES
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
char FSRTemp;							// Used for Interrupts
// Timer Variables
char T2ms;								// 2 millisecond counter
char T10ms;								// 10's millisecond counter
char T100ms;							// 100's millisecond counter
char SecondsFlag;						// Seconds Counter
// Serial Data
char RxBuffer[RxBufferLength];			
char PacketLength;
char PacketBytes;
char RxData;
char ProtocolState;
char printstate;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SNAP PROTOCOL VARIABLES
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
char HDB1;								// SNAP Header Definition Byte 2
char HDB2;								// SNAP Header Definition Byte 1
char SNAP_DAB;							// No. Destination Address Bytes
char SNAP_SAB;							// No. Source Address Bytes
char SNAP_PFB;
char SNAP_ACK_NACK;						// Ack / NACK mode
char SNAP_CMD;							// SNAP Command Mode
char SNAP_EDM;							// SNAP Error Definition Mode
char SNAP_NDB;							// No. data bytes
char SNAP_CHECKSUM;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Declare Function Prototypes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void InitialiseHardware(void);
void InitialiseGlobals(void);
void debounce(char input, char *state, char *counter);
void ConfigSerial(void);
void TxByte(char data);
char RxByte(void);
void CheckStates(void);
void SetRelays(void);

void SendString(char *Buffer[RxBufferLength]);

char ReadEEPROM(char);
void WriteEEPROM(char address, char data);





