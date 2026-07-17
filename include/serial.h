#ifndef _SERIAL_H
#define _SERIAL_H

#include "stdint.h"
#include "io.h"

void serial_init();
void serial_write_byte(char c);

#endif
