#include "button.h"
#include "lpc17xx.h"
#include "../RIT/RIT.h"
#include "../game/game.h"
#include "../game/graphic.h"
#include "../timer/timer.h"


void EINT0_IRQHandler (void)	  	/* INT0: start the game						 */
																	// pin 10
{
	NVIC_DisableIRQ(EINT0_IRQn);
	set_board_player(0);
	set_state(CHOOSE_MULTIPLAYER);
	
	draw_multiplayer_select();
	enable_RIT();
	
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
																	// pin 11
{
	if (is_this_board_playing()) {
		enable_timer(1);
		NVIC_DisableIRQ(EINT1_IRQn);
		LPC_PINCON->PINSEL4    &= ~(1 << 22);
	}
	
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
																	// pin 12
{
	if (is_this_board_playing()) {
		enable_timer(2);
		NVIC_DisableIRQ(EINT2_IRQn);
		LPC_PINCON->PINSEL4    &= ~(1 << 24);
	}

  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


