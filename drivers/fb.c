#include "stdint.h"
#include "fb.h"
#include "io.h"

// Usando uint8_t para ponteiro de memória crua garante matemática de ponteiro exata
uint8_t *fb = (uint8_t *) 0x000B8000;

void fb_write_cell(uint32_t i, char c, uint8_t fg, uint8_t bg) {
    uint32_t offset = i * 2; 
    fb[offset] = c;
    fb[offset + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

void fb_clear() {
    // Roda a tela toda preenchendo com vazios
    for (uint32_t i = 0; i < 80 * 25; i++) {
        fb_write_cell(i, ' ', 0, 0);
    }
}
