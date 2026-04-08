#include "kbc.h"
#include "i8042.h"
#include <minix/sysutil.h>

static int hook_id = 1;
uint8_t scancode = 0;
bool error_found = false;
extern uint32_t sys_inb_counter;

int kbd_subscribe_int(uint8_t *bit_no) {
    if (bit_no == NULL) return 1;
    *bit_no = hook_id;
    if (sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id) != 0) return 1;
    return 0;
}

int kbd_unsubscribe_int() {
    if (sys_irqrmpolicy(&hook_id) != 0) return 1;
    return 0;
}

void (kbc_ih)() {
    uint8_t status;
    error_found = false;

    if (util_sys_inb(KBD_STAT_REG, &status) != 0) {
        error_found = true;
        return;
    }

    if (status & KBD_OBF) {
        if (util_sys_inb(KBD_OUT_BUF, &scancode) != 0) {
            error_found = true;
            return;
        }
        if ((status & KBD_PARITY) || (status & KBD_TIMEOUT)) {
            error_found = true; 
        }
    } else {
        error_found = true; 
    }
}

int kbc_read_data_poll(uint8_t *data) {
    uint8_t status;
    while (1) {
        if (util_sys_inb(KBD_STAT_REG, &status) != 0) return 1;
        
        if (status & KBD_OBF) {
            if (util_sys_inb(KBD_OUT_BUF, data) != 0) return 1;
            if ((status & KBD_PARITY) || (status & KBD_TIMEOUT)) return 1;
            return 0;
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }
}

int kbc_restore_interrupts() {
    uint8_t cmd_byte;
    uint8_t status;

    while (1) {
        util_sys_inb(KBD_STAT_REG, &status);
        if ((status & KBD_IBF) == 0) {
            sys_outb(KBD_CMD_REG, KBC_READ_CMD);
            break;
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }

    while (1) {
        util_sys_inb(KBD_STAT_REG, &status);
        if (status & KBD_OBF) { 
            util_sys_inb(KBD_OUT_BUF, &cmd_byte);
            break;
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }

    cmd_byte |= KBC_INT_KBD;

    while (1) {
        util_sys_inb(KBD_STAT_REG, &status);
        if ((status & KBD_IBF) == 0) {
            sys_outb(KBD_CMD_REG, KBC_WRITE_CMD);
            break;
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }

    while (1) {
        util_sys_inb(KBD_STAT_REG, &status);
        if ((status & KBD_IBF) == 0) {
            sys_outb(KBD_IN_BUF, cmd_byte);
            break;
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }

    return 0;
}
