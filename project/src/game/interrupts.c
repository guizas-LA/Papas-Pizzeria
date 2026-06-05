/**
 * @file interrupts.c
 * @brief Device interrupt subscription/unsubscription and KBC byte reading.
 */

#include "interrupts.h"

#include <lcom/lcf.h>
#include <lcom/timer.h>

#include "graphics.h"
#include "kbc.h"
#include "mouse.h"

#define KBC_OUT_BUF  0x60 /**< KBC output buffer I/O port. */
#define KBC_STAT_REG 0x64 /**< KBC status register I/O port. */
#define KBC_OBF      BIT(0) /**< Output Buffer Full flag. */
#define KBC_AUX      BIT(5) /**< AUX (mouse) data flag. */
#define KBC_TIMEOUT  BIT(6) /**< Timeout error flag. */
#define KBC_PARITY   BIT(7) /**< Parity error flag. */

#define DIS_DATA_REPORT 0xF5 /**< Mouse command: disable data reporting. */

/**
 * @brief Reads one byte from the KBC output buffer after validating the status register.
 * @param byte       Output: byte read from the KBC.
 * @param from_mouse @c true to expect mouse data; @c false to expect keyboard data.
 * @return 0 on success, 1 on error or wrong channel.
 */
int read_kbc_byte(uint8_t *byte, bool from_mouse) {
  uint8_t status;

  if (util_sys_inb(KBC_STAT_REG, &status) != 0) return 1;
  if ((status & KBC_OBF) == 0) return 1;
  if ((status & (KBC_PARITY | KBC_TIMEOUT)) != 0) return 1;
  if (from_mouse && ((status & KBC_AUX) == 0)) return 1;
  if (!from_mouse && (status & KBC_AUX)) return 1;

  return util_sys_inb(KBC_OUT_BUF, byte);
}

/**
 * @brief Subscribes to timer, keyboard, and mouse hardware interrupts.
 * @param timer_irq Output: IRQ bit index for the timer.
 * @param kbd_irq   Output: IRQ bit index for the keyboard.
 * @param mouse_irq Output: IRQ bit index for the mouse.
 * @return 0 on success, 1 if any subscription fails.
 */
int subscribe_all(uint8_t *timer_irq, uint8_t *kbd_irq, uint8_t *mouse_irq) {
  if (timer_subscribe_int(timer_irq) != 0) return 1;
  if (kbd_subscribe_int(kbd_irq) != 0) return 1;
  if (mouse_subscribe_int(mouse_irq) != 0) return 1;
  return 0;
}

/**
 * @brief Unsubscribes from all previously subscribed hardware interrupts.
 * @return 0 if all steps succeed, 1 if any fail.
 */
int unsubscribe_all(void) {
  int failed = 0;

  if (mouse_unsubscribe_int() != 0) failed = 1;
  if (kbd_unsubscribe_int() != 0) failed = 1;
  if (timer_unsubscribe_int() != 0) failed = 1;

  return failed;
}

/**
 * @brief Disables mouse reporting, unsubscribes all interrupts, and exits VBE graphics mode.
 * @return 0 if all cleanup steps succeed, 1 if any fail.
 */
int cleanup_game_devices(void) {
  int failed = 0;

  if (mouse_write_cmd(DIS_DATA_REPORT) != 0) failed = 1;
  if (unsubscribe_all() != 0) failed = 1;
  if (vg_exit() != OK) failed = 1;

  return failed;
}
