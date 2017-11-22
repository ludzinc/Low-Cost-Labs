/* 
 * File:   Garage Door Interface.c
 * Author: Simon Ludborzs
 *
 * Created on 8 September 2015, 8:11 PM
 */
#define FCY 4000000 //Fcy = (Fosc/2); Fosc = 8MHz

#include <p24F16kl402.h>
#include <xc.h>
//#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "Garage Door Interface.h"
#include <libpic30.h>
#include <timer.h>
#include <uart.h>
#include <string.h>

#define USE_AND_MASKS
#define _ISR_PSV __attribute__((__interrupt__, __auto_psv__))

unsigned int valueToMatch = 500;    // 500 * 2us = 1ms interrupts
char *TXData = "NASCO12345";

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Interrupt Routine(s)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void _ISR_PSV _T1Interrupt()
{
    tick_1ms++;
    if(tick_1ms == 1000){
        tick_1ms = 0;
        Seconds++;          
   }
    IFS0bits.T1IF = 0; //clear T1 interrupt flagRead
}

int main(int argc, char** argv) {


    InitialiseHardware();
    InitialiseGlobals();
    InitialiseUART1();

    Status0 =1;
    int i =0;

    while(1){
        if(tick_1ms < 500){
            Status1=0;
        }else{
            Status1=1;
            if(Seconds == 1){
                Seconds = 0;
                putsUART1((unsigned int *)TXData);
            }
        }
    }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Function Code
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void InitialiseHardware(void) {
// Set up hardware devices
// Set Tristate and Analog Selection registers
    TRISA   = 0x21;             //RA0 Input, al oters outpus (expcet RA5, MCLR function)
    ANSELA  = 0x01;             //RA0, Analogue input, all others Digital
    TRISB   = 0x0006;
    ANSELB  = 0x0000;
// Set up Timer1
    ConfigIntTimer1(T1_INT_PRIOR_1 & T1_INT_ON);        //Lowest interrupt priority.  Being only interrupt, who cares :)
    OpenTimer1(T1_ON &                                  //Timer1 ON
            T1_GATE_OFF &                               //Gated mode OFF
            T1_IDLE_STOP &                              //Stop when controller is in sleep
            T1_PS_1_8 &                                 //Prescale 8, gives 2us ticks
            T1_SYNC_EXT_OFF &                           // Don't sync with external clock
            T1_SOURCE_INT,valueToMatch);
}

void InitialiseGlobals(void) {
// Timer Variables
    tick_1ms = 0;		// 1 millisecond counter
    Seconds = 0;
// Serial Data
    for(temp=0;temp<TxBufferLength;temp++){
        RxBuffer[temp] = 0;
        TxBuffer[temp] = 0;
    }
    RxData = 0;
}

void InitialiseUART1() {
    unsigned int UartMode = 0;
    unsigned int UartCtrl = 0;

    UartMode =  UART_EN &       //Enable UART
            UART_IDLE_CON &     //Continue working when Idle
            UART_IrDA_DISABLE & //Not using IrDA
            UART_MODE_SIMPLEX & //Not using Flow control
            UART_UEN_00 &       //Not using CTS/RTS pins by not setting it up
            UART_DIS_WAKE &     //Disable wakeup option
            UART_DIS_LOOPBACK & //Not using in loopback mode
            UART_DIS_ABAUD &    //Disable ABAUD
            UART_NO_PAR_8BIT &  //8bit data with none for parity
            UART_BRGH_SIXTEEN & //Clock pulse per bit
            UART_1STOPBIT;      // 1 stop bit

    UartCtrl = UART_TX_ENABLE &     //Enable UART transmit
            UART_IrDA_POL_INV_ZERO; //VERY IMPORTANT - sets the pin's idle state

    OpenUART1(UartMode,UartCtrl, BRGVAL);
}