#ifndef FB_H
#define FB_H

#include "stdint.h" // Adicionar isso no topo

// Atualizar os tipos para bater com o fb.c
void fb_write_cell(uint32_t i, char c, uint8_t fg, uint8_t bg);
void fb_clear();

#endif
