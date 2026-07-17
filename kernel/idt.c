#include "stdint.h"
#include "pic.h"
#include "io.h"
#include "fb.h"

// Estrutura que o processador espera para entender uma interrupção
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idt_p;

// Funções definidas no assembly
extern void load_idt(uint32_t idt_ptr);
extern void interrupt_handler_32();
extern void interrupt_handler_33();

// Prepara uma entrada na tabela para um evento específico
void idt_set_gate(int n, uint32_t handler) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08; // Seletor de código do kernel
    idt[n].zero        = 0;
    idt[n].type_attr   = 0x8E; // Define que é um interrupt gate de 32 bits
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_install() {
    idt_p.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_p.base  = (uint32_t) &idt;

    // Limpa a tabela antes de começar
    for(int i = 0; i < 256; i++) idt_set_gate(i, 0);

    // Registra nossas interrupções de hardware
    idt_set_gate(32, (uint32_t)interrupt_handler_32);
    idt_set_gate(33, (uint32_t)interrupt_handler_33);

    load_idt((uint32_t)&idt_p);
}

// Mapa simples para traduzir o clique do teclado para letras
uint8_t keyboard_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

uint32_t cursor_pos = 80; // Começa na segunda linha

void interrupt_handler() {
    // Verificamos se o teclado realmente tem um dado para nós
    if (inb(0x64) & 0x01) {
        uint8_t scancode = inb(0x60); 
        
        // Só processamos se for uma tecla sendo apertada (Make code)
        if (scancode < 0x80) {
            uint8_t ascii = keyboard_map[scancode];
            if (ascii != 0) {
                fb_write_cell(cursor_pos++, ascii, 0x0A, 0); 
            }
        }
        pic_acknowledge(33); // Avisa que terminamos com o teclado
    } else {
        pic_acknowledge(32); // Avisa que terminamos com o timer
    }
}
