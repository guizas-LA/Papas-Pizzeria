#include "game_logic.h"
#include "rtc.h"
#include <string.h>

static const char *CUSTOMER_NAMES[] = {
  "ANA", "JOAO", "MARIA", "TIAGO", "RITA", "PEDRO",
  "SOFIA", "MIGUEL", "INES", "DIOGO", "BEATRIZ", "AFONSO",
  "CARLA", "VASCO", "CLARA", "DAVID", "MARTA", "RAFAEL",
  "BIA", "BRUNO", "CATIA", "DUARTE", "HELENA", "JORGE",
  "LUCAS", "LEONOR", "LUISA", "MANUEL", "MATILDE", "NUNO",
  "PAULO", "SARA", "TOMAS", "VITOR"
};

#define NUM_CUSTOMERS ((int)(sizeof(CUSTOMER_NAMES) / sizeof(CUSTOMER_NAMES[0])))

static const int TOPPING_COMBOS[20][3] = {
  {0,1,2}, {0,1,3}, {0,1,4}, {0,1,5},
  {0,2,3}, {0,2,4}, {0,2,5},
  {0,3,4}, {0,3,5}, {0,4,5},
  {1,2,3}, {1,2,4}, {1,2,5},
  {1,3,4}, {1,3,5}, {1,4,5},
  {2,3,4}, {2,3,5}, {2,4,5},
  {3,4,5}
};

static int mix_n(int n) {
  unsigned int u = (unsigned int)n;
  u ^= (u << 13);
  u ^= (u >> 7);
  u ^= (u << 5);
  return (int)(u & 0x7FFFFFFF);
}

static int oven_target_ticks(Game *game) {
  return game->order.cook_seconds * GAME_FPS;
}

void make_order(Game *game) {
  int n     = game->order_number;
  int h     = mix_n(n);
  int combo = h % 20;
  int difficulty = n / 3;
  int cook_base, cook_range;

  game->order.sauce        = h % 2;
  game->order.toppings[0]  = TOPPING_COMBOS[combo][0];
  game->order.toppings[1]  = TOPPING_COMBOS[combo][1];
  game->order.toppings[2]  = TOPPING_COMBOS[combo][2];

  cook_base  = 5 - (difficulty > 1 ? 1 : 0);
  if (cook_base < 4) cook_base = 4;
  cook_range = 4 + difficulty;
  if (cook_range > 9) cook_range = 9;
  game->order.cook_seconds = cook_base + (h / 20) % cook_range;

  if (n < 3)
    game->order.slices = 6;
  else if (n < 8)
    game->order.slices = (h % 2 == 0) ? 6 : 8;
  else {
    int s = h % 3;
    game->order.slices = (s == 0) ? 4 : (s == 1) ? 6 : 8;
  }

  {
    int time_offset = game->current_time.hour * 13 +
                      game->current_time.min * 7 +
                      game->current_time.sec;
    const char *name = CUSTOMER_NAMES[(h + n * 7 + time_offset + 3) % NUM_CUSTOMERS];
    strncpy(game->order.name, name, sizeof(game->order.name) - 1);
    game->order.name[sizeof(game->order.name) - 1] = '\0';
  }

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
  int limit;
  game->order_number++;
  if      (game->order_limit == ORDERS_5)  limit = 5;
  else if (game->order_limit == ORDERS_20) limit = 20;
  else                                     limit = 10;
  if (game->order_number >= limit) {
    game->order_number = 0;
    game->state = GAME_STATE_MENU;
    make_order(game);
    return;
  }
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
  int i, matched, extra, off, num_req, extra_penalty;
  int sauce_s, top_s, oven_s, slice_s, total;

  if (strcmp(game->typed_name, game->order.name) != 0) {
    game->typed_name[0] = '\0';
    game->typed_len     = 0;
    return;
  }

  /* Hard: wrong sauce incurs a penalty instead of just giving 0 */
  sauce_s = (game->selected_sauce == game->order.sauce) ? 12 :
            (game->difficulty == DIFF_HARD) ? -12 : 0;

  /* Easy: only 2 toppings required; Hard: extra toppings cost double */
  num_req      = (game->difficulty == DIFF_EASY) ? 2 : 3;
  extra_penalty = (game->difficulty == DIFF_HARD) ? 6 : 3;

  for (i = 0; i < 6; i++) { placed[i] = false; required[i] = false; }
  for (i = 0; i < game->num_placements; i++)
    placed[game->topping_placements[i].type] = true;
  for (i = 0; i < num_req; i++)
    required[game->order.toppings[i]] = true;

  matched = 0;
  for (i = 0; i < num_req; i++)
    if (placed[game->order.toppings[i]]) matched++;
  extra = 0;
  for (i = 0; i < 6; i++)
    if (placed[i] && !required[i]) extra++;

  top_s = matched * 7;
  if (top_s > 20) top_s = 20;
  top_s -= extra * extra_penalty;
  if (top_s < 0) top_s = 0;

  {
    int cs = game->oven_ticks / GAME_FPS;
    int penalty;
    if (game->difficulty == DIFF_EASY)
      penalty = 4;                            /* tolerates ±3 s */
    else if (game->difficulty == DIFF_HARD)
      penalty = 12;                           /* tolerates ±1 s */
    else {
      penalty = 4 + game->order_number / 8;  /* gradual: 4 → max 8 */
      if (penalty > 8) penalty = 8;
    }
    off = cs - game->order.cook_seconds;
    if (off < 0) off = -off;
    oven_s = 12 - off * penalty;
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
  if (total < 0) total = 0;
  game->last_score_10 = total;

  {
    int star3, star2, star1;
    if (game->difficulty == DIFF_EASY) {
      star3 = 30; star2 = 20; star1 = 10;
    } else if (game->difficulty == DIFF_HARD) {
      star3 = 55; star2 = 40; star1 = 20;
    } else {
      /* Star thresholds get slightly harder over time */
      star3 = 42 + game->order_number / 5;
      star2 = 30 + game->order_number / 6;
      star1 = 15 + game->order_number / 8;
      if (star3 > 48) star3 = 48;  /* max score is 50, keep achievable */
      if (star2 > 38) star2 = 38;
      if (star1 > 22) star1 = 22;
    }

    if (total >= star3)      game->last_stars = 3;
    else if (total >= star2) game->last_stars = 2;
    else if (total >= star1) game->last_stars = 1;
    else                     game->last_stars = 0;
  }

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
