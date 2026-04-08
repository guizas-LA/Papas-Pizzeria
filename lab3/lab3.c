#include <lcom/lcf.h>
#include <lcom/lab3.h>
#include <stdbool.h>
#include <stdint.h>

#include "i8042.h"
#include "kbc.h"

extern uint8_t scancode;
extern bool error_found;
extern uint32_t sys_inb_counter;

extern int (timer_subscribe_int)(uint8_t *bit_no);
extern int (timer_unsubscribe_int)();
extern void (timer_int_handler)();
extern int timer_counter;

int main(int argc, char *argv[]) {
  lcf_set_language("EN-US");
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  if (lcf_start(argc, argv))
    return 1;

  lcf_cleanup();
  return 0;
}

int(kbd_test_scan)() {
  uint8_t bit_no;
  int ipc_status, r;
  message msg;

  if (kbd_subscribe_int(&bit_no) != 0) return 1;

  uint32_t irq_set = BIT(bit_no);
  bool is_two_bytes = false;
  uint8_t bytes;
  uint8_t size = 0;

  while (scancode != ESC_BREAKCODE) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) continue;

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih(); 

            if (!error_found) {
              if (scancode == TWO_BYTE_CODE) {
                is_two_bytes = true;
                bytes = scancode;
              } else {
                if (is_two_bytes) {
                  size = 2;
                  bytes = scancode;
                  is_two_bytes = false; 
                } else {
                  size = 1;
                  bytes = scancode;
                }
                bool make = !(scancode & BIT(7));
                kbd_print_scancode(make, size, &bytes);
              }
            }
          }
          break;
        default: break;
      }
    }
  }

  if (kbd_unsubscribe_int() != 0) return 1;
  kbd_print_no_sysinb(sys_inb_counter);
  return 0;
}

int(kbd_test_poll)() {
  bool is_two_bytes = false;
  uint8_t bytes;
  uint8_t size = 0;
  uint8_t code = 0;

  while (code != ESC_BREAKCODE) {
    if (kbc_read_data_poll(&code) == 0) {
      if (code == TWO_BYTE_CODE) {
        is_two_bytes = true;
        bytes = code;
      } else {
        if (is_two_bytes) {
          size = 2;
          bytes = code;
          is_two_bytes = false;
        } else {
          size = 1;
          bytes = code;
        }
        bool make = !(code & BIT(7));
        kbd_print_scancode(make, size, &bytes);
      }
    }
  }

  if (kbc_restore_interrupts() != 0) return 1;
  
  kbd_print_no_sysinb(sys_inb_counter);
  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
  uint8_t kbd_bit_no, timer_bit_no;
  int ipc_status, r;
  message msg;

  if (kbd_subscribe_int(&kbd_bit_no) != 0) return 1;
  if (timer_subscribe_int(&timer_bit_no) != 0) return 1;

  uint32_t kbd_irq_set = BIT(kbd_bit_no);
  uint32_t timer_irq_set = BIT(timer_bit_no);

  bool is_two_bytes = false;
  uint8_t bytes;
  uint8_t size = 0;

  timer_counter = 0;

  while (scancode != ESC_BREAKCODE && timer_counter < n * 60) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) continue;

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          
          if (msg.m_notify.interrupts & kbd_irq_set) {
            kbc_ih();
            if (!error_found) {
              if (scancode == TWO_BYTE_CODE) {
                is_two_bytes = true;
                bytes = scancode;
              } else {
                if (is_two_bytes) {
                  size = 2;
                  bytes = scancode;
                  is_two_bytes = false;
                } else {
                  size = 1;
                  bytes = scancode;
                }
                bool make = !(scancode & BIT(7));
                kbd_print_scancode(make, size, &bytes);
                
                timer_counter = 0; 
              }
            }
          }
          
          if (msg.m_notify.interrupts & timer_irq_set) {
            timer_int_handler();
          }
          
          break;
        default: break;
      }
    }
  }

  if (kbd_unsubscribe_int() != 0) return 1;
  if (timer_unsubscribe_int() != 0) return 1;

  kbd_print_no_sysinb(sys_inb_counter);
  return 0;
}

