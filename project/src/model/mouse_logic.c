#include "mouse_logic.h"
#include "game_logic.h"

#define SCREEN_W 800
#define SCREEN_H 600

#define PIZZA_CX 285
#define PIZZA_CY 265
#define PIZZA_R  150
#define SAUCE_R  (PIZZA_R * 88 / 105)

#define CUT_CX 300
#define CUT_CY 320
#define CUT_R  150


static bool mouse_inside(Game *game, int x, int y, int w, int h) {
  return game->mouse_x >= x && game->mouse_x < x + w && game->mouse_y >= y && game->mouse_y < y + h;
}

static void cut_point_offset(int N, int i, int r, int *dx, int *dy) {
  static const int S4[] = {    0, 1000,    0, -1000 };
  static const int C4[] = { 1000,    0, -1000,    0 };
  static const int S6[] = {    0,  866,  866,    0, -866, -866 };
  static const int C6[] = { 1000,  500, -500, -1000, -500,  500 };
  static const int S8[] = {    0,  707, 1000,  707,    0, -707, -1000, -707 };
  static const int C8[] = { 1000,  707,    0, -707, -1000, -707,     0,  707 };
  const int *S, *C;
  if      (N == 4) { S = S4; C = C4; }
  else if (N == 6) { S = S6; C = C6; }
  else             { S = S8; C = C8; }
  *dx =  r * S[i] / 1000;
  *dy = -r * C[i] / 1000;
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

        case PLAYING_CUT: {
          int N = game->order.slices;
          int req_cuts = N / 2;
          int j, pdx, pdy, mdx, mdy;
          if (mouse_inside(game, 605, 495, 174, 40) && game->num_cut_lines >= req_cuts) {
            game->playing_state = PLAYING_SERVE;
            break;
          }
          for (j = 0; j < N; j++) {
            cut_point_offset(N, j, CUT_R, &pdx, &pdy);
            mdx = game->mouse_x - (CUT_CX + pdx);
            mdy = game->mouse_y - (CUT_CY + pdy);
            if (mdx*mdx + mdy*mdy <= 16*16) {
              if (game->cut_selected < 0) {
                game->cut_selected = j;
              } else if (game->cut_selected == j) {
                game->cut_selected = -1;
              } else if (game->num_cut_lines < req_cuts) {
                int dup = 0, k, a, b;
                for (k = 0; k < game->num_cut_lines; k++) {
                  a = game->cut_lines[k].a; b = game->cut_lines[k].b;
                  if ((a == game->cut_selected && b == j) ||
                      (a == j && b == game->cut_selected)) { dup = 1; break; }
                }
                if (!dup) {
                  game->cut_lines[game->num_cut_lines].a = game->cut_selected;
                  game->cut_lines[game->num_cut_lines].b = j;
                  game->num_cut_lines++;
                }
                game->cut_selected = -1;
              }
              break;
            }
          }
          break;
        }

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
  if (!packet->x_ov) game->mouse_x += packet->delta_x;
  if (!packet->y_ov) game->mouse_y -= packet->delta_y;

  if (game->mouse_x < 7) game->mouse_x = 7;
  if (game->mouse_y < 7) game->mouse_y = 7;
  if (game->mouse_x >= SCREEN_W - 7) game->mouse_x = SCREEN_W - 8;
  if (game->mouse_y >= SCREEN_H - 7) game->mouse_y = SCREEN_H - 8;

  if (packet->lb) game->mouse_left_click = true;
}
