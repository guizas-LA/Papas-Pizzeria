#include "draw_elements.h"
#include "draw_utils.h"
#include "sprites.h"
#include <string.h>

static const char *SAUCE_NAMES[]   = { "MOLHO DE TOMATE", "MOLHO BRANCO" };
static const char *TOPPING_NAMES[] = { "COGUMELO", "PEPERONI", "FIAMBRE", "ANANÁS", "QUEIJO", "AZEITONAS" };
const uint32_t TOPPING_COLORS[] = {0x8C6432, 0xBE2814, 0xE68282, 0xF0C832, 0xF5E164, 0x284619 };
static const int DOT_DX[] = {-40,  30, -10,  50, -55, 15};
static const int DOT_DY[] = {-30, -30,  15,  30,  35, 50};


void draw_order_ticket(Game *game, int x, int y) {
  char buf[12];
  int i;

  if (drawOrderTicketSprite(x, y, 180, 434) != 0) {
    draw_panel(x, y, 180, 434, rgb(115, 76, 43));
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
  buf[1] = ' '; buf[2] = 'S'; buf[3] = 'E'; buf[4] = 'G'; buf[5] = 'U';
  buf[6] = 'N'; buf[7] = 'D'; buf[8] = 'O'; buf[9] = 'S'; buf[10] = '\0';
  draw_string(x + 18, y + 285, "FORNO:", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 303, buf, 2, rgb(30, 22, 16));

  buf[0] = '0' + game->order.slices;
  buf[1] = ' '; buf[2] = 'F'; buf[3] = 'A'; buf[4] = 'T'; buf[5] = 'I';
  buf[6] = 'A'; buf[7] = 'S'; buf[8] = '\0';
  draw_string(x + 18, y + 354, "CORTE:", 1, rgb(141, 90, 46));
  draw_string(x + 18, y + 372, buf, 2, rgb(30, 22, 16));
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
  draw_circle(cx, cy, r, rgb(222, 175, 82));
  if (game->selected_sauce >= 0)
    draw_circle(cx, cy, r * 88 / 105, game->selected_sauce == 1 ? rgb(245, 235, 180) : rgb(190, 45, 35));
  dot_r = r * 12 / 105;
  if (dot_r < 1) dot_r = 1;
  for (i = 0; i < game->num_selected_toppings; i++) {
    uint32_t color = TOPPING_COLORS[game->selected_toppings[i]];
    draw_circle(cx + DOT_DX[i*2]   * r / 105, cy + DOT_DY[i*2]   * r / 105, dot_r, color);
    draw_circle(cx + DOT_DX[i*2+1] * r / 105, cy + DOT_DY[i*2+1] * r / 105, dot_r, color);
  }
}


