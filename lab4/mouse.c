#include "mouse.h"
#include "i8042.h"
#include <minix/sysutil.h>

static int mouse_hook_id = 2;
uint8_t mouse_byte = 0;
bool mouse_error = false;

int (mouse_subscribe_int)(uint8_t *bit_no) {
    if (bit_no == NULL) return 1;
    *bit_no = mouse_hook_id;
    if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook_id) != 0) return 1;
    return 0;
}

int (mouse_unsubscribe_int)() {
    if (sys_irqrmpolicy(&mouse_hook_id) != 0) return 1;
    return 0;
}

void (mouse_ih)() {
    uint8_t status;
    mouse_error = false;

    if (util_sys_inb(KBC_STAT_REG, &status) != 0) {
        mouse_error = true;
        return;
    }

    if ((status & KBC_OBF) && (status & KBC_AUX)) {
        if (util_sys_inb(KBC_OUT_BUF, &mouse_byte) != 0) {
            mouse_error = true;
            return;
        }
        if ((status & KBC_PARITY) || (status & KBC_TIMEOUT)) {
            mouse_error = true;
        }
    } else {
        mouse_error = true;
    }
}

int (mouse_write_cmd)(uint8_t cmd) {
    uint8_t ack_byte;
    uint8_t status;
    int retries = 5;

    while (retries > 0) {
        util_sys_inb(KBC_STAT_REG, &status);
        if ((status & KBC_IBF) == 0) {
            sys_outb(KBC_CMD_REG, WRITE_TO_MOUSE);
        } else {
            tickdelay(micros_to_ticks(DELAY_US));
            continue;
        }

        tickdelay(micros_to_ticks(DELAY_US));

        util_sys_inb(KBC_STAT_REG, &status);
        if ((status & KBC_IBF) == 0) {
            sys_outb(KBC_IN_BUF, cmd);
        } else {
            tickdelay(micros_to_ticks(DELAY_US));
            continue;
        }

        tickdelay(micros_to_ticks(DELAY_US));
        util_sys_inb(KBC_STAT_REG, &status);
        if (status & KBC_OBF) {
            util_sys_inb(KBC_OUT_BUF, &ack_byte);
            if (ack_byte == ACK) return 0;
            if (ack_byte == NACK || ack_byte == ERROR) {
                retries--;
                continue;
            }
        }
        retries--;
    }
    return 1;
}

void (mouse_parse_packet)(const uint8_t *packet_bytes, struct packet *pp) {
    pp->bytes[0] = packet_bytes[0];
    pp->bytes[1] = packet_bytes[1];
    pp->bytes[2] = packet_bytes[2];

    pp->lb = packet_bytes[0] & BIT(0);
    pp->rb = packet_bytes[0] & BIT(1);
    pp->mb = packet_bytes[0] & BIT(2);
    
    pp->delta_x = packet_bytes[1];
    if (packet_bytes[0] & BIT(4)) pp->delta_x |= 0xFF00;

    pp->delta_y = packet_bytes[2];
    if (packet_bytes[0] & BIT(5)) pp->delta_y |= 0xFF00;

    pp->x_ov = packet_bytes[0] & BIT(6);
    pp->y_ov = packet_bytes[0] & BIT(7);
}
