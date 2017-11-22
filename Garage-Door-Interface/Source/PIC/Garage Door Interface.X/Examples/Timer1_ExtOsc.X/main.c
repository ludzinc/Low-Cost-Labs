/*
 * Using external secondary 32.768 KHz oscillator to drive Timer1
 *
 * Tested on:
 * Microstick + Microstick Plus Development Board
 * PIC24HJ64GP502
 *
 * File name: main.c
 * Author:    Jozsef KOPJAK
 * Info:      info@microstickplus.com
 * Web:       http://www.microstickplus.com
 *
 * Description:
 * Timer1 is initialized with interrupts enabled and the external 32.768 kHz
 * oscillator is enabled. The timer generates interrupt in every second. The
 * interrupt service routine increment the value of a global variable. The
 * value of the global variable affects the LED-s located on the development
 * board. The displayed LED design can be changed with the help of the button
 * on the board.
 *
 * Last modification: 10-04-2013
 */

#include <xc.h>
#include "HardwareProfile.h"
#include "typedefs.h"
#include "timer1.h"

/* Setting up configuration bits */
#if defined(__PIC24HJ64GP502__) || defined(__dsPIC33FJ64MC802__) || defined(__PIC24HJ128GP502__) || defined(__dsPIC33FJ128MC802__)
  _FOSCSEL(FNOSC_FRC);                              // Use internal oscillator  (FOSC ~ 8 Mhz)
  _FOSC(FCKSM_CSECMD & OSCIOFNC_ON & POSCMD_NONE);  // RA3 pin is output
  _FWDT(FWDTEN_OFF);                                // Watchdog timer is disabled
  _FICD(JTAGEN_OFF);                                // JTAG Port is disabled
#else
  #error "The config bits aren't configured! Please, configure it!"
#endif

/* Function prototypes */
void vInitPLLIntOsc( void );
void vInitApp( void );

/*---------------------------------------------------------------------
  Function name: main
  Description: Entry point of the program
  Input parameters: -
  Output parameters: -
-----------------------------------------------------------------------*/
int main( void )
{
    vInitApp(); // Hardware initialization

    /* Infinite loop */
    while(1)
    {
        // Read the state of the button
        if( mGetButtonState() )
        {
          // If the button is pressed:
            mWriteLEDs(uxTimer1Counter);
        }
        else
        {
          // If the button is not pressed:
            mWriteLEDs(1<<(uxTimer1Counter&0b11));
        }
    }
}

/*---------------------------------------------------------------------
  Function name: vInitApp
  Description: Startup initializations
  Input parameters: -
  Output parameters: -
-----------------------------------------------------------------------*/
void vInitApp( void )
{
    vInitPLLIntOsc(); // Initialization of PLL
    mTurnOnPower();   // Turn on 3.3V power pin
    mInitLEDs();      // Initialization of LEDs
    mInitButton();    // Initialization of Button
    vInitTimer1();    // Initialization of Timer1 with secondary oscillator
}

/*---------------------------------------------------------------------
  Function name: vInitPLLIntOsc
  Description: Configuring internal oscillator with PLL (7.37 MHz -> 80 MHz )
  Input parameters: -
  Output parameters: -
-----------------------------------------------------------------------*/
void vInitPLLIntOsc( void )
{
    PLLFBD = 41;            // M = 43
    CLKDIVbits.PLLPOST=0;   // N1 = 2
    CLKDIVbits.PLLPRE=0;    // N2 = 2

    __builtin_write_OSCCONH(0x01);
    __builtin_write_OSCCONL(0x01);

    while (OSCCONbits.COSC != 0b001);
    while(OSCCONbits.LOCK != 1);
}
