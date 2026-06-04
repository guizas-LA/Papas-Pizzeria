#include "draw_elements.h"
#include "draw_utils.h"
#include "sprites.h"
#include <string.h>

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

static void draw_diamond(int cx, int cy, int size, uint32_t color) {
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

  buf[0] = '0' + game->order.cook_seconds;
  buf[1]=' '; buf[2]='S'; buf[3]='E'; buf[4]='G'; buf[5]='U';
  buf[6]='N'; buf[7]='D'; buf[8]='O'; buf[9]='S'; buf[10]='\0';
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

void draw_pizza(Game *game, int cx, int cy, int r) {
  int i, dot_r;
  draw_circle(cx, cy, r, rgb(168, 128, 55));
  if (game->selected_sauce >= 0)
    draw_circle(cx, cy, r * 88 / 105,
                game->selected_sauce == 1 ? rgb(245, 235, 180) : rgb(190, 45, 35));
  dot_r = r * 12 / 105;
  if (dot_r < 1) dot_r = 1;
  for (i = 0; i < game->num_placements; i++) {
    uint32_t color = TOPPING_COLORS[game->topping_placements[i].type];
    draw_circle(cx + game->topping_placements[i].dx,
                cy + game->topping_placements[i].dy,
                dot_r, color);
  }
}
