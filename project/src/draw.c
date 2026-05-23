#include "draw.h"
#include "draw_utils.h"
#include "graphics.h"
#include <string.h>
#include "menu.xpm"
#include "take.xpm"

#define SCREEN_W 800
#define SCREEN_H 600

static void draw_order_ticket(Game *game, bool show_ingredients, int x, int y) {
  char s[2];
  draw_panel(x, y, 180, 390, rgb(115, 76, 43));
  draw_string(x + 16, y + 12, game->order.name, 2, rgb(50, 70, 160));
  if (show_ingredients) {
    vg_draw_rectangle(x + 35, y + 35, 70 + game->order.sauce * 70,   25, rgb(190, 50,  40));
    vg_draw_rectangle(x + 35, y + 75, 45 + game->order.topping * 35, 25, rgb(60,  140, 70));
  }
  s[0] = '0' + game->order.cook_seconds; s[1] = '\0';
  draw_string(x + 35, y + 118, s, 3, rgb(220, 130, 35));
  s[0] = '0' + game->order.slices; s[1] = '\0';
  draw_string(x + 35, y + 163, s, 3, rgb(90, 120, 200));
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

static void draw_state_label(PlayingState state) {
  uint32_t color = rgb(255, 240, 160);
  switch (state) {
    case PLAYING_TAKE_ORDER:    draw_string(730, 18, "ORDER",   2, color); break;
    case PLAYING_PREPARE_PIZZA: draw_string(706, 18, "PREPARE", 2, color); break;
    case PLAYING_CUT:           draw_string(754, 18, "CUT",     2, color); break;
    case PLAYING_SERVE:         draw_string(730, 18, "SERVE",   2, color); break;
  }
}


void game_draw(Game *game) {
  static uint8_t *menu_pixmap = NULL;
  static xpm_image_t menu_img;
  static uint8_t *take_pixmap = NULL;
  static xpm_image_t take_img;
  int oven_bar_width;
  int name_x, name_len;

  if (menu_pixmap == NULL)
    menu_pixmap = xpm_load((xpm_map_t) papas_pizzeria_bg, XPM_8_8_8, &menu_img);
  if (take_pixmap == NULL)
    take_pixmap = xpm_load((xpm_map_t) papas_clean_scene, XPM_8_8_8, &take_img);

  vg_clear_buffer(rgb(215, 220, 205));

  switch (game->state) {
    case GAME_STATE_MENU:
      vg_draw_xpm(menu_pixmap, menu_img, 0, 0);
      if (game->last_points > 0)
        draw_number(370, 430, game->last_points, rgb(180, 230, 130));
      break;

    case GAME_STATE_PLAYING:
      vg_draw_rectangle(0, 0, SCREEN_W, 60, rgb(165, 45, 40));
      vg_draw_rectangle(0, 560, SCREEN_W, 40, rgb(75, 55, 45));
      draw_string(10, 18, "SCORE", 2, rgb(255, 240, 160));
      draw_number(78, 12, game->score, rgb(255, 240, 160));
      draw_state_label(game->playing_state);
      

      switch (game->playing_state) {

        case PLAYING_TAKE_ORDER:
          vg_draw_xpm(take_pixmap, take_img, 0, 0);
          draw_order_ticket(game, false, 610, 80);
          draw_button(285, 460, 230, 70, false, rgb(235, 180, 70));
          draw_string(346, 488, "NEXT STEP", 2, rgb(80, 50, 20));
          break;

        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            draw_order_ticket(game, true, 40, 80);
            draw_pizza(game);
            draw_button(95,  420, 130, 70, game->selected_sauce   == 0, rgb(190, 45,  35));
            draw_button(245, 420, 130, 70, game->selected_sauce   == 1, rgb(245, 235, 180));
            draw_button(425, 420, 80,  70, game->selected_topping == 0, rgb(95,  45,  25));
            draw_button(525, 420, 80,  70, game->selected_topping == 1, rgb(40,  130, 60));
            draw_button(625, 420, 80,  70, game->selected_topping == 2, rgb(225, 225, 210));
            draw_button(285, 460, 230, 70, false, rgb(235, 180, 70));
            draw_string(346, 488, "NEXT STEP", 2, rgb(80, 50, 20));
          }
          else {
            oven_bar_width = game->oven_ticks * 430 / (game->order.cook_seconds * GAME_FPS * 2);
            if (oven_bar_width > 430) oven_bar_width = 430;

            vg_draw_rectangle(250, 135, 300, 210, rgb(80, 80, 80));
            vg_draw_rectangle(275, 160, 250, 160, rgb(230, 120, 45));
            draw_pizza(game);
            vg_draw_rectangle(180, 395, 440, 35, rgb(80, 80, 80));
            vg_draw_rectangle(185, 400, oven_bar_width, 25, rgb(235, 180, 70));
            draw_button(285, 460, 230, 70, false, rgb(235, 180, 70));
            draw_string(346, 488, "NEXT STEP", 2, rgb(80, 50, 20));
          }
          break;

        case PLAYING_CUT:
          draw_pizza(game);
          draw_button(215, 440, 110, 60, game->selected_slices == 4, rgb(235, 180, 70));
          draw_string(263, 460, "4", 3, rgb(80, 50, 20));
          draw_button(345, 440, 110, 60, game->selected_slices == 6, rgb(235, 180, 70));
          draw_string(393, 460, "6", 3, rgb(80, 50, 20));
          draw_button(475, 440, 110, 60, game->selected_slices == 8, rgb(235, 180, 70));
          draw_string(523, 460, "8", 3, rgb(80, 50, 20));
          draw_button(285, 520, 230, 50, false, rgb(235, 180, 70));
          draw_string(346, 539, "NEXT STEP", 2, rgb(80, 50, 20));
          break;

        case PLAYING_SERVE:
          draw_panel(270, 90, 260, 160, rgb(115, 76, 43));
          name_len = (int)strlen(game->order.name);
          name_x   = 400 - (name_len * 24 - 4) / 2;

          vg_draw_rectangle(200, 300, 400, 70, rgb(70, 70, 70));
          vg_draw_rectangle(206, 306, 388, 58, rgb(245, 240, 215));
          draw_string(216, 318, game->typed_name, 3, rgb(30, 30, 30));

          if ((game->tick / 30) % 2 == 0) {
            vg_draw_rectangle(216 + game->typed_len * 18, 320, 2, 28, rgb(30, 30, 30));
          }

          draw_button(285, 460, 230, 60, false, rgb(90, 160, 90));
          draw_string(358, 483, "DELIVER", 2, rgb(30, 60, 30));
          break;
      }
      break;
  }

  vg_draw_rectangle(game->mouse_x - 7, game->mouse_y - 1, 15, 3, rgb(20, 20, 20));
  vg_draw_rectangle(game->mouse_x - 1, game->mouse_y - 7, 3, 15, rgb(20, 20, 20));
  vg_swap_buffer();
}
