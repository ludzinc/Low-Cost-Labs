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

#define RxBufferLength		14
#define COMMAND_SNAP_SYNC	0x54
#define PROTOCOL_WAIT_START				0
#define PROTOCOL_GATHER_SNAP 			1
#define PROTOCOL_ERROR_UARTOVERRUN		254
#define PROTOCOL_ERROR_UARTFRAMING		255

#define MAX_DEBOUNCE_COUNT 	10			// How many times do we need the changed state = 100 ms

#define INPUT_ACTIVE	0

#define DOOR_TIME		1				// Close Relay for 1 second to operate door output
#define LIGHT_TIME		300				// Close Relay for 5 minutes for light output (300 seconds, 4 for debugging)


char FSRTemp;
char T2ms;								// 2 millisecond counter
char T10ms;								// 10's millisecond counter
char T100ms;							// 100's millisecond counter
char SecondsFlag;

char StartupFlag;

uns16 RelayTimers[4];
char loop;
uns16 temp;

char RxBuffer[RxBufferLength];
char PacketLength;
char PacketBytes;
char RxData;
char ProtocolState;
char Foo;

char Inputs[4];
char InputStates[4];
char InputCounters[4];

// Declare Function Prototypes
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





// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EEPROM LOCATIONS
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