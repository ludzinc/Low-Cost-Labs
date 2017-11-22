/*
 * File:   Garage Door Interface.c
 * Author: Simon Ludborzs
 *
 * Created on 8 September 2015, 8:11 PM
 */
#define FCY 4000000 //Fcy = (Fosc/2); Fosc = 8MHz

#include <p24Fv16km202.h>
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "Garage Door Interface.h"
//#include <libpic30.h>
#include <string.h>
//#include <uart.h>

#define USE_AND_MASKS
#define _ISR_PSV __attribute__((__interrupt__, __auto_psv__))

unsigned int PR1Match = 500; // 500 * 2us = 1ms interrupts
// Timer1 uses 1:8 prescaler, for 2us ticks

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Interrupt Routine(s)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//void _ISR_AUTO_PSV _U1RXInterrupt(){
//
//}

void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void) {
    int inByte = 0;
    // read UART2 data ???? = U2RXREG;
    if ((U2STAbits.OERR == 1)) { // Clear overrun flag if needed
        U2STAbits.OERR = 0;
    }
    inByte = U2RXREG; // Read Byte from UART
    switch (inByte) {
        case '\n': // end of text
            RxBuffer2[RxBuffer2Index] = inByte;
            RxBuffer2[RxBuffer2Index + 1] = 0; // terminating null byte
            //              ProcessString();
            BufferReadyFlag = 1;
            RxBuffer2Index = 0;
            //TxBuffer2 = RxBuffer2;
            break;
        default:
            // keep adding if not full ... allow for terminating null byte
            if (RxBuffer2Index < RxBuffer2Length - 1) {
                RxBuffer2[RxBuffer2Index++] = inByte;
            }// Need to capture buffer overfull 
            break;
    }//end switch
    IFS1bits.U2RXIF = 0;
}

void _ISR_PSV _T1Interrupt()
//void __attribute__((interrupt, no_auto_psv)) _T1Interrupt( void )
{
    tick_1ms++;
    if (tick_1ms == 1000) {
        tick_1ms = 0;
        Seconds++;
    }
    IFS0bits.T1IF = 0; // clear T1 interrupt flagRead
}

int main(int argc, char** argv) {
    //float showTemperature = 0;
    float voltage = 0;
    InitialiseHardware();
    InitialiseGlobals();
    FetchTemperature();
    ESP_Reset = 0; // Hold ESP8266 in reset
    Seconds = 0;
    while (Seconds < 1); // Delay 1 second before starting
    sprintf(TxBuffer1, "Low Cost Labs Garage Door Interface Starting\r\n");
    SendStringUART1((char *) TxBuffer1);
    Seconds = 0;
    while (Seconds < 1);
    ESP_Reset = 1; // Release ESP8266 from reset
//    sprintf(TxBuffer1, "ESP8266 Starting\r\n");
//    SendStringUART1((char *) TxBuffer1);
    //while (Seconds < 1); // wait while ESP spews it's bootup garbage
    while (1) {
        ClrWdt(); // Tickle the watchdg
        if (tick_1ms < 500) {
            Status1 = 0;
        } else {
            Status1 = 1;
            if (Seconds > 1) {
                Seconds = 0;
                minute++;
                Door1 = 0;
                Door2 = 0;
                Status0 = 0;
            }
        }
        if (minute == 60) {
            minute = 0;
            FetchTemperature();
            voltage = ReadADC();
            sprintf(TxBuffer1, "L - Status: Temp %i : %4.1f degC (%c) Supply %4.1f Volts\r\n", temperature, showTemperature, tempSetting, (voltage * ADCtoVolts));
            SendStringUART1((char *) TxBuffer1);
        }
        if (BufferReadyFlag == 1) {
            IEC1bits.U2RXIE = 0; // Disable Rx2 Interrupt
            memcpy(TxBuffer2, RxBuffer2, sizeof (RxBuffer2));
            IEC1bits.U2RXIE = 1; // Enable Rx2 Interrupt
            BufferReadyFlag = 0;
            ProcessString();
        }
    }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Function Code
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void InitialiseHardware(void) {
    // Set up hardware devices
    // Set Tristate and Analog Selection registers
    LATA = 0x00; // Initialise PORTA Output Latch
    TRISA = 0x21; // RA0 Input, all others outpus (except RA5, MCLR function) - 1 = input
    ANSA = 0x01; // RA0, Analogue input, all others Digital
    LATB = 0x00; // Initialie PORTB Output Latch
    TRISB = 0x0006; // RB1, RB2 inputs (Rx pins for UART2,UART1 respectivley), RB8 / RB9 for SCL / SDA (I2C Data)
    ANSB = 0x0000; // No analog inputs on PORTB
    // Set up Timer1
    T1CON = 0x8210; // TSIDL disabled; TGATE disabled; TCS FOSC/2; TSYNC disabled; TCKPS 1:8; TECS LPRC; TON enabled;
    // With 8MHz Oscillator, FOSC/2 as source and prescaler of 8, TMR1 ticks are every 2us
    TMR1 = 0x0000; // Initialise TMR1 Counter
    PR1 = PR1Match; // TMR1 Period Register.  Interrupt on match
    IFS0bits.T1IF = 0; // Clear TMR1 interrupt flag
    IEC0bits.T1IE = 1; // Enable TMR1 interrupts
    IPC0bits.T1IP = 3; // Timer 1 interrupt set to priority 3

    //    URXI: U2RX - UART2 Receiver
    //    Priority: 1
    IPC7bits.U2RXIP = 1;

    ESP_Reset = 0;
    Door1 = 0;
    Door2 = 0;
    Status0 = 0;
    Status1 = 0;

    InitialiseUART1();
    InitialiseUART2();
    //OpenUART2(0x8800,0x0000,BRGVAL);
    InitialiseMSSP(); //I2C Mode
    InitialiseADC();
    Status0 = 1;
    Status1 = 1;
}

void InitialiseGlobals(void) {
    // Timer Variables
    tick_1ms = 0; // 1 millisecond counter
    Seconds = 0;
    minute = 0;
    // Serial Data
    for (temp = 0; temp < TxBuffer1Length; temp++) {
        RxBuffer1[temp] = 0;
        TxBuffer1[temp] = 0;
    }
    RxData = 0;
    RxBuffer1Index = 0;
    TxBuffer1Index = 0;
    RxBuffer2Index = 0;
    TxBuffer2Index = 0;
}

void InitialiseUART1() {

    U1MODE = 0x8800; // RTSMD enabled; URXINV disabled; BRGH disabled; STSEL 1; UARTEN enabled; PDSEL 8N; LPBACK disabled; WAKE disabled; USIDL disabled; ABAUD disabled; IREN disabled; UEN TX_RX;
    U1STA = 0x0000; // UTXEN disabled; UTXINV disabled; URXISEL RX_ONE_CHAR; ADDEN disabled; UTXISEL0 TX_ONE_CHAR; UTXBRK COMPLETED; OERR NO_ERROR_cleared;
    U1TXREG = 0x0000; // Clear Tx Register
    U1RXREG = 0x0000; // Clear Rx Register
    U1BRG = BRGVAL; // See config.h for Baud Rate setup
    U1STAbits.UTXEN = 1;
}

void InitialiseUART2() {

    U2MODE = 0x8800; // RTSMD enabled; URXINV disabled; BRGH disabled; STSEL 1; UARTEN enabled; PDSEL 8N; LPBACK disabled; WAKE disabled; USIDL disabled; ABAUD disabled; IREN disabled; UEN TX_RX;
    U2STA = 0x0000; // UTXEN disabled; UTXINV disabled; URXISEL RX_ONE_CHAR; ADDEN disabled; UTXISEL0 TX_ONE_CHAR; UTXBRK COMPLETED; OERR NO_ERROR_cleared;
    U2TXREG = 0x0000; // Clear Tx Register
    U2RXREG = 0x0000; // Clear Rx Register
    U2BRG = BRGVAL; // See config.h for Baud Rate setup
    IEC1bits.U2RXIE = 1; // Enable Rx Interrupt
    U2STAbits.UTXEN = 1; // Enable Transmission (must do last)
}

void InitialiseMSSP() {
    SSP1STAT = 0x0080; // Slew Rate Control on, SMBus disabled
    SSP1CON1 = 0x0028; // Serial Port Enabled, I2C Master mode
    SSP1CON2 = 0x0000;
    SSP1CON3 = 0x0000;
    //SSP1ADD  = 0x0004;            // 400kHz (8MHz/400kHz/4)-1 = SSP1ADD
    SSP1ADD = 19; // 100kHz (8MHz/100kHz/4)-1 = SSP1ADD
}

void InitialiseADC() {
    AD1CON1 = 0x0070; // A/D of, On when idle, 10 bit operation, Right Justified unsiged output, internal counter, auto convert
    AD1CON2 = 0x8800; // 2*Vbg reference (2mV per bit of conversion), A/D results buffer is fixed, no scanning
    AD1CON3 = 0x1F04; // System clock, internal coutner ends smapling and starts conversion. SAMC = 11111 (31 Tad), 4xTcy = TAD
    // Tcy = 1/ 4Mhz = 250ns, Tad = 1us, x31 = 31us per sample
    AD1CON5 = 0x1000; // Autoscan disabled, full power mode, CTMU not enabled, Band Gap Enabled, No Auto-Scan, Legacy write mode.
    AD1CHS = 0x0000; // Channel B not used, MUX A uses Vss for negative reference, AN0 for positive reference.
}

void SendStringUART1(char *data) {
    while (*data) {
        SendByteUART1(*data++); // Send byte to UART Tx Routine, then increment pointer
    }
}

void SendByteUART1(char data) {
    U1TXREG = data; // Stick byte into UART buffer
    while (U1STAbits.TRMT == 0); // Wait for Byte to finsih being sent
}

char GetByteUART1() {
    while (!(U1STAbits.URXDA == 1)) {
    } // Wait for serial data.
    if ((U1STAbits.OERR == 1)) {
        U1STAbits.OERR = 0;
    }
    return U1RXREG;
}

void SendStringUART2(char *data) {
    while (*data) {
        SendByteUART2(*data++); // Send byte to UART Tx Routine, then increment pointer
    }
}

void SendByteUART2(char data) {
    U2TXREG = data; // Stick byte into UART buffer
    while (U2STAbits.TRMT == 0); // Wait for Byte to finsih being sent
}

char GetByteUART2() {
    if ((U2STAbits.OERR == 1)) {
        U2STAbits.OERR = 0;
    }
    return U2RXREG;
}

void ProcessIncomingByteUART2() {
    char inByte = 0;
    if (U2STAbits.URXDA == 1) { // check if data is in UART Rx Register
        inByte = GetByteUART2(); // read byte to buffer
        switch (inByte) {
            case '\n': // end of text
                RxBuffer2[RxBuffer2Index] = inByte;
                RxBuffer2[RxBuffer2Index + 1] = 0; // terminating null byte
                ProcessString();
                RxBuffer2Index = 0;
                break;
                //case '\r': // discard carriage return
                //    break;
            default:
                // keep adding if not full ... allow for terminating null byte
                if (RxBuffer2Index < RxBuffer2Length - 1) {
                    RxBuffer2[RxBuffer2Index++] = inByte;
                    if (RxBuffer2Index == RxBuffer2Length) {
                        ProcessString();
                        RxBuffer2Index = 0;
                    }
                }
                break;
        }//end switch
    }
}

void ProcessString() {
    // look for command characters at start of string frm ESP here
    char thisByte = TxBuffer2[0];
    //SendStringUART2((char *) TxBuffer2); // Transmit Rx'd string
    SendStringUART1((char *) TxBuffer2); // Echo ESP data to PC Rx'd string

    switch (thisByte) {
        case 'Q': // Query request from ESP8266
            sprintf(TxBuffer2, "T: %4.1f degrees C\r\n", showTemperature);
            SendStringUART2((char *) TxBuffer2); // respond to ESP
            break;
        case 'S': // Settings info from ESP8266
            tempSetting = TxBuffer2[2];
            break;
        case 'I':
            if (TxBuffer2[4] == 'A') {
                triggerDoor(0);
            } else if (TxBuffer2[8] == 'A') {
                triggerDoor(2);
            }
            break;
        default:
            break;
    }
}

void SendStartI2C() {
    SSP1CON2bits.SEN = 1; // Initiates Start condition on I2C bus
    while (SSP1CON2bits.SEN == 1); // Wait until Start is done.
    IFS1bits.SSP1IF = 0; // Clear the flag.
}

int SendDataI2C(char data) { // use pointer reference later?
    SSP1BUF = data; // Send data
    while (SSP1STATbits.BF == 1); // Wait until the buffer is cleared
    WaitIdleI2C();
    IFS1bits.SSP1IF = 0; // Clear the flag.
    //Done, now how did slave respond?
    if (SSP1CON2bits.ACKSTAT) { // 1=NACK
        return 1; // Slave did not acknowledge byte.
    } else {
        return 0; // Slave Acknowledges byte received.
    }
}

int ReadDataI2C(void) {
    SSP1CON2bits.RCEN = 1; // Start Data Read on I2C bus
    while (!SSP1STATbits.BF); // Wait until buffer is full
    IFS1bits.SSP1IF = 0; // Clear the flag
    WaitIdleI2C();
    return SSP1BUF; // Return I2C data buffer
}

void AckI2C(void) {
    SSP1CON2bits.ACKDT = 0; // Acknowledge data read, 0 = ACK
    SSP1CON2bits.ACKEN = 1; // Ack data enabled
    while (SSP1CON2bits.ACKEN); // Wait for ack data to be sent on bus
}

void NackI2C(void) {
    SSP1CON2bits.ACKDT = 1; // Acknowledge data read and cycle is to stop, 1 = NACK
    SSP1CON2bits.ACKEN = 1; // Ack data enabled
    while (SSP1CON2bits.ACKEN); // Wait for ack data to be sent on bus
}

void WaitIdleI2C(void) {
    while ((SSP1CON2 & 0x1F) || (SSP1STAT & 0x04)); // Check bus is idle
}

void SendStopI2C() {
    SSP1CON2bits.PEN = 1; // Initiates Stop condition on I2C bus
    while (SSP1CON2bits.PEN); // Wait until complete
    IFS1bits.SSP1IF = 0; //Clear the flag.
}

void FetchTemperature(void) {

    //int temperature = 0;

    SendStartI2C();
    SendDataI2C(TCN75A_Write); // Address TCN75A and put in write mode             (Slave to ACK)
    SendDataI2C(TCN75A_TAReg); // Set pointer to TA (temppeature reading) register (Slave to ACK)
    SendStartI2C();
    SendDataI2C(TCN75A_Read); // Address TCN75A in read mode
    temperature = ReadDataI2C();
    temperature = temperature << 8;
    AckI2C();
    temperature = temperature | ReadDataI2C(); // decimal points of temperature...
    NackI2C();
    SendStopI2C();
    if (tempSetting == 'F') {
        showTemperature = temperature / 256;
        showTemperature = showTemperature * 1.8;
        showTemperature = showTemperature + 32;
    } else {
        showTemperature = temperature / 256;
    }
    //return temperature;
}

int ReadADC(void) {
    IFS0bits.AD1IF = 0; // Clear A/D conversion interrupt
    AD1CON1bits.ADON = 1; // Turn ADC ON
    AD1CON1bits.SAMP = 1; // Start sampling, then after 31Tad go to conversion
    while (!AD1CON1bits.DONE); // Conversion done?
    return ADC1BUF0; // Yes then return ADC value
}

void triggerDoor(int door) {

    Seconds = 0;
    switch (door) {
        case 0:
            Status0 = 1;
            Door1 = 1;
            //sprintf(TxBuffer2, "Door 1 Active \r\n");
            //SendStringUART2((char *) TxBuffer2); // respond to ESP
            break;
        case 2:
            Status0 = 1;
            Door2 = 1;
            // sprintf(TxBuffer2, "Door 2 Active \r\n");
            //SendStringUART2((char *) TxBuffer2); // respond to ESP
            break;
        default:
            break;
    }
}