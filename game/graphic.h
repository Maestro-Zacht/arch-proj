#include "../GLCD/GLCD.h"

void init_game_graphic(void);
void highlight_square(unsigned i, unsigned j, uint16_t color);
void draw_player(unsigned i, unsigned j, uint16_t color);
void erase_player(unsigned i, unsigned j);
void set_player_walls(unsigned n, unsigned player);
void draw_wall(unsigned i, unsigned j, unsigned dir, uint16_t color);
void erase_wall(unsigned i, unsigned j, unsigned dir);
void notify(const char* str, uint16_t color, uint16_t bg);
void clear_notification(void);
void write_player_turn(unsigned short player_id);
void write_remaining_time(short time);
void draw_multiplayer_select(void);
void highlight_choice(unsigned choice);
void draw_other_player_select(void);
void draw_this_player_select(void);
void draw_waiting_screen(void);
