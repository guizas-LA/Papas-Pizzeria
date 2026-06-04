#pragma once
#include "game_state.h"

void make_order(Game *game);
void toggle_topping(Game *game, int t);
bool oven_ready(Game *game);
void start_oven(Game *game);
void serve_pizza(Game *game);
void try_deliver(Game *game);
