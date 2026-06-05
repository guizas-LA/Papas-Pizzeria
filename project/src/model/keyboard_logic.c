#include "keyboard_logic.h"
#include "game_logic.h"

#pragma clang optimize off

#define ESC_MAKE       0x01
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

#define NUM_MENU_OPTIONS     3
#define NUM_SETTINGS_OPTIONS 8


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
  static bool esc_in_play = false;

  if (game->tick < 120) return;

  if (scancode == ESC_MAKE) {
    esc_in_play = (game->state == GAME_STATE_PLAYING);
    return;
  }

  if (scancode == ESC_BREAK) {
    if (game->show_exit_popup) {
      game->show_exit_popup = false;
    } else if (game->show_back_popup) {
      game->show_back_popup = false;
    } else if (game->state == GAME_STATE_SETTINGS) {
      game->state = GAME_STATE_MENU;
    } else if (game->state == GAME_STATE_PLAYING && esc_in_play) {
      game->show_exit_popup = true;
    }
    esc_in_play = false;
    return;
  }

  if (game->show_exit_popup || game->show_back_popup) return;

  if (scancode == EXTENDED_PREFIX) { extended = true; return; }

  if (extended) {
    extended = false;
    if (game->state == GAME_STATE_MENU) {
      if (scancode == ARROW_UP_MAKE)
        game->menu_option = (game->menu_option + NUM_MENU_OPTIONS - 1) % NUM_MENU_OPTIONS;
      else if (scancode == ARROW_DOWN_MAKE)
        game->menu_option = (game->menu_option + 1) % NUM_MENU_OPTIONS;
    } else if (game->state == GAME_STATE_SETTINGS) {
      if (scancode == ARROW_UP_MAKE)
        game->settings_option = (game->settings_option + NUM_SETTINGS_OPTIONS - 1) % NUM_SETTINGS_OPTIONS;
      else if (scancode == ARROW_DOWN_MAKE)
        game->settings_option = (game->settings_option + 1) % NUM_SETTINGS_OPTIONS;
    }
    return;
  }

  switch (game->state) {
    case GAME_STATE_DAY_INTRO:
      if (scancode == ENTER_BREAK) {
        game->state_ticks = 0;
        game->state = GAME_STATE_OPEN_SCREEN;
      }
      break;

    case GAME_STATE_DAY_SUMMARY:
      if (scancode == ENTER_BREAK) {
        game->day_number++;
        game->day_orders_total += game->day_increment;
        game->day_orders_done  = 0;
        game->day_total_stars  = 0;
        game->day_total_score  = 0;
        game->state_ticks      = 0;
        game->state            = GAME_STATE_DAY_INTRO;
      }
      break;

    case GAME_STATE_MENU:
      if (scancode == ENTER_BREAK) {
        switch (game->menu_option) {
          case 0:
            game->day_number       = 1;
            game->day_orders_total = 3;
            game->day_orders_done  = 0;
            game->day_total_stars  = 0;
            game->day_total_score  = 0;
            game->state_ticks      = 0;
            game->order_number     = 0;
            game->state            = GAME_STATE_DAY_INTRO;
            break;
          case 1:
            game->settings_option = (int)game->difficulty;
            game->state = GAME_STATE_SETTINGS;
            break;
          case 2: game->running = false; break;
        }
      }
      break;

    case GAME_STATE_SETTINGS:
      if (scancode == ENTER_BREAK) {
        if (game->settings_option == 0) game->difficulty = DIFF_EASY;
        else if (game->settings_option == 1) game->difficulty = DIFF_NORMAL;
        else if (game->settings_option == 2) game->difficulty = DIFF_HARD;
        else if (game->settings_option == 3) game->day_increment = 0;
        else if (game->settings_option == 4) game->day_increment = 1;
        else if (game->settings_option == 5) game->day_increment = 3;
        else if (game->settings_option == 6) game->day_increment = 5;
        game->state = GAME_STATE_MENU;
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
          else if (scancode == ENTER_BREAK) {
            game->playing_state = PLAYING_CUT;
          }
          break;

        case PLAYING_CUT:
          if (scancode == ENTER_BREAK)
            game->playing_state = PLAYING_SERVE;
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

    default:
      break;
  }
}

#pragma clang optimize on
