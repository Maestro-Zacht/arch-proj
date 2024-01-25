/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <string.h>
#include "lpc17xx.h"
#include "timer.h"
#include "../game/game.h" 

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/


void TIMER0_IRQHandler (void)
{	
  LPC_TIM0->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
	static int down = 0;
	down++;
	if ((LPC_GPIO2->FIOPIN & (1<<11)) == 0) {
		switch(down) {
			case 1:
				change_mode();
				break;
			default:
				break;
		}
	}
	else {
		down = 0;
		disable_timer(1);
		reset_timer(1);
		NVIC_EnableIRQ(EINT1_IRQn);
		LPC_PINCON->PINSEL4 |= (1 << 22);
	}
	
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/

void TIMER2_IRQHandler (void)
{
	static int down = 0;
	down++;
	if ((LPC_GPIO2->FIOPIN & (1<<12)) == 0) {
		switch(down) {
			case 1:
				rotate_wall();
				break;
			default:
				break;
		}
	}
	else {
		down = 0;
		disable_timer(2);
		reset_timer(2);
		NVIC_EnableIRQ(EINT2_IRQn);
		LPC_PINCON->PINSEL4 |= (1 << 24);
	}
	
  LPC_TIM2->IR = 1;			/* clear interrupt flag */
  return;
}

void TIMER3_IRQHandler (void)
{
	tick_tack();
  LPC_TIM3->IR = 1;			/* clear interrupt flag */
  return;
}
