/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../joystick/joystick.h"

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/


void RIT_IRQHandler (void)
{					
	static int up=0, down=0, right=0, left=0, select=0;
	
	if((LPC_GPIO1->FIOPIN & (1<<29)) == 0) {	
		/* Joytick UP pressed */
		up++;
		switch(up){
			case 1:
				joystick_up();
				break;
			default:
				break;
		}
	}
	else
			up=0;
	
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0) {	
		/* Joytick RIGHT pressed */
		right++;
		switch(right){
			case 1:
				joystick_right();
				break;
			default:
				break;
		}
	}
	else
			right=0;
	
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0) {	
		/* Joytick LEFT pressed */
		left++;
		switch(left){
			case 1:
				joystick_left();
				break;
			default:
				break;
		}
	}
	else
			left=0;
	
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0) {	
		/* Joytick DOWN pressed */
		down++;
		switch(down){
			case 1:
				joystick_down();
				break;
			default:
				break;
		}
	}
	else
			down=0;
	
	if((LPC_GPIO1->FIOPIN & (1<<25)) == 0) {	
		/* Joytick SELECT pressed */
		select++;
		switch(select){
			case 1:
				joystick_select();
				break;
			default:
				break;
		}
	}
	else
			select=0;
	
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
