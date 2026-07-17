#ifndef _IO_H
#define _IO_H

#include "stdint.h"

// Envia um byte para uma porta de hardware (ex: PIC, Teclado)
extern void outb(uint16_t port, uint8_t data);

// Lê um byte de uma porta de hardware
extern uint8_t inb(uint16_t port);

#endif
