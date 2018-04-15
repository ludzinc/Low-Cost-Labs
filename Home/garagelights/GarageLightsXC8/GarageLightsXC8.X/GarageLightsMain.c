/* 
 * File:   GarageLightsMain.c
 * Author: Simon
 *
 * Created on 24 June 2013, 9:46 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "PIC18F2480.H"													// CHECK THIS IN DATASHEET.  WHAT 0x0004 ???
#include <xc.h>
#include <plib.h>
#include <stdint.h>

#define MAX_DEBOUNCE_COUNT 	50			// How many times do we need the changed state
#define DOOR1_RUNTIME		1			// 1s
#define DOOR2_RUNTIME		1			// 1s
#define LIGHTS_RUNTIME		5			// in seconds

// Register: CMCON
extern volatile unsigned char           CMCON               @ 0xFB4;


// DEFINE FUNCTION PROTOTYPES
void InitialiseHardware (void);
void InitialiseGlobals(void);
void debounce(char test, char *state, char *counter);

// DEFINE PINOUTS
#define GREEN_LED   PORTBbits.RB0
#define RED_LED     PORTBbits.RB2
#define LIGHT_RELAY PORTCbits.RC0
#define SPARE_RELAY PORTCbits.RC1
#define DOOR1_RELAY PORTCbits.RC2
#define DOOR2_RELAY PORTCbits.RC3

#define BUTTON1     PORTAbits.RA1
#define BUTTON2     PORTAbits.RA2
#define BUTTON3     PORTAbits.RA0


// DEFINE GLOBALS
unsigned int Tms =0;

char FSRTemp;
char T1ms;								// 1 millisecond counter
char T10ms;								// 10's millisecond counter
char T100ms;							// 100's millisecond counter

char blink;

char StartButtonState;
char StartButtonPrev;
char StartButtonCounter;

uint16_t    TIMERS[4];
char        INPUTS_RAW[4];
char        INPUTS_STATE[4];
uint16_t    OUTPUTS[4];
char	DebounceCounters[4];

char 	LoopCounter;


/*
 * 
 */

void interrupt isr()
{
    if(PIR1bits.TMR2IF == 1){
        Tms ++;
        if (Tms==500){
            blink = 1;
        }
        if (Tms==1000){
            Tms=0;
            blink = 1;       
            for(LoopCounter=0;LoopCounter<3;LoopCounter++){
                if(OUTPUTS[LoopCounter]>0){			// IF output flag has been set
                    OUTPUTS[LoopCounter]--;
                }
            }
        }
        //reset the interrupt flag
    PIR1bits.TMR2IF = 0;
    }
}


void main(void) {

int i = 0;
int temp = 0;
int time = 0;
int offflag = 0; //used to turn off light is light button pressed again

	InitialiseHardware();
	InitialiseGlobals();

	while (1) {

            if(blink == 1){
            GREEN_LED=!GREEN_LED;
            blink = 0;
            }

            temp = PORTA;
            for(i=0;i<3;i++){
		INPUTS_RAW[i]=temp&0x01;
		if(INPUTS_RAW[i]==0){
			RED_LED = 1;
			} else {
			RED_LED= 0;
		}
		temp = temp >> 1;
		debounce(INPUTS_RAW[i], &INPUTS_STATE[i], &DebounceCounters[i]);
		if(INPUTS_STATE[i]==0){
                        OUTPUTS[i]=TIMERS[i];
			}
		}

            if(OUTPUTS[0] == 0) {LIGHT_RELAY = 0;} else {LIGHT_RELAY = 1;}
            if(OUTPUTS[1] == 0) {DOOR1_RELAY = 0;} else {DOOR1_RELAY = 1; OUTPUTS[0] = TIMERS[0];}
            if(OUTPUTS[2] == 0) {DOOR2_RELAY = 0;} else {DOOR2_RELAY = 1; OUTPUTS[0] = TIMERS[0];}
            
	}
}

void InitialiseHardware(void) {
    
        // PIC18F2480 Configuration Bit Settings

// CONFIG1H
#pragma config OSC = IRCIO67    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
                                // 8MHz internal, 500ns ticks
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = BOHW     // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown-out Reset Voltage bits (VBOR set to 2.1V)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer 1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config BBSIZ = 1024     // Boot Block Size Select bit (1K words (2K bytes) boot block)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

  	TRISA  = 0x0F; 				// PORTA (0 = OUTPUT)
  	TRISB  = 0x00; 				// PORTB (0 = OUTPUT)
  	TRISC  = 0x00; 				// PORTC (0 = OUTPUT)
	PORTA  = 0x00;				// Initialise PORTA
	PORTB  = 0x00;				// Initialise PORTB
	PORTC  = 0X00;
	ADCON1 = 0x0F;				// All portA inputs are digital
	INTCON = 0b11000000;
        CMCON  = 0x07;
	PIE1   = 0x02;
	IPR1   = 0x00;
        OSCCONbits.IRCF= 7;                                     // INternal osciallator set to 8 MHz (default is 1MHz)
        OpenTimer2(TIMER_INT_ON & T2_PS_1_4 & T2_POST_1_2);     // TMR2 on, prescale = 1:4, and postscale = 1:2  (4us ticks with 8MHz oscillator)
        PR2    = 250;                                           // 250 * 4us = 1ms interrupts
}

void InitialiseGlobals(void) {
	int i = 0;

	T1ms = 0;							// Reset Timers
	T10ms = 0;
	T100ms = 0;
//	SecondsFlag=0;
	for(i=0;i<4;i++){
		INPUTS_RAW[i]=0;
		INPUTS_STATE[i]=1;
		OUTPUTS[i]=0;
	}
	LoopCounter=0;
	TIMERS[0] = LIGHTS_RUNTIME;
	TIMERS[1] = DOOR1_RUNTIME;
	TIMERS[2] = DOOR2_RUNTIME;
}

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

