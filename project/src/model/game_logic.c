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
  int n     = game->order_number;
  int combo = n % 20;
  const char *name = CUSTOMER_NAMES[n % NUM_CUSTOMERS];

  game->order.sauce        = n % 2;
  game->order.toppings[0]  = TOPPING_COMBOS[combo][0];
  game->order.toppings[1]  = TOPPING_COMBOS[combo][1];
  game->order.toppings[2]  = TOPPING_COMBOS[combo][2];
  game->order.cook_seconds = 5 + (n % 4);
  game->order.slices       = (n % 2 == 0) ? 6 : 8;
  strncpy(game->order.name, name, 7);
  game->order.name[7] = '\0';

  game->selected_sauce  = -1;
  game->active_topping  = -1;
  game->num_placements  = 0;
  game->oven_ticks      = 0;
  game->pizza_in_oven   = false;
  game->cut_selected  = -1;
  game->num_cut_lines = 0;
  game->typed_name[0]   = '\0';
  game->typed_len       = 0;
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

void toggle_topping(Game *game, int t) {
  game->active_topping = (game->active_topping == t) ? -1 : t;
}

void record_order_start(Game *game) {
  RtcTime t;
  rtc_read_datetime(&t);
  game->order_time_str[0] = (char)('0' + (t.hour / 10) % 10);
  game->order_time_str[1] = (char)('0' + t.hour % 10);
  game->order_time_str[2] = ':';
  game->order_time_str[3] = (char)('0' + (t.min / 10) % 10);
  game->order_time_str[4] = (char)('0' + t.min % 10);
  game->order_time_str[5] = ':';
  game->order_time_str[6] = (char)('0' + (t.sec / 10) % 10);
  game->order_time_str[7] = (char)('0' + t.sec % 10);
  game->order_time_str[8] = '\0';
}

void try_deliver(Game *game) {
  RtcTime t;
  bool placed[6];
  bool required[6];
  int i, matched, extra, off;
  int sauce_s, top_s, oven_s, slice_s, total;

  if (strcmp(game->typed_name, game->order.name) != 0) {
    game->typed_name[0] = '\0';
    game->typed_len     = 0;
    return;
  }

  sauce_s = (game->selected_sauce == game->order.sauce) ? 12 : 0;

  for (i = 0; i < 6; i++) { placed[i] = false; required[i] = false; }
  for (i = 0; i < game->num_placements; i++)
    placed[game->topping_placements[i].type] = true;
  for (i = 0; i < 3; i++)
    required[game->order.toppings[i]] = true;

  matched = 0;
  for (i = 0; i < 3; i++)
    if (placed[game->order.toppings[i]]) matched++;
  extra = 0;
  for (i = 0; i < 6; i++)
    if (placed[i] && !required[i]) extra++;

  top_s = matched * 7;
  if (top_s > 20) top_s = 20;
  top_s -= extra * 3;
  if (top_s < 0) top_s = 0;

  {
    int cs = game->oven_ticks / GAME_FPS;
    off = cs - game->order.cook_seconds;
    if (off < 0) off = -off;
    oven_s = 12 - off * 4;
    if (oven_s < 0) oven_s = 0;
  }

  {
    int N = game->order.slices;
    int req_cuts = N / 2;
    int correct = 0, a, b;
    for (i = 0; i < game->num_cut_lines; i++) {
      a = game->cut_lines[i].a;
      b = game->cut_lines[i].b;
      if (b == (a + req_cuts) % N || a == (b + req_cuts) % N)
        correct++;
    }
    slice_s = (req_cuts > 0) ? (correct * 6 / req_cuts) : 6;
    if (slice_s > 6) slice_s = 6;
  }

  total = sauce_s + top_s + oven_s + slice_s;
  game->last_score_10 = total;

  if (total >= 42)      game->last_stars = 3;
  else if (total >= 30) game->last_stars = 2;
  else if (total >= 15) game->last_stars = 1;
  else                  game->last_stars = 0;

  rtc_read_datetime(&t);
  game->delivery_time_str[0] = (char)('0' + (t.hour / 10) % 10);
  game->delivery_time_str[1] = (char)('0' + t.hour % 10);
  game->delivery_time_str[2] = ':';
  game->delivery_time_str[3] = (char)('0' + (t.min / 10) % 10);
  game->delivery_time_str[4] = (char)('0' + t.min % 10);
  game->delivery_time_str[5] = ':';
  game->delivery_time_str[6] = (char)('0' + (t.sec / 10) % 10);
  game->delivery_time_str[7] = (char)('0' + t.sec % 10);
  game->delivery_time_str[8] = '\0';

  game->playing_state = PLAYING_DELIVERED;
}
