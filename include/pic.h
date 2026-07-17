#ifndef _PIC_H
#define _PIC_H
#include "stdint.h"

// Gerenciamento do controlador de interrupções
void pic_remap();
void pic_acknowledge(uint32_t interrupt);
#endif
