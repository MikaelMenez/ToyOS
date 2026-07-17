#include "pic.h"
#include "io.h"

void pic_remap() {
    // Reconfigura os PICs para evitar conflitos com interrupções da CPU
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0x00); outb(0xA1, 0x00);
}

void pic_acknowledge(uint32_t interrupt) {
    // Notifica ao PIC que o processamento da interrupção terminou
    if (interrupt >= 0x28) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
