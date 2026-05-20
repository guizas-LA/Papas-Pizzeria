#include "draw.h"
#include "graphics.h"
#include <string.h>

#define SCREEN_W 800
#define SCREEN_H 600

static const uint8_t FONT_DATA[][7] = {
  /* A */ {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
  /* D */ {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E},
  /* E */ {0x1F, 0x10, 0x10, 0x1C, 0x10, 0x10, 0x1F},
  /* I */ {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F},
  /* L */ {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},
  /* M */ {0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11},
  /* N */ {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},
  /* O */ {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
  /* P */ {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
  /* R */ {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11},
  /* S */ {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E},
  /* T */ {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
  /* U */ {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
  /* Y */ {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04},
  /* Z */ {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F},
  /* B */ {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},
  /* C */ {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},
  /* G */ {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E},
  /* ? */ {0x0E, 0x11, 0x01, 0x06, 0x04, 0x00, 0x04},
};

static int char_to_font_idx(char c) {
  switch (c) {
    case 'A': return 0;  case 'D': return 1;  case 'E': return 2;
    case 'I': return 3;  case 'L': return 4;  case 'M': return 5;
    case 'N': return 6;  case 'O': return 7;  case 'P': return 8;
    case 'R': return 9;  case 'S': return 10; case 'T': return 11;
    case 'U': return 12; case 'Y': return 13; case 'Z': return 14;
    case 'B': return 15; case 'C': return 16; case 'G': return 17;
    default:  return 18;
  }
}

static uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) | (g << 8) | b;
}

static void draw_char(int x, int y, char c, int scale, uint32_t color) {
  const uint8_t *rows;
  int row, col;
  if (c == ' ') return;
  rows = FONT_DATA[char_to_font_idx(c)];
  for (row = 0; row < 7; row++) {
    for (col = 0; col < 5; col++) {
      if (rows[row] & (1 << (4 - col))) {
        vg_draw_rectangle(x + col * scale, y + row * scale, scale, scale, color);
      }
    }
  }
}

static void draw_string(int x, int y, const char *s, int scale, uint32_t color) {
  int char_step = 6 * scale;
  int i;
  for (i = 0; s[i] != '\0'; i++) {
    draw_char(x + i * char_step, y, s[i], scale, color);
  }
}

static void draw_circle(int cx, int cy, int radius, uint32_t color) {
  int y, x;
  for (y = -radius; y <= radius; y++) {
    for (x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius) {
        vg_draw_pixel(cx + x, cy + y, color);
      }
    }
  }
}

static void draw_panel(int x, int y, int w, int h, uint32_t color) {
  vg_draw_rectangle(x, y, w, h, color);
  vg_draw_rectangle(x + 4, y + 4, w - 8, h - 8, rgb(252, 238, 202));
}

static void draw_button(int x, int y, int w, int h, bool selected, uint32_t color) {
  vg_draw_rectangle(x, y, w, h, selected ? rgb(48, 120, 70) : rgb(80, 80, 80));
  vg_draw_rectangle(x + 4, y + 4, w - 8, h - 8, color);
}

static void draw_number(int x, int y, int value, uint32_t color) {
  int width = 0;
  int temp  = value;
  int i;

  if (temp == 0) width = 1;
  while (temp > 0) { width++; temp /= 10; }

  for (i = width - 1; i >= 0; i--) {
    int digit = value % 10;
    int px    = x + i * 22;

    if (digit != 1 && digit != 4)                              vg_draw_rectangle(px,      y,      16, 4,  color);
    if (digit != 1 && digit != 2 && digit != 3 && digit != 7) vg_draw_rectangle(px,      y,      4,  18, color);
    if (digit != 5 && digit != 6)                              vg_draw_rectangle(px + 12, y,      4,  18, color);
    if (digit != 0 && digit != 1 && digit != 7)               vg_draw_rectangle(px,      y + 17, 16, 4,  color);
    if (digit == 0 || digit == 2 || digit == 6 || digit == 8) vg_draw_rectangle(px,      y + 20, 4,  18, color);
    if (digit != 2)                                            vg_draw_rectangle(px + 12, y + 20, 4,  18, color);
    if (digit != 1 && digit != 4 && digit != 7)               vg_draw_rectangle(px,      y + 36, 16, 4,  color);

    value /= 10;
  }
}

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
