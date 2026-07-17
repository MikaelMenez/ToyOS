#include "fb.h"
#include "io.h"

// Endereço de memória de vídeo VGA
char *fb = (char *) 0x000B8000;

void fb_write_cell(uint32_t i, char c, uint8_t fg, uint8_t bg) {
    fb[i * 2] = c;                                // Escreve o caractere
    fb[i * 2 + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F); // Define cores
}

void fb_clear() {
    // Preenche a tela com espaços para limpar
    for (uint32_t i = 0; i < 80 * 25; i++) {
        fb_write_cell(i, ' ', 0, 0);
    }
}
