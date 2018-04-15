// HEADER FILE
#if !defined ICD2_DEBUG
 #pragma chip PIC16F627A, core 14, code 1024, ram 32 : 0x14F

 #define INT_lgen_style
 #define INT_rambank  0

#else /* ICD2_DEBUG */
 #pragma chip PIC16F627A, core 14, code 1024, ram 32 : 0x14F

 //NOTE : PIC16F648A-ICD is actual device for ICD 2 debugging

 //RESERVED RAM LOCATIONS
 char ICD2R1 @ 0x70;

 #pragma stackLevels 7   // reserve one level for debugging

 #define INT_gen_style
 #define INT_rambank  0
#endif

#pragma ramdef  0x70 : 0x7F mapped_into_all_banks

#pragma config_def 0x1118

/* Predefined:
  char W;
  char INDF, TMR0, PCL, STATUS, FSR, PORTA, PORTB;
  char OPTION, TRISA, TRISB;
  char PCLATH, INTCON;
  bit PS0, PS1, PS2, PSA, T0SE, T0CS, INTEDG, RBPU_;
  bit Carry, DC, Zero_, PD, TO, RP0, RP1, IRP;
  bit RBIF, INTF, T0IF, RBIE, INTE, T0IE, GIE;
  bit PA0, PA1;  // PCLATH
*/

#pragma char PIR1    @ 12

#pragma char TMR1L   @ 14
#pragma char TMR1H   @ 15
#pragma char T1CON   @ 16
#pragma char TMR2    @ 17
#pragma char T2CON   @ 18

#pragma char CCPR1L  @ 21
#pragma char CCPR1H  @ 22
#pragma char CCP1CON @ 23
#pragma char RCSTA   @ 24
#pragma char TXREG   @ 25
#pragma char RCREG   @ 26

#pragma char CMCON   @ 31

#pragma char PIE1    @ 0x8C

#pragma char PCON    @ 0x8E

#pragma char PR2     @ 0x92

#pragma char TXSTA   @ 0x98
#pragma char SPBRG   @ 0x99
#pragma char EEDATA  @ 0x9A
#pragma char EEADR   @ 0x9B
#pragma char EECON1  @ 0x9C
#pragma char EECON2  @ 0x9D

#pragma char VRCON   @ 0x9F

#pragma bit  RA0     @ 5.0
#pragma bit  RA1     @ 5.1
#pragma bit  RA2     @ 5.2
#pragma bit  RA3     @ 5.3
#pragma bit  RA4     @ 5.4
#pragma bit  RA5     @ 5.5
#pragma bit  RA6     @ 5.6
#pragma bit  RA7     @ 5.7

#pragma bit  RB0     @ 6.0
#pragma bit  RB1     @ 6.1
#pragma bit  RB2     @ 6.2
#pragma bit  RB3     @ 6.3
#pragma bit  RB4     @ 6.4
#pragma bit  RB5     @ 6.5
#pragma bit  RB6     @ 6.6
#pragma bit  RB7     @ 6.7

#pragma bit  PEIE    @ 11.6  mapped_into_all_banks

#pragma bit  TMR1IF  @ 12.0
#pragma bit  TMR2IF  @ 12.1
#pragma bit  CCP1IF  @ 12.2
#pragma bit  TXIF    @ 12.4
#pragma bit  RCIF    @ 12.5
#pragma bit  CMIF    @ 12.6
#pragma bit  EEIF    @ 12.7

#pragma bit  TMR1ON  @ 16.0
#pragma bit  TMR1CS  @ 16.1
#pragma bit  T1SYNC_ @ 16.2
#pragma bit  T1OSCEN @ 16.3
#pragma bit  T1CKPS0 @ 16.4
#pragma bit  T1CKPS1 @ 16.5

#pragma bit  T2CKPS0 @ 18.0
#pragma bit  T2CKPS1 @ 18.1
#pragma bit  TMR2ON  @ 18.2
#pragma bit  TOUTPS0 @ 18.3
#pragma bit  TOUTPS1 @ 18.4
#pragma bit  TOUTPS2 @ 18.5
#pragma bit  TOUTPS3 @ 18.6

#pragma bit  CCP1M0  @ 23.0
#pragma bit  CCP1M1  @ 23.1
#pragma bit  CCP1M2  @ 23.2
#pragma bit  CCP1M3  @ 23.3
#pragma bit  CCP1Y   @ 23.4
#pragma bit  CCP1X   @ 23.5

#pragma bit  RX9D    @ 24.0
#pragma bit  OERR    @ 24.1
#pragma bit  FERR    @ 24.2
#pragma bit  ADDEN   @ 24.3
#pragma bit  ADEN    @ 24.3
#pragma bit  CREN    @ 24.4
#pragma bit  SREN    @ 24.5
#pragma bit  RX9     @ 24.6
#pragma bit  SPEN    @ 24.7

#pragma bit  CM0     @ 31.0
#pragma bit  CM1     @ 31.1
#pragma bit  CM2     @ 31.2
#pragma bit  CIS     @ 31.3
#pragma bit  C1INV   @ 31.4
#pragma bit  C2INV   @ 31.5
#pragma bit  C1OUT   @ 31.6
#pragma bit  C2OUT   @ 31.7

#pragma bit  TRISA0  @ 0x85.0
#pragma bit  TRISA1  @ 0x85.1
#pragma bit  TRISA2  @ 0x85.2
#pragma bit  TRISA3  @ 0x85.3
#pragma bit  TRISA4  @ 0x85.4
#pragma bit  TRISA5  @ 0x85.5
#pragma bit  TRISA6  @ 0x85.6
#pragma bit  TRISA7  @ 0x85.7

#pragma bit  TRISB0  @ 0x86.0
#pragma bit  TRISB1  @ 0x86.1
#pragma bit  TRISB2  @ 0x86.2
#pragma bit  TRISB3  @ 0x86.3
#pragma bit  TRISB4  @ 0x86.4
#pragma bit  TRISB5  @ 0x86.5
#pragma bit  TRISB6  @ 0x86.6
#pragma bit  TRISB7  @ 0x86.7

#pragma bit  TMR1IE  @ 0x8C.0
#pragma bit  TMR2IE  @ 0x8C.1
#pragma bit  CCP1IE  @ 0x8C.2
#pragma bit  TXIE    @ 0x8C.4
#pragma bit  RCIE    @ 0x8C.5
#pragma bit  CMIE    @ 0x8C.6
#pragma bit  EEIE    @ 0x8C.7

#pragma bit  BOR_    @ 0x8E.0
#pragma bit  POR_    @ 0x8E.1
#pragma bit  OSCF    @ 0x8E.3

#pragma bit  TX9D    @ 0x98.0
#pragma bit  TRMT    @ 0x98.1
#pragma bit  BRGH    @ 0x98.2
#pragma bit  SYNC    @ 0x98.4
#pragma bit  TXEN    @ 0x98.5
#pragma bit  TX9     @ 0x98.6
#pragma bit  CSRC    @ 0x98.7

#pragma bit  RD      @ 0x9C.0
#pragma bit  WR      @ 0x9C.1
#pragma bit  WREN    @ 0x9C.2
#pragma bit  WRERR   @ 0x9C.3

#pragma bit  VR0     @ 0x9F.0
#pragma bit  VR1     @ 0x9F.1
#pragma bit  VR2     @ 0x9F.2
#pragma bit  VR3     @ 0x9F.3
#pragma bit  VRR     @ 0x9F.5
#pragma bit  VROE    @ 0x9F.6
#pragma bit  VREN    @ 0x9F.7
