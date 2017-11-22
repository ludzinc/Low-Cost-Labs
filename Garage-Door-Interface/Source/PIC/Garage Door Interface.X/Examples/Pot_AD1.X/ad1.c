/*
 * Analog to Digital Converter 
 *
 * Tested on:
 * Microstick + Microstick Plus Development Board
 * PIC24HJ64GP502
 *
 * File name: ad1.c
 * Author:    Jozsef KOPJAK
 * Info:      info@microstickplus.com
 * Web:       http://www.microstickplus.com
 *
 * Last modification: 10-04-2013
 */

#include <xc.h>
#include "HardwareProfile.h"
#include "typedefs.h"
#include "ad1.h"

/*---------------------------------------------------------------------
  Function name: xReadADC1
  Description: Read ADC value from specified analog channel
  Input parameters: Number of channel 
  Output parameters: ADC value
-----------------------------------------------------------------------*/
BASE_TYPE xReadADC1( BASE_TYPE xChanelNumber )
{
    BASE_TYPE xValue, xIndex;

    AD1CON1 = 0;
    AD1CON1bits.AD12B = 1;  // Mode: 12 bit
    AD1CON1bits.SSRC = 7;   // Auto convert
    AD1CON2 = 0;
    AD1CON3 = 0;
    AD1CON3bits.ADRC = 1;   // Use internal oscillator
    AD1CON3bits.SAMC = 31;
    AD1CON4 = 0;
    AD1CHS123 = 0;
    AD1CHS0 = xChanelNumber;
    AD1PCFGL &= ~(1<<xChanelNumber);
    _AD1IE = 0;
    AD1CON1bits.ADON = 1;	//Turn on ADC

    // Samples and convert 4 times the value of analog channel
    for( xValue = 0, xIndex = 4; xIndex != 0; xIndex-- )
    {
        AD1CON1bits.SAMP = 1;
        while(!AD1CON1bits.DONE);
        xValue += ADC1BUF0;
    }

    return xValue>>2;
}
