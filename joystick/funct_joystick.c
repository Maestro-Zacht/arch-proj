#include "lpc17xx.h"
#include "joystick.h"

#include "../game/game.h"
#include "../game/graphic.h"

typedef enum {
	INVALID,
	UP,
	DOWN
} choice_e;

choice_e choice = INVALID;

void joystick_up(void) {
	if(get_state() == PLAYING && is_this_board_playing())
		select_move(0);
	else if (get_state() == CHOOSE_MULTIPLAYER || get_state() == CHOOSE_OTHER_PLAYER || get_state() == CHOOSE_THIS_PLAYER) {
		choice = UP;
		highlight_choice(0);
	}
}

void joystick_down(void) {
	if(get_state() == PLAYING && is_this_board_playing())
		select_move(2);
	else if (get_state() == CHOOSE_MULTIPLAYER || get_state() == CHOOSE_OTHER_PLAYER || get_state() == CHOOSE_THIS_PLAYER) {
		choice = DOWN;
		highlight_choice(1);
	}
}

void joystick_left(void) {
	if(get_state() == PLAYING && is_this_board_playing())
		select_move(3);
}

void joystick_right(void) {
	if(get_state() == PLAYING && is_this_board_playing())
		select_move(1);
}

void joystick_select(void) {
	int i;
	if(get_state() == PLAYING && is_this_board_playing())
		confirm_move();
	else if (get_state() == CHOOSE_MULTIPLAYER && choice != INVALID) {
		if (choice == DOWN) {
			send_handshake();
			for(i=0;i<100000;i++);			// wait otherwise error not set
			if(((LPC_CAN1->GSR >> 6) & 0x1) == 1) {
				set_mode(SINGLE_PLAYER);
				set_state(CHOOSE_OTHER_PLAYER);
				set_player_type(0, HUMAN);
				draw_other_player_select();
				GUI_Text(2, 280, (uint8_t *)"No board connected, play single player", White, Red);
			}
			else {
				set_mode(MULTIPLAYER);
				set_state(CHOOSE_THIS_PLAYER);
				set_player_type(1, REMOTE);
				set_player_ready(0, 0);
				set_player_ready(1, 0);
				draw_this_player_select();
			}
		}
		else {
			set_mode(SINGLE_PLAYER);
			set_state(CHOOSE_OTHER_PLAYER);
			set_player_type(0, HUMAN);
			draw_other_player_select();
		}
		choice = INVALID;
	}
	else if (get_state() == CHOOSE_OTHER_PLAYER && choice != INVALID) {
		set_player_type(1, (choice == DOWN) ? NPC : HUMAN);
		start_game();
	}
	else if (get_state() == CHOOSE_THIS_PLAYER && choice != INVALID) {
		set_player_type(get_board_player(), (choice == DOWN) ? NPC : HUMAN);
		set_player_ready(get_board_player(), 1);
		send_handshake();
		
		if (both_players_ready())
			start_game();
		else {
			set_state(WAITING_MULTIPLAYER);
			draw_waiting_screen();
		}
		choice = INVALID;
	}
}
