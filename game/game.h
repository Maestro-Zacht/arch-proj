#include "../GLCD/GLCD.h"

#define SIZE_SCACCHIERA 7

typedef enum {
	STARTING,
	CHOOSE_MULTIPLAYER,
	CHOOSE_THIS_PLAYER,
	CHOOSE_OTHER_PLAYER,
	WAITING_MULTIPLAYER,
	PLAYING
} state_e;

typedef enum {
	MULTIPLAYER,
	SINGLE_PLAYER
} pvp_e;

typedef enum {
	HUMAN,
	NPC,
	REMOTE
} player_b;

void send_handshake(void);

void set_player_ready(unsigned player, unsigned ready);
int both_players_ready(void);
void set_state(state_e new_state);
state_e get_state(void);
void set_mode(pvp_e new_mode);
pvp_e get_mode(void);
void set_board_player(unsigned player);
unsigned get_board_player(void);
int is_this_board_playing(void);
void set_player_type(unsigned player, player_b type);
void AI(void);

void init_game(void);
void start_game(void);

void select_move(unsigned short dir); // 0: up, 1: right, 2: down, 3: left
void rotate_wall(void);
void change_mode(void);
void confirm_move(void);
void tick_tack(void);
void receive_move(uint32_t received_move);
