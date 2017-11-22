/*
 * Timer1 module with external secondary 32.768 KHz oscillator
 *
 * Tested on:
 * Microstick + Microstick Plus Development Board
 * PIC24HJ64GP502
 *
 * File name: timer1.c
 * Author:    Jozsef KOPJAK
 * Info:      info@microstickplus.com
 * Web:       http://www.microstickplus.com
 *
 * Last modification: 10-04-2013
 */

#include <xc.h>
#include "HardwareProfile.h"
#include "typedefs.h"
#include "timer1.h"

/* Global variables */
UBASE_TYPE uxTimer1Counter; // Counter variable - Incremented by Timer1 ISR

/*---------------------------------------------------------------------
  Function name: vInitCN
  Description: Initialization of Timer1 with external secondary 32.768 KHz oscillator
  Input parameters: -
  Output parameters: -
-----------------------------------------------------------------------*/
void vInitTimer1( void )
{
    __builtin_write_OSCCONL( OSCCONL | 0x02 ); // Enable secondary oscillator
    T1CON = 0;                // Clear control register of Timer 1
    T1CONbits.TCS = 1;        // Use external oscillator
    PR1 = (F_SEC_OSC/2)-1;	  // Set Period Register 1, deltaT=500ms 
    T1CONbits.TON = 1;	      // Turn On Timer 1
    IFS0bits.T1IF = 0;        // Clear Timer1 Interrupt Flag
    IPC0bits.T1IP = 1;        // Set priority of Timer1 Interrupt
    IEC0bits.T1IE = 1;        // Enable Timer1 Interrupt
}

/*---------------------------------------------------------------------
  Function name: _T1Interrupt
  Description: Timer1 Interrupt Service Routine
  Input parameters: -
  Output parameters: -
-----------------------------------------------------------------------*/
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt( void )
{
    IFS0bits.T1IF = 0;        // Clear Timer1 Interrupt Flag
    uxTimer1Counter++;        // Incrementing Timer Counter
}

