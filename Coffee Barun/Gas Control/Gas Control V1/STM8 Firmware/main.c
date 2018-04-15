#include <iostm8s003f3.h>
#include <intrinsics.h>
#include "led.c"
#include "setup.c"

//
//  Timer 2 Overflow handler.
//
#pragma vector = TIM2_OVR_UIF_vector
__interrupt void TIM2_UPD_OVF_IRQHandler(void)
{
    tickCounter++;   
    if(tickCounter > 10){
      ms5Counter++;
      tickCounter = 0;
    }
    TIM2_SR1_UIF = 0;               //  Reset the interrupt otherwise it will fire again straight away.
}

//main entry point
void main( void )
{
  setupHardware(); 
  SetupADC();
  __disable_interrupt();
  InitialiseSystemClock();
  SetupTimer2();
  __enable_interrupt();
  
  while (1){
    __wait_for_interrupt();
  // read and scale ADC
    readADC();
    //populateDisplay(seconds);
    populateDisplay(ADCRead);
    if(ms5Counter> 50){
      ms5Counter = 0;
      seconds++;
    }
  }
}