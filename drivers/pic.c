#include "stdint.h"
#include "pic.h"
#include "io.h"

void pic_remap() {
    // Remapeia as IRQs dos PICs pra não bater de frente com as exceções internas da CPU
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0x00); outb(0xA1, 0x00);
}

void pic_acknowledge(uint32_t interrupt) {
    // Se veio do PIC escravo (IRQs 8-15)
    if (interrupt >= 0x28) outb(0xA0, 0x20);
    
    // Sempre avisa o mestre
    outb(0x20, 0x20);
}
