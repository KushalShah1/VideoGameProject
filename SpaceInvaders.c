// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 11/20/2018 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2018

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2018

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"


#define PB0 (*((volatile unsigned long *)0x40005004))


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
typedef struct sprite{
	const uint16_t *image;
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
	enum {alive,dead,coinCollision} state;
}sprite;
uint16_t lastx=0;
uint16_t lasty=127;
sprite man={john,0,127,20,21,alive};
sprite objects[4];
uint16_t score;
uint16_t coinScore;
uint8_t gameStart;

uint8_t collisionDetect(sprite one,sprite two){

	if((two.x>=one.x)&&(two.x<=(one.x+one.w))){
		if((two.y<=one.y)&&(two.y>=(one.y-one.h)))return 1;
	}
	return 0;

}

void generateObstacles(uint8_t size){
	int m = 0;
	for(int i=0; i<size;i++){
		m=rand()%3; // a number from 0 to 2
		switch(m){
			case 0:
				objects[i].image=laser;
				objects[i].x=160+(rand()%50);
				objects[i].y=50+(rand()%70);
				objects[i].w=10;
				objects[i].h=50;
				objects[i].state=alive;
				break;
			case 1:
				objects[i].image=bomb;
				objects[i].x=160+(rand()%50);
				objects[i].y=20+(rand()%120);
				objects[i].w=11;
				objects[i].h=12;
				objects[i].state=alive;
				break;
			default:
				objects[i].image=coin;
				objects[i].x=160+(rand()%50);
				objects[i].y=20+(rand()%127);
				objects[i].w=7;
				objects[i].h=10;
				objects[i].state=alive;
				break;
			
		}
	}
	
	for(int i=0;i<4;i++){
		for(int j=i+1;j<4;j++){
			if(collisionDetect(objects[i],objects[j])==1)objects[j].x+=50;
		}
	}
	
}
void SysTick_Init(){
	NVIC_ST_CTRL_R = 0x0000;
	NVIC_ST_RELOAD_R = 5000000;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_ST_CTRL_R = 0x0007; // enable SysTick with core clock and interrupts
}
void ButtonInit(){
		SYSCTL_RCGCGPIO_R|=0X2;	//turn on port b
		volatile int d=0;
		d++;
		d++;
		d++;
		d++;
		d++;
		d++;
		d++;
		d++;
		d++;
		GPIO_PORTB_DIR_R=0;		//b0-b1 is on
		GPIO_PORTB_DEN_R=1;//3
}
void waitforStart(){
	ST7735_FillScreen(0);
	ST7735_OutString("Press any button to start");
	while((GPIO_PORTB_DATA_R&3)==0){};
	ST7735_FillScreen(0);
}
void InitAll(){
	PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
	ST7735_InitR(INITR_REDTAB);
	ST7735_SetRotation(1);
	ButtonInit();
	ADC_Init();
	waitforStart();
	srand(NVIC_ST_CTRL_R);//need to use user input change to more random
	SysTick_Init();
	generateObstacles(4);
	score=0;
	coinScore=0;
	gameStart=1;
	//Sound_Init();
}
void getSliceMan(uint16_t x,uint16_t y, uint16_t newX, uint16_t newY,uint8_t w, uint8_t h){
	
	if(newX>x)
	ST7735_FillRect(x,y-h,newX-x,h,0);
	else 	ST7735_FillRect(newX+w,y-h+1,x-newX,h,0);

	if(newY>y)
	ST7735_FillRect(x,y-h,w,newY-y+1,0);
	else 	ST7735_FillRect(x,newY,w,y-newY+1,0);
	
	ST7735_DrawBitmap(newX,newY,john, w, h);
	lastx=newX;
	lasty=newY;
}


void gameEnd(){
	ST7735_FillScreen(0);
	LCD_OutDec(score);
	gameStart=0;
	ST7735_OutString("Press any button to restart");
}

int main(void){
	InitAll();
	while(1){
		if(GPIO_PORTB_DATA_R&3)gameStart=1;
		while(gameStart){
			
			getSliceMan(lastx,lasty, man.x,man.y, man.w, man.h);
			if(man.state==dead){
				NVIC_ST_CTRL_R=0x0;
				gameEnd();
				break;
			}
			if(man.state==coinCollision){
				coinScore++;
			}
			for(int i=0;i<4;i++){
				if(objects[i].state==alive)
					ST7735_DrawBitmap(objects[i].x,objects[i].y,objects[i].image,objects[i].w,objects[i].h);
				else if (objects[i].state==dead){
					ST7735_FillRect(0,objects[i].y-objects[i].h,objects[i].w,objects[i].h+1,0);
					objects[i].state=alive;
					objects[i].y=rand()%128;
				}
			}

		}
	}
}






int8_t parabola=1;
int8_t lastButton;
void SysTick_Handler(void){
	man.x =ADC_In()/26;
	if(lastButton!=(PB0&1))parabola=0;
	if((PB0==1)&&(man.y>=27)){
		lastButton=1;
		if(parabola==15)man.y-=parabola;
		else{
			man.y-=parabola;
			parabola++;
		}
	}
	else if(man.y<=120){
		lastButton=0;
		if(parabola==15)man.y+=parabola;
		else{
			man.y+=parabola;
			parabola++;
		}
	}
	for(int i=0;i<4;i++){
			objects[i].x--;
			if(objects[i].x==0) {
				objects[i].x=170;
				objects[i].state=dead;
			}
			
			if(collisionDetect(man,objects[i])==1){
				if(objects[i].image==coin){
					objects[i].state=dead;
					man.state=coinCollision;
				}
				else {
					man.state=dead;
					break;
				}
			}			
		}
		score++;
}


