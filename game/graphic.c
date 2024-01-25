#include <stdio.h>

#include "graphic.h"

unsigned short has_notification;


unsigned get_center_axis(unsigned co) {
	return 33*co + 19;
}

void draw_rectangle(unsigned x_center, unsigned y_center, unsigned semi_length, unsigned semi_height, uint16_t color) {
	LCD_DrawLine(x_center-semi_length, y_center-semi_height, x_center+semi_length+1, y_center-semi_height, color);
	LCD_DrawLine(x_center+semi_length+1, y_center-semi_height, x_center+semi_length+1, y_center+semi_height+1, color);
	LCD_DrawLine(x_center+semi_length+1, y_center+semi_height+1, x_center-semi_length, y_center+semi_height+1, color);
	LCD_DrawLine(x_center-semi_length, y_center+semi_height+1, x_center-semi_length, y_center-semi_height, color);
}

void draw_square(unsigned i, unsigned j) {
	draw_rectangle(get_center_axis(j), get_center_axis(i), 13, 13, White);
}

// fills a rectangle-like shape
void fill_rectangle(unsigned x_start, unsigned y_start, unsigned x_end, unsigned y_end, uint16_t color) {
	unsigned x;
	for(x = x_start; x <= x_end; x++)
		LCD_DrawLine(x, y_start, x, y_end, color);
}

// highlights a square in the matrix
void highlight_square(unsigned i, unsigned j, uint16_t color) {
	unsigned x_center = get_center_axis(j), y_center = get_center_axis(i);
	fill_rectangle(x_center-12, y_center-12, x_center+13, y_center+13, color);
}

void draw_player(unsigned i, unsigned j, uint16_t color) {
	unsigned x_center = get_center_axis(j), y_center = get_center_axis(i);
	
	fill_rectangle(x_center-3, y_center-10, x_center+4, y_center-3, color);
	LCD_DrawLine(x_center, y_center-3, x_center, y_center+9, color);
	LCD_DrawLine(x_center+1, y_center-3, x_center+1, y_center+9, color);
}

void erase_player(unsigned i, unsigned j) {
	draw_player(i, j, Blue);
}

void set_player_walls(unsigned n, unsigned player) {
	char num[10];
	sprintf(num, "%d", n);
	GUI_Text((player == 0) ? 37 : 197, 265, (uint8_t *)num, (n>0) ? White : Red, Blue);
}

void draw_wall(unsigned i, unsigned j, unsigned dir, uint16_t color) {
	unsigned x_start, y_start, x_end, y_end;
	if(dir == 0) { // horizontal
		x_start = get_center_axis(j) - 13;
		y_start = get_center_axis(i) + 15;
		x_end = get_center_axis(j+1) + 14;
		y_end = y_start + 4;
	}
	else {
		x_start = get_center_axis(j) + 15;
		y_start = get_center_axis(i) - 13;
		x_end = x_start + 4;
		y_end = get_center_axis(i+1) + 14;
	}
	fill_rectangle(x_start, y_start, x_end, y_end, color);
}

void erase_wall(unsigned i, unsigned j, unsigned dir) {
	draw_wall(i, j, dir, Blue);
}

void notify(const char* str, uint16_t color, uint16_t bg) {
	clear_notification();
	GUI_Text(0, 300, (uint8_t *)str, color, bg);
	has_notification = 1;
}

void clear_notification(void) {
	if(has_notification) {
		fill_rectangle(0, 294, 240, 320, Blue);
		has_notification = 0;
	}
}

void write_player_turn(unsigned short player_id) {
	char player_str[10];
	sprintf(player_str, "P%d", player_id + 1);
	GUI_Text(114, 255, (uint8_t *)player_str, White, Blue);
}

void write_remaining_time(short time) {
	char time_str[10];
	sprintf(time_str, "%02d s", time);
	GUI_Text(104, 272, (uint8_t *)time_str, White, Blue);
}

void init_game_graphic(void) {
	int i, j;
	LCD_Clear(Blue);
	for(i=0;i<7;i++)
		for(j=0;j<7;j++)
			draw_square(i, j);
	
	draw_rectangle(40, 267, 35, 25, White);
	draw_rectangle(120, 267, 35, 25, White);
	draw_rectangle(200, 267, 35, 25, White);

	GUI_Text(9, 240, (uint8_t *)"P1 Walls", White, Blue);
	GUI_Text(93, 240, (uint8_t *)"*Timer*", White, Blue);
	GUI_Text(169, 240, (uint8_t *)"P2 Walls", White, Blue);
}

void highlight_choice(unsigned choice) { // 0: uo 1: down
	unsigned i;
	for(i=1;i<6;i++) {
		draw_rectangle(120, (choice == 0) ? 130 : 200, 70-i, 20-i, Green);
		draw_rectangle(120, (choice == 0) ? 200 : 130, 70-i, 20-i, Blue);
	}
}

void draw_multiplayer_select(void) {
	clear_notification();
	GUI_Text(37, 40, (uint8_t *)"Choose the game mode", White, Blue);
	
	draw_rectangle(120, 130, 70, 20, White);
	GUI_Text(70, 125, (uint8_t *)"Single Board", White, Blue);
	
	draw_rectangle(120, 200, 70, 20, White);
	GUI_Text(80, 195, (uint8_t *)"Two Board", White, Blue);
}

void draw_other_player_select(void) {
	LCD_Clear(Blue);
	GUI_Text(65, 40, (uint8_t *)"Single Board", White, Blue);
	GUI_Text(15, 60, (uint8_t *)"select the opposite player", White, Blue);
	
	draw_rectangle(120, 130, 70, 20, White);
	GUI_Text(100, 125, (uint8_t *)"Human", White, Blue);
	
	draw_rectangle(120, 200, 70, 20, White);
	GUI_Text(110, 195, (uint8_t *)"NPC", White, Blue);
}

void draw_this_player_select(void) {
	LCD_Clear(Blue);
	GUI_Text(65, 40, (uint8_t *)"Two Boards", White, Blue);
	GUI_Text(35, 60, (uint8_t *)"select your player", White, Blue);
	
	draw_rectangle(120, 130, 70, 20, White);
	GUI_Text(100, 125, (uint8_t *)"Human", White, Blue);
	
	draw_rectangle(120, 200, 70, 20, White);
	GUI_Text(110, 195, (uint8_t *)"NPC", White, Blue);
}

void draw_waiting_screen(void) {
	LCD_Clear(Blue);
	GUI_Text(10, 150, (uint8_t *)"Waiting for the other player", White, Blue);
}
