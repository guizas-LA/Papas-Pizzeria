#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if(timer > 2) return 1;
  uint8_t st;

  if(timer_get_conf(timer, &st))return 1;

  uint8_t control =(st & 0x0F) |TIMER_LSB_MSB |(timer << 6);

  if(sys_outb(TIMER_CTRL, control))return 1;

  uint16_t divisor = TIMER_FREQ / freq;

  uint8_t lsb, msb;

  util_get_LSB(divisor, &lsb);
  util_get_MSB(divisor, &msb);

  if(sys_outb(TIMER_0 + timer, lsb))
      return 1;

  if(sys_outb(TIMER_0 + timer, msb))
      return 1;

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
    /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

int (timer_unsubscribe_int)() {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

void (timer_int_handler)() {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  if(timer > 2 || st == NULL) return 1;
  uint8_t readBack =TIMER_RB_CMD |TIMER_RB_COUNT_ |TIMER_RB_SEL(timer);
  if(sys_outb(TIMER_CTRL, readBack) != OK)return 1;
  if(util_sys_inb(TIMER_0 + timer, st) != OK)return 1;
  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st,enum timer_status_field field) {
  union timer_status_field_val val;
  switch(field){
    case tsf_all:
        val.byte = st;
        break;

    case tsf_initial:
        val.in_mode = (st >> 4) & 0x03;
        break;

    case tsf_mode:
        val.count_mode = (st >> 1) & 0x07;
        if(val.count_mode > 5)
            val.count_mode &= 0x03;
        break;

    case tsf_base:
        val.bcd = st & 0x01;
        break;
  }
  return timer_print_config(timer, field, val);
}
