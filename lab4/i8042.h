#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>

#define KBD_IRQ 1
#define MOUSE_IRQ 12

#define KBC_OUT_BUF 0x60
#define KBC_IN_BUF 0x60
#define KBC_STAT_REG 0x64
#define KBC_CMD_REG 0x64

#define KBC_OBF BIT(0)
#define KBC_IBF BIT(1)
#define KBC_AUX BIT(5)
#define KBC_TIMEOUT BIT(6)
#define KBC_PARITY BIT(7)

#define WRITE_TO_MOUSE 0xD4
#define EN_DATA_REPORT 0xF4
#define DIS_DATA_REPORT 0xF5

#define ACK 0xFA
#define NACK 0xFE
#define ERROR 0xFC

#define DELAY_US 20000

#endif /* _LCOM_I8042_H_ */
