#ifndef PROJECT_GAME_STATE_H
#define PROJECT_GAME_STATE_H

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>

#define GAME_VIDEO_MODE 0x115
#define GAME_FPS 60

typedef enum {
  GAME_STATE_MENU,
  GAME_STATE_PLAYING
} GameState;

typedef enum {
  PLAYING_TAKE_ORDER,
  PLAYING_PREPARE_PIZZA,
  PLAYING_CUT,
  PLAYING_SERVE
} PlayingState;

typedef struct {
  int sauce;
  int toppings[3];
  int cook_seconds;
  int slices;
  char name[8];
} Order;

typedef struct {
  GameState state;
  PlayingState playing_state;
  bool running;
  int tick;
  int order_number;
  int score;
  int last_points;

  Order order;
  int selected_sauce;
  int selected_toppings[3];
  int num_selected_toppings;
  int oven_ticks;
  bool pizza_in_oven;
  int selected_slices;

  char typed_name[8];
  int typed_len;

  int mouse_x;
  int mouse_y;
  bool mouse_left_click;

  int menu_option;
} Game;

void game_init(Game *game);
bool game_is_running(Game *game);
void game_handle_keyboard(Game *game, uint8_t scancode);
void game_handle_mouse_packet(Game *game, struct packet *packet);
void game_update(Game *game);

#endif
