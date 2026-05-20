#include "game.h"

#include "graphics.h"

#define SCREEN_W 800
#define SCREEN_H 600

#define ESC_BREAK 0x81
#define ENTER_BREAK 0x9C
#define KEY_1_BREAK 0x82
#define KEY_2_BREAK 0x83
#define KEY_3_BREAK 0x84
#define KEY_4_BREAK 0x85
#define KEY_5_BREAK 0x86
#define KEY_6_BREAK 0x87
#define KEY_8_BREAK 0x89

static uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) | (g << 8) | b;
}

static int abs_int(int value) {
  return value < 0 ? -value : value;
}

static void make_order(Game *game) {
  int n = game->order_number;

  game->order.sauce = n % 2;
  game->order.topping = n % 3;
  game->order.cook_seconds = 5 + (n % 4);
  game->order.slices = (n % 2 == 0) ? 6 : 8;

  game->selected_sauce = -1;
  game->selected_topping = -1;
  game->oven_ticks = 0;
  game->selected_slices = 0;
  game->last_points = 0;
}

void game_init(Game *game) {
  game->state = STATE_ORDER;
  game->running = true;
  game->tick = 0;
  game->order_number = 0;
  game->score = 0;
  game->mouse_x = SCREEN_W / 2;
  game->mouse_y = SCREEN_H / 2;
  game->mouse_left_click = false;
  make_order(game);
}

bool game_is_running(Game *game) {
  return game->running;
}

static void serve_pizza(Game *game) {
  int points = 100;
  int cooked_seconds = game->oven_ticks / GAME_FPS;

  if (game->selected_sauce != game->order.sauce) points -= 25;
  if (game->selected_topping != game->order.topping) points -= 25;

  points -= abs_int(cooked_seconds - game->order.cook_seconds) * 10;
  points -= abs_int(game->selected_slices - game->order.slices) * 8;

  if (points < 0) points = 0;

  game->last_points = points;
  game->score += points;
  game->state = STATE_SCORE;
}

static bool mouse_inside(Game *game, int x, int y, int w, int h) {
  return game->mouse_x >= x && game->mouse_x < x + w &&
         game->mouse_y >= y && game->mouse_y < y + h;
}

static void handle_click(Game *game) {
  if (!game->mouse_left_click) return;

  if (game->state == STATE_ORDER) {
    if (mouse_inside(game, 285, 460, 230, 70)) game->state = STATE_PREPARE;
  } else if (game->state == STATE_PREPARE) {
    if (mouse_inside(game, 95, 420, 130, 70)) game->selected_sauce = 0;
    if (mouse_inside(game, 245, 420, 130, 70)) game->selected_sauce = 1;
    if (mouse_inside(game, 425, 420, 80, 70)) game->selected_topping = 0;
    if (mouse_inside(game, 525, 420, 80, 70)) game->selected_topping = 1;
    if (mouse_inside(game, 625, 420, 80, 70)) game->selected_topping = 2;
    if (mouse_inside(game, 285, 510, 230, 55)) game->state = STATE_OVEN;
  } else if (game->state == STATE_OVEN) {
    if (mouse_inside(game, 285, 500, 230, 60)) game->state = STATE_CUT;
  } else if (game->state == STATE_CUT) {
    if (mouse_inside(game, 135, 440, 110, 60)) game->selected_slices = 4;
    if (mouse_inside(game, 265, 440, 110, 60)) game->selected_slices = 6;
    if (mouse_inside(game, 395, 440, 110, 60)) game->selected_slices = 8;
    if (mouse_inside(game, 555, 440, 110, 60)) serve_pizza(game);
  } else if (game->state == STATE_SCORE) {
    if (mouse_inside(game, 285, 475, 230, 65)) {
      game->order_number++;
      make_order(game);
      game->state = STATE_ORDER;
    }
  }

  game->mouse_left_click = false;
}

void game_handle_keyboard(Game *game, uint8_t scancode) {
  if (scancode == ESC_BREAK) {
    game->running = false;
    return;
  }

  if (game->state == STATE_ORDER && scancode == ENTER_BREAK) {
    game->state = STATE_PREPARE;
  } else if (game->state == STATE_PREPARE) {
    if (scancode == KEY_1_BREAK) game->selected_sauce = 0;
    if (scancode == KEY_2_BREAK) game->selected_sauce = 1;
    if (scancode == KEY_3_BREAK) game->selected_topping = 0;
    if (scancode == KEY_4_BREAK) game->selected_topping = 1;
    if (scancode == KEY_5_BREAK) game->selected_topping = 2;
    if (scancode == ENTER_BREAK) game->state = STATE_OVEN;
  } else if (game->state == STATE_OVEN && scancode == ENTER_BREAK) {
    game->state = STATE_CUT;
  } else if (game->state == STATE_CUT) {
    if (scancode == KEY_4_BREAK) game->selected_slices = 4;
    if (scancode == KEY_6_BREAK) game->selected_slices = 6;
    if (scancode == KEY_8_BREAK) game->selected_slices = 8;
    if (scancode == ENTER_BREAK) serve_pizza(game);
  } else if (game->state == STATE_SCORE && scancode == ENTER_BREAK) {
    game->order_number++;
    make_order(game);
    game->state = STATE_ORDER;
  }
}

void game_handle_mouse_packet(Game *game, struct packet *packet) {
  game->mouse_x += packet->delta_x;
  game->mouse_y -= packet->delta_y;

  if (game->mouse_x < 0) game->mouse_x = 0;
  if (game->mouse_y < 0) game->mouse_y = 0;
  if (game->mouse_x >= SCREEN_W) game->mouse_x = SCREEN_W - 1;
  if (game->mouse_y >= SCREEN_H) game->mouse_y = SCREEN_H - 1;

  if (packet->lb) game->mouse_left_click = true;
}

void game_update(Game *game) {
  game->tick++;

  if (game->state == STATE_OVEN) {
    game->oven_ticks++;
  }

  handle_click(game);
}

static void draw_circle(int cx, int cy, int radius, uint32_t color) {
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
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
  int temp = value;

  if (temp == 0) width = 1;
  while (temp > 0) {
    width++;
    temp /= 10;
  }

  for (int i = width - 1; i >= 0; i--) {
    int digit = value % 10;
    int px = x + i * 22;

    if (digit != 1 && digit != 4) vg_draw_rectangle(px, y, 16, 4, color);
    if (digit != 1 && digit != 2 && digit != 3 && digit != 7) vg_draw_rectangle(px, y, 4, 18, color);
    if (digit != 5 && digit != 6) vg_draw_rectangle(px + 12, y, 4, 18, color);
    if (digit != 0 && digit != 1 && digit != 7) vg_draw_rectangle(px, y + 17, 16, 4, color);
    if (digit == 0 || digit == 2 || digit == 6 || digit == 8) vg_draw_rectangle(px, y + 20, 4, 18, color);
    if (digit != 2) vg_draw_rectangle(px + 12, y + 20, 4, 18, color);
    if (digit != 1 && digit != 4 && digit != 7) vg_draw_rectangle(px, y + 36, 16, 4, color);

    value /= 10;
  }
}

static void draw_order_ticket(Game *game) {
  draw_panel(40, 80, 210, 210, rgb(115, 76, 43));
  vg_draw_rectangle(75, 115, 70 + game->order.sauce * 70, 25, rgb(190, 50, 40));
  vg_draw_rectangle(75, 165, 45 + game->order.topping * 35, 25, rgb(60, 140, 70));
  vg_draw_rectangle(75, 215, game->order.cook_seconds * 12, 25, rgb(220, 130, 35));
  vg_draw_rectangle(75, 255, game->order.slices * 12, 18, rgb(90, 120, 200));
}

static void draw_pizza(Game *game) {
  draw_circle(400, 235, 105, rgb(222, 175, 82));
  draw_circle(400, 235, 88, game->selected_sauce == 1 ? rgb(245, 235, 180) : rgb(190, 45, 35));

  if (game->selected_topping >= 0) {
    uint32_t topping_color = rgb(95, 45, 25);
    if (game->selected_topping == 1) topping_color = rgb(40, 130, 60);
    if (game->selected_topping == 2) topping_color = rgb(225, 225, 210);

    draw_circle(360, 205, 12, topping_color);
    draw_circle(430, 205, 12, topping_color);
    draw_circle(390, 250, 12, topping_color);
    draw_circle(450, 265, 12, topping_color);
    draw_circle(345, 270, 12, topping_color);
  }
}

void game_draw(Game *game) {
  int oven_bar_width = 0;

  vg_clear_buffer(rgb(215, 220, 205));
  vg_draw_rectangle(0, 0, SCREEN_W, 60, rgb(165, 45, 40));
  vg_draw_rectangle(0, 560, SCREEN_W, 40, rgb(75, 55, 45));

  draw_number(25, 12, game->score, rgb(255, 240, 160));
  draw_number(700, 12, game->state + 1, rgb(255, 240, 160));

  draw_order_ticket(game);

  if (game->state == STATE_ORDER) {
    draw_panel(300, 120, 300, 250, rgb(115, 76, 43));
    draw_circle(400, 230, 65, rgb(238, 190, 90));
    draw_button(285, 460, 230, 70, false, rgb(235, 180, 70));
  } else if (game->state == STATE_PREPARE) {
    draw_pizza(game);
    draw_button(95, 420, 130, 70, game->selected_sauce == 0, rgb(190, 45, 35));
    draw_button(245, 420, 130, 70, game->selected_sauce == 1, rgb(245, 235, 180));
    draw_button(425, 420, 80, 70, game->selected_topping == 0, rgb(95, 45, 25));
    draw_button(525, 420, 80, 70, game->selected_topping == 1, rgb(40, 130, 60));
    draw_button(625, 420, 80, 70, game->selected_topping == 2, rgb(225, 225, 210));
    draw_button(285, 510, 230, 55, false, rgb(220, 130, 35));
  } else if (game->state == STATE_OVEN) {
    oven_bar_width = game->oven_ticks * 430 / (game->order.cook_seconds * GAME_FPS * 2);
    if (oven_bar_width > 430) oven_bar_width = 430;

    vg_draw_rectangle(250, 135, 300, 210, rgb(80, 80, 80));
    vg_draw_rectangle(275, 160, 250, 160, rgb(230, 120, 45));
    draw_pizza(game);
    vg_draw_rectangle(180, 395, 440, 35, rgb(80, 80, 80));
    vg_draw_rectangle(185, 400, oven_bar_width, 25, rgb(235, 180, 70));
    draw_button(285, 500, 230, 60, false, rgb(235, 180, 70));
  } else if (game->state == STATE_CUT) {
    draw_pizza(game);
    draw_number(390, 355, game->selected_slices, rgb(80, 50, 30));
    draw_button(135, 440, 110, 60, game->selected_slices == 4, rgb(235, 180, 70));
    draw_button(265, 440, 110, 60, game->selected_slices == 6, rgb(235, 180, 70));
    draw_button(395, 440, 110, 60, game->selected_slices == 8, rgb(235, 180, 70));
    draw_button(555, 440, 110, 60, false, rgb(90, 160, 90));
  } else {
    draw_panel(275, 130, 250, 260, rgb(115, 76, 43));
    draw_number(360, 215, game->last_points, rgb(60, 120, 70));
    draw_button(285, 475, 230, 65, false, rgb(235, 180, 70));
  }

  vg_draw_rectangle(game->mouse_x, game->mouse_y, 12, 3, rgb(20, 20, 20));
  vg_draw_rectangle(game->mouse_x, game->mouse_y, 3, 12, rgb(20, 20, 20));
  vg_swap_buffer();
}
