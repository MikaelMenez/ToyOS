#include "stdint.h"
#include "io.h"

// O endereço de memória onde a placa de vídeo mapeia o texto na tela
char *fb = (char *) 0x000B8000;

void fb_write_cell(uint32_t i, char c, uint8_t fg, uint8_t bg) {
    // Escreve o caractere na memória de vídeo
    fb[i * 2] = c;
    // Define a cor do caractere e do fundo (bitmask)
    fb[i * 2 + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

void fb_clear() {
    // Simplesmente preenchemos toda a tela com espaços vazios
    for (uint32_t i = 0; i < 80 * 25; i++) {
        fb_write_cell(i, ' ', 0, 0);
    }
}
