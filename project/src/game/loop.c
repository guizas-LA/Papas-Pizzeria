#include "loop.h"

#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>
#include <stdio.h>

#include "game_state.h"
#include "draw.h"
#include "graphics.h"
#include "interrupts.h"
#include "mouse.h"
#include "sprites.h"

#define EN_DATA_REPORT 0xF4

int game_loop(int argc, char *argv[]) {
  uint8_t timer_irq;
  uint8_t kbd_irq;
  uint8_t mouse_irq;
  uint8_t mouse_bytes[3];
  int mouse_byte_count = 0;
  Game game;

  (void) argc;
  (void) argv;

  if (map_video_memory(GAME_VIDEO_MODE) != 0) return 1;
  if (set_graphics_mode(GAME_VIDEO_MODE) != 0) return 1;

  if (subscribe_all(&timer_irq, &kbd_irq, &mouse_irq) != 0) {
    vg_exit();
    return 1;
  }

  if (mouse_write_cmd(EN_DATA_REPORT) != 0) {
    unsubscribe_all();
    vg_exit();
    return 1;
  }

  if (timer_set_frequency(0, GAME_FPS) != 0) {
    cleanup_game_devices();
    return 1;
  }

  if (loadSprites() != 0) {
    cleanup_game_devices();
    return 1;
  }

  game_init(&game);

  while (game_is_running(&game)) {
    int ipc_status;
    message msg;
    int r = driver_receive(ANY, &msg, &ipc_status);

    if (r != 0) {
      printf("driver_receive failed: %d\n", r);
      continue;
    }

    if (!is_ipc_notify(ipc_status)) continue;

    if (_ENDPOINT_P(msg.m_source) == HARDWARE) {
      if (msg.m_notify.interrupts & BIT(timer_irq)) {
        timer_int_handler();
        game_update(&game);
        game_draw(&game);
      }

      if (msg.m_notify.interrupts & BIT(kbd_irq)) {
        uint8_t scancode;
        if (read_kbc_byte(&scancode, false) == 0) {
          game_handle_keyboard(&game, scancode);
        }
      }

      if (msg.m_notify.interrupts & BIT(mouse_irq)) {
        uint8_t byte;
        if (read_kbc_byte(&byte, true) == 0) {
          if (mouse_byte_count == 0 && ((byte & BIT(3)) == 0)) continue;

          mouse_bytes[mouse_byte_count] = byte;
          mouse_byte_count++;

          if (mouse_byte_count == 3) {
            struct packet packet;
            mouse_parse_packet(mouse_bytes, &packet);
            game_handle_mouse_packet(&game, &packet);
            mouse_byte_count = 0;
          }
        }
      }
    }
  }

  unloadSprites();
  return cleanup_game_devices();
}
