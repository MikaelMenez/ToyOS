#ifndef _SERIAL_H
#define _SERIAL_H

#include "stdint.h"

//funções para inicializar e escrever na porta serial
void serial_init();
void serial_write(char *str);

#endif
