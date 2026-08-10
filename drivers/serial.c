/* ============================================================================
 * serial.c — Driver da porta serial COM1
 * ----------------------------------------------------------------------------
 * A porta serial (também capítulo 4 do livro) é nossa "impressora" de debug:
 * tudo que o kernel quer reportar vai pra cá e o QEMU grava no `serial.log`.
 * Depois de configurada, é só ir mandando bytes pela porta 0x3F8.
 * ============================================================================ */

#include "stdint.h"
#include "serial.h"
#include "io.h"

#define COM1 0x3F8

/* Configura a porta serial no padrão clássico: 8 bits, sem paridade, 1 stop
 * bit, com filas FIFO ligadas e pins prontos pra mandar dados. */
void serial_init() {
    outb(COM1 + 1, 0x00); // Desliga interrupções da porta serial por enquanto
    outb(COM1 + 3, 0x80); // Libera o ajuste de velocidade (DLAB)
    outb(COM1 + 0, 0x03); // Velocidade baixa (divisor = 3)
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03); // Padrão clássico: 8 bits, sem paridade, 1 stop bit
    outb(COM1 + 2, 0xC7); // Ativa as filas FIFO e dá flush
    outb(COM1 + 4, 0x0B); // Prepara os pinos de comunicação
}

/* Manda um caractere pra porta serial. Segura o loop até o hardware dizer
 * que está pronto pra receber mais (bit 5 do status, port+5). */
void serial_write_byte(char c) {
    // Segura o loop até o bit 5 avisar que o hardware tá livre pra enviar
    while ((inb(COM1 + 5) & 0x20) == 0);
    outb(COM1, c);
}