#include "draw_elements.h"
#include "draw_utils.h"
#include "sprites.h"
#include "rtc.h"
#include <string.h>

static const char *SAUCE_NAMES[]   = { "MOLHO DE TOMATE", "MOLHO BRANCO" };
static const char *TOPPING_NAMES[] = { "COGUMELO", "CHORICO", "FIAMBRE", "ANANAS", "QUEIJO", "AZEITONA" };
const uint32_t TOPPING_COLORS[] = {
  0x8C6432,
  0xBE2814,
  0xE68282,
  0xF0C832,
  0xF5E164,
  0x284619,
};

static const int DOT_DX[] = {-40,  30, -10,  50, -55, 15};
static const int DOT_DY[] = {-30, -30,  15,  30,  35, 50};

static void fmt2(char *s, int pos, uint8_t v) {
  s[pos]     = (char)('0' + v / 10);
  s[pos + 1] = (char)('0' + v % 10);
}

void draw_order_ticket(Game *game, int x, int y) {
  char buf[12];
  int i;

  if (drawOrderTicketSprite(x, y, 180, 434) != 0) {
    draw_panel(x, y, 180, 434, rgb(115, 76, 43));
  }

  draw_string(x + 48, y + 22, "ORDER", 2, rgb(94, 59, 34));
  draw_string(x + 18, y + 62, "CLIENTE", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 78, game->order.name, 2, rgb(30, 22, 16));

  draw_string(x + 18, y + 120, "MOLHO", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 137, SAUCE_NAMES[game->order.sauce], 1, rgb(30, 22, 16));

  draw_string(x + 18, y + 176, "TOPPINGS", 1, rgb(141, 90, 46));
  for (i = 0; i < 3; i++)
    draw_string(x + 18, y + 194 + i * 20, TOPPING_NAMES[game->order.toppings[i]], 1, rgb(30, 22, 16));

  buf[0] = '0' + game->order.cook_seconds;
  buf[1] = ' '; buf[2] = 'S'; buf[3] = 'E'; buf[4] = 'G'; buf[5] = 'U';
  buf[6] = 'N'; buf[7] = 'D'; buf[8] = 'O'; buf[9] = 'S'; buf[10] = '\0';
  draw_string(x + 18, y + 285, "FORNO", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 303, buf, 2, rgb(30, 22, 16));

  buf[0] = '0' + game->order.slices;
  buf[1] = ' '; buf[2] = 'F'; buf[3] = 'A'; buf[4] = 'T'; buf[5] = 'I';
  buf[6] = 'A'; buf[7] = 'S'; buf[8] = '\0';
  draw_string(x + 18, y + 354, "CORTE", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 367, buf, 2, rgb(30, 22, 16));

  /* Bottom section: only shown after the player accepts the order */
  if (game->playing_state != PLAYING_TAKE_ORDER) {
    char hbuf[14]; /* "HORA HH:MM:SS\0" */
    hbuf[0]='H'; hbuf[1]='O'; hbuf[2]='R'; hbuf[3]='A'; hbuf[4]=' ';
    fmt2(hbuf,  5, game->order_time.hour);  hbuf[7]  = ':';
    fmt2(hbuf,  8, game->order_time.min);   hbuf[10] = ':';
    fmt2(hbuf, 11, game->order_time.sec);   hbuf[13] = '\0';

    vg_draw_rectangle(x + 14, y + 383, 152, 1, rgb(141, 90, 46));
    draw_string(x + 18, y + 388, hbuf, 1, rgb(94, 59, 34));

    /* Elapsed timer — only visible while preparing */
    if (game->playing_state == PLAYING_PREPARE_PIZZA) {
      int elapsed_s = (game->tick - game->accept_tick) / GAME_FPS;
      int elapsed_m = elapsed_s / 60;
      char lbuf[12]; /* "TEMPO MM:SS\0" */
      elapsed_s %= 60;
      lbuf[0]='T'; lbuf[1]='E'; lbuf[2]='M'; lbuf[3]='P'; lbuf[4]='O'; lbuf[5]=' ';
      lbuf[6]  = (char)('0' + elapsed_m / 10);
      lbuf[7]  = (char)('0' + elapsed_m % 10);
      lbuf[8]  = ':';
      lbuf[9]  = (char)('0' + elapsed_s / 10);
      lbuf[10] = (char)('0' + elapsed_s % 10);
      lbuf[11] = '\0';
      draw_string(x + 18, y + 398, lbuf, 1, rgb(30, 22, 16));
    }
  }
}

void draw_button_label(int bx, int by, int bw, int bh, const char *s, int scale, uint32_t color) {
  int tx = bx + (bw - (int)strlen(s) * 6 * scale) / 2;
  int ty = by + (bh - 7 * scale) / 2;
  draw_string(tx, ty, s, scale, color);
}

bool topping_selected(Game *game, int t) {
  int i;
  for (i = 0; i < game->num_selected_toppings; i++)
    if (game->selected_toppings[i] == t) return true;
  return false;
}

void draw_pizza(Game *game, int cx, int cy, int r) {
  int i, dot_r;
  draw_circle(cx, cy, r,            rgb(222, 175, 82));
  draw_circle(cx, cy, r * 88 / 105, game->selected_sauce == 1 ? rgb(245, 235, 180) : rgb(190, 45, 35));
  dot_r = r * 12 / 105;
  if (dot_r < 1) dot_r = 1;
  for (i = 0; i < game->num_selected_toppings; i++) {
    uint32_t color = TOPPING_COLORS[game->selected_toppings[i]];
    draw_circle(cx + DOT_DX[i*2]   * r / 105, cy + DOT_DY[i*2]   * r / 105, dot_r, color);
    draw_circle(cx + DOT_DX[i*2+1] * r / 105, cy + DOT_DY[i*2+1] * r / 105, dot_r, color);
  }
}

void draw_state_label(PlayingState state) {
  uint32_t color = rgb(255, 240, 160);
  switch (state) {
    case PLAYING_TAKE_ORDER:    draw_string(730, 18, "ORDER",   2, color); break;
    case PLAYING_PREPARE_PIZZA: draw_string(706, 18, "PREPARE", 2, color); break;
    case PLAYING_CUT:           draw_string(754, 18, "CUT",     2, color); break;
    case PLAYING_SERVE:         draw_string(730, 18, "SERVE",   2, color); break;
  }
}
