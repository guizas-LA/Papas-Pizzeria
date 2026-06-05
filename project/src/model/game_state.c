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
  game->difficulty       = DIFF_NORMAL;
  game->settings_option  = DIFF_NORMAL;
  game->day_increment    = 1;
  game->day_number       = 1;
  game->day_orders_total = 3;
  game->day_orders_done  = 0;
  game->day_total_stars  = 0;
  game->day_total_score  = 0;
  game->state_ticks      = 0;
  game->last_score_10    = 0;
  game->last_stars       = 0;
  game->show_back_popup  = false;
  game->show_exit_popup  = false;
  game->delivery_time_str[0] = '\0';
  game->order_time_str[0]    = '\0';
  rtc_read_datetime(&game->current_time);
  make_order(game);
}

bool game_is_running(Game *game) {
  return game->running;
}

#pragma clang optimize off
void game_update(Game *game) {
  GameState st;
  static int open_ctr  = -1;
  static int close_ctr = -1;

  game->tick++;
  if (game->tick >= 60000) game->tick = 0;

  if (game->tick % GAME_FPS == 0)
    rtc_read_datetime(&game->current_time);

  st = game->state;

  if (st == GAME_STATE_OPEN_SCREEN) {
    if (open_ctr < 0) { open_ctr = 0; game->state_ticks = 0; }
    open_ctr++;
    game->state_ticks++;
    if (open_ctr >= 5 * GAME_FPS) {
      open_ctr = -1;
      game->state_ticks = 0;
      game->state = GAME_STATE_PLAYING;
      game->playing_state = PLAYING_TAKE_ORDER;
      make_order(game);
    }
    handle_click(game);
    return;
  }

  if (st == GAME_STATE_CLOSED_SCREEN) {
    if (close_ctr < 0) { close_ctr = 0; game->state_ticks = 0; }
    close_ctr++;
    game->state_ticks++;
    if (close_ctr >= 5 * GAME_FPS) {
      close_ctr = -1;
      game->state_ticks = 0;
      game->state = GAME_STATE_DAY_SUMMARY;
    }
    handle_click(game);
    return;
  }

  open_ctr  = -1;
  close_ctr = -1;

  if (st == GAME_STATE_DAY_INTRO) {
    handle_click(game);
    return;
  }

  if (st == GAME_STATE_PLAYING &&
      game->playing_state == PLAYING_PREPARE_PIZZA &&
      game->pizza_in_oven)
    game->oven_ticks++;

  handle_click(game);
}
#pragma clang optimize on
