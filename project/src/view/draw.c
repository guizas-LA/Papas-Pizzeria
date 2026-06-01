#include "draw.h"
#include "draw_utils.h"
#include "graphics.h"
#include <string.h>
#include "menu.xpm"
#include "take.xpm"
#include "prepare.xpm"
#include "cook.xpm"
#include "cut.xpm"

#define SCREEN_W 800
#define SCREEN_H 600

static const char *SAUCE_NAMES[]   = { "MOLHO DE TOMATE", "MOLHO BRANCO" };
static const char *TOPPING_NAMES[] = {"COGUMELO", "CHORICO", "FIAMBRE", "ANANAS", "QUEIJO", "AZEITONA"};
static const uint32_t TOPPING_COLORS[] = {
  0x8C6432,
  0xBE2814,
  0xE68282,
  0xF0C832,
  0xF5E164,
  0x284619,
};

static void draw_order_ticket(Game *game, int x, int y) {
  char buf[12];
  int i;
  draw_panel(x, y, 180, 390, rgb(115, 76, 43));
  draw_string(x + 12, y + 12, game->order.name, 2, rgb(0, 0, 0));
  draw_string(x + 12, y + 42, SAUCE_NAMES[game->order.sauce], 1, rgb(0, 0, 0));
  for (i = 0; i < 3; i++)
    draw_string(x + 12, y + 58 + i * 16, TOPPING_NAMES[game->order.toppings[i]], 1, rgb(0, 0, 0));

  buf[0] = '0' + game->order.cook_seconds;
  buf[1] = ' '; buf[2] = 'S'; buf[3] = 'E'; buf[4] = 'G'; buf[5] = 'U';
  buf[6] = 'N'; buf[7] = 'D'; buf[8] = 'O'; buf[9] = 'S'; buf[10] = '\0';
  draw_string(x + 12, y + 115, buf, 2, rgb(0, 0, 0));

  buf[0] = '0' + game->order.slices;
  buf[1] = ' '; buf[2] = 'F'; buf[3] = 'A'; buf[4] = 'T'; buf[5] = 'I';
  buf[6] = 'A'; buf[7] = 'S'; buf[8] = '\0';
  draw_string(x + 12, y + 140, buf, 2, rgb(0, 0, 0));
}

static bool topping_selected(Game *game, int t) {
  int i;
  for (i = 0; i < game->num_selected_toppings; i++)
    if (game->selected_toppings[i] == t) return true;
  return false;
}

static const int PIZZA_DOT_X[] = {360, 430, 390, 450, 345, 415};
static const int PIZZA_DOT_Y[] = {205, 205, 250, 265, 270, 285};

static void draw_pizza(Game *game) {
  int i;
  draw_circle(400, 235, 105, rgb(222, 175, 82));
  draw_circle(400, 235, 88,  game->selected_sauce == 1 ? rgb(245, 235, 180) : rgb(190, 45, 35));
  for (i = 0; i < game->num_selected_toppings; i++) {
    uint32_t color = TOPPING_COLORS[game->selected_toppings[i]];
    draw_circle(PIZZA_DOT_X[i * 2],     PIZZA_DOT_Y[i * 2],     12, color);
    draw_circle(PIZZA_DOT_X[i * 2 + 1], PIZZA_DOT_Y[i * 2 + 1], 12, color);
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
  static uint8_t *menu_pixmap    = NULL; static xpm_image_t menu_img;
  static uint8_t *take_pixmap    = NULL; static xpm_image_t take_img;
  static uint8_t *prepare_pixmap = NULL; static xpm_image_t prepare_img;
  static uint8_t *cook_pixmap    = NULL; static xpm_image_t cook_img;
  static uint8_t *cut_pixmap     = NULL; static xpm_image_t cut_img;
  int oven_bar_width;
  int name_x, name_len;

  if (menu_pixmap    == NULL) menu_pixmap    = xpm_load((xpm_map_t) papas_pizzeria_bg, XPM_8_8_8, &menu_img);
  if (take_pixmap    == NULL) take_pixmap    = xpm_load((xpm_map_t) papas_clean_scene, XPM_8_8_8, &take_img);
  if (prepare_pixmap == NULL) prepare_pixmap = xpm_load((xpm_map_t) prepare_xpm,       XPM_8_8_8, &prepare_img);
  if (cook_pixmap    == NULL) cook_pixmap    = xpm_load((xpm_map_t) cook_xpm,          XPM_8_8_8, &cook_img);
  if (cut_pixmap     == NULL) cut_pixmap     = xpm_load((xpm_map_t) cut_xpm,           XPM_8_8_8, &cut_img);

  vg_clear_buffer(rgb(215, 220, 205));

  switch (game->state) {
    case GAME_STATE_MENU: {
      int hover;
      vg_draw_xpm(menu_pixmap, menu_img, 0, 0);

      hover = -1;
      if (game->mouse_x >= 285 && game->mouse_x < 515) {
        if      (game->mouse_y >= 355 && game->mouse_y < 425) hover = 0;
        else if (game->mouse_y >= 445 && game->mouse_y < 515) hover = 1;
        else if (game->mouse_y >= 530 && game->mouse_y < 600) hover = 2;
      }

      draw_button(285, 355, 230, 70, game->menu_option == 0 || hover == 0, rgb(180, 40,  40));
      draw_string(345, 378, "JOGAR",  3, rgb(255, 220, 220));
      draw_button(285, 445, 230, 70, game->menu_option == 1 || hover == 1, rgb(40,  80,  180));
      draw_string(337, 468, "OPCOES", 3, rgb(200, 220, 255));
      draw_button(285, 530, 230, 70, game->menu_option == 2 || hover == 2, rgb(50,  50,  50));
      draw_string(357, 553, "SAIR",   3, rgb(200, 200, 200));

      if (game->last_points > 0)
        draw_number(370, 490, game->last_points, rgb(180, 230, 130));
      break;
    }

    case GAME_STATE_PLAYING:
      vg_draw_rectangle(0, 0, SCREEN_W, 60, rgb(165, 45, 40));
      vg_draw_rectangle(0, 560, SCREEN_W, 40, rgb(75, 55, 45));
      draw_string(10, 18, "SCORE", 2, rgb(255, 240, 160));
      draw_number(78, 12, game->score, rgb(255, 240, 160));
      draw_state_label(game->playing_state);

      switch (game->playing_state) {

        case PLAYING_TAKE_ORDER:
          vg_draw_xpm(take_pixmap, take_img, 0, 0);
          draw_order_ticket(game, 615, 80);
          draw_button(618, 480, 174, 40, false, rgb(90, 160, 90));
          draw_string(635, 491, "TAKE ORDER", 2, rgb(30, 60, 30));
          break;

        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            vg_draw_xpm(prepare_pixmap, prepare_img, 0, 0);
            draw_order_ticket(game, 615, 80);
            draw_pizza(game);
            draw_button(250, 365, 120, 50, game->selected_sauce == 0, rgb(190, 45, 35));
            draw_string(274, 383, "TOMATE", 2, rgb(255, 210, 200));
            draw_button(380, 365, 120, 50, game->selected_sauce == 1, rgb(245, 235, 180));
            draw_string(404, 383, "BRANCO", 2, rgb(80, 70, 40));
            draw_button(250, 425, 80, 50, topping_selected(game, 0), TOPPING_COLORS[0]);
            draw_string(258, 446, "COGUMELO", 1, rgb(255, 240, 210));
            draw_button(340, 425, 80, 50, topping_selected(game, 1), TOPPING_COLORS[1]);
            draw_string(352, 446, "CHORICO", 1, rgb(255, 210, 200));
            draw_button(430, 425, 80, 50, topping_selected(game, 2), TOPPING_COLORS[2]);
            draw_string(442, 446, "FIAMBRE", 1, rgb(80, 40, 40));
            draw_button(250, 485, 80, 50, topping_selected(game, 3), TOPPING_COLORS[3]);
            draw_string(264, 506, "ANANAS", 1, rgb(80, 60, 10));
            draw_button(340, 485, 80, 50, topping_selected(game, 4), TOPPING_COLORS[4]);
            draw_string(354, 506, "QUEIJO", 1, rgb(80, 70, 10));
            draw_button(430, 485, 80, 50, topping_selected(game, 5), TOPPING_COLORS[5]);
            draw_string(434, 506, "AZEITONA", 1, rgb(200, 230, 180));
            draw_button(618, 480, 174, 40, false, rgb(90, 160, 90));
            draw_string(635, 491, "MAKE PIZZA", 2, rgb(30, 60, 30));
          }
          else {
            oven_bar_width = game->oven_ticks * 430 / (game->order.cook_seconds * GAME_FPS * 2);
            if (oven_bar_width > 430) oven_bar_width = 430;

            vg_draw_xpm(cook_pixmap, cook_img, 0, 0);
            draw_order_ticket(game, 615, 80);
            draw_pizza(game);
            vg_draw_rectangle(180, 395, 440, 35, rgb(80, 80, 80));
            vg_draw_rectangle(185, 400, oven_bar_width, 25, rgb(235, 180, 70));
            draw_button(618, 480, 174, 40, false, rgb(90, 160, 90));
            draw_string(635, 491, "STOP", 2, rgb(30, 60, 30));
          }
          break;

        case PLAYING_CUT:
          vg_draw_xpm(cut_pixmap, cut_img, 0, 0);
          draw_order_ticket(game, 615, 80);
          draw_pizza(game);
          draw_button(215, 440, 110, 60, game->selected_slices == 4, rgb(235, 180, 70));
          draw_string(263, 460, "4", 3, rgb(80, 50, 20));
          draw_button(345, 440, 110, 60, game->selected_slices == 6, rgb(235, 180, 70));
          draw_string(393, 460, "6", 3, rgb(80, 50, 20));
          draw_button(475, 440, 110, 60, game->selected_slices == 8, rgb(235, 180, 70));
          draw_string(523, 460, "8", 3, rgb(80, 50, 20));
          draw_button(618, 480, 174, 40, false, rgb(90, 160, 90));
          draw_string(635, 491, "NEXT STEP", 2, rgb(30, 60, 30));
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
