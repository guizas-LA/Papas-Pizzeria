#ifndef PROJECT_INTERRUPTS_H
#define PROJECT_INTERRUPTS_H

#include <stdbool.h>
#include <stdint.h>

int subscribe_all(uint8_t *timer_irq, uint8_t *kbd_irq, uint8_t *mouse_irq);
int unsubscribe_all(void);
int cleanup_game_devices(void);
int read_kbc_byte(uint8_t *byte, bool from_mouse);

#endif
