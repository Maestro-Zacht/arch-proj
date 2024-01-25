/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "timer/timer.h"
#include "button_EXINT/button.h"
#include "joystick/joystick.h"
#include "RIT/RIT.h"
#include "CAN/CAN.h"
#include "game/game.h"
#include "game/graphic.h"

#define SIMULATOR 1

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif


int main(void)
{
  SystemInit();  												/* System Initialization (i.e., PLL)  */
	LCD_Initialization();
	BUTTON_init();
	joystick_init();											/* Joystick Initialization            */
	init_RIT(0x004C4B40);									/* RIT for joystick Initialization 50 msec       	*/
	
	// timers for button to 50 ms
	//init_timer(0, 0x1312D0);
	init_timer(1, 0x1312D0);
	init_timer(2, 0x1312D0);
	
	// 1s countdown timer
	init_timer(3, 0x17D7840);
		
	set_state(STARTING);
	
	CAN_Init();
	
	LCD_Clear(Blue);
	notify("Press INT0 to become P1", White, Blue);
	NVIC_EnableIRQ(EINT0_IRQn);

		
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);						
	
  while (1)	
  {
		__ASM("wfi");
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
