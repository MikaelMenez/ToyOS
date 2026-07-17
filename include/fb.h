#ifndef _FB_H
#define _FB_H
#include "stdint.h"

// Funções para manipular a saída de vídeo (Framebuffer)
void fb_write_cell(uint32_t i, char c, uint8_t fg, uint8_t bg);
void fb_clear();
#endif
