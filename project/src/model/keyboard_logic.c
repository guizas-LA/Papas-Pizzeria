#include "keyboard_logic.h"
#include "game_logic.h"

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

#define EXTENDED_PREFIX  0xE0
#define ARROW_UP_MAKE    0x48
#define ARROW_DOWN_MAKE  0x50

#define NUM_MENU_OPTIONS 3


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


void game_handle_keyboard(Game *game, uint8_t scancode) {
  static bool extended = false;

  if (scancode == ESC_BREAK) { game->running = false; return; }

  if (scancode == EXTENDED_PREFIX) { extended = true; return; }

  if (extended) {
    extended = false;
    if (game->state == GAME_STATE_MENU) {
      if (scancode == ARROW_UP_MAKE)
        game->menu_option = (game->menu_option + NUM_MENU_OPTIONS - 1) % NUM_MENU_OPTIONS;
      else if (scancode == ARROW_DOWN_MAKE)
        game->menu_option = (game->menu_option + 1) % NUM_MENU_OPTIONS;
    }
    return;
  }

  switch (game->state) {
    case GAME_STATE_MENU:
      if (scancode == ENTER_BREAK) {
        switch (game->menu_option) {
          case 0: game->state = GAME_STATE_PLAYING; game->playing_state = PLAYING_TAKE_ORDER; break;
          case 1: break;
          case 2: game->running = false; break;
        }
      }
      break;

    case GAME_STATE_PLAYING:
      switch (game->playing_state) {
        case PLAYING_TAKE_ORDER:
          if (scancode == ENTER_BREAK) {
            record_order_start(game);
            game->playing_state = PLAYING_PREPARE_PIZZA;
          }
          break;

        case PLAYING_PREPARE_PIZZA:
          if (!game->pizza_in_oven) {
            switch (scancode) {
              case KEY_1_BREAK: game->selected_sauce = 0;  break;
              case KEY_2_BREAK: game->selected_sauce = 1;  break;
              case KEY_3_BREAK: toggle_topping(game, 0);   break;
              case KEY_4_BREAK: toggle_topping(game, 1);   break;
              case KEY_5_BREAK: toggle_topping(game, 2);   break;
              case KEY_6_BREAK: toggle_topping(game, 3);   break;
              case KEY_7_BREAK: toggle_topping(game, 4);   break;
              case KEY_8_BREAK: toggle_topping(game, 5);   break;
              case ENTER_BREAK: start_oven(game);           break;
              default: break;
            }
          } 
          else if (scancode == ENTER_BREAK && oven_ready(game)) {
            game->playing_state = PLAYING_CUT;
          }
          break;

        case PLAYING_CUT:
          switch (scancode) {
            case KEY_4_BREAK: game->selected_slices = 4;              break;
            case KEY_6_BREAK: game->selected_slices = 6;              break;
            case KEY_8_BREAK: game->selected_slices = 8;              break;
            case ENTER_BREAK: game->playing_state = PLAYING_SERVE;    break;
            default: break;
          }
          break;

        case PLAYING_SERVE: {
          char c = scancode_to_char(scancode);
          if (c != 0 && game->typed_len < 7) {
            game->typed_name[game->typed_len++] = c;
            game->typed_name[game->typed_len]   = '\0';
          }
          if (scancode == BACKSPACE_MAKE && game->typed_len > 0)
            game->typed_name[--game->typed_len] = '\0';
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
