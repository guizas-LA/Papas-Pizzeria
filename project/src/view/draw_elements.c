#include "draw_elements.h"
#include "draw_utils.h"
#include "sprites.h"
#include <string.h>

#pragma clang optimize off

static const char *SAUCE_NAMES[]   = { "MOLHO DE TOMATE", "MOLHO BRANCO" };
static const char *TOPPING_NAMES[] = { "COGUMELO", "PEPERONI", "FIAMBRE", "ANANÁS", "QUEIJO", "AZEITONAS" };
const uint32_t TOPPING_COLORS[] = {
  0x8C6432,  /* cogumelo */
  0x7A1A0E,  /* peperoni — dark red */
  0xE68282,  /* fiambre  */
  0xF0C832,  /* ananás   */
  0xF5E164,  /* queijo   */
  0x284619,  /* azeitona */
};

void draw_diamond(int cx, int cy, int size, uint32_t color) {
  int i;
  for (i = -size; i <= size; i++) {
    int half = size - (i < 0 ? -i : i);
    vg_draw_rectangle(cx - half, cy + i, 2 * half + 1, 1, color);
  }
}

static void draw_delivered_ticket(Game *game, int x, int y) {
  int stars = game->last_stars;
  int sc    = game->last_score_10;
  uint32_t gold = rgb(255, 200, 0);
  uint32_t grey = rgb(90, 78, 58);

  draw_string(x + 52, y + 22, "PEDIDO:", 2, rgb(94, 59, 34));

  vg_draw_rectangle(x + 14, y + 54, 152, 1, rgb(141, 90, 46));

  draw_diamond(x + 64,  y + 72, 8, stars >= 1 ? gold : grey);
  draw_diamond(x + 89,  y + 72, 8, stars >= 2 ? gold : grey);
  draw_diamond(x + 114, y + 72, 8, stars >= 3 ? gold : grey);

  draw_string(x + 18, y + 100, "PONTOS", 1, rgb(141, 90, 46));

  draw_char(x + 70, y + 114, (char)('0' + sc / 10), 3, rgb(30, 22, 16));
  vg_draw_rectangle(x + 89, y + 132, 3, 3, rgb(30, 22, 16));
  draw_char(x + 93, y + 114, (char)('0' + sc % 10), 3, rgb(30, 22, 16));

  vg_draw_rectangle(x + 14, y + 150, 152, 1, rgb(141, 90, 46));

  draw_string(x + 18, y + 160, "INICIO", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 174, game->order_time_str, 2, rgb(30, 22, 16));

  draw_string(x + 18, y + 200, "ENTREGA", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 214, game->delivery_time_str, 2, rgb(30, 22, 16));
}

void draw_order_ticket(Game *game, int x, int y) {
  char buf[12];
  int i;

  if (drawOrderTicketSprite(x, y, 180, 434) != 0)
    draw_panel(x, y, 180, 434, rgb(115, 76, 43));

  if (game->playing_state == PLAYING_DELIVERED) {
    draw_delivered_ticket(game, x, y);
    return;
  }

  draw_string(x + 52, y + 22, "PEDIDO:", 2, rgb(94, 59, 34));
  draw_string(x + 18, y + 62, "CLIENTE:", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 78, game->order.name, 2, rgb(30, 22, 16));

  draw_string(x + 18, y + 120, "MOLHO:", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 137, SAUCE_NAMES[game->order.sauce], 1, rgb(30, 22, 16));

  draw_string(x + 18, y + 176, "TOPPINGS:", 1, rgb(141, 90, 46));
  for (i = 0; i < 3; i++)
    draw_string(x + 18, y + 194 + i * 20, TOPPING_NAMES[game->order.toppings[i]], 1, rgb(30, 22, 16));

  {
    int cs = game->order.cook_seconds;
    int bi = 0;
    if (cs >= 10) buf[bi++] = (char)('0' + cs / 10);
    buf[bi++] = (char)('0' + cs % 10);
    buf[bi++]=' '; buf[bi++]='S'; buf[bi++]='E'; buf[bi++]='G'; buf[bi++]='U';
    buf[bi++]='N'; buf[bi++]='D'; buf[bi++]='O'; buf[bi++]='S'; buf[bi]= '\0';
  }
  draw_string(x + 18, y + 285, "FORNO:", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 303, buf, 2, rgb(30, 22, 16));

  buf[0] = '0' + game->order.slices;
  buf[1]=' '; buf[2]='F'; buf[3]='A'; buf[4]='T'; buf[5]='I';
  buf[6]='A'; buf[7]='S'; buf[8]='\0';
  draw_string(x + 18, y + 354, "CORTE:", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 367, buf, 2, rgb(30, 22, 16));
}

void draw_button_label(int bx, int by, int bw, int bh, const char *s, int scale, uint32_t color) {
  int tx = bx + (bw - (int)strlen(s) * 6 * scale) / 2;
  int ty = by + (bh - 7 * scale) / 2;
  draw_string(tx, ty, s, scale, color);
}

bool topping_selected(Game *game, int t) {
  return game->active_topping == t;
}

static uint32_t darken(uint32_t color, int pct) {
  uint8_t r = (uint8_t)(((color >> 16) & 0xFF) * (100 - pct) / 100);
  uint8_t g = (uint8_t)(((color >>  8) & 0xFF) * (100 - pct) / 100);
  uint8_t b = (uint8_t)(((color      ) & 0xFF) * (100 - pct) / 100);
  return rgb(r, g, b);
}

void draw_pizza(Game *game, int cx, int cy, int r) {
  int i, dot_r, darken_pct, overtime, has_cheese;
  uint32_t sauce_col;

  darken_pct = 0;
  if (game->pizza_in_oven) {
    overtime = game->oven_ticks - game->order.cook_seconds * 60;
    if      (overtime > 5 * 60) darken_pct = 65;
    else if (overtime > 3 * 60) darken_pct = 35;
    else if (overtime >= 0)     darken_pct = 15;
  }

  draw_circle(cx, cy, r, darken(rgb(168, 128, 55), darken_pct));
  if (game->selected_sauce >= 0) {
    sauce_col = game->selected_sauce == 1 ? rgb(245, 235, 180) : rgb(190, 45, 35);
    draw_circle(cx, cy, r * 88 / 105, darken(sauce_col, darken_pct));
  }

  has_cheese = 0;
  for (i = 0; i < game->num_placements; i++) {
    if (game->topping_placements[i].type == 4) { has_cheese = 1; break; }
  }
  if (has_cheese)
    draw_circle(cx, cy, r * 83 / 105, darken(TOPPING_COLORS[4], darken_pct));

  dot_r = r * 12 / 105;
  if (dot_r < 1) dot_r = 1;
  for (i = 0; i < game->num_placements; i++) {
    uint32_t color;
    if (game->topping_placements[i].type == 4) continue;
    color = darken(TOPPING_COLORS[game->topping_placements[i].type], darken_pct);
    draw_circle(cx + game->topping_placements[i].dx,
                cy + game->topping_placements[i].dy,
                dot_r, color);
  }
}

static void draw_line_seg(int x0, int y0, int x1, int y1, uint32_t color) {
  int dx, dy, sx, sy, ax, ay, err, e2;
  dx = x1 - x0; dy = y1 - y0;
  sx = dx > 0 ? 1 : (dx < 0 ? -1 : 0);
  sy = dy > 0 ? 1 : (dy < 0 ? -1 : 0);
  ax = dx < 0 ? -dx : dx;
  ay = dy < 0 ? -dy : dy;
  err = ax - ay;
  for (;;) {
    vg_draw_rectangle(x0 - 1, y0 - 1, 3, 3, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 > -ay) { err -= ay; x0 += sx; }
    if (e2 <  ax) { err += ax; y0 += sy; }
  }
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

void draw_pizza_cuts(Game *game, int cx, int cy, int r) {
  int N = game->order.slices;
  int i, adx, ady, bdx, bdy, pdx, pdy, dot_r;
  for (i = 0; i < game->num_cut_lines; i++) {
    cut_point_offset(N, game->cut_lines[i].a, r, &adx, &ady);
    cut_point_offset(N, game->cut_lines[i].b, r, &bdx, &bdy);
    draw_line_seg(cx + adx, cy + ady, cx + bdx, cy + bdy, rgb(20, 20, 20));
  }
  for (i = 0; i < N; i++) {
    cut_point_offset(N, i, r, &pdx, &pdy);
    dot_r = (i == game->cut_selected) ? 10 : 7;
    draw_circle(cx + pdx, cy + pdy, dot_r,
                (i == game->cut_selected) ? rgb(255, 200, 0) : rgb(20, 20, 20));
  }
}

#pragma clang optimize on
