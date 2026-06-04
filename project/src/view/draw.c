#include "draw.h"
#include "draw_elements.h"
#include "draw_utils.h"
#include "graphics.h"
#include "sprites.h"
#include "rtc.h"
#include "menu.xpm"
#include "take.xpm"
#include "prepare.xpm"
#include "cook.xpm"
#include "cut.xpm"

static void fmt2_draw(char *s, int pos, uint8_t v) {
  s[pos]     = (char)('0' + v / 10);
  s[pos + 1] = (char)('0' + v % 10);
}

static void build_time_str(char *tbuf, const RtcTime *t) {
  fmt2_draw(tbuf, 0, t->hour); tbuf[2] = ':';
  fmt2_draw(tbuf, 3, t->min);  tbuf[5] = ':';
  fmt2_draw(tbuf, 6, t->sec);  tbuf[8] = '\0';
}

static void build_date_str(char *dbuf, const RtcTime *t) {
  fmt2_draw(dbuf, 0, t->day);  dbuf[2] = '/';
  fmt2_draw(dbuf, 3, t->month); dbuf[5] = '/';
  fmt2_draw(dbuf, 6, (uint8_t)(t->year % 100)); dbuf[8] = '\0';
}

#define SCREEN_W 800
#define SCREEN_H 600

void game_draw(Game *game) {
  static uint8_t *menu_pixmap    = NULL; static xpm_image_t menu_img;
  static uint8_t *take_pixmap    = NULL; static xpm_image_t take_img;
  static uint8_t *prepare_pixmap = NULL; static xpm_image_t prepare_img;
  static uint8_t *cook_pixmap    = NULL; static xpm_image_t cook_img;
  static uint8_t *cut_pixmap     = NULL; static xpm_image_t cut_img;
  char tbuf[9], dbuf[9];
  int oven_bar_width;
  int oven_target_ticks;
  int remaining_seconds;
  int name_x, name_len;

  if (menu_pixmap    == NULL) menu_pixmap    = xpm_load((xpm_map_t) papas_pizzeria_bg,   XPM_8_8_8, &menu_img);
  if (take_pixmap    == NULL) take_pixmap    = xpm_load((xpm_map_t) papas_takeorder_xpm, XPM_8_8_8, &take_img);
  if (prepare_pixmap == NULL) prepare_pixmap = xpm_load((xpm_map_t) pizza_prepare_xpm,   XPM_8_8_8, &prepare_img);
  if (cook_pixmap    == NULL) cook_pixmap    = xpm_load((xpm_map_t) pizza_cook_xpm,      XPM_8_8_8, &cook_img);
  if (cut_pixmap     == NULL) cut_pixmap     = xpm_load((xpm_map_t) pizza_cut_xpm,       XPM_8_8_8, &cut_img);

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
      draw_button_label(285, 355, 230, 70, "JOGAR",  3, rgb(255, 220, 220));
      draw_button(285, 445, 230, 70, game->menu_option == 1 || hover == 1, rgb(40,  80,  180));
      draw_button_label(285, 445, 230, 70, "OPCOES", 3, rgb(200, 220, 255));
      draw_button(285, 530, 230, 70, game->menu_option == 2 || hover == 2, rgb(50,  50,  50));
      draw_button_label(285, 530, 230, 70, "SAIR",   3, rgb(200, 200, 200));

      if (game->last_points > 0)
        draw_number(370, 490, game->last_points, rgb(180, 230, 130));

      /* Clock overlay — top-right dark panel */
      build_time_str(tbuf, &game->current_time);
      build_date_str(dbuf, &game->current_time);
      vg_draw_rectangle(620, 6, 174, 48, rgb(25, 15, 10));
      draw_string(628, 13, tbuf, 2, rgb(255, 220, 80));
      draw_string(640, 33, dbuf, 2, rgb(200, 180, 80));
      break;
    }

    case GAME_STATE_PLAYING:
      vg_draw_rectangle(0, 560, SCREEN_W, 40, rgb(75, 55, 45));

      /* --- Background + state content (drawn first) --- */
      switch (game->playing_state) {

        case PLAYING_TAKE_ORDER:
          vg_draw_xpm(take_pixmap, take_img, 0, 0);
          draw_order_ticket(game, 602, 40);
          draw_button(605, 495, 174, 40, false, rgb(90, 160, 90));
          draw_button_label(605, 495, 174, 40, "TAKE ORDER", 2, rgb(30, 60, 30));
          break;

        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            vg_draw_xpm(prepare_pixmap, prepare_img, 0, 0);
            draw_order_ticket(game, 602, 40);
            draw_button(605, 495, 174, 40, false, rgb(90, 160, 90));
            draw_button_label(605, 495, 174, 40, "MAKE PIZZA", 2, rgb(30, 60, 30));
            draw_pizza(game, 285, 265, 150);
            draw_button(250, 365, 120, 50, game->selected_sauce == 0, rgb(190, 45, 35));
            draw_button_label(250, 365, 120, 50, "TOMATE", 2, rgb(255, 210, 200));
            draw_button(380, 365, 120, 50, game->selected_sauce == 1, rgb(245, 235, 180));
            draw_button_label(380, 365, 120, 50, "BRANCO", 2, rgb(80, 70, 40));
            draw_button(250, 425, 80, 50, topping_selected(game, 0), TOPPING_COLORS[0]);
            draw_button_label(250, 425, 80, 50, "COGUMELO", 1, rgb(255, 240, 210));
            draw_button(340, 425, 80, 50, topping_selected(game, 1), TOPPING_COLORS[1]);
            draw_button_label(340, 425, 80, 50, "CHORICO", 1, rgb(255, 210, 200));
            draw_button(430, 425, 80, 50, topping_selected(game, 2), TOPPING_COLORS[2]);
            draw_button_label(430, 425, 80, 50, "FIAMBRE", 1, rgb(80, 40, 40));
            draw_button(250, 485, 80, 50, topping_selected(game, 3), TOPPING_COLORS[3]);
            draw_button_label(250, 485, 80, 50, "ANANAS", 1, rgb(80, 60, 10));
            draw_button(340, 485, 80, 50, topping_selected(game, 4), TOPPING_COLORS[4]);
            draw_button_label(340, 485, 80, 50, "QUEIJO", 1, rgb(80, 70, 10));
            draw_button(430, 485, 80, 50, topping_selected(game, 5), TOPPING_COLORS[5]);
            draw_button_label(430, 485, 80, 50, "AZEITONA", 1, rgb(200, 230, 180));
          }
          else {
            oven_target_ticks = game->order.cook_seconds * GAME_FPS;
            oven_bar_width = game->oven_ticks * 420 / oven_target_ticks;
            if (oven_bar_width > 420) oven_bar_width = 420;
            remaining_seconds = (oven_target_ticks - game->oven_ticks + GAME_FPS - 1) / GAME_FPS;
            if (remaining_seconds < 0) remaining_seconds = 0;

            vg_draw_xpm(cook_pixmap, cook_img, 0, 0);
            draw_order_ticket(game, 602, 40);
            draw_button(605, 495, 174, 40, remaining_seconds == 0, rgb(90, 160, 90));
            draw_button_label(605, 495, 174, 40, remaining_seconds == 0 ? "STOP" : "WAIT", 2, rgb(30, 60, 30));
            draw_pizza(game, 300, 320, 150);
            draw_string(85, 76, "TEMPO", 2, rgb(255, 240, 160));
            draw_number(165, 70, remaining_seconds, rgb(255, 240, 160));
          }
          break;

        case PLAYING_CUT:
          vg_draw_xpm(cut_pixmap, cut_img, 0, 0);
          draw_order_ticket(game, 602, 40);
          draw_button(605, 495, 174, 40, false, rgb(90, 160, 90));
          draw_button_label(605, 495, 174, 40, "TAKE ORDER", 2, rgb(30, 60, 30));
          draw_pizza(game, 300, 320, 150);
          draw_button(115, 525, 110, 60, game->selected_slices == 4, rgb(235, 180, 70));
          draw_button_label(115, 525, 110, 60, "4", 3, rgb(80, 50, 20));
          draw_button(245, 525, 110, 60, game->selected_slices == 6, rgb(235, 180, 70));
          draw_button_label(245, 525, 110, 60, "6", 3, rgb(80, 50, 20));
          draw_button(375, 525, 110, 60, game->selected_slices == 8, rgb(235, 180, 70));
          draw_button_label(375, 525, 110, 60, "8", 3, rgb(80, 50, 20));
          break;

        case PLAYING_SERVE:
          draw_panel(270, 90, 260, 160, rgb(115, 76, 43));
          name_len = (int)strlen(game->order.name);
          name_x   = 400 - (name_len * 24 - 4) / 2;
          vg_draw_rectangle(200, 300, 400, 70, rgb(70, 70, 70));
          vg_draw_rectangle(206, 306, 388, 58, rgb(245, 240, 215));
          draw_string(216, 318, game->typed_name, 3, rgb(30, 30, 30));
          if ((game->tick / 30) % 2 == 0)
            vg_draw_rectangle(216 + game->typed_len * 18, 320, 2, 28, rgb(30, 30, 30));
          draw_button(285, 460, 230, 60, false, rgb(90, 160, 90));
          draw_button_label(285, 460, 230, 60, "DELIVER", 2, rgb(30, 60, 30));
          break;
      }

      /* --- Top bar drawn LAST so it's never overwritten by XPM --- */
      vg_draw_rectangle(0, 0, SCREEN_W, 60, rgb(165, 45, 40));
      draw_string(10, 18, "SCORE", 2, rgb(255, 240, 160));
      draw_number(78, 12, game->score, rgb(255, 240, 160));
      draw_state_label(game->playing_state);
      build_time_str(tbuf, &game->current_time);
      draw_string(352, 22, tbuf, 2, rgb(255, 240, 160));

      /* Oven bar overlaps the top bar zone: redraw on top of it */
      if (game->playing_state == PLAYING_PREPARE_PIZZA && game->pizza_in_oven) {
        vg_draw_rectangle(85, 45, 420, 22, rgb(60, 60, 60));
        vg_draw_rectangle(85, 45, oven_bar_width, 22, rgb(235, 180, 70));
      }
      break;
  }

  if (drawMouseCursorSprite(game->mouse_x, game->mouse_y) != 0) {
    vg_draw_rectangle(game->mouse_x - 7, game->mouse_y - 1, 15, 3, rgb(20, 20, 20));
    vg_draw_rectangle(game->mouse_x - 1, game->mouse_y - 7, 3, 15, rgb(20, 20, 20));
  }
  vg_swap_buffer();
}
