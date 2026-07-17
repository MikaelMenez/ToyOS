#include "serial.h"
#include "io.h"  

#define COM1 0x3F8

void serial_init() {
    outb(COM1 + 1, 0x00); // Desativa interrupções
    outb(COM1 + 3, 0x80); // Habilita configuração de DLAB
    outb(COM1 + 0, 0x03); // Define divisor de velocidade (baixa velocidade)
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03); // 8 bits, sem paridade, 1 stop bit
    outb(COM1 + 2, 0xC7); // Habilita FIFO, limpa buffers
    outb(COM1 + 4, 0x0B); // IRQs ativadas, RTS/DSR definido
}

// Envia um byte pela porta serial
void serial_write_byte(char c) {
    // Espera o hardware estar pronto (bit 5 do registro de status)
    while ((inb(COM1 + 5) & 0x20) == 0);
    outb(COM1, c);
}
