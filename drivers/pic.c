/* ============================================================================
 * pic.c — Programmable Interrupt Controller (capítulo 6)
 * ----------------------------------------------------------------------------
 * Antes de usar interrupções de hardware, a gente precisa "remascarar" os dois
 * PICs (mestre e escravo). Por padrão eles disparam as IRQs nas portas 0-15,
 * mas essas portas são exatamente as exceções internas da CPU! Por isso
 * remapeamos: IRQ0-7 viram 0x20-0x27 e IRQ8-15 viram 0x28-0x2F. Assim o
 * teclado (IRQ1) cai na porta 33, que é a que configuramos no idt.c.
 * ============================================================================ */

#include "stdint.h"
#include "pic.h"
#include "io.h"

/* Reconfigura os dois PICs com os novos offsets. A sequência de bytes é o
 * "protocolo" clássico de inicialização (ICW1..ICW4). */
void pic_remap() {
    // Remapeia as IRQs dos PICs pra não bater de frente com as exceções internas da CPU
    outb(0x20, 0x11); outb(0xA0, 0x11); // ICW1: modo cascata
    outb(0x21, 0x20); outb(0xA1, 0x28); // ICW2: novos offsets (0x20 e 0x28)
    outb(0x21, 0x04); outb(0xA1, 0x02); // ICW3: mestre/escravo conectados
    outb(0x21, 0x01); outb(0xA1, 0x01); // ICW4: modo 8086
    outb(0x21, 0x00); outb(0xA1, 0x00); // OCW1: libera todas as interrupções
}

/* "Acusa recebimento" (End of Interrupt) ao PIC. Se não mandarmos esse EOI,
 * o PIC acha que a gente continua ocupado e nunca dispara a próxima IRQ. */
void pic_acknowledge(uint32_t interrupt) {

    // Se a IRQ veio do escravo (0x28+), o escravo também precisa do EOI
    if (interrupt >= 0x28) outb(0xA0, 0x20);

    // Sempre avisa o mestre
    outb(0x20, 0x20);
}