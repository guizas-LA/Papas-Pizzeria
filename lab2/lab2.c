#include <lcom/lcf.h>
#include <lcom/lab2.h>

#include <stdbool.h>
#include <stdint.h>

extern int timer_counter;

int main(int argc, char *argv[]) {
  lcf_set_language("EN-US");

  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  if (lcf_start(argc, argv))
    return 1;

  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {
  uint8_t st;

  if (timer_get_conf(timer, &st) != 0) return 1;
  if (timer_display_conf(timer, st, field) != 0) return 1;

  return 0;
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
  if (timer_set_frequency(timer, freq) != 0) return 1;

  return 0;
}

int(timer_test_int)(uint8_t time) {
  uint8_t bit_no;
  if (timer_subscribe_int(&bit_no) != 0) return 1;

  int ipc_status;
  message msg;
  int irq_set = BIT(bit_no);

  timer_counter = 0;

  while (timer_counter < time * 60) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0) continue;
    
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            timer_int_handler();

            if (timer_counter % 60 == 0) {
              timer_print_elapsed_time();
            }
          }
          break;
        default:
          break;
      }
    }
  }

  if (timer_unsubscribe_int() != 0) return 1;
  
  return 0;
}

