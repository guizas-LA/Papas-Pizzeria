#include "game_logic.h"
#include "rtc.h"
#include <string.h>

static const char *CUSTOMER_NAMES[] = { "ANA", "AFONSO", "GUI", "BRUNO", "DAGA" };
#define NUM_CUSTOMERS 5

static const int TOPPING_COMBOS[20][3] = {
  {0,1,2}, {0,1,3}, {0,1,4}, {0,1,5},
  {0,2,3}, {0,2,4}, {0,2,5},
  {0,3,4}, {0,3,5}, {0,4,5},
  {1,2,3}, {1,2,4}, {1,2,5},
  {1,3,4}, {1,3,5}, {1,4,5},
  {2,3,4}, {2,3,5}, {2,4,5},
  {3,4,5}
};

static int oven_target_ticks(Game *game) {
  return game->order.cook_seconds * GAME_FPS;
}

void make_order(Game *game) {
  int n = game->order_number;
  int combo = n % 20;
  const char *name = CUSTOMER_NAMES[n % NUM_CUSTOMERS];

  game->order.sauce = n % 2;
  game->order.toppings[0] = TOPPING_COMBOS[combo][0];
  game->order.toppings[1] = TOPPING_COMBOS[combo][1];
  game->order.toppings[2] = TOPPING_COMBOS[combo][2];
  game->order.cook_seconds = 5 + (n % 4);
  game->order.slices = (n % 2 == 0) ? 6 : 8;
  strncpy(game->order.name, name, 7);
  game->order.name[7] = '\0';

  game->selected_sauce = -1;
  game->num_selected_toppings = 0;
  game->oven_ticks = 0;
  game->pizza_in_oven = false;
  game->selected_slices = 0;
  game->typed_name[0] = '\0';
  game->typed_len = 0;
}

void toggle_topping(Game *game, int t) {
  int i;
  for (i = 0; i < game->num_selected_toppings; i++) {
    if (game->selected_toppings[i] == t) {
      game->selected_toppings[i] = game->selected_toppings[--game->num_selected_toppings];
      return;
    }
  }
  if (game->num_selected_toppings < 3)
    game->selected_toppings[game->num_selected_toppings++] = t;
}

bool oven_ready(Game *game) {
  return game->oven_ticks >= oven_target_ticks(game);
}

void start_oven(Game *game) {
  game->oven_ticks    = 0;
  game->pizza_in_oven = true;
}

void serve_pizza(Game *game) {
  game->order_number++;
  make_order(game);
  game->playing_state = PLAYING_TAKE_ORDER;
}

void try_deliver(Game *game) {
  rtc_time_t t;
  if (strcmp(game->typed_name, game->order.name) != 0) {
    game->typed_name[0] = '\0';
    game->typed_len     = 0;
    return;
  }
  rtc_read_time(&t);
  game->delivery_time_str[0] = (char)('0' + t.hour / 10);
  game->delivery_time_str[1] = (char)('0' + t.hour % 10);
  game->delivery_time_str[2] = ':';
  game->delivery_time_str[3] = (char)('0' + t.min / 10);
  game->delivery_time_str[4] = (char)('0' + t.min % 10);
  game->delivery_time_str[5] = ':';
  game->delivery_time_str[6] = (char)('0' + t.sec / 10);
  game->delivery_time_str[7] = (char)('0' + t.sec % 10);
  game->delivery_time_str[8] = '\0';
  game->playing_state = PLAYING_DELIVERED;
}
