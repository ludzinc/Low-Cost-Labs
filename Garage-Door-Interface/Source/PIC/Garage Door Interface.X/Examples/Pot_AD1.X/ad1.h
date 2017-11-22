/*
 * Analog to Digital Converter 
 *
 * Tested on:
 * Microstick + Microstick Plus Development Board
 * PIC24HJ64GP502
 *
 * File name: ad1.h
 * Author:    Jozsef KOPJAK
 * Info:      info@microstickplus.com
 * Web:       http://www.microstickplus.com
 *
 * Last modification: 10-04-2013
 */

#ifndef _AD1_H
#define _AD1_H

#include "typedefs.h"

/*---------------------------------------------------------------------
  Function name: xReadADC1
  Description: Read ADC value from specified analog channel
  Input parameters: Number of channel 
  Output parameters: ADC value
-----------------------------------------------------------------------*/
BASE_TYPE xReadADC1( BASE_TYPE xChanelNumber );

#endif
