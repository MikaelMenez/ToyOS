#include "stdint.h"
#include "fb.h"
#include "io.h"


uint8_t *fb = (uint8_t *) 0xC00B8000;

static uint32_t fb_pos = 0; /* cursor do console de texto */

void fb_write_cell(uint32_t i, char c, uint8_t fg, uint8_t bg) {
    uint32_t offset = i * 2; 
    fb[offset] = c;
    fb[offset + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

void fb_clear() {
    for (uint32_t i = 0; i < 80 * 25; i++) {
        fb_write_cell(i, ' ', 0, 0);
    }
    fb_pos = 0;
}

void fb_set_cursor_pos(uint32_t cell) {
    fb_pos = cell;
    fb_hw_cursor((uint16_t)cell);
}

/* Move o cursor visivel/piscante do hardware VGA (portas 0x3D4/0x3D5) */
void fb_hw_cursor(uint16_t pos) {
    outb(0x3D4, 0x0E);
    outb(0x3D5, (pos >> 8) & 0xFF);
    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);
}

/* Sobe uma linha e limpa a ultima (rolagem simples) */
static void fb_scroll(void) {
    for (uint32_t i = 0; i < 80 * 24; i++) {
        fb[i * 2]     = fb[(i + 80) * 2];
        fb[i * 2 + 1] = fb[(i + 80) * 2 + 1];
    }
    for (uint32_t i = 80 * 24; i < 80 * 25; i++) {
        fb_write_cell(i, ' ', 0x0F, 0x00);
    }
    fb_pos = 80 * 24;
}

/* Escreve um caractere no console de texto do framebuffer */
void fb_putc(char c) {
    if (c == '\n') {
        fb_pos = ((fb_pos / 80) + 1) * 80;
    } else if (c == '\r') {
        fb_pos = (fb_pos / 80) * 80;
    } else if (c == '\b') {
        if (fb_pos > 0) fb_pos--;
        fb_write_cell(fb_pos, ' ', 0x0F, 0x00);
    } else {
        fb_write_cell(fb_pos, c, 0x0F, 0x00);
        fb_pos++;
    }
    if (fb_pos >= 80 * 25) fb_scroll();
    fb_hw_cursor((uint16_t)(fb_pos >= 80 * 25 ? 80 * 25 - 1 : fb_pos));
}

void fb_puts(const char *s) {
    while (*s) fb_putc(*s++);
}
