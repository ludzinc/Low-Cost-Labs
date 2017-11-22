/*
 * Timer1 module with external secondary 32.768 KHz oscillator
 *
 * Tested on:
 * Microstick + Microstick Plus Development Board
 * PIC24HJ64GP502
 *
 * File name: timer1.h
 * Author:    Jozsef KOPJAK
 * Info:      info@microstickplus.com
 * Web:       http://www.microstickplus.com
 *
 * Last modification: 10-04-2013
 */

#include "typedefs.h"

#ifndef _TIMER1_H
#define _TIMER1_H

#include "typedefs.h"

/* Frequency of secondary oscillator*/
#define F_SEC_OSC   32768

/* Global variables */
extern UBASE_TYPE uxTimer1Counter; // Counter variable - Incremented by Timer1 ISR

/*---------------------------------------------------------------------
  Function name: vInitCN
  Description: Initialization of Timer1 with external secondary 32.768 KHz oscillator
  Input parameters: -
  Output parameters: -
-----------------------------------------------------------------------*/
void vInitTimer1( void );

#endif
