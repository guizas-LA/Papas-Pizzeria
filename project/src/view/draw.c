#include "draw.h"
#include "draw_elements.h"
#include "draw_utils.h"
#include "graphics.h"
#include "sprites.h"
#include "menu.xpm"
#include "take.xpm"
#include "prepare.xpm"
#include "cook.xpm"
#include "cut.xpm"
#include "deliver.xpm"

#define SCREEN_W 800
#define SCREEN_H 600

void game_draw(Game *game) {
  static uint8_t *menu_pixmap    = NULL; static xpm_image_t menu_img;
  static uint8_t *take_pixmap    = NULL; static xpm_image_t take_img;
  static uint8_t *prepare_pixmap = NULL; static xpm_image_t prepare_img;
  static uint8_t *cook_pixmap    = NULL; static xpm_image_t cook_img;
  static uint8_t *cut_pixmap     = NULL; static xpm_image_t cut_img;
  static uint8_t *deliver_pixmap = NULL; static xpm_image_t deliver_img;
  int oven_bar_width;
  int oven_target_ticks;
  int remaining_seconds;
  int name_len;

  if (menu_pixmap    == NULL) menu_pixmap    = xpm_load((xpm_map_t) papas_pizzeria_bg,   XPM_8_8_8, &menu_img);
  if (take_pixmap    == NULL) take_pixmap    = xpm_load((xpm_map_t) papas_takeorder_xpm, XPM_8_8_8, &take_img);
  if (prepare_pixmap == NULL) prepare_pixmap = xpm_load((xpm_map_t) pizza_prepare_xpm,   XPM_8_8_8, &prepare_img);
  if (cook_pixmap    == NULL) cook_pixmap    = xpm_load((xpm_map_t) cook_xpm,            XPM_8_8_8, &cook_img);
  if (cut_pixmap     == NULL) cut_pixmap     = xpm_load((xpm_map_t) pizza_cut_xpm,       XPM_8_8_8, &cut_img);
  if (deliver_pixmap == NULL) deliver_pixmap = xpm_load((xpm_map_t) pizza_delivery_xpm,  XPM_8_8_8, &deliver_img);

  vg_clear_buffer(rgb(215, 220, 205));

  switch (game->state) {

    case GAME_STATE_MENU: {
      int hover;
      vg_draw_xpm(menu_pixmap, menu_img, 0, 0);

      hover = -1;
      if (game->mouse_x >= 510 && game->mouse_x < 740) {
        if      (game->mouse_y >= 355 && game->mouse_y < 425) hover = 0;
        else if (game->mouse_y >= 445 && game->mouse_y < 515) hover = 1;
        else if (game->mouse_y >= 530 && game->mouse_y < 600) hover = 2;
      }

      draw_button(510, 355, 230, 70, game->menu_option == 0 || hover == 0, rgb(180, 40,  40));
      draw_button_label(510, 355, 230, 70, "JOGAR",  3, rgb(255, 220, 220));
      draw_button(510, 445, 230, 70, game->menu_option == 1 || hover == 1, rgb(40,  80,  180));
      draw_button_label(510, 445, 230, 70, "OPÇÕES", 3, rgb(200, 220, 255));
      draw_button(510, 530, 230, 70, game->menu_option == 2 || hover == 2, rgb(50,  50,  50));
      draw_button_label(510, 530, 230, 70, "SAIR",   3, rgb(200, 200, 200));

      {
        char tstr[9], dstr[14];
        RtcTime *t = &game->current_time;
        int di = 0;
        tstr[0] = (char)('0' + (t->hour/10)%10); tstr[1] = (char)('0' + t->hour%10);
        tstr[2] = ':';
        tstr[3] = (char)('0' + (t->min/10)%10);  tstr[4] = (char)('0' + t->min%10);
        tstr[5] = ':';
        tstr[6] = (char)('0' + (t->sec/10)%10);  tstr[7] = (char)('0' + t->sec%10);
        tstr[8] = '\0';
        dstr[di++] = (char)('0' + (t->day/10)%10);   dstr[di++] = (char)('0' + t->day%10);
        dstr[di++] = ' ';
        dstr[di++] = (char)('0' + (t->month/10)%10); dstr[di++] = (char)('0' + t->month%10);
        dstr[di++] = ' ';
        dstr[di++] = (char)('0' + (t->year/1000)%10);
        dstr[di++] = (char)('0' + (t->year/100)%10);
        dstr[di++] = (char)('0' + (t->year/10)%10);
        dstr[di++] = (char)('0' + t->year%10);
        dstr[di]   = '\0';
        draw_panel(560, 10, 230, 80, rgb(115, 76, 43));
        draw_string(572, 26, tstr, 2, rgb(30, 22, 16));
        draw_string(572, 58, dstr, 2, rgb(30, 22, 16));
      }

      break;
    }

    case GAME_STATE_PLAYING:

      switch (game->playing_state) {

        case PLAYING_TAKE_ORDER:
          vg_draw_xpm(take_pixmap, take_img, 0, 0);
          draw_order_ticket(game, 602, 40);
          draw_button(605, 495, 174, 40, false, rgb(90, 160, 90));
          draw_button_label(605, 495, 174, 40, "TIRAR PEDIDO", 2, rgb(30, 60, 30));
          break;

        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            vg_draw_xpm(prepare_pixmap, prepare_img, 0, 0);
            draw_order_ticket(game, 602, 40);
            draw_button(605, 495, 174, 40, false, rgb(90, 160, 90));
            draw_button_label(605, 495, 174, 40, "FORNO", 2, rgb(30, 60, 30));

            draw_pizza(game, 285, 265, 150);

            draw_button(10, 480, 120, 50, game->selected_sauce == 0, rgb(190, 45, 35));
            draw_button_label(10, 480, 120, 50, "TOMATE", 2, rgb(255, 210, 200));
            draw_button(10, 540, 120, 50, game->selected_sauce == 1, rgb(245, 235, 180));
            draw_button_label(10, 540, 120, 50, "BRANCO", 2, rgb(80, 70, 40));

            draw_button(195, 480, 80, 50, topping_selected(game, 0), TOPPING_COLORS[0]);
            draw_button_label(195, 480, 80, 50, "COGUMELO", 1, rgb(255, 240, 210));
            draw_button(285, 480, 80, 50, topping_selected(game, 1), TOPPING_COLORS[1]);
            draw_button_label(285, 480, 80, 50, "PEPERONI", 1, rgb(255, 210, 200));
            draw_button(375, 480, 80, 50, topping_selected(game, 2), TOPPING_COLORS[2]);
            draw_button_label(375, 480, 80, 50, "FIAMBRE", 1, rgb(80, 40, 40));

            draw_button(195, 540, 80, 50, topping_selected(game, 3), TOPPING_COLORS[3]);
            draw_button_label(195, 540, 80, 50, "ANANÁS", 1, rgb(80, 60, 10));
            draw_button(285, 540, 80, 50, topping_selected(game, 4), TOPPING_COLORS[4]);
            draw_button_label(285, 540, 80, 50, "QUEIJO", 1, rgb(80, 70, 10));
            draw_button(375, 540, 80, 50, topping_selected(game, 5), TOPPING_COLORS[5]);
            draw_button_label(375, 540, 80, 50, "AZEITONAS", 1, rgb(200, 230, 180));

          } else {
            oven_target_ticks = game->order.cook_seconds * GAME_FPS;
            oven_bar_width = game->oven_ticks * 420 / oven_target_ticks;
            if (oven_bar_width > 420) oven_bar_width = 420;
            remaining_seconds = (oven_target_ticks - game->oven_ticks + GAME_FPS - 1) / GAME_FPS;
            if (remaining_seconds < 0) remaining_seconds = 0;

            vg_draw_xpm(cook_pixmap, cook_img, 0, 0);
            draw_order_ticket(game, 602, 40);
            draw_button(605, 495, 174, 40, remaining_seconds == 0, rgb(90, 160, 90));
            draw_button_label(605, 495, 174, 40, remaining_seconds == 0 ? "PARAR" : "ESPERA", 2, rgb(30, 60, 30));

            vg_draw_rectangle(75, 38, 440, 50, rgb(40, 25, 10));
            vg_draw_rectangle(85, 45, 420, 18, rgb(60, 60, 60));
            vg_draw_rectangle(85, 45, oven_bar_width, 18, rgb(235, 180, 70));
            draw_string(85, 70, "TEMPO:", 2, rgb(255, 240, 160));
            { char sec_buf[2] = { (char)('0' + remaining_seconds), '\0' };
              draw_string(175, 70, sec_buf, 2, rgb(255, 240, 160)); }

            draw_pizza(game, 300, 320, 150);
          }
          break;

        case PLAYING_CUT:
          vg_draw_xpm(cut_pixmap, cut_img, 0, 0);
          draw_order_ticket(game, 602, 40);
          draw_button(605, 495, 174, 40, game->num_cut_lines >= game->order.slices / 2, rgb(90, 160, 90));
          draw_button_label(605, 495, 174, 40, "CORTAR", 2, rgb(30, 60, 30));
          draw_pizza(game, 300, 320, 150);
          draw_pizza_cuts(game, 300, 320, 150);
          break;

        case PLAYING_SERVE:
          vg_draw_xpm(deliver_pixmap, deliver_img, 0, 0);
          draw_panel(602, 40, 180, 434, rgb(115, 76, 43));
          draw_string(615, 180, "NOME:", 2, rgb(94, 59, 34));
          draw_string(615, 205, game->typed_name, 3, rgb(30, 30, 30));

          name_len = (int)strlen(game->order.name);
          (void)name_len;
          if ((game->tick / 30) % 2 == 0) {
            vg_draw_rectangle(616 + game->typed_len * 18, 205, 2, 28, rgb(30, 30, 30));
          }

          draw_button(605, 495, 174, 40, false, rgb(90, 160, 90));
          draw_button_label(605, 495, 174, 40, "ENTREGAR", 2, rgb(30, 60, 30));
          break;

        case PLAYING_DELIVERED:
          vg_draw_xpm(deliver_pixmap, deliver_img, 0, 0);
          draw_order_ticket(game, 602, 40);

          draw_button(605, 495, 174, 40, false, rgb(90, 160, 90));
          draw_button_label(605, 495, 174, 40, "CONTINUAR", 2, rgb(30, 60, 30));
          break;
      }
      break;
  }

  if (drawMouseCursorSprite(game->mouse_x, game->mouse_y) != 0) {
    vg_draw_rectangle(game->mouse_x - 7, game->mouse_y - 1, 15, 3, rgb(20, 20, 20));
    vg_draw_rectangle(game->mouse_x - 1, game->mouse_y - 7, 3, 15, rgb(20, 20, 20));
  }
  vg_swap_buffer();
}
