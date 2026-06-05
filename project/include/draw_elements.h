#ifndef PROJECT_DRAW_ELEMENTS_H
#define PROJECT_DRAW_ELEMENTS_H

#include <stdbool.h>
#include <stdint.h>
#include "game_state.h"

extern const uint32_t TOPPING_COLORS[6];

void draw_diamond(int cx, int cy, int size, uint32_t color);
void draw_order_ticket(Game *game, int x, int y);
void draw_button_label(int bx, int by, int bw, int bh, const char *s, int scale, uint32_t color);
bool topping_selected(Game *game, int t);
void draw_pizza(Game *game, int cx, int cy, int r);
void draw_pizza_cuts(Game *game, int cx, int cy, int r);

#endif
