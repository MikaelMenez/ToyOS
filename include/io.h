#ifndef _IO_H
#define _IO_H
#include "stdint.h"

// Protótipos para as funções de acesso a portas de E/S
extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);
#endif
