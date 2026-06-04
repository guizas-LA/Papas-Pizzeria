#pragma once

#include <stdint.h>

typedef struct {
  uint8_t day;
  uint8_t month;
  uint8_t year;
} rtc_date;

/*
Funcionalidade extra que adicionamos na library , já que a libray apenas serve para dias
*/

typedef struct {
  uint8_t  hour;
  uint8_t  min;
  uint8_t  sec;
  uint8_t  day;
  uint8_t  month;
  uint16_t year;
} RtcTime;

int rtc_read_date(rtc_date *date);
int rtc_read_datetime(RtcTime *t);
