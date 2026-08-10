/* ============================================================================
 * idt.c — A Tabela de Descritores de Interrupção (IDT)
 * ----------------------------------------------------------------------------
 * O processador, quando acontece uma interrupção ou exceção, olha pra essa
 * tabela pra saber qual função chamar. Aqui a gente instala:
 *
 *   - IRQ0 (timer, porta 32) e IRQ1 (teclado, porta 33);
 *   - a porta 0x80, que é o nosso gateway de system calls — os programas
 *     em Ring 3 disparam "int $0x80" pra pedir coisas pro kernel.
 *
 * Esse é o capítulo 6 do livro (interrupções e entrada). O teclado é lido
 * aqui mesmo, na função de tratamento, e tudo que você digita vai pra tela
 * e pro serial.log.
 * ============================================================================ */

#include "stdint.h"
#include "pic.h"
#include "io.h"
#include "fb.h"
#include "serial.h"

/* Entrada da IDT: pra onde pula o manuseador + qual segmento usar. */
struct idt_entry { uint16_t offset_low; uint16_t selector; uint8_t zero; uint8_t type_attr; uint16_t offset_high; } __attribute__((packed));
struct idt_ptr { uint16_t limit; uint32_t base; } __attribute__((packed));

struct idt_entry idt[256]; // a tabela em si (256 vetores)
struct idt_ptr idt_p;      // estrutura que a instrução LIDT espera

extern void load_idt(uint32_t idt_ptr); // definida em interrupts.s
extern void interrupt_handler_32();     // timer
extern void interrupt_handler_33();     // teclado
extern void syscall_entry();            // int $0x80

/* Preenche uma entrada da IDT apontando pro manuseador em assembly.
 * type_attr 0x8E = presente, gate de interrupção (privilegio do kernel). */
void idt_set_gate(uint32_t n, uint32_t handler) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08;
    idt[n].zero        = 0;
    idt[n].type_attr   = 0x8E;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

/* Igual ao de cima, mas com DPL configurável. Usamos isso pra porta 0x80
 * ficar acessível a partir do Ring 3 (DPL=3). Se não fosse assim, qualquer
 * "int $0x80" vindo do usuário geraria uma exceção de privilégio. */
void idt_set_gate_dpl(uint32_t n, uint32_t handler, uint8_t dpl) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08;
    idt[n].zero        = 0;
    idt[n].type_attr   = 0x8E | ((dpl & 0x03) << 5);
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

/* Instala a IDT: aponta todas as entradas pra um handler "mudo" (0) e depois
 * liga as que nos interessam. No fim, chama LIDT pra ativar. */
void idt_install() {
    idt_p.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_p.base  = (uint32_t) &idt;
    for(uint32_t i = 0; i < 256; i++) idt_set_gate(i, 0);
    idt_set_gate(32, (uint32_t)interrupt_handler_32);        // timer
    idt_set_gate(33, (uint32_t)interrupt_handler_33);        // teclado
    idt_set_gate_dpl(0x80, (uint32_t)syscall_entry, 3);      // syscalls do Ring 3
    load_idt((uint32_t)&idt_p);
}

/* Tabela que traduz o scancode do teclado (o número que a porta 0x60 entrega)
 * pro caractere ASCII correspondente. Baseada no layout americano. */
uint8_t keyboard_map[128] = { 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ' };

/* Onde o cursor "textual" está na tela (em células de 80x25). */
uint32_t cursor_pos = 80;

/* Tratador principal de interrupções (chamado pelo interrupts.s).
 * Atende o teclado: se há um byte esperando na porta 0x60, vira o scancode
 * em ASCII, imprime na tela e registra no serial.log. Depois "acena" pro PIC
 * pra liberar a próxima interrupção. */
void interrupt_handler() {
    if (inb(0x64) & 0x01) { // bit 0 da porta 0x64 = há tecla pronta pra ler
        uint8_t scancode = inb(0x60);

        if (scancode < 0x80) { // 0x80+ é tecla solta; só interessa pressionar
            uint8_t ascii = keyboard_map[scancode];
            if (ascii != 0) {
                // 1. Imprime na tela (Framebuffer)
                fb_write_cell(cursor_pos++, ascii, 0x0F, 0); // Texto branco

                // 2. Grava a interrupção no serial.log
                serial_write_byte('[');
                serial_write_byte('K');
                serial_write_byte('E');
                serial_write_byte('Y');
                serial_write_byte(']');
                serial_write_byte(' ');
                serial_write_byte(ascii);
                serial_write_byte('\n');
            }
        }
        pic_acknowledge(33); // IRQ1 — acusa recebimento do teclado
    } else {
        pic_acknowledge(32); // IRQ0 — timer, só acusa pra não repetir
    }
}