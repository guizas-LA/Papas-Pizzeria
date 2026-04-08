#ifndef _LCOM_KBC_H_
#define _LCOM_KBC_H_

#include <lcom/lcf.h>

int kbd_subscribe_int(uint8_t *bit_no);
int kbd_unsubscribe_int();
void (kbc_ih)();

int kbc_read_data_poll(uint8_t *data);
int kbc_restore_interrupts();

#endif /* _LCOM_KBC_H_ */
