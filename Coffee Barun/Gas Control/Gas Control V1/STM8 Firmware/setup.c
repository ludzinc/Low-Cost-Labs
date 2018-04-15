//
//  Setup the system clock to run at 16MHz using the internal oscillator.
//
void InitialiseSystemClock() {
    CLK_ICKR = 0;                       //  Reset the Internal Clock Register.
    CLK_ICKR_HSIEN = 1;                 //  Enable the HSI.
    CLK_ECKR = 0;                       //  Disable the external clock.
    while (CLK_ICKR_HSIRDY == 0);       //  Wait for the HSI to be ready for use.
    CLK_CKDIVR = 0;                     //  Ensure the clocks are running at full speed.
    CLK_PCKENR1 = 0xff;                 //  Enable all peripheral clocks.
    CLK_PCKENR2 = 0xff;                 //  Ditto.
    CLK_CCOR = 0;                       //  Turn off CCO.
    CLK_HSITRIMR = 0;                   //  Turn off any HSIU trimming.
    CLK_SWIMCCR = 0;                    //  Set SWIM to run at clock / 2.
    CLK_SWR = 0xe1;                     //  Use HSI as the clock source.
    CLK_SWCR = 0;                       //  Reset the clock switch control register.
    CLK_SWCR_SWEN = 1;                  //  Enable switching.
    while (CLK_SWCR_SWBSY != 0);        //  Pause while the clock switch is busy.
}
/*
//
//  Setup Timer 2 to generate a 1 Hz interrupt based upon a 16 MHz timer.
//  Prescaler = 1024, Counter = 15625
//
void SetupTimer2() {
    TIM2_PSCR = 0x0A;       //  Prescaler = 2^10 = 1024.
    TIM2_ARRH = 0x3D;       //  High byte of 15625
    TIM2_ARRL = 0x099;      //  Low byte of 15625
    TIM2_IER_UIE = 1;       //  Enable the update interrupts.
    TIM2_CR1_CEN = 1;       //  Finally enable the timer.
}
*/
//
//  Setup Timer 2 to generate a 2kHz interrupt (1ms ticks) based upon a 16 MHz timer.
//  Prescaler = 2 (2^1), Counter = 4,000 (0x0FA0)
//
void SetupTimer2() {
    TIM2_PSCR = 0x03;       //  Prescaler = 2^3 = 8.
    TIM2_ARRH = 0x0F;       //  High byte of Counter
    TIM2_ARRL = 0xA0;       //  Low byte of Counter
    TIM2_IER_UIE = 1;       //  Enable the update interrupts.
    TIM2_CR1_CEN = 1;       //  Finally enable the timer.
} 

void SetupADC() {
   
    ADC_CSR_CH = 5;         //  Select ADC Channel
    ADC_CR3_DBUF = 0;       //  Don't  buffer data
    ADC_CR2_ALIGN = 0;      //  Data is left aligned.
    ADC_CR1 = 0x70;         //  ADC = Fmaster / 18
    ADC_CR1_ADON = 1;       //  Turn ADC on, note a second set is required to start the conversion.
}

void readADC() {
    ADC_CR1_ADON = 1;           // Second write starts the conversion.
    while(ADC_CSR_EOC == 0);    // Wait for ADC to perform conversion
    ADCRead = ADC_DRH;          // Read upper 8 bits of the ADC conversion   
    ADC_CSR_EOC = 0;            // Clear EOC flag
}

