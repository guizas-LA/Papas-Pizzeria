#include "game_state.h"
#include "game_logic.h"
#include "mouse_logic.h"

#define SCREEN_W 800
#define SCREEN_H 600

void game_init(Game *game) {
  game->state            = GAME_STATE_MENU;
  game->playing_state    = PLAYING_TAKE_ORDER;
  game->running          = true;
  game->tick             = 0;
  game->order_number     = 0;
  game->mouse_x          = SCREEN_W / 2;
  game->mouse_y          = SCREEN_H / 2;
  game->mouse_left_click = false;
  game->menu_option      = 0;
  game->last_score_10    = 0;
  game->last_stars       = 0;
  game->delivery_time_str[0] = '\0';
  make_order(game);
}

bool game_is_running(Game *game) {
  return game->running;
}

void game_update(Game *game) {
  game->tick++;

  if (game->state == GAME_STATE_PLAYING &&
      game->playing_state == PLAYING_PREPARE_PIZZA &&
      game->pizza_in_oven)
    game->oven_ticks++;

  handle_click(game);
}
