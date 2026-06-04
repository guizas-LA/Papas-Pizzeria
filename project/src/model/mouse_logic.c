#include "mouse_logic.h"
#include "game_logic.h"

#define SCREEN_W 800
#define SCREEN_H 600

#define PIZZA_CX 285
#define PIZZA_CY 265
#define PIZZA_R  150
#define SAUCE_R  (PIZZA_R * 88 / 105)


static bool mouse_inside(Game *game, int x, int y, int w, int h) {
  return game->mouse_x >= x && game->mouse_x < x + w && game->mouse_y >= y && game->mouse_y < y + h;
}


void handle_click(Game *game) {
  if (!game->mouse_left_click) return;

  switch (game->state) {
    case GAME_STATE_MENU:
      if (mouse_inside(game, 510, 355, 230, 70)) {
        game->state         = GAME_STATE_PLAYING;
        game->playing_state = PLAYING_TAKE_ORDER;
      }
      else if (mouse_inside(game, 510, 445, 230, 70)) {
      }
      else if (mouse_inside(game, 510, 530, 230, 70)) {
        game->running = false;
      }
      break;

    case GAME_STATE_PLAYING:
      switch (game->playing_state) {
        case PLAYING_TAKE_ORDER:
          if (mouse_inside(game, 605, 495, 174, 40)) {
            record_order_start(game);
            game->playing_state = PLAYING_PREPARE_PIZZA;
          }
          break;

        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            /* Click inside sauce circle → place active topping */
            {
              int dx = game->mouse_x - PIZZA_CX;
              int dy = game->mouse_y - PIZZA_CY;
              if (dx*dx + dy*dy <= SAUCE_R*SAUCE_R &&
                  game->active_topping >= 0 &&
                  game->num_placements < MAX_PLACEMENTS) {
                game->topping_placements[game->num_placements].type = game->active_topping;
                game->topping_placements[game->num_placements].dx   = dx;
                game->topping_placements[game->num_placements].dy   = dy;
                game->num_placements++;
                break;
              }
            }
            if (mouse_inside(game,  10, 480, 120, 50)) game->selected_sauce = 0;
            if (mouse_inside(game,  10, 540, 120, 50)) game->selected_sauce = 1;
            if (mouse_inside(game, 195, 480,  80, 50)) toggle_topping(game, 0);
            if (mouse_inside(game, 285, 480,  80, 50)) toggle_topping(game, 1);
            if (mouse_inside(game, 375, 480,  80, 50)) toggle_topping(game, 2);
            if (mouse_inside(game, 195, 540,  80, 50)) toggle_topping(game, 3);
            if (mouse_inside(game, 285, 540,  80, 50)) toggle_topping(game, 4);
            if (mouse_inside(game, 375, 540,  80, 50)) toggle_topping(game, 5);
            if (mouse_inside(game, 605, 495, 174, 40)) start_oven(game);
          } else {
            if (oven_ready(game) && mouse_inside(game, 605, 495, 174, 40))
              game->playing_state = PLAYING_CUT;
          }
          break;

        case PLAYING_CUT:
          if (mouse_inside(game, 115, 40, 110, 60)) game->selected_slices = 4;
          if (mouse_inside(game, 245, 40, 110, 60)) game->selected_slices = 6;
          if (mouse_inside(game, 375, 40, 110, 60)) game->selected_slices = 8;
          if (mouse_inside(game, 605, 495, 174, 40))
            game->playing_state = PLAYING_SERVE;
          break;

        case PLAYING_SERVE:
          if (mouse_inside(game, 605, 495, 174, 40))
            try_deliver(game);
          break;

        case PLAYING_DELIVERED:
          break;
      }
      break;
  }

  game->mouse_left_click = false;
}


void game_handle_mouse_packet(Game *game, struct packet *packet) {
  game->mouse_x += packet->delta_x;
  game->mouse_y -= packet->delta_y;

  if (game->mouse_x < 7) game->mouse_x = 7;
  if (game->mouse_y < 7) game->mouse_y = 7;
  if (game->mouse_x >= SCREEN_W - 7) game->mouse_x = SCREEN_W - 8;
  if (game->mouse_y >= SCREEN_H - 7) game->mouse_y = SCREEN_H - 8;

  if (packet->lb) game->mouse_left_click = true;
}
