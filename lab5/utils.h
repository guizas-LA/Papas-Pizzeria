#ifndef UTILS_H
#define UTILS_H

#include <lcom/lcf.h>

#include <stdint.h>

int(util_sys_inb_local)(int port, uint8_t *value);
int(keyboard_subscribe_int)(uint8_t *bit_no);
int(keyboard_unsubscribe_int)();
void(kbc_ih)();
int(wait_for_esc_breakcode)();

#endif
