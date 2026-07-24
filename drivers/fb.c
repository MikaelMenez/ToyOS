#include "stdint.h"
#include "fb.h"
#include "io.h"


uint8_t *fb = (uint8_t *) 0xC00B8000;

void fb_write_cell(uint32_t i, char c, uint8_t fg, uint8_t bg) {
    uint32_t offset = i * 2; 
    fb[offset] = c;
    fb[offset + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

void fb_clear() {
    for (uint32_t i = 0; i < 80 * 25; i++) {
        fb_write_cell(i, ' ', 0, 0);
    }
}
