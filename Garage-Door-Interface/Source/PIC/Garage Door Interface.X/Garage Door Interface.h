/*
 * File:   Garage Door Interface.h
 * Author: Simon
 */

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GLOBAL VARIABLES
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Timer Variables
volatile int tick_1ms;
volatile int Seconds;
volatile int minute;
// Serial Data
char RxBuffer1[RxBuffer1Length + 1];
char TxBuffer1[TxBuffer1Length + 1];
char RxBuffer2[RxBuffer2Length + 1];
char TxBuffer2[TxBuffer2Length + 1];

int RxBuffer1Index = 0;
int TxBuffer1Index = 0;
int RxBuffer2Index = 0;
int TxBuffer2Index = 0;

char RxData;
int temp;
int BufferReadyFlag = 0;
int tempSetting = 'C';
int temperature = 0;
float showTemperature = 0;
// Miscellaneous
int x;

int ADCResultLow;
int ADCResultHi;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Declare Function Prototypes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Hardware Initialisation
void InitialiseHardware(void);
void InitialiseGlobals(void);
void InitialiseUART1(void);
void InitialiseUART2(void);
void InitialiseMSSP(void);
void InitialiseADC(void);
// UART Routines
void SendStringUART1(char *);
void SendByteUART1(char);
char GetByteUART1(void);

void SendStringUART2(char *);
void SendByteUART2(char);
char GetByteUART2(void);
void ProcessIncomingByteUART2(void);

void ProcessString(void);
// I2C (MSSP1) Routines
void SendStartI2C(void);
int SendDataI2C(char); //use pointer reference later?
int ReadDataI2C(void);
void AckI2C(void);
void NackI2C(void);
void WaitIdleI2C(void);
void SendStopI2C(void);
// Higher Level Prototypes
// int FetchTemperature(void);
void FetchTemperature(void);
int ReadADC(void);
void triggerDoor(int);

//char ReadEEPROM(char);
//void WriteEEPROM(char address, char data);