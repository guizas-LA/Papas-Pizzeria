#include "utils.h"

#include <stdbool.h>
#include <stdint.h>

#define KBC_IRQ 1
#define KBC_ST_REG 0x64
#define KBC_OUT_BUF 0x60

#define KBC_OBF BIT(0)
#define KBC_AUX BIT(5)
#define KBC_PAR_ERR BIT(7)
#define KBC_TO_ERR BIT(6)

#define ESC_BREAKCODE 0x81

static int kbd_hook_id = 1;
static uint8_t scancode_byte = 0;
static bool kbc_error = false;

int(util_sys_inb_local)(int port, uint8_t *value) {
  uint32_t temp;

  if (value == NULL) {
    return 1;
  }

  if (sys_inb(port, &temp) != OK) {
    return 1;
  }

  *value = (uint8_t) temp;
  return 0;
}

int(keyboard_subscribe_int)(uint8_t *bit_no) {
  if (bit_no == NULL) {
    return 1;
  }

  *bit_no = BIT(kbd_hook_id);

  if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbd_hook_id) != OK) {
    return 1;
  }

  return 0;
}

int(keyboard_unsubscribe_int)() {
  if (sys_irqrmpolicy(&kbd_hook_id) != OK) {
    return 1;
  }

  return 0;
}

void(kbc_ih)() {
  uint8_t status;

  kbc_error = true;

  if (util_sys_inb_local(KBC_ST_REG, &status) != OK) {
    return;
  }

  if ((status & KBC_OBF) == 0) {
    return;
  }

  if (util_sys_inb_local(KBC_OUT_BUF, &scancode_byte) != OK) {
    return;
  }

  if (status & (KBC_PAR_ERR | KBC_TO_ERR | KBC_AUX)) {
    return;
  }

  kbc_error = false;
}

int(wait_for_esc_breakcode)() {
  uint8_t bit_no;
  int ipc_status, r;
  message msg;

  if (keyboard_subscribe_int(&bit_no) != 0) {
    return 1;
  }

  scancode_byte = 0;
  kbc_error = false;

  while (scancode_byte != ESC_BREAKCODE || kbc_error) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      continue;
    }

    if (!is_ipc_notify(ipc_status)) {
      continue;
    }

    if (_ENDPOINT_P(msg.m_source) == HARDWARE && (msg.m_notify.interrupts & bit_no)) {
      kbc_ih();
    }
  }

  if (keyboard_unsubscribe_int() != 0) {
    return 1;
  }

  return 0;
}
