#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>

#define KBD_IRQ 1

#define KBD_OUT_BUF  0x60
#define KBD_IN_BUF   0x60
#define KBD_STAT_REG 0x64
#define KBD_CMD_REG  0x64

#define KBD_OBF      BIT(0)
#define KBD_IBF      BIT(1)
#define KBD_PARITY   BIT(7)
#define KBD_TIMEOUT  BIT(6)

#define ESC_BREAKCODE 0x81
#define TWO_BYTE_CODE 0xE0

#define KBC_READ_CMD  0x20
#define KBC_WRITE_CMD 0x60
#define KBC_INT_KBD   BIT(0)

#define DELAY_US      20000

#endif /* _LCOM_I8042_H_ */
