/* 
 * File:   DistAudio.c
 * Author: Simon
 *
 * Created on 28 June 2013, 11:12 PM
 */

#include <xc.h>
//#include "config.h"
//#include <plib/usart.h>
#include <stdio.h>
#include <stdlib.h>
//#include "i2c.h"
#include <stdint.h>

// PIC16F87 Configuration Bit Settings

#include <xc.h>

// CONFIG1
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
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

#define _XTAL_FREQ 8000000 //The speed of your internal(or)external oscillator
#define USE_AND_MASKS

#define LED_1   PORTBbits.RB6
#define LED_2   PORTBbits.RB3

//#include <p18cxxx.h>

/*
 * 
 */
int main(int argc, char** argv) {

    LED_1=1;
    LED_2=1;

    return (EXIT_SUCCESS);
}

