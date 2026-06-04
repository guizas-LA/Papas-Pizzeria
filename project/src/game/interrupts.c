#include "interrupts.h"

#include <lcom/lcf.h>
#include <lcom/timer.h>

#include "graphics.h"
#include "kbc.h"
#include "mouse.h"

#define KBC_OUT_BUF 0x60
#define KBC_STAT_REG 0x64
#define KBC_OBF BIT(0)
#define KBC_AUX BIT(5)
#define KBC_TIMEOUT BIT(6)
#define KBC_PARITY BIT(7)

#define DIS_DATA_REPORT 0xF5

int read_kbc_byte(uint8_t *byte, bool from_mouse) {
  uint8_t status;

  if (util_sys_inb(KBC_STAT_REG, &status) != 0) return 1;
  if ((status & KBC_OBF) == 0) return 1;
  if ((status & (KBC_PARITY | KBC_TIMEOUT)) != 0) return 1;
  if (from_mouse && ((status & KBC_AUX) == 0)) return 1;
  if (!from_mouse && (status & KBC_AUX)) return 1;

  return util_sys_inb(KBC_OUT_BUF, byte);
}

int subscribe_all(uint8_t *timer_irq, uint8_t *kbd_irq, uint8_t *mouse_irq) {
  if (timer_subscribe_int(timer_irq) != 0) return 1;
  if (kbd_subscribe_int(kbd_irq) != 0) return 1;
  if (mouse_subscribe_int(mouse_irq) != 0) return 1;
  return 0;
}

int unsubscribe_all(void) {
  int failed = 0;

  if (mouse_unsubscribe_int() != 0) failed = 1;
  if (kbd_unsubscribe_int() != 0) failed = 1;
  if (timer_unsubscribe_int() != 0) failed = 1;

  return failed;
}

int cleanup_game_devices(void) {
  int failed = 0;

  if (mouse_write_cmd(DIS_DATA_REPORT) != 0) failed = 1;
  if (unsubscribe_all() != 0) failed = 1;
  if (vg_exit() != OK) failed = 1;

  return failed;
}
