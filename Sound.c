// Sound.c
// This module contains the SysTick ISR that plays sound
// Runs on LM4F120 or TM4C123
// Program written by: put your names here
// Date Created: 3/6/17 
// Last Modified: 3/5/18 
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "dac.h"
//#include "song.h"
#include "../inc/tm4c123gh6pm.h"

uint8_t Index;

// 6-bit 64-element sine wave
const unsigned short wave[64] = {
  32,35,38,41,44,47,49,52,54,56,58,
  59,61,62,62,63,63,63,62,62,61,59,
  58,56,54,52,49,47,44,41,38,35,
  32,29,26,23,20,17,15,12,10,8,
  6,5,3,2,2,1,1,1,2,2,3,
  5,6,8,10,12,15,17,20,23,26,29};

// **************Sound_Init*********************
// Initialize digital outputs and SysTick timer
// Called once, with sound/interrupts initially off
// Input: none
// Output: none
void Sound_Init(void){
  DAC_Init();
	Index=0;
	NVIC_ST_CTRL_R = 0;		//disable SysTick during setup
}


// **************Sound_Play*********************
// Start sound output, and set Systick interrupt period 
// Sound continues until Sound_Play called again
// This function returns right away and sound is produced using a periodic interrupt
// Input: interrupt period
//           Units of period is 12.5ns
//           Maximum period is 2^24-1
//           Minimum period is length of the ISR
//         if period equals zero, disable sound output
// Output: none
void Sound_Play(uint32_t period){
	if(period){
	NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_ST_CTRL_R = 0x0007; // enable SysTick with core clock and interrupts
	}
	else{
		NVIC_ST_CTRL_R = 0x0000;
	}
}

/*void SysTick_Handler(void) {
	Index = (Index+1)&0x03F;      // 4,5,6,7,7,7,6,5,4,3,2,1,1,1,2,3,... 
  DAC_Out(wave[Index]);    // output one value each interrupt
}*/


