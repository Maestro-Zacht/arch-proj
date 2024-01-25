/*----------------------------------------------------------------------------
 * Name:    Can.c
 * Purpose: CAN interface for for LPC17xx with MCB1700
 * Note(s): see also http://www.port.de/engl/canprod/sv_req_form.html
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <lpc17xx.h>                  /* LPC17xx definitions */
#include "CAN.h"                      /* LPC17xx CAN adaption layer */
#include "../RIT/RIT.h"
#include "../GLCD/GLCD.h"
#include "../game/game.h"
#include "../game/graphic.h"


/*----------------------------------------------------------------------------
  CAN interrupt handler
 *----------------------------------------------------------------------------*/
void CAN_IRQHandler (void)  {
	uint32_t received_move;
	icr = 0;
  icr = (LPC_CAN1->ICR | icr) & 0xFF;
	
	if (icr & (1 << 0)) {
		CAN_rdMsg (1, &CAN_RxMsg);	                		/* Read the message */
		LPC_CAN1->CMR = (1 << 2);                    		/* Release receive buffer */
		if (get_state() == STARTING || ((CAN_RxMsg.id - 1) == (get_board_player() + 1) % 2)) {
			if (get_state() == PLAYING && !is_this_board_playing()) {
				received_move = (CAN_RxMsg.data[0] << 24) |
												(CAN_RxMsg.data[1] << 16) |
												(CAN_RxMsg.data[2] << 8) |
												 CAN_RxMsg.data[3];
				receive_move(received_move);
			}
			else if (get_state() == STARTING && CAN_RxMsg.data[0] == 0xFF) {
				set_mode(MULTIPLAYER);
				set_board_player(1);
				set_player_type(0, REMOTE);
				set_state(CHOOSE_THIS_PLAYER);
				draw_this_player_select();
				enable_RIT();
			}
			else if ((get_state() == CHOOSE_THIS_PLAYER || get_state() == WAITING_MULTIPLAYER) && CAN_RxMsg.data[0] == 0xFF) {
				set_player_ready((get_board_player() + 1) % 2, 1);
				if (both_players_ready())
					start_game();
			}
		}
	}
	
//  if (icr & (1 << 0)) {                          		/* CAN Controller #1 meassage is received */
//		CAN_rdMsg (1, &CAN_RxMsg);	                		/* Read the message */
//		GUI_Text(120, 120, (uint8_t*)CAN_RxMsg.data, Black, White);	
//    LPC_CAN1->CMR = (1 << 2);                    		/* Release receive buffer */ //RESET INTERRUPT
//	}

//  /* check CAN controller 1 */
//	icr = 0;
//  icr = (LPC_CAN1->ICR | icr) & 0xFF;               /* clear interrupts */
//	
//  if (icr & (1 << 0)) {                          		/* CAN Controller #1 meassage is received */
//		CAN_rdMsg (1, &CAN_RxMsg);	                		/* Read the message */
//    LPC_CAN1->CMR = (1 << 2);                    		/* Release receive buffer */ RESET INTERRUPT
//		
//		val_RxCoordX = (CAN_RxMsg.data[0] << 8)  ;
//		val_RxCoordX = val_RxCoordX | CAN_RxMsg.data[1];
//		
//		val_RxCoordY = (CAN_RxMsg.data[2] << 8);
//		val_RxCoordY = val_RxCoordY | CAN_RxMsg.data[3];
//		
//		display.x = val_RxCoordX;
//		display.y = val_RxCoordY-140;
//		TP_DrawPoint_Magnifier(&display);
//		
//		puntiRicevuti1++;
//  }
//	if (icr & (1 << 1)) {                         /* CAN Controller #1 meassage is transmitted */
//		// do nothing in this example
//		puntiInviati1++;
//	}
//		
//	/* check CAN controller 2 */
//	icr = 0;
//	icr = (LPC_CAN2->ICR | icr) & 0xFF;             /* clear interrupts */

//	if (icr & (1 << 0)) {                          	/* CAN Controller #2 meassage is received */
//		CAN_rdMsg (2, &CAN_RxMsg);	                		/* Read the message */
//    LPC_CAN2->CMR = (1 << 2);                    		/* Release receive buffer */
//		
//		val_RxCoordX = (CAN_RxMsg.data[0] << 8)  ;
//		val_RxCoordX = val_RxCoordX | CAN_RxMsg.data[1];
//		
//		val_RxCoordY = (CAN_RxMsg.data[2] << 8);
//		val_RxCoordY = val_RxCoordY | CAN_RxMsg.data[3];
//		
//		display.x = val_RxCoordX;
//		display.y = val_RxCoordY+140;
//		TP_DrawPoint_Magnifier(&display);
//		
//		puntiRicevuti2++;
//	}
//	if (icr & (1 << 1)) {                         /* CAN Controller #2 meassage is transmitted */
//		// do nothing in this example
//		puntiInviati2++;
//	}
}
