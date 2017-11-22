/*
 * Hardware Profile
 *
 * Tested on:
 * Microstick + Microstick Plus Development Board
 * PIC24HJ64GP502
 *
 * File name: HardwareProfile.h
 * Author:    Jozsef KOPJAK
 * Info:      info@microstickplus.com
 * Web:       http://www.microstickplus.com
 *
 * Last modification: 10-04-2013
 */

#ifndef _HARDWAREPROFILE_H
#define _HARDWAREPROFILE_H

// Definition of Microstick Plus Development Board revision numbers
#define RevA 1
#define RevB 2
#define RevC 3

// Current revision is:
//#define MICROSTICKPLUS_REV  RevA
//#define MICROSTICKPLUS_REV  RevB
#define MICROSTICKPLUS_REV  RevC

// Oscillator frequency
#define FOSC	80000000L
#define FCY	(FOSC/2)

// 3.3V power pin
#define POWER_LAT   _LATA3
#define POWER_PORT  _RA3
#define POWER_TRIS  _TRISA3

// Turn on and turn of 3.3V power
#define mTurnOnPower()  {POWER_LAT = 1; POWER_TRIS = 0;}
#define mTurnOffPower()  {POWER_LAT = 0; POWER_TRIS = 1;}

// LEDs
#define LEDs_LAT        LATB
#define LEDs_TRIS       TRISB

// Macros to menage LEDs
#define mInitLEDs() {LEDs_TRIS &= 0x0FFF;}
#define mWriteLEDs( n ) {LEDs_LAT = (LEDs_LAT&0x0FFF) | (((n)&0xF)<<12);}
#define mReadLEDs() ( (LEDs_LAT >> 12) & 0x000F )

// Button
#define BUTTON_PORT  _RA2
#define BUTTON_TRIS  _TRISA2

// Macros to manage Button
#define mInitButton()   {BUTTON_TRIS = 1;}
#define mGetButtonState()   (!BUTTON_PORT)

// Rotary Encoder
#define ROTARY_LAT        LATB
#define ROTARY_PORT       PORTB
#define ROTARY_TRIS       TRISB

// Read state of Rotary Encoder
#define mReadRotaryState() ( (~(ROTARY_PORT) >> 6) & 0x0003 )

#endif