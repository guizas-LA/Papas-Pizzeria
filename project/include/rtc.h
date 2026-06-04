#ifndef PROJECT_RTC_H
#define PROJECT_RTC_H

#include <stdint.h>

typedef struct {
  uint8_t  hour;
  uint8_t  min;
  uint8_t  sec;
  uint8_t  day;
  uint8_t  month;
  uint16_t year;
} RtcTime;

int rtc_read_time(RtcTime *t);

#endif
