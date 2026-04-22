#ifndef _LCOM_MOUSE_H_
#define _LCOM_MOUSE_H_

#include <lcom/lcf.h>

int(mouse_subscribe_int)(uint8_t *bit_no);
int(mouse_unsubscribe_int)();
void(mouse_ih)();
int(mouse_write_cmd)(uint8_t cmd);
void(mouse_parse_packet)(const uint8_t *packet_bytes, struct packet *pp);

#endif /* _LCOM_MOUSE_H_ */
