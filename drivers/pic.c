#include "stdint.h"
#include "io.h"

void pic_remap() {
    // Sequência padrão para remapear o PIC para um intervalo seguro (32-47)
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0x00); outb(0xA1, 0x00);
}


void pic_acknowledge(uint32_t interrupt) {
    if (interrupt >= 0x28) outb(0xA0, 0x20); // Se for o PIC secundário
    outb(0x20, 0x20); // Avisa o PIC primário
}
