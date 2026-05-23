#include "draw.h"
#include "draw_utils.h"
#include "graphics.h"
#include <string.h>

#define SCREEN_W 800
#define SCREEN_H 600

static void draw_order_ticket(Game *game, bool show_ingredients) {
  draw_panel(40, 80, 210, 210, rgb(115, 76, 43));
  draw_string(56, 92, game->order.name, 2, rgb(50, 70, 160));
  if (show_ingredients) {
    vg_draw_rectangle(75, 115, 70 + game->order.sauce * 70,   25, rgb(190, 50,  40));
    vg_draw_rectangle(75, 155, 45 + game->order.topping * 35, 25, rgb(60,  140, 70));
  }
  vg_draw_rectangle(75, 195, game->order.cook_seconds * 12, 25, rgb(220, 130, 35));
  vg_draw_rectangle(75, 240, game->order.slices * 12,       18, rgb(90,  120, 200));
}

static void draw_pizza(Game *game) {
  uint32_t topping_color;

  draw_circle(400, 235, 105, rgb(222, 175, 82));
  draw_circle(400, 235, 88,  game->selected_sauce == 1 ? rgb(245, 235, 180) : rgb(190, 45, 35));

  if (game->selected_topping >= 0) {
    topping_color = rgb(95, 45, 25);
    switch (game->selected_topping) {
      case 1: topping_color = rgb(40,  130, 60);  break;
      case 2: topping_color = rgb(225, 225, 210); break;
      default: break;
    }
    draw_circle(360, 205, 12, topping_color);
    draw_circle(430, 205, 12, topping_color);
    draw_circle(390, 250, 12, topping_color);
    draw_circle(450, 265, 12, topping_color);
    draw_circle(345, 270, 12, topping_color);
  }
}

void game_draw(Game *game) {
  int oven_bar_width;
  int name_x, name_len;

  vg_clear_buffer(rgb(215, 220, 205));

  switch (game->state) {
    case GAME_STATE_MENU:
      vg_draw_rectangle(0, 0, SCREEN_W, SCREEN_H, rgb(115, 76, 43));
      draw_string(161, 185, "PAPAS PIZZERIA", 5, rgb(252, 220, 140));
      draw_string(196, 340, "PRESS ENTER TO PLAY", 3, rgb(222, 190, 120));
      if (game->last_points > 0)
        draw_number(370, 430, game->last_points, rgb(180, 230, 130));
      break;

    case GAME_STATE_PLAYING:
      vg_draw_rectangle(0, 0, SCREEN_W, 60, rgb(165, 45, 40));
      vg_draw_rectangle(0, 560, SCREEN_W, 40, rgb(75, 55, 45));
      draw_number(25, 12, game->score, rgb(255, 240, 160));
      draw_number(700, 12, game->playing_state + 1, rgb(255, 240, 160));
      draw_order_ticket(game, game->playing_state == PLAYING_PREPARE_PIZZA);

      switch (game->playing_state) {

        case PLAYING_TAKE_ORDER:
          draw_panel(300, 120, 300, 250, rgb(115, 76, 43));
          draw_circle(450, 220, 55, rgb(238, 190, 90));
          name_len = (int)strlen(game->order.name);
          name_x   = 450 - (name_len * 18 - 3) / 2;
          draw_string(name_x, 300, game->order.name, 3, rgb(170, 45, 25));
          draw_button(285, 460, 230, 70, false, rgb(235, 180, 70));
          break;

        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            draw_pizza(game);
            draw_button(95,  420, 130, 70, game->selected_sauce   == 0, rgb(190, 45,  35));
            draw_button(245, 420, 130, 70, game->selected_sauce   == 1, rgb(245, 235, 180));
            draw_button(425, 420, 80,  70, game->selected_topping == 0, rgb(95,  45,  25));
            draw_button(525, 420, 80,  70, game->selected_topping == 1, rgb(40,  130, 60));
            draw_button(625, 420, 80,  70, game->selected_topping == 2, rgb(225, 225, 210));
            draw_button(285, 510, 230, 55, false,                       rgb(220, 130, 35));
          }
          else {
            oven_bar_width = game->oven_ticks * 430 / (game->order.cook_seconds * GAME_FPS * 2);
            if (oven_bar_width > 430) oven_bar_width = 430;

            vg_draw_rectangle(250, 135, 300, 210, rgb(80, 80, 80));
            vg_draw_rectangle(275, 160, 250, 160, rgb(230, 120, 45));
            draw_pizza(game);
            vg_draw_rectangle(180, 395, 440, 35, rgb(80, 80, 80));
            vg_draw_rectangle(185, 400, oven_bar_width, 25, rgb(235, 180, 70));
            draw_button(285, 500, 230, 60, false, rgb(235, 180, 70));
          }
          break;

        case PLAYING_CUT:
          draw_pizza(game);
          draw_number(390, 355, game->selected_slices, rgb(80, 50, 30));
          draw_button(135, 440, 110, 60, game->selected_slices == 4, rgb(235, 180, 70));
          draw_button(265, 440, 110, 60, game->selected_slices == 6, rgb(235, 180, 70));
          draw_button(395, 440, 110, 60, game->selected_slices == 8, rgb(235, 180, 70));
          draw_button(555, 440, 110, 60, false, rgb(90, 160, 90));
          break;

        case PLAYING_SERVE:
          draw_panel(270, 90, 260, 160, rgb(115, 76, 43));
          name_len = (int)strlen(game->order.name);
          name_x   = 400 - (name_len * 24 - 4) / 2;
          draw_string(name_x, 145, game->order.name, 4, rgb(170, 45, 25));

          vg_draw_rectangle(200, 300, 400, 70, rgb(70, 70, 70));
          vg_draw_rectangle(206, 306, 388, 58, rgb(245, 240, 215));
          draw_string(216, 318, game->typed_name, 3, rgb(30, 30, 30));

          if ((game->tick / 30) % 2 == 0) {
            vg_draw_rectangle(216 + game->typed_len * 18, 320, 2, 28, rgb(30, 30, 30));
          }

          draw_button(285, 460, 230, 60, false, rgb(90, 160, 90));
          break;
      }
      break;
  }

  vg_draw_rectangle(game->mouse_x, game->mouse_y, 12, 3, rgb(20, 20, 20));
  vg_draw_rectangle(game->mouse_x, game->mouse_y, 3, 12, rgb(20, 20, 20));
  vg_swap_buffer();
}
