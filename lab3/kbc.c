#include "kbc.h"
#include "i8042.h"

static int kbc_hook_id = KBC_IRQ;
static uint8_t scancode_byte = 0;
static bool valid_byte = false;

#ifdef LAB3
static uint32_t sys_inb_counter = 0;
#endif

static int read_sys_inb(int port, uint8_t *value) {
  uint32_t temp;
  if (sys_inb(port, &temp) != OK) return 1;
  *value = (uint8_t) temp;
#ifdef LAB3
  sys_inb_counter++;
#endif
  return 0;
}

uint8_t kbc_get_scancode_byte(void) {
  return scancode_byte;
}

bool kbc_get_valid(void) {
  return valid_byte;
}

int kbc_subscribe_int(uint8_t *bit_no) {
  if (bit_no == NULL) return 1;
  *bit_no = kbc_hook_id;
  return sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbc_hook_id);
}

int kbc_unsubscribe_int(void) {
  return sys_irqrmpolicy(&kbc_hook_id);
}

int kbc_read_status(uint8_t *status) {
  if (status == NULL) return 1;
  return read_sys_inb(KBC_ST_REG, status);
}

void (kbc_ih)(void) {
  uint8_t status;
  uint8_t data;

  valid_byte = false;

  if (kbc_read_status(&status) != 0) return;

  if (status & KBC_OBF) {
    if (read_sys_inb(KBC_OUT_BUF, &data) != 0) return;

    /* Mesmo havendo erro, o buffer deve ser lido; aqui só descartamos */
    if ((status & (KBC_PARITY | KBC_TIMEOUT)) != 0) return;

    /* Ignorar bytes do rato */
    if (status & KBC_AUX) return;

    scancode_byte = data;
    valid_byte = true;
  }
}

int kbc_read_outbuf(uint8_t *data) {
  uint8_t status;

  if (data == NULL) return 1;

  if (kbc_read_status(&status) != 0) return 1;

  if (status & KBC_OBF) {
    if (read_sys_inb(KBC_OUT_BUF, data) != 0) return 1;

    if (status & (KBC_PARITY | KBC_TIMEOUT)) return 1;
    if (status & KBC_AUX) return 1;

    return 0;
  }

  return 1;
}

int kbc_read_outbuf_poll(uint8_t *data) {
  uint8_t status;
  int tries = MAX_RETRIES;

  if (data == NULL) return 1;

  while (tries--) {
    if (kbc_read_status(&status) != 0) return 1;

    if (status & KBC_OBF) {
      if (read_sys_inb(KBC_OUT_BUF, data) != 0) return 1;

      if (status & (KBC_PARITY | KBC_TIMEOUT)) return 1;
      if (status & KBC_AUX) return 1;

      return 0;
    }

    tickdelay(micros_to_ticks(DELAY_US));
  }

  return 1;
}

int kbc_write_cmd(uint8_t cmd) {
  uint8_t status;
  int tries = MAX_RETRIES;

  while (tries--) {
    if (kbc_read_status(&status) != 0) return 1;

    if ((status & KBC_IBF) == 0) {
      if (sys_outb(KBC_CMD_REG, cmd) != OK) return 1;
      return 0;
    }

    tickdelay(micros_to_ticks(DELAY_US));
  }

  return 1;
}

int kbc_write_arg(uint8_t arg) {
  uint8_t status;
  int tries = MAX_RETRIES;

  while (tries--) {
    if (kbc_read_status(&status) != 0) return 1;

    if ((status & KBC_IBF) == 0) {
      if (sys_outb(KBC_IN_BUF, arg) != OK) return 1;
      return 0;
    }

    tickdelay(micros_to_ticks(DELAY_US));
  }

  return 1;
}

int kbc_read_cmd_byte(uint8_t *cmd_byte) {
  if (cmd_byte == NULL) return 1;

  if (kbc_write_cmd(READ_CMD_BYTE) != 0) return 1;

  int tries = MAX_RETRIES;
  uint8_t status, data;

  while (tries--) {
    if (kbc_read_status(&status) != 0) return 1;

    if (status & KBC_OBF) {
      if (read_sys_inb(KBC_OUT_BUF, &data) != 0) return 1;

      if (status & (KBC_PARITY | KBC_TIMEOUT)) return 1;
      if (status & KBC_AUX) return 1;

      *cmd_byte = data;
      return 0;
    }

    tickdelay(micros_to_ticks(DELAY_US));
  }

  return 1;
}

int kbc_write_cmd_byte(uint8_t cmd_byte) {
  if (kbc_write_cmd(WRITE_CMD_BYTE) != 0) return 1;
  if (kbc_write_arg(cmd_byte) != 0) return 1;
  return 0;
}

int kbc_enable_interrupts(void) {
  uint8_t cmd_byte;

  if (kbc_read_cmd_byte(&cmd_byte) != 0) return 1;

  cmd_byte |= KBC_INT;

  if (kbc_write_cmd_byte(cmd_byte) != 0) return 1;

  return 0;
}

#ifdef LAB3
uint32_t kbc_get_sys_inb_count(void) {
  return sys_inb_counter;
}
#endif
