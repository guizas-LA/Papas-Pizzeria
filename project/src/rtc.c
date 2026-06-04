#include "rtc.h"
#include <lcom/lcf.h>

#define RTC_ADDR  0x70
#define RTC_DATA  0x71

#define RTC_SEC   0x00
#define RTC_MIN   0x02
#define RTC_HOUR  0x04
#define RTC_DAY   0x07
#define RTC_MONTH 0x08
#define RTC_YEAR  0x09
#define RTC_STATB 0x0B

#define RTC_BCD   BIT(2)

static bool rtc_ok = true;

static int rtc_read_reg(uint8_t reg, uint8_t *val) {
  if (sys_outb(RTC_ADDR, reg) != OK) return 1;
  if (util_sys_inb(RTC_DATA, val) != OK) return 1;
  return 0;
}

static uint8_t bcd_to_bin(uint8_t v) {
  return (uint8_t)((v >> 4) * 10 + (v & 0x0F));
}

int rtc_read_time(RtcTime *t) {
  uint8_t statb, s, m, h, d, mo, y;

  if (!rtc_ok) return 1;

  if (rtc_read_reg(RTC_SEC,   &s)     != 0) { rtc_ok = false; return 1; }
  if (rtc_read_reg(RTC_MIN,   &m)     != 0) { rtc_ok = false; return 1; }
  if (rtc_read_reg(RTC_HOUR,  &h)     != 0) { rtc_ok = false; return 1; }
  if (rtc_read_reg(RTC_DAY,   &d)     != 0) { rtc_ok = false; return 1; }
  if (rtc_read_reg(RTC_MONTH, &mo)    != 0) { rtc_ok = false; return 1; }
  if (rtc_read_reg(RTC_YEAR,  &y)     != 0) { rtc_ok = false; return 1; }
  if (rtc_read_reg(RTC_STATB, &statb) != 0) { rtc_ok = false; return 1; }

  if (!(statb & RTC_BCD)) {
    s  = bcd_to_bin(s);
    m  = bcd_to_bin(m);
    h  = bcd_to_bin(h);
    d  = bcd_to_bin(d);
    mo = bcd_to_bin(mo);
    y  = bcd_to_bin(y);
  }

  t->sec   = s;
  t->min   = m;
  t->hour  = h;
  t->day   = d;
  t->month = mo;
  t->year  = (uint16_t)(2000 + y);
  return 0;
}
