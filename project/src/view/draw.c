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
#include "open.xpm"
#include "close.xpm"

#include <string.h>

#pragma clang optimize off

#define SCREEN_W 800
#define SCREEN_H 600

void game_draw(Game *game) {
  static uint8_t *menu_pixmap         = NULL; static xpm_image_t menu_img;
  static uint8_t *take_pixmap         = NULL; static xpm_image_t take_img;
  static uint8_t *prepare_pixmap      = NULL; static xpm_image_t prepare_img;
  static uint8_t *cook_pixmap         = NULL; static xpm_image_t cook_img;
  static uint8_t *cut_pixmap          = NULL; static xpm_image_t cut_img;
  static uint8_t *deliver_pixmap      = NULL; static xpm_image_t deliver_img;
  static uint8_t *open_screen_pixmap  = NULL; static xpm_image_t open_screen_img;
  static uint8_t *close_screen_pixmap = NULL; static xpm_image_t close_screen_img;
  int oven_bar_width;
  int oven_target_ticks;
  int remaining_seconds;
  int name_len;

  if (menu_pixmap         == NULL) menu_pixmap         = xpm_load((xpm_map_t) papas_pizzeria_bg,                                           XPM_8_8_8, &menu_img);
  if (take_pixmap         == NULL) take_pixmap         = xpm_load((xpm_map_t) papas_takeorder_xpm,                                         XPM_8_8_8, &take_img);
  if (prepare_pixmap      == NULL) prepare_pixmap      = xpm_load((xpm_map_t) pizza_prepare_xpm,                                           XPM_8_8_8, &prepare_img);
  if (cook_pixmap         == NULL) cook_pixmap         = xpm_load((xpm_map_t) cook_xpm,                                                   XPM_8_8_8, &cook_img);
  if (cut_pixmap          == NULL) cut_pixmap          = xpm_load((xpm_map_t) pizza_cut_xpm,                                               XPM_8_8_8, &cut_img);
  if (deliver_pixmap      == NULL) deliver_pixmap      = xpm_load((xpm_map_t) pizza_delivery_xpm,                                          XPM_8_8_8, &deliver_img);
  if (open_screen_pixmap  == NULL) open_screen_pixmap  = xpm_load((xpm_map_t) f2a3fbb0a3df4a57f9195f171b9727cd1drfa9J7j21wCTz5,            XPM_8_8_8, &open_screen_img);
  if (close_screen_pixmap == NULL) close_screen_pixmap = xpm_load((xpm_map_t) e19de3ab2636463e81a8d8cebcb35f47mId022ybinaBO6ty,            XPM_8_8_8, &close_screen_img);

  draw_clear(rgb(215, 220, 205));

  switch (game->state) {

    case GAME_STATE_MENU: {
      int hover;
      draw_xpm(menu_pixmap, menu_img, 0, 0);

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

    case GAME_STATE_SETTINGS: {
      int hover;
      vg_draw_xpm(menu_pixmap, menu_img, 0, 0);

      /* title — centred in the button column (x 510..740, 10 chars × scale 3 = 180px) */
      draw_string(535, 165, "DEFINICOES", 3, rgb(252, 238, 202));

      hover = -1;
      if (game->mouse_x >= 510 && game->mouse_x < 740) {
        if      (game->mouse_y >= 195 && game->mouse_y < 245) hover = 0;
        else if (game->mouse_y >= 255 && game->mouse_y < 305) hover = 1;
        else if (game->mouse_y >= 315 && game->mouse_y < 365) hover = 2;
        else if (game->mouse_y >= 460 && game->mouse_y < 510) hover = 7;
      }
      if (game->mouse_y >= 398 && game->mouse_y < 448) {
        if      (game->mouse_x >= 510 && game->mouse_x < 566) hover = 3;
        else if (game->mouse_x >= 568 && game->mouse_x < 624) hover = 4;
        else if (game->mouse_x >= 626 && game->mouse_x < 682) hover = 5;
        else if (game->mouse_x >= 684 && game->mouse_x < 740) hover = 6;
      }

      draw_button(510, 195, 230, 50, game->settings_option == 0 || hover == 0, rgb(70,  160,  70));
      draw_button_label(510, 195, 230, 50, "FACIL",   3, rgb(220, 255, 210));
      draw_button(510, 255, 230, 50, game->settings_option == 1 || hover == 1, rgb(40,   80, 180));
      draw_button_label(510, 255, 230, 50, "NORMAL",  3, rgb(200, 220, 255));
      draw_button(510, 315, 230, 50, game->settings_option == 2 || hover == 2, rgb(180,  40,  40));
      draw_button_label(510, 315, 230, 50, "DIFICIL", 3, rgb(255, 220, 220));

      /* "INCREMENTO" — 10 × 12px = 120px → centred: 510 + (230-120)/2 = 565 */
      draw_string(565, 378, "INCREMENTO", 2, rgb(252, 238, 202));

      draw_button(510, 398, 56, 50, game->day_increment == 0 || hover == 3, rgb(70,  160,  70));
      draw_button_label(510, 398, 56, 50, "+0", 2, rgb(220, 255, 210));
      draw_button(568, 398, 56, 50, game->day_increment == 1 || hover == 4, rgb(40,   80, 180));
      draw_button_label(568, 398, 56, 50, "+1", 2, rgb(200, 220, 255));
      draw_button(626, 398, 56, 50, game->day_increment == 3 || hover == 5, rgb(180,  40,  40));
      draw_button_label(626, 398, 56, 50, "+3", 2, rgb(255, 220, 220));
      draw_button(684, 398, 56, 50, game->day_increment == 5 || hover == 6, rgb(130,  70,  10));
      draw_button_label(684, 398, 56, 50, "+5", 2, rgb(255, 230, 180));

      draw_button(510, 460, 230, 50, hover == 7, rgb(50,  50,  50));
      draw_button_label(510, 460, 230, 50, "VOLTAR", 2, rgb(200, 200, 200));

      break;
    }

    case GAME_STATE_PLAYING:

      switch (game->playing_state) {

        case PLAYING_TAKE_ORDER:
          draw_xpm(take_pixmap, take_img, 0, 0);
          draw_order_ticket(game, 602, 40);
          draw_button(605, 495, 174, 40, false, rgb(90, 160, 90));
          draw_button_label(605, 495, 174, 40, "TIRAR PEDIDO", 2, rgb(30, 60, 30));
          if (game->day_orders_done >= game->day_orders_total - 1) {
            if (drawClosedSignSprite(420, 442, 160, 56) != 0) {
              draw_panel(420, 442, 160, 56, rgb(130, 30, 30));
              draw_string(458, 463, "FECHADO", 2, rgb(110, 25, 25));
            }
          } else {
            if (drawOpenSignSprite(420, 442, 160, 56) != 0) {
              draw_panel(420, 442, 160, 56, rgb(30, 120, 50));
              draw_string(464, 463, "ABERTO", 2, rgb(25, 90, 40));
            }
          }
          break;

        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            draw_xpm(prepare_pixmap, prepare_img, 0, 0);
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

            draw_xpm(cook_pixmap, cook_img, 0, 0);
            draw_order_ticket(game, 602, 40);
            draw_button(605, 495, 174, 40, remaining_seconds == 0, rgb(90, 160, 90));
            draw_button_label(605, 495, 174, 40, remaining_seconds == 0 ? "PARAR" : "ESPERA", 2, rgb(30, 60, 30));

            draw_rect(75, 38, 440, 50, rgb(40, 25, 10));
            draw_rect(85, 45, 420, 18, rgb(60, 60, 60));
            draw_rect(85, 45, oven_bar_width, 18, rgb(235, 180, 70));
            draw_string(85, 70, "TEMPO:", 2, rgb(255, 240, 160));
            { char sec_buf[4]; int si = 0;
              if (remaining_seconds >= 10) sec_buf[si++] = (char)('0' + remaining_seconds / 10);
              sec_buf[si++] = (char)('0' + remaining_seconds % 10);
              sec_buf[si] = '\0';
              draw_string(175, 70, sec_buf, 2, rgb(255, 240, 160)); }

            draw_pizza(game, 300, 320, 150);
          }
          break;

        case PLAYING_CUT:
          draw_xpm(cut_pixmap, cut_img, 0, 0);
          draw_order_ticket(game, 602, 40);
          draw_button(605, 495, 174, 40, game->num_cut_lines >= game->order.slices / 2, rgb(90, 160, 90));
          draw_button_label(605, 495, 174, 40, "CORTAR", 2, rgb(30, 60, 30));
          draw_pizza(game, 300, 320, 150);
          draw_pizza_cuts(game, 300, 320, 150);
          break;

        case PLAYING_SERVE:
          draw_xpm(deliver_pixmap, deliver_img, 0, 0);
          draw_panel(602, 40, 180, 434, rgb(115, 76, 43));
          draw_string(615, 180, "NOME:", 2, rgb(94, 59, 34));
          draw_string(615, 205, game->typed_name, 3, rgb(30, 30, 30));

          name_len = (int)strlen(game->order.name);
          (void)name_len;
          if ((game->tick / 30) % 2 == 0) {
            draw_rect(616 + game->typed_len * 18, 205, 2, 28, rgb(30, 30, 30));
          }

          draw_button(605, 495, 174, 40, false, rgb(90, 160, 90));
          draw_button_label(605, 495, 174, 40, "ENTREGAR", 2, rgb(30, 60, 30));
          if (game->day_orders_done >= game->day_orders_total - 1) {
            if (drawClosedSignSprite(420, 442, 160, 56) != 0) {
              draw_panel(420, 442, 160, 56, rgb(130, 30, 30));
              draw_string(458, 463, "FECHADO", 2, rgb(110, 25, 25));
            }
          } else {
            if (drawOpenSignSprite(420, 442, 160, 56) != 0) {
              draw_panel(420, 442, 160, 56, rgb(30, 120, 50));
              draw_string(464, 463, "ABERTO", 2, rgb(25, 90, 40));
            }
          }
          break;

        case PLAYING_DELIVERED:
          draw_xpm(deliver_pixmap, deliver_img, 0, 0);
          draw_order_ticket(game, 602, 40);

          draw_button(605, 495, 174, 40, false, rgb(90, 160, 90));
          draw_button_label(605, 495, 174, 40, "CONTINUAR", 2, rgb(30, 60, 30));
          break;
      }
      {
        int hb = game->mouse_x >= 10 && game->mouse_x < 100 &&
                 game->mouse_y >= 10 && game->mouse_y < 40;
        draw_button(10, 10, 90, 30, hb, rgb(160, 40, 40));
        draw_button_label(10, 10, 90, 30, "MENU", 2, rgb(255, 220, 220));
      }
      break;

    case GAME_STATE_DAY_INTRO: {
      char day_buf[10];
      int di = 0, tx, hb;
      vg_draw_rectangle(0, 0, SCREEN_W, SCREEN_H, rgb(0, 0, 0));
      draw_string(283, 50, "BEM VINDO AO DIA", 2, rgb(220, 200, 150));
      day_buf[di++] = 'D'; day_buf[di++] = 'I'; day_buf[di++] = 'A'; day_buf[di++] = ' ';
      if (game->day_number >= 10) day_buf[di++] = (char)('0' + (game->day_number / 10) % 10);
      day_buf[di++] = (char)('0' + game->day_number % 10);
      day_buf[di] = '\0';
      tx = (SCREEN_W - di * 24) / 2;
      draw_string(tx, 110, day_buf, 4, rgb(255, 240, 200));
      draw_string(310, 335, "PEDIDOS HOJE", 2, rgb(180, 180, 180));
      {
        char tot_buf[4]; int ti = 0;
        if (game->day_orders_total >= 10) tot_buf[ti++] = (char)('0' + game->day_orders_total / 10);
        tot_buf[ti++] = (char)('0' + game->day_orders_total % 10);
        tot_buf[ti] = '\0';
        tx = (SCREEN_W - ti * 24) / 2;
        draw_string(tx, 370, tot_buf, 4, rgb(255, 240, 200));
      }
      hb = game->mouse_x >= 300 && game->mouse_x < 500 &&
           game->mouse_y >= 510 && game->mouse_y < 560;
      draw_button(300, 510, 200, 50, hb, rgb(90, 160, 90));
      draw_button_label(300, 510, 200, 50, "COMECAR", 2, rgb(30, 60, 30));
      break;
    }

    case GAME_STATE_OPEN_SCREEN: {
      if (open_screen_pixmap != NULL)
        vg_draw_xpm_scaled(open_screen_pixmap, open_screen_img, SCREEN_W, SCREEN_H);
      else
        vg_draw_rectangle(0, 0, SCREEN_W, SCREEN_H, rgb(0, 0, 0));
      break;
    }

    case GAME_STATE_CLOSED_SCREEN: {
      if (close_screen_pixmap != NULL)
        vg_draw_xpm_scaled(close_screen_pixmap, close_screen_img, SCREEN_W, SCREEN_H);
      else
        vg_draw_rectangle(0, 0, SCREEN_W, SCREEN_H, rgb(0, 0, 0));
      break;
    }

    case GAME_STATE_DAY_SUMMARY: {
      int avg_stars, avg_score, hb;
      char day_buf[10];
      int di = 0, tx;
      uint32_t gold = rgb(255, 200, 0);
      uint32_t grey = rgb(80, 70, 50);

      vg_draw_rectangle(0, 0, SCREEN_W, SCREEN_H, rgb(0, 0, 0));

      /* "RESUMO DO DIA" scale 3: 13 × 18 = 234px. tx=(800-234)/2=283 */
      draw_string(283, 50, "RESUMO DO DIA", 3, rgb(220, 200, 150));

      day_buf[di++] = 'D'; day_buf[di++] = 'I'; day_buf[di++] = 'A'; day_buf[di++] = ' ';
      if (game->day_number >= 10) day_buf[di++] = (char)('0' + (game->day_number / 10) % 10);
      day_buf[di++] = (char)('0' + game->day_number % 10);
      day_buf[di] = '\0';
      tx = (SCREEN_W - di * 18) / 2;
      draw_string(tx, 110, day_buf, 3, rgb(255, 240, 200));

      avg_stars = game->day_orders_done > 0 ? game->day_total_stars / game->day_orders_done : 0;
      /* "ESTRELAS MEDIA" scale 2: 14 × 12 = 168px. x=(800-168)/2=316 */
      draw_string(316, 205, "ESTRELAS MEDIA", 2, rgb(180, 180, 180));
      draw_diamond(310, 258, 12, avg_stars >= 1 ? gold : grey);
      draw_diamond(400, 258, 12, avg_stars >= 2 ? gold : grey);
      draw_diamond(490, 258, 12, avg_stars >= 3 ? gold : grey);

      avg_score = game->day_orders_done > 0 ? game->day_total_score / game->day_orders_done : 0;
      /* "PONTUACAO MEDIA" scale 2: 15 × 12 = 180px. x=(800-180)/2=310 */
      draw_string(310, 335, "PONTUACAO MEDIA", 2, rgb(180, 180, 180));
      draw_char(370, 362, (char)('0' + avg_score / 10), 4, rgb(255, 240, 200));
      vg_draw_rectangle(392, 386, 4, 4, rgb(255, 240, 200));
      draw_char(398, 362, (char)('0' + avg_score % 10), 4, rgb(255, 240, 200));

      hb = game->mouse_x >= 300 && game->mouse_x < 500 &&
           game->mouse_y >= 510 && game->mouse_y < 560;
      draw_button(300, 510, 200, 50, hb, rgb(90, 160, 90));
      draw_button_label(300, 510, 200, 50, "NEXT DAY", 2, rgb(30, 60, 30));
      break;
    }
  }

  if (game->show_back_popup) {
    int sim_hover = game->mouse_x >= 280 && game->mouse_x < 380 &&
                    game->mouse_y >= 295 && game->mouse_y < 330;
    int nao_hover = game->mouse_x >= 420 && game->mouse_x < 520 &&
                    game->mouse_y >= 295 && game->mouse_y < 330;
    draw_panel(190, 220, 420, 150, rgb(80, 40, 40));
    draw_string(316, 248, "TEM A CERTEZA?", 2, rgb(80, 40, 40));
    draw_string(322, 277, "VOLTAR AO MENU?", 1, rgb(100, 60, 40));
    draw_button(280, 295, 100, 35, sim_hover, rgb(70, 150, 70));
    draw_button_label(280, 295, 100, 35, "SIM", 2, rgb(200, 255, 200));
    draw_button(420, 295, 100, 35, nao_hover, rgb(160, 40, 40));
    draw_button_label(420, 295, 100, 35, "NAO", 2, rgb(255, 210, 210));
  }

  if (drawMouseCursorSprite(game->mouse_x, game->mouse_y) != 0) {
    draw_rect(game->mouse_x - 7, game->mouse_y - 1, 15, 3, rgb(20, 20, 20));
    draw_rect(game->mouse_x - 1, game->mouse_y - 7, 3, 15, rgb(20, 20, 20));
  }
  draw_swap();
}

#pragma clang optimize on
