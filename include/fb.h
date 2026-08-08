#ifndef FB_H
#define FB_H

#include "stdint.h" // Adicionar isso no topo

// Atualizar os tipos para bater com o fb.c
void fb_write_cell(uint32_t i, char c, uint8_t fg, uint8_t bg);
void fb_clear();
void fb_putc(char c);
void fb_puts(const char *s);
void fb_set_cursor_pos(uint32_t cell);
void fb_hw_cursor(uint16_t pos);

#endif
