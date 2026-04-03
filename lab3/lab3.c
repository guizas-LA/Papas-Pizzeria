#include <lcom/lcf.h>
#include <lcom/lab3.h>
#include <stdbool.h>
#include <stdint.h>

#include "i8042.h"
#include "kbc.h"

/* TIMER */
#define TIMER0_IRQ 0
#define TIMER_FREQ 60

static int timer_hook_id = 0;
static uint32_t timer_counter = 0;

int (timer_subscribe_local_int)(uint8_t *bit_no) {
  if (bit_no == NULL) return 1;
  *bit_no = timer_hook_id;
  return sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &timer_hook_id);
}

int (timer_unsubscribe_local_int)(void) {
  return sys_irqrmpolicy(&timer_hook_id);
}

void (timer_int_handler_local)(void) {
  timer_counter++;
}

int main(int argc, char *argv[]) {
  lcf_set_language("EN-US");

  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  if (lcf_start(argc, argv))
    return 1;

  lcf_cleanup();

  return 0;
}

static int print_scancode_from_bytes(uint8_t bytes[], uint8_t size) {
  bool make = ((bytes[size - 1] & BIT(7)) == 0);
  return kbd_print_scancode(make, size, bytes);
}

int(kbd_test_scan)() {
  uint8_t bit_no;
  int ipc_status, r;
  message msg;

  uint8_t bytes[2];
  uint8_t size = 0;
  bool done = false;

  if (kbc_subscribe_int(&bit_no) != 0) return 1;
  uint32_t irq_set = BIT(bit_no);

  while (!done) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d\n", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();

            if (kbc_get_valid()) {
              uint8_t byte = kbc_get_scancode_byte();

              if (byte == TWO_BYTE_CODE) {
                bytes[0] = byte;
                size = 2;
              }
              else {
                if (size == 2) {
                  bytes[1] = byte;
                }
                else {
                  bytes[0] = byte;
                  size = 1;
                }

                if (print_scancode_from_bytes(bytes, size) != 0) {
                  kbc_unsubscribe_int();
                  return 1;
                }

                if (byte == ESC_BREAKCODE) done = true;

                size = 0;
              }
            }
          }
          break;
        default:
          break;
      }
    }
  }

  if (kbc_unsubscribe_int() != 0) return 1;

  return 0;
}

int(kbd_test_poll)() {
  uint8_t bytes[2];
  uint8_t size = 0;
  uint8_t data;
  bool done = false;

  while (!done) {
    if (kbc_read_outbuf_poll(&data) == 0) {
      if (data == TWO_BYTE_CODE) {
        bytes[0] = data;
        size = 2;
      }
      else {
        if (size == 2) {
          bytes[1] = data;
        }
        else {
          bytes[0] = data;
          size = 1;
        }

        if (print_scancode_from_bytes(bytes, size) != 0) return 1;

        if (data == ESC_BREAKCODE) done = true;

        size = 0;
      }
    }
  }

  if (kbc_enable_interrupts() != 0) return 1;

#ifdef LAB3
  extern uint32_t kbc_get_sys_inb_count(void);
  if (kbd_print_no_sysinb(kbc_get_sys_inb_count()) != 0) return 1;
#endif

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
  uint8_t kbd_bit_no, timer_bit_no;
  int ipc_status, r;
  message msg;

  uint8_t bytes[2];
  uint8_t size = 0;
  bool done = false;

  timer_counter = 0;

  if (kbc_subscribe_int(&kbd_bit_no) != 0) return 1;
  if (timer_subscribe_local_int(&timer_bit_no) != 0) {
    kbc_unsubscribe_int();
    return 1;
  }

  uint32_t kbd_irq_set = BIT(kbd_bit_no);
  uint32_t timer_irq_set = BIT(timer_bit_no);

  while (!done) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d\n", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & timer_irq_set) {
            timer_int_handler_local();

            if (timer_counter >= (uint32_t)n * TIMER_FREQ) {
              done = true;
            }
          }

          if (msg.m_notify.interrupts & kbd_irq_set) {
            kbc_ih();

            if (kbc_get_valid()) {
              uint8_t byte = kbc_get_scancode_byte();

              timer_counter = 0; /* reset idle time */

              if (byte == TWO_BYTE_CODE) {
                bytes[0] = byte;
                size = 2;
              }
              else {
                if (size == 2) {
                  bytes[1] = byte;
                }
                else {
                  bytes[0] = byte;
                  size = 1;
                }

                if (print_scancode_from_bytes(bytes, size) != 0) {
                  timer_unsubscribe_local_int();
                  kbc_unsubscribe_int();
                  return 1;
                }

                if (byte == ESC_BREAKCODE) done = true;

                size = 0;
              }
            }
          }
          break;
        default:
          break;
      }
    }
  }

  if (timer_unsubscribe_local_int() != 0) {
    kbc_unsubscribe_int();
    return 1;
  }

  if (kbc_unsubscribe_int() != 0) return 1;

  return 0;
}
