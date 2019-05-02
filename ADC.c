// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 11/14/2018
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
void ADC_Init(void){ 
	
	SYSCTL_RCGCGPIO_R |= 8;   // 1) activate clock for Port d
	volatile int i=0;
	i++;
	i++;
	i++;
	i++;
	i++;
  GPIO_PORTD_DIR_R = 0x00;      // 2) make Pd2 input
  GPIO_PORTD_AFSEL_R = 0x04;     // 3) enable alternate function on Pd2
  GPIO_PORTD_DEN_R = 0x00;      // 4) disable digital I/O on PD2
  GPIO_PORTD_AMSEL_R = 0x04;     // 5) enable analog function on PD2
  SYSCTL_RCGCADC_R |= 0x0001;   // 6) activate ADC0
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
  ADC0_PC_R = 0;  // 7) configure for 125K
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
  ADC0_SSPRI_R = 0x0123;          // 8) Sequencer 3 is highest priority
  ADC0_ACTSS_R &= ~0x0008;        // 9) disable sample sequencer 3
  ADC0_EMUX_R &= ~0xF000;         // 10) seq3 is software trigger
  ADC0_SSMUX3_R &= ~0x000F;       // 11) clear SS3 field
  ADC0_SSMUX3_R |= 5;             //    set channel Ain5 (PD2)
  ADC0_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
  ADC0_ACTSS_R |= 0x0008;         // 13) enable sample sequencer 3
	ADC0_SAC_R = 0x06;

}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
	uint32_t data=0;  
	ADC0_PSSI_R = 0x0008;              //turn on sampling
	while((ADC0_RIS_R&0x08)==0){};     //while sampling
	data = ADC0_SSFIFO3_R&0xFFF;   			//set data to adc value
	ADC0_ISC_R = 0x0008;   						//clear flag
	return data;
}


