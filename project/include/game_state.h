#ifndef PROJECT_GAME_STATE_H
#define PROJECT_GAME_STATE_H

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "rtc.h"

#define GAME_VIDEO_MODE 0x115
#define GAME_FPS 60
#define MAX_PLACEMENTS 15
#define MAX_CUT_LINES  4

typedef struct {
  int a;
  int b;
} CutLine;

typedef enum {
  GAME_STATE_MENU,
  GAME_STATE_PLAYING,
  GAME_STATE_SETTINGS
} GameState;

typedef enum {
  DIFF_EASY,
  DIFF_NORMAL,
  DIFF_HARD
} Difficulty;

typedef enum {
  PLAYING_TAKE_ORDER,
  PLAYING_PREPARE_PIZZA,
  PLAYING_CUT,
  PLAYING_SERVE,
  PLAYING_DELIVERED
} PlayingState;

typedef struct {
  int sauce;
  int toppings[3];
  int cook_seconds;
  int slices;
  char name[8];
} Order;

typedef struct {
  int type;
  int dx;
  int dy;
} ToppingPlacement;

typedef struct {
  GameState state;
  PlayingState playing_state;
  bool running;
  int tick;
  int order_number;

  Order order;
  int selected_sauce;
  int active_topping;
  ToppingPlacement topping_placements[MAX_PLACEMENTS];
  int num_placements;
  int oven_ticks;
  bool pizza_in_oven;
  int cut_selected;
  int num_cut_lines;
  CutLine cut_lines[MAX_CUT_LINES];

  char typed_name[8];
  int typed_len;
  char delivery_time_str[9];
  char order_time_str[9];

  int mouse_x;
  int mouse_y;
  bool mouse_left_click;

  int menu_option;
  Difficulty difficulty;
  int settings_option;

  int last_score_10;
  int last_stars;

  RtcTime current_time;
  RtcTime order_time;
} Game;

void game_init(Game *game);
bool game_is_running(Game *game);
void game_update(Game *game);

#endif
