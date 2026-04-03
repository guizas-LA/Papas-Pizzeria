#ifndef KBC_H
#define KBC_H

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>

int kbc_subscribe_int(uint8_t *bit_no);
int kbc_unsubscribe_int(void);

void (kbc_ih)(void);

int kbc_read_status(uint8_t *status);
int kbc_read_outbuf(uint8_t *data);
int kbc_read_outbuf_poll(uint8_t *data);

int kbc_write_cmd(uint8_t cmd);
int kbc_write_arg(uint8_t arg);

int kbc_read_cmd_byte(uint8_t *cmd_byte);
int kbc_write_cmd_byte(uint8_t cmd_byte);
int kbc_enable_interrupts(void);

uint8_t kbc_get_scancode_byte(void);
bool kbc_get_valid(void);

#endif
