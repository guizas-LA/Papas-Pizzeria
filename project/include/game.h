#ifndef PROJECT_GAME_H
#define PROJECT_GAME_H

#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

#define GAME_VIDEO_MODE 0x115
#define GAME_FPS 60

typedef enum {
  STATE_ORDER,
  STATE_PREPARE,
  STATE_OVEN,
  STATE_CUT,
  STATE_SCORE
} GameState;

typedef struct {
  int sauce;
  int topping;
  int cook_seconds;
  int slices;
} Order;

typedef struct {
  GameState state;
  bool running;
  int tick;
  int order_number;
  int score;
  int last_points;

  Order order;
  int selected_sauce;
  int selected_topping;
  int oven_ticks;
  int selected_slices;

  int mouse_x;
  int mouse_y;
  bool mouse_left_click;
} Game;

void game_init(Game *game);
bool game_is_running(Game *game);
void game_handle_keyboard(Game *game, uint8_t scancode);
void game_handle_mouse_packet(Game *game, struct packet *packet);
void game_update(Game *game);
void game_draw(Game *game);

#endif
