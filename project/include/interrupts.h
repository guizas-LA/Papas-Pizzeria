/**
 * @file interrupts.h
 * @brief Declarations for device interrupt subscription and KBC byte reading.
 */

#ifndef PROJECT_INTERRUPTS_H
#define PROJECT_INTERRUPTS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Subscribes to timer, keyboard, and mouse hardware interrupts.
 * @param timer_irq Output: IRQ bit index for the timer.
 * @param kbd_irq   Output: IRQ bit index for the keyboard.
 * @param mouse_irq Output: IRQ bit index for the mouse.
 * @return 0 on success, 1 if any subscription fails.
 */
int subscribe_all(uint8_t *timer_irq, uint8_t *kbd_irq, uint8_t *mouse_irq);

/**
 * @brief Unsubscribes from all previously subscribed hardware interrupts.
 * @return 0 if all unsubscriptions succeed, 1 if any fail.
 */
int unsubscribe_all(void);

/**
 * @brief Disables mouse reporting, unsubscribes all interrupts, and exits VBE graphics mode.
 * @return 0 if all cleanup steps succeed, 1 if any fail.
 */
int cleanup_game_devices(void);

/**
 * @brief Reads one byte from the KBC output buffer after checking the status register.
 * @param byte       Output: the byte read from the KBC.
 * @param from_mouse @c true to read a mouse byte (KBC AUX channel); @c false for keyboard.
 * @return 0 on success, 1 if the buffer is empty, contains errors, or the wrong channel.
 */
int read_kbc_byte(uint8_t *byte, bool from_mouse);

#endif
