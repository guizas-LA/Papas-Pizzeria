#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

static int hook_id = 0;
int timer_counter = 0;

int(timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if (timer > 2) return 1;
  if (freq == 0 || freq > TIMER_FREQ) return 1;

  uint16_t div = (uint16_t)(TIMER_FREQ / freq);

  uint8_t st;
  if (timer_get_conf(timer, &st) != 0) return 1;

  uint8_t control = st & 0x0F;

  switch (timer) {
    case 0:
      control |= TIMER_SEL0 | TIMER_LSB_MSB;
      break;
    case 1:
      control |= TIMER_SEL1 | TIMER_LSB_MSB;
      break;
    case 2:
      control |= TIMER_SEL2 | TIMER_LSB_MSB;
      break;
    default:
      return 1;
  }

  if (sys_outb(TIMER_CTRL, control) != 0) return 1;

  uint8_t lsb, msb;
  if (util_get_LSB(div, &lsb) != 0) return 1;
  if (util_get_MSB(div, &msb) != 0) return 1;

  int timer_port;
  switch (timer) {
    case 0:
      timer_port = TIMER_0;
      break;
    case 1:
      timer_port = TIMER_1;
      break;
    case 2:
      timer_port = TIMER_2;
      break;
    default:
      return 1;
  }

  if (sys_outb(timer_port, lsb) != 0) return 1;
  if (sys_outb(timer_port, msb) != 0) return 1;

  return 0;
}

int(timer_subscribe_int)(uint8_t *bit_no) {
  if (bit_no == NULL) return 1;

  *bit_no = hook_id;

  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != 0) return 1;

  return 0;
}

int(timer_unsubscribe_int)() {
  if (sys_irqrmpolicy(&hook_id) != 0) return 1;

  return 0;
}

void(timer_int_handler)() {
  timer_counter++;
}

int(timer_get_conf)(uint8_t timer, uint8_t *st) {
  if (st == NULL) return 1;
  if (timer > 2) return 1;

  uint8_t rb_cmd = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);

  if (sys_outb(TIMER_CTRL, rb_cmd) != 0) return 1;

  int timer_port;
  switch (timer) {
    case 0:
      timer_port = TIMER_0;
      break;
    case 1:
      timer_port = TIMER_1;
      break;
    case 2:
      timer_port = TIMER_2;
      break;
    default:
      return 1;
  }

  if (util_sys_inb(timer_port, st) != 0) return 1;

  return 0;
}

int(timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
  union timer_status_field_val val;

  switch (field) {
    case tsf_all:
      val.byte = st;
      break;

    case tsf_initial:
      val.in_mode = (st >> 4) & 0x03;
      if (val.in_mode == 0x03) val.in_mode = MSB_after_LSB;
      break;

    case tsf_mode:
      val.count_mode = (st >> 1) & 0x07;
      if (val.count_mode == 6) val.count_mode = 2;
      if (val.count_mode == 7) val.count_mode = 3;
      break;

    case tsf_base:
      val.bcd = (st & TIMER_BCD) != 0;
      break;

    default:
      return 1;
  }

  if (timer_print_config(timer, field, val) != 0) return 1;

  return 0;
}
