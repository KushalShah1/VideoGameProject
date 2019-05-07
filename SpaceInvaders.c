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
// 8*R resistor DAC bit 0 on PE0 (least significant bit)
// 4*R resistor DAC bit 1 on PE1
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
#include "PLL.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"


#define PE0 (*((volatile unsigned long *)0x40024004))
#define PE1 (*((volatile unsigned long *)0x40024008))

#define obstacleNumber 8

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
sprite objects[obstacleNumber];
uint16_t score;
uint32_t coinScore;
char coinString[5];
uint8_t gameStart;
uint8_t immunity;
uint32_t powerUpTimer=0;
//Detects Collisions between two sprites
uint8_t collisionDetect(sprite one,sprite two){

	  int left1, left2;
    int right1, right2;
    int top1, top2;
    int bottom1, bottom2;
		//The given coordinates of the 2 sprites
    left1 = one.x;
    left2 = two.x;
    right1 = one.x + one.w;
    right2 = two.x + two.w;
    bottom1 = one.y;
    bottom2 = two.y;
    top1 = one.y -one.h;
    top2 = two.y-two.h;

    if ((bottom1 < top2)||(top1 > bottom2)||(right1 < left2)||(left1 > right2))return 0;
		return 1;

}
//Generates lasers,bombs, and coins for the initial start of the game
//laser and bombs are 25% of the time each, and coins are 50%
void generateObstacles(uint8_t size){
	int m = 0;
	int offset=0;
	for(int i=0; i<size;i++){
		m=rand()%4; // a number from 0 to 3
		switch(m){
			//generates a laser
			case 0:
				objects[i].image=laser;
				objects[i].x=160+offset;
				objects[i].y=58+(rand()%70);
				objects[i].w=10;
				objects[i].h=50;
				objects[i].state=alive;
				break;
			//generates a bomb
			case 1:
				objects[i].image=bomb;
				objects[i].x=160+offset;
				objects[i].y=20+(rand()%108);
				objects[i].w=11;
				objects[i].h=12;
				objects[i].state=alive;
				break;
			//generates a coin
			case 2:
				objects[i].image=coin;
				objects[i].x=160+offset;
				objects[i].y=18+(rand()%110);
				objects[i].w=7;
				objects[i].h=10;
				objects[i].state=alive;
			//generates a coin
			default:
				objects[i].image=coin;
				objects[i].x=160+offset;
				objects[i].y=18+(rand()%110);
				objects[i].w=7;
				objects[i].h=10;
				objects[i].state=alive;
				break;
		}
		offset+=40;//ensures that the obstacles dont overlap
	}
}
//Regenerates obstacles that have been deleted and puts them after all previous obstacles
void regenerateObstacle(uint8_t i){
	int m = 0;
	m=rand()%3; // a number from 0 to 2
	int largest=0;
	for(int k=0;k<obstacleNumber;k++){
		if(objects[k].x>largest)largest=objects[k].x;//find the largest x value of the obstacles so that the new obstacle can be behind it
	}
		switch(m){
			//generates laser
			case 0:
				objects[i].image=laser;
				objects[i].x=largest+40;
				objects[i].y=58+(rand()%70);
				objects[i].w=10;
				objects[i].h=50;
				objects[i].state=alive;
				break;
			//generates bomb
			case 1:
				objects[i].image=bomb;
				objects[i].x=largest+40;
				objects[i].y=20+(rand()%108);
				objects[i].w=11;
				objects[i].h=12;
				objects[i].state=alive;
				break;
			//generates coin
			default:
				objects[i].image=coin;
				objects[i].x=largest+40;
				objects[i].y=18+(rand()%110);
				objects[i].w=7;
				objects[i].h=10;
				objects[i].state=alive;
				break;
		}
	
}
//Initializes the systick to go at 60hz
void SysTick_Init(){
	NVIC_ST_CTRL_R = 0x0000;
	NVIC_ST_RELOAD_R = 5000000;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_ST_CTRL_R = 0x0007; // enable SysTick with core clock and interrupts
}
//Initalizes the buttons on port e
void ButtonInit(){
		SYSCTL_RCGCGPIO_R|=0X10;	//turn on port e
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
		GPIO_PORTE_DIR_R=0;		//b0-b1 is on
		GPIO_PORTE_DEN_R=3;//3
}
//Waits to the user to press a button to start the game
void waitforStart(){
	ST7735_FillScreen(0);
	ST7735_DrawBitmap(0,127,start,160,128);
	while((GPIO_PORTE_DATA_R&3)==0){};
	ST7735_FillScreen(0);
}
//Initalizes all Subroutines, Hardware, Interrupts, and Variables. Also sets up the inital game state
void InitAll(){
	NVIC_ST_RELOAD_R=0xFFFFFFFF;
	NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC;
	PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
	ST7735_InitR(INITR_REDTAB);
	ST7735_SetRotation(1);		//Screen is Landscape
	ButtonInit();							//Button init
	ADC_Init();								//ADC init for the slidepod
	Sound_Init();							//Turns on sound
	setSoundState(background);//Sound to background music
	waitforStart();						//wait for the start of game
	srand(NVIC_ST_CURRENT_R);//seeds the random number with time
	generateObstacles(obstacleNumber);//generates the obstacles
	score=0;
	coinScore=0;
	gameStart=1;					//turns on the game
	SysTick_Init();				//turns on game updating
}
//Gets the difference in position of the old and new location of the man and draws a black box in the area so it gives the illusion of movement
void getSliceMan(uint16_t x,uint16_t y, uint16_t newX, uint16_t newY,uint8_t w, uint8_t h){
	
	if(newX>x)
	ST7735_FillRect(x,y-h,newX-x,h,0);
	else 	ST7735_FillRect(newX+w,y-h+1,x-newX,h,0);

	if(newY>y)
	ST7735_FillRect(x,y-h,w,newY-y+1,0);
	else 	ST7735_FillRect(x,newY,w,y-newY+1,0);
	
	//If the man is immune it will draw a different sprite
	if(immunity==0) ST7735_DrawBitmap(newX,newY,john, w, h);
	else ST7735_DrawBitmap(newX,newY,manimmune, w, h);
	lastx=newX;
	lasty=newY;
}
//Resets the game
void resetgame(){
	generateObstacles(obstacleNumber);//generates new obstacles
	man.x=0;													//the next few lines resets the man and variables
	man.y=127;
	man.state=alive;
	score=0;
	coinScore=0;
	for(int i=0;i<10000000;i++){};		//waits for a little bit so that the game does not restart immediately
	while((GPIO_PORTE_DATA_R&3)==0){};
	gameStart=1;
	srand(NVIC_ST_CTRL_R);						//seeds the random number
	ST7735_FillScreen(0);
	SysTick_Init();										//turns on systick
}

//Outputs a graphical version of the score
void OutputScore(){
	int8_t offset=0;
	int16_t mod=1000;
	int8_t out=0;
	ST7735_DrawBitmap(0,127,end,160,128);
	
	//outputs the score number by drawing one digit at a time
	while(mod){
		out=score/mod;
		switch(out){
			case 0:
				ST7735_DrawBitmap(30+offset,70,slide0,20,40);
				break;
			case 1:
				ST7735_DrawBitmap(30+offset,70,slide1,20,40);
				break;
			case 2:
				ST7735_DrawBitmap(30+offset,70,slide2,20,40);
				break;
			case 3:
				ST7735_DrawBitmap(30+offset,70,slide3,20,40);
				break;
			case 4:
				ST7735_DrawBitmap(30+offset,70,slide4,20,40);
				break;
			case 5:
				ST7735_DrawBitmap(30+offset,70,slide5,20,40);
				break;
			case 6:
				ST7735_DrawBitmap(30+offset,70,slide6,20,40);
				break;
			case 7:
				ST7735_DrawBitmap(30+offset,70,slide7,20,40);
				break;
			case 8:
				ST7735_DrawBitmap(30+offset,70,slide8,20,40);
				break;
			case 9:
				ST7735_DrawBitmap(30+offset,70,slide9,20,40);
				break;
		}
		score%=mod;
		mod/=10;
		offset+=20;
	}
	ST7735_DrawBitmap(30+offset,70,meter,20,40);


}
//Updates the coin score in the game
void currentScoreDisplay(){
	ST7735_SetCursor(0,0);
	ST7735_OutString("Coins: ");
	sprintf(coinString,"%d",coinScore);//Converts the coinScore number into a char array/string
	ST7735_OutString(coinString);
	ST7735_OutString("  ");
	//ST7735_OutChar((coinScore/10)+0x30);
	//ST7735_OutChar((coinScore%10)+0x30);

}

int main(void){
	InitAll();//Initalizes everything
	while(1){
		while(gameStart){
			currentScoreDisplay();//displays score
			getSliceMan(lastx,lasty, man.x,man.y, man.w, man.h);//covers the last location of the man

			//If the man is dead turn of the game and display score and reset
			if(man.state==dead){
				setSoundState(death);
				NVIC_ST_CTRL_R=0x0;
				OutputScore();
				gameStart=0;
				resetgame();
				break;
			}
			//if the man hit a coin then increment the coin score
			if(man.state==coinCollision){
				coinScore++;
				man.state=alive;
				setSoundState(coinSound);
			}
			//for loop to draw the obstacles and to regenerate new obstacles
			for(int i=0;i<obstacleNumber;i++){
				if(objects[i].state==alive)
					ST7735_DrawBitmap(objects[i].x,objects[i].y,objects[i].image,objects[i].w,objects[i].h);
				else if (objects[i].state==dead){
					ST7735_FillRect(objects[i].x-170,objects[i].y-objects[i].h,objects[i].w,objects[i].h+1,0);
					regenerateObstacle(i);
				}
			}

		}
	}
}






int8_t parabola=1;
int8_t lastButton;
void SysTick_Handler(void){
	man.x =ADC_In()/26;//sets the x position
	if(lastButton!=(PE0&1))parabola=0;
	//if the button is pressed decrease y, at a rate of y=x
	if(PE0==1){
		lastButton=1;
		if(parabola==15)man.y-=parabola;
		else{
			man.y-=parabola;
			parabola++;
		}
		if(man.y<=29)man.y=29;
	}
	else{//else increase y at a rate of y=x
		lastButton=0;
		if(parabola==15)man.y+=parabola;
		else{
			man.y+=parabola;
			parabola++;
		}
		if(man.y>=127) man.y=127;
	}

	//check if the player is pressing the powerup button and if it is valid to powerup
	if((PE1==2)&&(coinScore>=5)){
		immunity=1;
		powerUpTimer=100;
		coinScore-=5;
		setSoundState(powerup);
	}
	if(powerUpTimer!=0){		//powerup only lasts for 100 systick cycles
		powerUpTimer--;
	}
	else{
		immunity=0;
	}
	
	
	//Updates all the obstacles, by incrementing their x position and checking for collisions
	for(int i=0;i<obstacleNumber;i++){
			objects[i].x--;
			if(objects[i].x==0) {
				objects[i].x+=170;
				objects[i].state=dead;
			}
			
			if(collisionDetect(man,objects[i])==1){//check for the man hitting something
				if((immunity==1)&&(objects[i].image!=coin)){//if immune and hits an obstacle, only the obstacle dies
					objects[i].state=dead;
					objects[i].x+=170;
				}
			 else if(objects[i].image==coin){//if the object is a coin set the mans state to coin
					objects[i].state=dead;
					objects[i].x+=170;
					man.state=coinCollision;
				}
				else {
					man.state=dead;			//if the man hit an obstacle he dies
					break;
				}
			}			
		}
		score++;			//increase the score
	
}


