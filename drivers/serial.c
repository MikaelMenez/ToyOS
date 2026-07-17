#include "io.h"

#define SERIAL_COM1_BASE 0x3F8
#define SERIAL_DATA_PORT(base) (base)
#define SERIAL_LINE_COMMAND_PORT(base) (base + 3)
#define SERIAL_LINE_STATUS_PORT(base) (base + 5)
#define SERIAL_LINE_ENABLE_DLAB 0x80

void serial_configure_baud_rate(unsigned short com, unsigned short divisor) {
    outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
    outb(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);
    outb(SERIAL_DATA_PORT(com), divisor & 0x00FF);
}

void serial_configure_line(unsigned short com) {
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

int serial_is_transmit_fifo_empty(unsigned int com) {
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

void serial_write(unsigned int com, char *buf, unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        while (serial_is_transmit_fifo_empty(com) == 0);
        outb(SERIAL_DATA_PORT(com), buf[i]);
    }
}
