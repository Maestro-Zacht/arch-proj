#include "game.h"
#include "graphic.h"
#include "../RIT/RIT.h"
#include "../timer/timer.h"
#include "../CAN/CAN.h"

typedef struct {
	unsigned short x, y;
} posizione;

typedef struct {
	unsigned short can_go[4];		// 0: up, 1: right, 2: down, 3: left
	short player; 							// 0 / 1 / -1
	short visited;
} cella;

typedef struct {
	posizione center;
	unsigned short dir; 				// 0: h, 1: v
} wall;

typedef struct {
	posizione pos;
	unsigned short remaining_walls;
	uint16_t color;
	player_b type;
	unsigned short multiplayer_ready;
} player;

typedef struct {
	unsigned short moves[SIZE_SCACCHIERA*SIZE_SCACCHIERA], i;
	int n;
} ai_t;

cella scacchiera[SIZE_SCACCHIERA][SIZE_SCACCHIERA];

wall walls[8 * 2];
unsigned short n_walls;

player p0, p1;
unsigned this_board_player;

ai_t ai_moves;

state_e state;
pvp_e pvp_mode;

unsigned short turn; // 0: p0, 1: p1
unsigned short mode; // 0: players, 1: walls
wall new_wall;
posizione chosen_pos;

const uint16_t can_move_color = Yellow;
const uint16_t chosen_square_color = Green;
const uint16_t new_wall_color = Red;
const uint16_t placed_wall_color = Black;

short time_left;

uint32_t last_move;

void send_handshake(void) {
	CAN_TxMsg.data[0] = 0xFF;
	CAN_TxMsg.data[1] = 0;
	CAN_TxMsg.data[2] = 0;
	CAN_TxMsg.data[3] = 0;
	CAN_TxMsg.len = 4;
	CAN_TxMsg.id = 1;
	CAN_TxMsg.format = STANDARD_FORMAT;
	CAN_TxMsg.type = DATA_FRAME;
	CAN_wrMsg (1, &CAN_TxMsg);               /* transmit message on CAN1 */
}

void send_move(void) {
	CAN_TxMsg.data[0] = (last_move >> 24) & 0xFF;
	CAN_TxMsg.data[1] = (last_move >> 16) & 0xFF;
	CAN_TxMsg.data[2] = (last_move >> 8) & 0xFF;
	CAN_TxMsg.data[3] = last_move & 0xFF;
	CAN_TxMsg.len = 4;
	CAN_TxMsg.id = 1;
	CAN_TxMsg.format = STANDARD_FORMAT;
	CAN_TxMsg.type = DATA_FRAME;
	CAN_wrMsg (1, &CAN_TxMsg);               /* transmit message on CAN1 */
}

void set_player_ready(unsigned player, unsigned ready) {
	if (player == 0)
		p0.multiplayer_ready = ready;
	else
		p1.multiplayer_ready = ready;
}

int both_players_ready(void) {
	return (pvp_mode == MULTIPLAYER && p0.multiplayer_ready == 1 && p1.multiplayer_ready == 1);
}

void set_state(state_e new_state) {
	state = new_state;
}

state_e get_state(void) {
	return state;
}

void set_mode(pvp_e new_mode) {
	pvp_mode = new_mode;
}

pvp_e get_mode(void) {
	return pvp_mode;
}

void set_board_player(unsigned player) {
	this_board_player = player;
}

unsigned get_board_player(void) {
	return this_board_player;
}

int is_this_board_playing(void) {
	return (state == PLAYING && (pvp_mode == SINGLE_PLAYER || turn == this_board_player));
}

void set_player_type(unsigned player, player_b type) {
	if(player == 0)
		p0.type = type;
	else
		p1.type = type;
}

posizione move_pos(posizione old_pos, int dir) {
	posizione new_pos = old_pos;
	switch(dir) {
		case 0:
			new_pos.x--;
			break;
		case 1:
			new_pos.y++;
			break;
		case 2:
			new_pos.x++;
			break;
		case 3:
			new_pos.y--;
			break;
	}
	return new_pos;
}

void init_game(void) {
	int i, j;
	
	for(i=0;i<SIZE_SCACCHIERA;i++) {
		for(j=0;j<SIZE_SCACCHIERA;j++) {
			scacchiera[i][j].player = -1;
			scacchiera[i][j].can_go[0] = (i == 0) ? 0 : 1;												// up
			scacchiera[i][j].can_go[1] = (j == SIZE_SCACCHIERA - 1) ? 0 : 1;			// right
			scacchiera[i][j].can_go[2] = (i == SIZE_SCACCHIERA - 1) ? 0 : 1;			// down
			scacchiera[i][j].can_go[3] = (j == 0) ? 0 : 1;												// left
		}
	}
	
	p0.pos.x = 0;
	p0.pos.y = 3;
	p0.remaining_walls = 8;
	p0.color = White;
	scacchiera[p0.pos.x][p0.pos.y].player = 0;
	p1.pos.x = 6;
	p1.pos.y = 3;
	p1.remaining_walls = 8;
	p1.color = Red;
	scacchiera[p1.pos.x][p1.pos.y].player = 1;
	n_walls = 0;
	
	ai_moves.n = 0;
	ai_moves.i = 0;
	
	init_game_graphic();
	
	draw_player(p0.pos.x, p0.pos.y, p0.color);
	draw_player(p1.pos.x, p1.pos.y, p1.color);
	
	set_player_walls(p0.remaining_walls, 0);
	set_player_walls(p1.remaining_walls, 1);
}

int pos_eq(posizione pos1, posizione pos2) {
	return (pos1.x == pos2.x && pos1.y == pos2.y);
}

int can_go(posizione from, int dir) { // 0: up, 1: right, 2: down, 3: left
	return scacchiera[from.x][from.y].can_go[dir];
}

int find_exit(int player, posizione pos){
	int res;
	posizione newpos;
	if((player == 0 && pos.x == SIZE_SCACCHIERA - 1) || (player == 1 && pos.x == 0))
		return 1;
		
	scacchiera[pos.x][pos.y].visited = 1;
	
	if (player == 1) {
		newpos = move_pos(pos, 0);
		if(can_go(pos, 0) && scacchiera[newpos.x][newpos.y].visited == 0){
			res = find_exit(player, newpos);
			if(res != -1)
				return 0;
		}
	}
	else {
		newpos = move_pos(pos, 2);
		if(can_go(pos, 2) && scacchiera[newpos.x][newpos.y].visited == 0){
			res = find_exit(player, newpos);
			if(res != -1)
				return 2;
		}
	}
	
	newpos = move_pos(pos, 1);
	if(can_go(pos, 1) && scacchiera[newpos.x][newpos.y].visited == 0){
		res = find_exit(player, newpos);
		if(res != -1)
			return 1;
	}
	
	newpos = move_pos(pos, 3);
	if(can_go(pos, 3) && scacchiera[newpos.x][newpos.y].visited == 0){
		res = find_exit(player, newpos);
		if(res != -1)
			return 3;
	}
	
	if (player == 0) {
		newpos = move_pos(pos, 0);
		if(can_go(pos, 0) && scacchiera[newpos.x][newpos.y].visited == 0){
			res = find_exit(player, newpos);
			if(res != -1)
				return 0;
		}
	}
	else {
		newpos = move_pos(pos, 2);
		if(can_go(pos, 2) && scacchiera[newpos.x][newpos.y].visited == 0){
			res = find_exit(player, newpos);
			if(res != -1)
				return 2;
		}
	}
	
	return -1;
}

void reset_visited(void) {
	int i, j;
	for(i=0;i<SIZE_SCACCHIERA;i++)
		for(j=0;j<SIZE_SCACCHIERA;j++)
			scacchiera[i][j].visited = 0;
}

int is_valid_wall(void){
	int res;
	reset_visited();
	
	res = find_exit(0, p0.pos);
	if(res == -1)
		return 0;
	
	reset_visited();
	
	res = find_exit(1, p1.pos);
	if(res == -1)
		return 0;
	
	return 1;
}

int place_wall(wall w){
	int i;
	if(w.center.x > SIZE_SCACCHIERA - 1 || w.center.y > SIZE_SCACCHIERA - 1) // no need to check < 0, is unsigned
		return 0;
	for(i=0;i<n_walls;i++) {
		if(
			(walls[i].center.x == w.center.x && walls[i].center.y == w.center.y) ||																																			// same center
			(walls[i].dir == 0 && w.dir == 0 && (walls[i].center.x == w.center.x && (walls[i].center.y == w.center.y + 1 || walls[i].center.y == w.center.y - 1))) || // overlaps horizontally
			(walls[i].dir == 1 && w.dir == 1 && ((walls[i].center.x == w.center.x + 1 || walls[i].center.x == w.center.x - 1) && walls[i].center.y == w.center.y))		// overlaps vertically
		)
			return 0;
	}
	
	walls[n_walls].dir = w.dir;
	walls[n_walls].center.x = w.center.x;
	walls[n_walls].center.y = w.center.y;
	n_walls++;
	if(w.dir == 0){ // horizontal
		scacchiera[w.center.x][w.center.y].can_go[2] = 0;
		scacchiera[w.center.x][w.center.y+1].can_go[2] = 0;
		scacchiera[w.center.x+1][w.center.y].can_go[0] = 0;
		scacchiera[w.center.x+1][w.center.y+1].can_go[0] = 0;
	}
	else {
		scacchiera[w.center.x][w.center.y].can_go[1] = 0;
		scacchiera[w.center.x][w.center.y+1].can_go[3] = 0;
		scacchiera[w.center.x+1][w.center.y].can_go[1] = 0;
		scacchiera[w.center.x+1][w.center.y+1].can_go[3] = 0;
	}
	
	if(!is_valid_wall()){
		n_walls--;
		if(w.dir == 0){ // horizontal
			scacchiera[w.center.x][w.center.y].can_go[2] = 1;
			scacchiera[w.center.x][w.center.y+1].can_go[2] = 1;
			scacchiera[w.center.x+1][w.center.y].can_go[0] = 1;
			scacchiera[w.center.x+1][w.center.y+1].can_go[0] = 1;
		}
		else {
			scacchiera[w.center.x][w.center.y].can_go[1] = 1;
			scacchiera[w.center.x][w.center.y+1].can_go[3] = 1;
			scacchiera[w.center.x+1][w.center.y].can_go[1] = 1;
			scacchiera[w.center.x+1][w.center.y+1].can_go[3] = 1;
		}
		return 0;
	}
	return 1;
}

void reset_all_walls(void) {
	unsigned i;
	for(i=0;i<n_walls;i++)
		draw_wall(walls[i].center.x, walls[i].center.y, walls[i].dir, placed_wall_color);
}

void apply_mode(unsigned short reset) { // reset=1 -> clear
	player *current_player;
	if (mode == 0) {
		current_player = (turn == 0) ? &p0 : &p1;
		
		if(!reset)
			chosen_pos = current_player->pos;

		if(can_go(current_player->pos, 0)) {
			if(scacchiera[current_player->pos.x-1][current_player->pos.y].player != -1) {
				if (scacchiera[current_player->pos.x-1][current_player->pos.y].can_go[0])			
					highlight_square(current_player->pos.x-2, current_player->pos.y, (reset) ? Blue : can_move_color);
				else {
					if (scacchiera[current_player->pos.x-1][current_player->pos.y].can_go[1])
						highlight_square(current_player->pos.x-1, current_player->pos.y+1, (reset) ? Blue : can_move_color);
					if (scacchiera[current_player->pos.x-1][current_player->pos.y].can_go[3])
						highlight_square(current_player->pos.x-1, current_player->pos.y-1, (reset) ? Blue : can_move_color);
				}
			}
			else
				highlight_square(current_player->pos.x-1, current_player->pos.y, (reset) ? Blue : can_move_color);
		}
		
		if(can_go(current_player->pos, 2)) {
			if(scacchiera[current_player->pos.x+1][current_player->pos.y].player != -1) {
				if (scacchiera[current_player->pos.x+1][current_player->pos.y].can_go[2])
					highlight_square(current_player->pos.x+2, current_player->pos.y, (reset) ? Blue : can_move_color);
				else {
					if (scacchiera[current_player->pos.x+1][current_player->pos.y].can_go[1])
						highlight_square(current_player->pos.x+1, current_player->pos.y+1, (reset) ? Blue : can_move_color);
					if (scacchiera[current_player->pos.x-1][current_player->pos.y].can_go[3])
						highlight_square(current_player->pos.x+1, current_player->pos.y-1, (reset) ? Blue : can_move_color);
				}
			}
			else
				highlight_square(current_player->pos.x+1, current_player->pos.y, (reset) ? Blue : can_move_color);
		}
		
		if(can_go(current_player->pos, 3)) {
			if(scacchiera[current_player->pos.x][current_player->pos.y-1].player != -1) {
				if (scacchiera[current_player->pos.x][current_player->pos.y-1].can_go[3])
					highlight_square(current_player->pos.x, current_player->pos.y-2, (reset) ? Blue : can_move_color);
				else {
					if (scacchiera[current_player->pos.x][current_player->pos.y-1].can_go[0])
						highlight_square(current_player->pos.x-1, current_player->pos.y-1, (reset) ? Blue : can_move_color);
					if (scacchiera[current_player->pos.x][current_player->pos.y-1].can_go[2])
						highlight_square(current_player->pos.x+1, current_player->pos.y-1, (reset) ? Blue : can_move_color);
				}
			}
			else
				highlight_square(current_player->pos.x, current_player->pos.y-1, (reset) ? Blue : can_move_color);
		}
		
		if(can_go(current_player->pos, 1)) {
			if(scacchiera[current_player->pos.x][current_player->pos.y+1].player != -1) {
				if (scacchiera[current_player->pos.x][current_player->pos.y+1].can_go[1])
					highlight_square(current_player->pos.x, current_player->pos.y+2, (reset) ? Blue : can_move_color);
				else {
					if (scacchiera[current_player->pos.x][current_player->pos.y+1].can_go[0])
						highlight_square(current_player->pos.x-1, current_player->pos.y+1, (reset) ? Blue : can_move_color);
					if (scacchiera[current_player->pos.x][current_player->pos.y+1].can_go[2])
						highlight_square(current_player->pos.x+1, current_player->pos.y+1, (reset) ? Blue : can_move_color);
				}
			}
			else
				highlight_square(current_player->pos.x, current_player->pos.y+1, (reset) ? Blue : can_move_color);
		}
	}
	else {
		draw_wall(new_wall.center.x, new_wall.center.y, new_wall.dir, (reset) ? Blue : new_wall_color);
		if (reset)
			reset_all_walls();
	}
}

void select_move(unsigned short dir) { // 0: up, 1: right, 2: down, 3: left
	player *current_player;
	posizione new_pos;
	
	clear_notification();
	
	if(mode == 0) {
		current_player = (turn == 0) ? &p0 : &p1;
		new_pos = move_pos(current_player->pos, dir);
		
		if(can_go(current_player->pos, dir)) {
			if (scacchiera[new_pos.x][new_pos.y].player == -1) {
				apply_mode(0);
				draw_player(new_pos.x, new_pos.y, chosen_square_color);
				chosen_pos = new_pos;
			}
			else if (can_go(new_pos, dir)) {
				new_pos = move_pos(new_pos, dir);
				apply_mode(0);
				draw_player(new_pos.x, new_pos.y, chosen_square_color);
				chosen_pos = new_pos;
			}
			else {
				if (
					can_go(new_pos, (dir+1) % 4) && 
					!pos_eq(chosen_pos, move_pos(new_pos, (dir+1) % 4))
				) {
					new_pos = move_pos(new_pos, (dir+1) % 4);
					apply_mode(0);
					draw_player(new_pos.x, new_pos.y, chosen_square_color);
					chosen_pos = new_pos;
				}
				else if (
					can_go(new_pos, (dir+3) % 4) &&
					(
						pos_eq(chosen_pos, move_pos(new_pos, (dir+1) % 4)) ||
						!can_go(new_pos, (dir+1) % 4)
					)
				) {
					new_pos = move_pos(new_pos, (dir+3) % 4);
					apply_mode(0);
					draw_player(new_pos.x, new_pos.y, chosen_square_color);
					chosen_pos = new_pos;
				}
			}
		}
	}
	else {
		apply_mode(1);
		switch(dir) {
			case 0:
				if(new_wall.center.x > 0)
					new_wall.center.x--;
				break;
			case 1:
				if(new_wall.center.y < 5)
					new_wall.center.y++;
				break;
			case 2:
				if(new_wall.center.x < 5)
					new_wall.center.x++;
				break;
			case 3:
				if(new_wall.center.y > 0)
					new_wall.center.y--;
				break;
		}
		apply_mode(0);
	}
}

void rotate_wall(void) {
	if(mode == 1) {
		clear_notification();
		apply_mode(1);
		new_wall.dir = (new_wall.dir+1) % 2;
		apply_mode(0);
	}
}

void change_mode(void) {
	player *current_player = (turn == 0) ? &p0 : &p1;
	
	clear_notification();
	
	if(mode == 0 && current_player->remaining_walls == 0)
		notify("No walls available", Red, Blue);
	else {
		apply_mode(1);
		mode = (mode+1) % 2;
		if(mode == 1) {
			new_wall.center.x = 2;
			new_wall.center.y = 3;
			new_wall.dir = 0;
		}
		apply_mode(0);
	}
}

void register_move(uint8_t player, uint8_t move_wall, uint8_t vert_hor, uint8_t y, uint8_t x) {
	last_move = player << 4;
	last_move |= move_wall;
	last_move <<= 4;
	last_move |= vert_hor;
	last_move <<= 8;
	last_move |= y;
	last_move <<= 8;
	last_move |= x;
	
	if (pvp_mode == MULTIPLAYER && is_this_board_playing())
		send_move();
}

void reset_time_left(void) {
	disable_timer(3);
	reset_timer(3);
	time_left = 19;
	write_remaining_time(time_left);
	enable_timer(3);
}

int check_win_condition(void) {
	if (turn == 0 && p0.pos.x == SIZE_SCACCHIERA - 1)
		return 0;
	if(turn == 1 && p1.pos.x == 0)
		return 1;
	return -1;
}

void finish_turn(void) {
	turn = (turn+1) % 2;
	mode = 0;
	write_player_turn(turn);
	apply_mode(0);
	reset_time_left();
	if(is_this_board_playing())
		AI();
}

void time_finished(void) {
	register_move(turn, 0, 1, 0, 0);
	apply_mode(1);
	finish_turn();
}

void tick_tack(void) {
	time_left--;
	if(time_left < 0) {
		if (!is_this_board_playing()) {
			time_left = 0;
			write_remaining_time(time_left);
		}
		else
			time_finished();
	}
	else
		write_remaining_time(time_left);
}

void confirm_move(void) {
	player *current_player = (turn == 0) ? &p0 : &p1;
	int w;
	
	if(mode == 0) {		
		if(is_this_board_playing() && pos_eq(chosen_pos, current_player->pos))
			notify("You need to choose a position", Red, Blue);
		else {
			disable_timer(3);
			apply_mode(1);
			erase_player(current_player->pos.x, current_player->pos.y);
			scacchiera[current_player->pos.x][current_player->pos.y].player = -1;
			current_player->pos = chosen_pos;
			scacchiera[chosen_pos.x][chosen_pos.y].player = turn;
			draw_player(current_player->pos.x, current_player->pos.y, current_player->color);
			
			register_move(turn+1, mode, 0, current_player->pos.x, current_player->pos.y); 						// pos.x is a row, so it's a y-coordinate
			w = check_win_condition();
			if (w == -1)
				finish_turn();
			else {
				NVIC_DisableIRQ(EINT1_IRQn);
				NVIC_DisableIRQ(EINT2_IRQn);
				disable_RIT();
				current_player = (w == 0) ? &p0 : &p1;
				highlight_square(current_player->pos.x, current_player->pos.y, Green);
				draw_player(current_player->pos.x, current_player->pos.y, current_player->color);
				notify("VICTORY! Press RESET to start", Black, Green);
			}
		}
	}
	else {
		if(place_wall(new_wall)) {
			disable_timer(3);
			apply_mode(1);
			current_player->remaining_walls--;
			set_player_walls(current_player->remaining_walls, turn);
			
			register_move(turn+1, mode, (new_wall.dir+1) % 2, new_wall.center.x, new_wall.center.y);	// center.x is a row, so it's a y-coordinate
																																															  // wall.dir 0 is horizontal in my code, so it needs conversion
			finish_turn();
		}
		else
			notify("You cannot place a wall here", Red, Blue);
	}
}

int register_ai_move(int player, posizione pos, unsigned short i) {
	int res;
	posizione newpos;
	if((player == 0 && pos.x == SIZE_SCACCHIERA - 1) || (player == 1 && pos.x == 0))
		return 0;
		
	scacchiera[pos.x][pos.y].visited = 1;
	
	if (player == 1) {
		newpos = move_pos(pos, 0);
		if(can_go(pos, 0) && scacchiera[newpos.x][newpos.y].visited == 0){
			res = register_ai_move(player, newpos, i+1);
			if(res != -1) {
				ai_moves.moves[i] = 0;
				return res+1;
			}
		}
	}
	else {
		newpos = move_pos(pos, 2);
		if(can_go(pos, 2) && scacchiera[newpos.x][newpos.y].visited == 0){
			res = register_ai_move(player, newpos, i+1);
			if(res != -1) {
				ai_moves.moves[i] = 2;
				return res+1;
			}
		}
	}
	
	newpos = move_pos(pos, 1);
	if(can_go(pos, 1) && scacchiera[newpos.x][newpos.y].visited == 0){
		res = register_ai_move(player, newpos, i+1);
		if(res != -1) {
			ai_moves.moves[i] = 1;
			return res+1;
		}
	}
	
	newpos = move_pos(pos, 3);
	if(can_go(pos, 3) && scacchiera[newpos.x][newpos.y].visited == 0){
		res = register_ai_move(player, newpos, i+1);
		if(res != -1) {
			ai_moves.moves[i] = 3;
			return res+1;
		}
	}
	
	if (player == 0) {
		newpos = move_pos(pos, 0);
		if(can_go(pos, 0) && scacchiera[newpos.x][newpos.y].visited == 0){
			res = register_ai_move(player, newpos, i+1);
			if(res != -1) {
				ai_moves.moves[i] = 0;
				return res+1;
			}
		}
	}
	else {
		newpos = move_pos(pos, 2);
		if(can_go(pos, 2) && scacchiera[newpos.x][newpos.y].visited == 0){
			res = register_ai_move(player, newpos, i+1);
			if(res != -1) {
				ai_moves.moves[i] = 2;
				return res+1;
			}
		}
	}
	
	return -1;
}

void rebuild_ai_moves(posizione pos) {	
	reset_visited();
	ai_moves.n = register_ai_move(turn, pos, 0);
	ai_moves.i = 0;
}

void AI(void) {
	player *current_player = (turn == 0) ? &p0 : &p1;
	
	if (current_player->type == NPC) {
		if (ai_moves.n == 0 || !can_go(current_player->pos, ai_moves.moves[ai_moves.i]))
			rebuild_ai_moves(current_player->pos);
		
		select_move(ai_moves.moves[ai_moves.i++]);
		confirm_move();
	}
}

void start_game(void) {
	state = PLAYING;
	init_game();
	
	turn = 0;
	mode = 0;
	
	clear_notification();
	apply_mode(0);
	write_player_turn(turn);
	reset_time_left();
	if (is_this_board_playing())
		AI();
	

	NVIC_EnableIRQ(EINT1_IRQn);
	NVIC_EnableIRQ(EINT2_IRQn);
}

void receive_move(uint32_t received_move) {
	uint8_t //player = (received_move >> 24) & 0xFF,
		move_wall = (received_move >> 20) & 0x0F,
		vert_hor = (received_move >> 16) & 0x0F,
		y = (received_move >> 8) & 0xFF,
		x = received_move & 0xFF;
	
	if (move_wall == 0 && vert_hor == 1)
		time_finished();
	else {
		apply_mode(1);
		if (move_wall == 0) {
			mode = 0;
			chosen_pos.x = y;
			chosen_pos.y = x;
		}
		else {
			mode = 1;
			new_wall.center.x = y;
			new_wall.center.y = x;
			new_wall.dir = (vert_hor+1) % 2;
		}
		confirm_move();
	}
}
