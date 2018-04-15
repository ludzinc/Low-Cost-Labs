/*
 * BARUN GAS MODULE CONTROLLER
 * Simple project.  Monitors state of input buttons, set relay output as required
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdint.h>

#define buttonManual PORTBbits.RB1
#define buttonPID    PORTAbits.RA2
#define ledManual    PORTBbits.RB2
#define ledPID       PORTAbits.RA3
#define ledStatus    PORTBbits.RB3
#define relay        PORTAbits.RA1
#define testpoint   PORTAbits.RA0

#define MAX_DEBOUNCE_COUNT 20

#define buttonActive 0
#define relayManual  0
#define relayPID     1
#define ledON        1
#define ledOFF       0

#define _XTAL_FREQ = 8000000;

// CONFIG1
//#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin)
#pragma config FOSC = INTOSCCLK // Oscillator Selection bits (INTRC oscillator; CLKO function on RA6/OSC2/CLKO pin and port I/O function on RA7/OSC1/CLKI pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off)
#pragma config CCPMX = RB0      // CCP1 Pin Selection bit (CCP1 function on RB0)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)
// CONFIG2
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal External Switchover mode enabled)


// DEFINE GLOBALS
char buttonManualState;
char buttonPIDState;
char DebounceCounters[2];
char msCounter;
char ms100Counter;

//Function Prototypes
void initialiseVariables(void);
void initialiseHardware(void);
void checkButtons(void);
void debounce(char test, char *state, char *counter);
//Interrupt Functions

void interrupt checkInterrupts(void) {
    if (TMR0IE && TMR0IF) //are TMR0 interrupts enabled and is the TMR0 interrupt flag set?
    {
        TMR0 = 205; // sets overflow to happen in 250 more ticks for an even 1ms interrupt
        TMR0IF = 0; //TMR0 interrupt flag must be cleared in software to allow subsequent interrupts
        ++msCounter;
        //ledStatus=!ledStatus;
        if (msCounter > 99) {
            ms100Counter++;
            msCounter = 0;
        }
        if (ms100Counter < 51) { // Blink Status LED at 1Hz to show board is running
            ledStatus = ledOFF;
        }
        if (ms100Counter > 50) {
            ledStatus = ledON;
        }
        if (ms100Counter > 100) {
            ms100Counter = 0;
        }
    }
}
//Main Function

void main(void) {
    initialiseVariables();
    initialiseHardware();
    while (1) {
        checkButtons();
        if (buttonManualState == buttonActive) {
            relay = relayManual;
            ledManual = ledON;
            ledPID = ledOFF;
        }
        if (buttonPIDState == buttonActive) {
            relay = relayPID;
            ledManual = ledOFF;
            ledPID = ledON;
        }
    }
}
// FUNCTIONS
// Initalise software variables.

void initialiseVariables(void) {
    DebounceCounters[0] = 0;
    DebounceCounters[1] = 0;
    buttonManualState = 1; // Active Low
    buttonPIDState = 1; // Active Low
    msCounter = 0;
    ms100Counter = 0;
};
// Initialise System Hardware

void initialiseHardware(void) {
    PORTA = 0x00;
    PORTB = 0x00;
    TRISA = 0b00000100; // RA2 = PID button input, all others are output.
    TRISB = 0b00000010; // RB1 = Manual button input, all others are output.
    CMCON = 0b00000111; // Analog Comparators are off
    //OPTION_REG = 0b00000100; // TMR0 pre-scaler = 32 (4uS ticks)
    OPTION_REG = 0b00000001; // TMR0 pre-scaler = 32 (4uS ticks)
    OSCCON = 0b01110000; // 8MHz internal clock
    INTCON = 0b10100000; // Interrupts enabled, TMR0 interrupt enabled

    relay = relayPID;
    ledManual = ledOFF;
    ledPID = ledON;
};
// Check input button status

void checkButtons(void) {
    debounce(buttonManual, &buttonManualState, &DebounceCounters[0]);
    debounce(buttonPID, &buttonPIDState, &DebounceCounters[1]);
}
// debounce buttons

void debounce(char test, char *state, char *counter) {
    if ((*state) != test) {
        (*counter)++;
        if ((*counter) >= MAX_DEBOUNCE_COUNT) {
            (*counter) = 0;
            (*state) = test;
        }
    } else {
        (*counter) = 0;
    }
}