#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>

#include "i8042.h"
#include "mouse.h"

extern uint8_t mouse_byte;
extern bool mouse_error;

extern int (timer_subscribe_int)(uint8_t *bit_no);
extern int (timer_unsubscribe_int)();
extern void (timer_int_handler)();
extern int timer_counter;

int main(int argc, char *argv[]) {
  lcf_set_language("EN-US");
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");
  lcf_log_output("/home/lcom/labs/lab4/output.txt");
  if (lcf_start(argc, argv)) return 1;
  lcf_cleanup();
  return 0;
}

int (mouse_test_packet)(uint32_t cnt) {
    uint8_t bit_no;
    int ipc_status, r;
    message msg;

    if (mouse_enable_data_reporting() != 0) return 1;
    if (mouse_subscribe_int(&bit_no) != 0) return 1;

    uint32_t irq_set = BIT(bit_no);
    uint8_t packet_bytes[3];
    int byte_index = 0;
    uint32_t packets_read = 0;

    while (packets_read < cnt) {
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) continue;

        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set) {
                        mouse_ih();

                        if (mouse_error) continue;

                        if (byte_index == 0 && (mouse_byte & BIT(3)) == 0) {
                            continue; 
                        }

                        packet_bytes[byte_index++] = mouse_byte;

                        if (byte_index == 3) {
                            struct packet pp;
                            mouse_parse_packet(packet_bytes, &pp);
                            mouse_print_packet(&pp);
                            byte_index = 0;
                            packets_read++;
                        }
                    }
                    break;
                default: break;
            }
        }
    }

    if (mouse_write_cmd(DIS_DATA_REPORT) != 0) return 1;

    if (mouse_unsubscribe_int() != 0) return 1;

    return 0;
}

int (mouse_test_async)(uint8_t idle_time) {
    uint8_t mouse_bit_no, timer_bit_no;
    int ipc_status, r;
    message msg;

    if (mouse_write_cmd(EN_DATA_REPORT) != 0) return 1;
    
    if (mouse_subscribe_int(&mouse_bit_no) != 0) return 1;
    if (timer_subscribe_int(&timer_bit_no) != 0) return 1;

    uint32_t mouse_irq_set = BIT(mouse_bit_no);
    uint32_t timer_irq_set = BIT(timer_bit_no);

    uint8_t packet_bytes[3];
    int byte_index = 0;
    
    timer_counter = 0;
    uint32_t freq = sys_hz();

    while ((uint32_t) timer_counter < idle_time * freq) {
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) continue;

        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & timer_irq_set) {
                        timer_int_handler();
                    }

                    if (msg.m_notify.interrupts & mouse_irq_set) {
                        mouse_ih();
                        if (mouse_error) continue;

                        if (byte_index == 0 && (mouse_byte & BIT(3)) == 0) continue;

                        packet_bytes[byte_index++] = mouse_byte;

                        if (byte_index == 3) {
                            struct packet pp;
                            mouse_parse_packet(packet_bytes, &pp);
                            mouse_print_packet(&pp);
                            byte_index = 0;
                            
                            timer_counter = 0;
                        }
                    }
                    break;
                default: break;
            }
        }
    }

    if (mouse_write_cmd(DIS_DATA_REPORT) != 0) return 1;

    if (mouse_unsubscribe_int() != 0) return 1;
    if (timer_unsubscribe_int() != 0) return 1;

    return 0;
}
