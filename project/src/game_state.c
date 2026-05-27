#include "game_state.h"
#include <string.h>

#define SCREEN_W 800
#define SCREEN_H 600

#define ESC_BREAK      0x81
#define ENTER_BREAK    0x9C
#define BACKSPACE_MAKE 0x0E
#define KEY_1_BREAK    0x82
#define KEY_2_BREAK    0x83
#define KEY_3_BREAK    0x84
#define KEY_4_BREAK    0x85
#define KEY_5_BREAK    0x86
#define KEY_6_BREAK    0x87
#define KEY_8_BREAK    0x89

#define EXTENDED_PREFIX   0xE0
#define ARROW_UP_MAKE     0x48
#define ARROW_DOWN_MAKE   0x50

#define NUM_MENU_OPTIONS 3

static const char *CUSTOMER_NAMES[] = { "ANA", "BOUCA", "GUI", "BRUNO", "DAGA" };
#define NUM_CUSTOMERS 5

static int abs_int(int value) {
  return value < 0 ? -value : value;
}

static char scancode_to_char(uint8_t sc) {
  switch (sc) {
    case 0x10: return 'Q'; case 0x11: return 'W'; case 0x12: return 'E';
    case 0x13: return 'R'; case 0x14: return 'T'; case 0x15: return 'Y';
    case 0x16: return 'U'; case 0x17: return 'I'; case 0x18: return 'O';
    case 0x19: return 'P'; case 0x1E: return 'A'; case 0x1F: return 'S';
    case 0x20: return 'D'; case 0x21: return 'F'; case 0x22: return 'G';
    case 0x23: return 'H'; case 0x24: return 'J'; case 0x25: return 'K';
    case 0x26: return 'L'; case 0x2C: return 'Z'; case 0x2D: return 'X';
    case 0x2E: return 'C'; case 0x2F: return 'V'; case 0x30: return 'B';
    case 0x31: return 'N'; case 0x32: return 'M';
    default: return 0;
  }
}

static void make_order(Game *game) {
  int n = game->order_number;
  const char *name = CUSTOMER_NAMES[n % NUM_CUSTOMERS];

  game->order.sauce        = n % 2;
  game->order.topping      = n % 3;
  game->order.cook_seconds = 5 + (n % 4);
  game->order.slices       = (n % 2 == 0) ? 6 : 8;
  strncpy(game->order.name, name, 7);
  game->order.name[7] = '\0';

  game->selected_sauce   = -1;
  game->selected_topping = -1;
  game->oven_ticks       = 0;
  game->pizza_in_oven    = false;
  game->selected_slices  = 0;
  game->typed_name[0]    = '\0';
  game->typed_len        = 0;
}

void game_init(Game *game) {
  game->state         = GAME_STATE_MENU;
  game->playing_state = PLAYING_TAKE_ORDER;
  game->running       = true;
  game->tick          = 0;
  game->order_number  = 0;
  game->score         = 0;
  game->last_points   = 0;
  game->mouse_x       = SCREEN_W / 2;
  game->mouse_y       = SCREEN_H / 2;
  game->mouse_left_click = false;
  game->menu_option   = 0;
  make_order(game);
}

bool game_is_running(Game *game) {
  return game->running;
}

static void serve_pizza(Game *game) {
  int points = 100;
  int cooked_seconds = game->oven_ticks / GAME_FPS;

  if (game->selected_sauce   != game->order.sauce)   points -= 25;
  if (game->selected_topping != game->order.topping) points -= 25;

  points -= abs_int(cooked_seconds - game->order.cook_seconds) * 10;
  points -= abs_int(game->selected_slices - game->order.slices) * 8;

  if (strcmp(game->typed_name, game->order.name) != 0) points -= 20;

  if (points < 0) points = 0;

  game->last_points = points;
  game->score += points;
  game->order_number++;
  make_order(game);
  game->playing_state = PLAYING_TAKE_ORDER;
}

static bool mouse_inside(Game *game, int x, int y, int w, int h) {
  return game->mouse_x >= x && game->mouse_x < x + w &&
         game->mouse_y >= y && game->mouse_y < y + h;
}

static void handle_click(Game *game) {
  if (!game->mouse_left_click) return;

  switch (game->state) {
    case GAME_STATE_MENU:
      if (mouse_inside(game, 285, 355, 230, 70)) {
        game->state         = GAME_STATE_PLAYING;
        game->playing_state = PLAYING_TAKE_ORDER;
      } else if (mouse_inside(game, 285, 445, 230, 70)) {
        /* OPCOES — no action */
      } else if (mouse_inside(game, 285, 530, 230, 70)) {
        game->running = false;
      }
      break;

    case GAME_STATE_PLAYING:
      switch (game->playing_state) {
        case PLAYING_TAKE_ORDER:
          if (mouse_inside(game, 285, 460, 230, 70))
            game->playing_state = PLAYING_PREPARE_PIZZA;
          break;

        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            if (mouse_inside(game, 95,  420, 130, 70)) game->selected_sauce   = 0;
            if (mouse_inside(game, 245, 420, 130, 70)) game->selected_sauce   = 1;
            if (mouse_inside(game, 425, 420, 80,  70)) game->selected_topping = 0;
            if (mouse_inside(game, 525, 420, 80,  70)) game->selected_topping = 1;
            if (mouse_inside(game, 625, 420, 80,  70)) game->selected_topping = 2;
            if (mouse_inside(game, 285, 510, 230, 55)) game->pizza_in_oven    = true;
          }
          else {
            if (mouse_inside(game, 285, 500, 230, 60))
              game->playing_state = PLAYING_CUT;
          }
          break;

        case PLAYING_CUT:
          if (mouse_inside(game, 215, 440, 110, 60)) game->selected_slices = 4;
          if (mouse_inside(game, 345, 440, 110, 60)) game->selected_slices = 6;
          if (mouse_inside(game, 475, 440, 110, 60)) game->selected_slices = 8;
          if (mouse_inside(game, 285, 520, 230, 50))
            game->playing_state = PLAYING_SERVE;
          break;

        case PLAYING_SERVE:
          if (mouse_inside(game, 285, 460, 230, 60))
            serve_pizza(game);
          break;
      }
      break;
  }

  game->mouse_left_click = false;
}

void game_handle_keyboard(Game *game, uint8_t scancode) {
  static bool extended = false;

  if (scancode == ESC_BREAK) {
    game->running = false;
    return;
  }

  if (scancode == EXTENDED_PREFIX) {
    extended = true;
    return;
  }

  if (extended) {
    extended = false;
    if (game->state == GAME_STATE_MENU) {
      if (scancode == ARROW_UP_MAKE) {
        game->menu_option = (game->menu_option + NUM_MENU_OPTIONS - 1) % NUM_MENU_OPTIONS;
      } else if (scancode == ARROW_DOWN_MAKE) {
        game->menu_option = (game->menu_option + 1) % NUM_MENU_OPTIONS;
      }
    }
    return;
  }

  switch (game->state) {
    case GAME_STATE_MENU:
      if (scancode == ENTER_BREAK) {
        switch (game->menu_option) {
          case 0:
            game->state         = GAME_STATE_PLAYING;
            game->playing_state = PLAYING_TAKE_ORDER;
            break;
          case 1:
            /* OPCOES — no action */
            break;
          case 2:
            game->running = false;
            break;
        }
      }
      break;

    case GAME_STATE_PLAYING:
      switch (game->playing_state) {
        case PLAYING_TAKE_ORDER:
          if (scancode == ENTER_BREAK)
            game->playing_state = PLAYING_PREPARE_PIZZA;
          break;
        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            switch (scancode) {
              case KEY_1_BREAK: game->selected_sauce   = 0; break;
              case KEY_2_BREAK: game->selected_sauce   = 1; break;
              case KEY_3_BREAK: game->selected_topping = 0; break;
              case KEY_4_BREAK: game->selected_topping = 1; break;
              case KEY_5_BREAK: game->selected_topping = 2; break;
              case ENTER_BREAK: game->pizza_in_oven    = true; break;
              default: break;
            }
          }
          else if (scancode == ENTER_BREAK) {
            game->playing_state = PLAYING_CUT;
          }
          break;

        case PLAYING_CUT:
          switch (scancode) {
            case KEY_4_BREAK: game->selected_slices = 4; break;
            case KEY_6_BREAK: game->selected_slices = 6; break;
            case KEY_8_BREAK: game->selected_slices = 8; break;
            case ENTER_BREAK: game->playing_state = PLAYING_SERVE; break;
            default: break;
          }
          break;
          
        case PLAYING_SERVE: {
          char c = scancode_to_char(scancode);
          if (c != 0 && game->typed_len < 7) {
            game->typed_name[game->typed_len++] = c;
            game->typed_name[game->typed_len]   = '\0';
          }
          if (scancode == BACKSPACE_MAKE && game->typed_len > 0) {
            game->typed_name[--game->typed_len] = '\0';
          }
          if (scancode == ENTER_BREAK) serve_pizza(game);
          break;
        }
      }
      break;
  }
}

void game_handle_mouse_packet(Game *game, struct packet *packet) {
  game->mouse_x += packet->delta_x;
  game->mouse_y -= packet->delta_y;

  if (game->mouse_x < 0)        game->mouse_x = 0;
  if (game->mouse_y < 0)        game->mouse_y = 0;
  if (game->mouse_x >= SCREEN_W) game->mouse_x = SCREEN_W - 1;
  if (game->mouse_y >= SCREEN_H) game->mouse_y = SCREEN_H - 1;

  if (packet->lb) game->mouse_left_click = true;
}

void game_update(Game *game) {
  game->tick++;

  switch (game->state) {
    case GAME_STATE_PLAYING:
      if (game->playing_state == PLAYING_PREPARE_PIZZA && game->pizza_in_oven)
        game->oven_ticks++;
      break;
    default:
      break;
  }

  handle_click(game);
}
