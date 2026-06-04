#include "game_state.h"
#include "rtc.h"
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
#define KEY_7_BREAK    0x88
#define KEY_8_BREAK    0x89

#define EXTENDED_PREFIX   0xE0
#define ARROW_UP_MAKE     0x48
#define ARROW_DOWN_MAKE   0x50

#define NUM_MENU_OPTIONS 3

static const char *CUSTOMER_NAMES[] = { "ANA", "BOUÇA", "GUI", "BRUNO", "DAGA" };
#define NUM_CUSTOMERS 5

static const int TOPPING_COMBOS[20][3] = {{0,1,2}, {0,1,3}, {0,1,4}, {0,1,5},{0,2,3}, {0,2,4}, {0,2,5},{0,3,4}, {0,3,5}, {0,4,5},{1,2,3}, {1,2,4}, {1,2,5},{1,3,4}, {1,3,5}, {1,4,5},{2,3,4}, {2,3,5}, {2,4,5},{3,4,5}};

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
  int combo = n % 20;
  const char *name = CUSTOMER_NAMES[n % NUM_CUSTOMERS];

  game->order.sauce        = n % 2;
  game->order.toppings[0]  = TOPPING_COMBOS[combo][0];
  game->order.toppings[1]  = TOPPING_COMBOS[combo][1];
  game->order.toppings[2]  = TOPPING_COMBOS[combo][2];
  game->order.cook_seconds = 5 + (n % 4);
  game->order.slices       = (n % 2 == 0) ? 6 : 8;
  strncpy(game->order.name, name, 7);
  game->order.name[7] = '\0';

  game->selected_sauce          = -1;
  game->num_selected_toppings   = 0;
  game->oven_ticks              = 0;
  game->pizza_in_oven           = false;
  game->selected_slices         = 0;
  game->typed_name[0]           = '\0';
  game->typed_len               = 0;
}

static void toggle_topping(Game *game, int t) {
  int i;
  for (i = 0; i < game->num_selected_toppings; i++) {
    if (game->selected_toppings[i] == t) {
      game->selected_toppings[i] = game->selected_toppings[--game->num_selected_toppings];
      return;
    }
  }
  if (game->num_selected_toppings < 3)
    game->selected_toppings[game->num_selected_toppings++] = t;
}

static int oven_target_ticks(Game *game) {
  return game->order.cook_seconds * GAME_FPS;
}

static bool oven_ready(Game *game) {
  return game->oven_ticks >= oven_target_ticks(game);
}

static void start_oven(Game *game) {
  game->oven_ticks = 0;
  game->pizza_in_oven = true;
}

void game_init(Game *game) {
  game->state         = GAME_STATE_MENU;
  game->playing_state = PLAYING_TAKE_ORDER;
  game->running       = true;
  game->tick          = 0;
  game->order_number  = 0;
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
  game->order_number++;
  make_order(game);
  game->playing_state = PLAYING_TAKE_ORDER;
}

static void try_deliver(Game *game) {
  rtc_time_t t;
  if (strcmp(game->typed_name, game->order.name) != 0) {
    game->typed_name[0] = '\0';
    game->typed_len = 0;
    return;
  }
  rtc_read_time(&t);
  game->delivery_time_str[0] = (char)('0' + t.hour / 10);
  game->delivery_time_str[1] = (char)('0' + t.hour % 10);
  game->delivery_time_str[2] = ':';
  game->delivery_time_str[3] = (char)('0' + t.min / 10);
  game->delivery_time_str[4] = (char)('0' + t.min % 10);
  game->delivery_time_str[5] = ':';
  game->delivery_time_str[6] = (char)('0' + t.sec / 10);
  game->delivery_time_str[7] = (char)('0' + t.sec % 10);
  game->delivery_time_str[8] = '\0';
  game->playing_state = PLAYING_DELIVERED;
}

static bool mouse_inside(Game *game, int x, int y, int w, int h) {
  return game->mouse_x >= x && game->mouse_x < x + w && game->mouse_y >= y && game->mouse_y < y + h;}

static void handle_click(Game *game) {
  if (!game->mouse_left_click) return;

  switch (game->state) {
    case GAME_STATE_MENU:
      if (mouse_inside(game, 510, 355, 230, 70)) {
        game->state = GAME_STATE_PLAYING;
        game->playing_state = PLAYING_TAKE_ORDER;
      }
      else if (mouse_inside(game, 510, 445, 230, 70)) {
      }
      else if (mouse_inside(game, 510, 530, 230, 70)) {
        game->running = false;
      }
      break;

    case GAME_STATE_PLAYING:
      switch (game->playing_state) {
        case PLAYING_TAKE_ORDER:
          if (mouse_inside(game, 605, 495, 174, 40))
            game->playing_state = PLAYING_PREPARE_PIZZA;
          break;

        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            if (mouse_inside(game,  10, 480, 120, 50)) game->selected_sauce = 0;
            if (mouse_inside(game,  10, 540, 120, 50)) game->selected_sauce = 1;
            if (mouse_inside(game, 195, 480,  80, 50)) toggle_topping(game, 0);
            if (mouse_inside(game, 285, 480,  80, 50)) toggle_topping(game, 1);
            if (mouse_inside(game, 375, 480,  80, 50)) toggle_topping(game, 2);
            if (mouse_inside(game, 195, 540,  80, 50)) toggle_topping(game, 3);
            if (mouse_inside(game, 285, 540,  80, 50)) toggle_topping(game, 4);
            if (mouse_inside(game, 375, 540,  80, 50)) toggle_topping(game, 5);
            if (mouse_inside(game, 605, 495, 174, 40)) start_oven(game);
          }
          else {
            if (oven_ready(game) && mouse_inside(game, 605, 495, 174, 40))
              game->playing_state = PLAYING_CUT;
          }
          break;

        case PLAYING_CUT:
          if (mouse_inside(game, 115, 40, 110, 60)) game->selected_slices = 4;
          if (mouse_inside(game, 245, 40, 110, 60)) game->selected_slices = 6;
          if (mouse_inside(game, 375, 40, 110, 60)) game->selected_slices = 8;
          if (mouse_inside(game, 605, 495, 174, 40))
            game->playing_state = PLAYING_SERVE;
          break;

        case PLAYING_SERVE:
          if (mouse_inside(game, 605, 495, 174, 40))
            try_deliver(game);
          break;

        case PLAYING_DELIVERED:
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
            game->state = GAME_STATE_PLAYING;
            game->playing_state = PLAYING_TAKE_ORDER;
            break;
          case 1:
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
              case KEY_1_BREAK: game->selected_sauce = 0;       break;
              case KEY_2_BREAK: game->selected_sauce = 1;       break;
              case KEY_3_BREAK: toggle_topping(game, 0);        break;
              case KEY_4_BREAK: toggle_topping(game, 1);        break;
              case KEY_5_BREAK: toggle_topping(game, 2);        break;
              case KEY_6_BREAK: toggle_topping(game, 3);        break;
              case KEY_7_BREAK: toggle_topping(game, 4);        break;
              case KEY_8_BREAK: toggle_topping(game, 5);        break;
              case ENTER_BREAK: start_oven(game);               break;
              default: break;
            }
          }
          else if (scancode == ENTER_BREAK && oven_ready(game)) {
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
          if (scancode == ENTER_BREAK) try_deliver(game);
          break;
        }

        case PLAYING_DELIVERED:
          if (scancode == ENTER_BREAK) serve_pizza(game);
          break;
      }
      break;
  }
}

void game_handle_mouse_packet(Game *game, struct packet *packet) {
  game->mouse_x += packet->delta_x;
  game->mouse_y -= packet->delta_y;

  if (game->mouse_x < 7) game->mouse_x = 7;
  if (game->mouse_y < 7) game->mouse_y = 7;
  if (game->mouse_x >= SCREEN_W - 7) game->mouse_x = SCREEN_W - 8;
  if (game->mouse_y >= SCREEN_H - 7) game->mouse_y = SCREEN_H - 8;

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
