
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_dma.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXSNAKE 128
#define NULL 0
//------------------------------------
//Snake is a coordinate and a link to a previous or next snake segment
struct Snake { struct Snake *prev; int x; int y; struct Snake *next;} snake[MAXSNAKE];
struct Snake *front;	//Points out the front of the snake
struct Snake *back;		//Points out the back of the snake
//The directional vector always applying to the new front of the snake relative to the old
int x,y;
//The size of the snake for selecting new elements of the array to add to snake
int snakelength = 2;
//----------------------------------
//Initializes snake with two segments, a front and back and the rest as null
void initializeSnake(void){
	int i;
	for(i=0;i<MAXSNAKE;i++){
		snake[i] = (struct Snake){.prev = NULL, .next = NULL, .x = 0, .y = 0};
	}
	//Setting snake segment 0 as center of screen, snake one as just below snake zero
	snake[0].x = 240/2; snake[0].y = 160/2;
	snake[1].x = snake[0].x; snake[1].x = snake[0].y + 1;
	//Linking them together
	snake[0].prev = &snake[1];
	snake[1].next = &snake[0];
	front = &snake[0];
	back = &snake[1];
}

void moveSnake(void){
	struct Snake *newfront,*newback;
	//Take the back of the snake and move it to the front with a location
	//Created by adding the directional vector with the old front
	newback = back->next;	//The new back will be the second from last
	newfront = back;		//The new front will be created from the last
	newback->prev = NULL;	//The newback is still bound to the old, unbind
	newfront->next = NULL;	//The newfront is still bound to the new back, unbind
	front->next = newfront;	//The oldfront now has newfront next, bind
	newfront->prev = front;	//The newfront now has oldfront prev, bind
	front = newfront;		//Update with a current front and back
	back = newback;
	front->x = front->prev->x + x;	//The front is located relative to the old using vector
	front->y = front->prev->y + y;	
}
void snakeAdd(void){
	snakelength++;
	snake[snakelength-1].next = back;
	back->prev = &snake[snakelength-1];
	back = &snake[snakelength-1];
	//Don't worry about setting x and y, it'll be thrown in front anyway
}
	
void drawSnake(void){
	struct Snake *snakescan = front;	//This will run through the linked list
	short BLACK = 0;					//This is to be copied to screen as black
	CpuFastSet(&BLACK, (unsigned short *)VRAM,DMA_SRC_FIXED|19200);//Black out screen
	while(snakescan != NULL){			//As long as snakescan isn't null
		((unsigned short*)VRAM)[snakescan->x+snakescan->y*240] = 0xFFFF;	//Plot coordinate
		snakescan = snakescan->prev;	//Snakescan becomes the previous item in the list
	}
}
//---------------------------------------------------------------------
//Write to screen
//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
	initializeSnake();
	x = 0; y = -1;
	irqInit();
	irqEnable(IRQ_VBLANK);
	REG_DISPCNT = MODE_3 | BG2_ENABLE;
	int timer = 0;
	snakeAdd();snakeAdd();snakeAdd();snakeAdd();
	while(1){
		VBlankIntrWait();
				timer++;

		if ((timer % 10) == 1){
			drawSnake();
			moveSnake();
		}else if((timer % 100) == 50){ 
			int nx,ny;
			nx=-y;
			ny=x;
			x=nx;
			y=ny;
			snakeAdd();
		}
		if(front->x >= 240 || front->x <=0)
			x = 0; y = -y;
		if(front->y >= 160 || front->y <= 0)
			y = 0; x = -x;
		
	}
}
