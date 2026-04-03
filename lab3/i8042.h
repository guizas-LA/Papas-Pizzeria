#ifndef I8042_H
#define I8042_H

#include <lcom/lcf.h>

/* IRQ line */
#define KBC_IRQ 1

/* KBC ports */
#define KBC_OUT_BUF 0x60
#define KBC_IN_BUF 0x60
#define KBC_ST_REG 0x64
#define KBC_CMD_REG 0x64

/* Status Register bits */
#define KBC_OBF BIT(0)
#define KBC_IBF BIT(1)
#define KBC_AUX BIT(5)
#define KBC_TIMEOUT BIT(6)
#define KBC_PARITY BIT(7)

/* KBC commands */
#define READ_CMD_BYTE 0x20
#define WRITE_CMD_BYTE 0x60
#define DISABLE_KBD 0xAD
#define ENABLE_KBD 0xAE

/* Command byte bits */
#define KBC_INT BIT(0)

/* Scancodes */
#define TWO_BYTE_CODE 0xE0
#define ESC_BREAKCODE 0x81

/* Delay / retries */
#define DELAY_US 20000
#define MAX_RETRIES 10

#endif
