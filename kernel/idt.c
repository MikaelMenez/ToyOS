#include "stdint.h"
#include "pic.h"
#include "io.h"
#include "fb.h"

// Estruturas da Tabela de Descritores de Interrupção
struct idt_entry { uint16_t offset_low; uint16_t selector; uint8_t zero; uint8_t type_attr; uint16_t offset_high; } __attribute__((packed));
struct idt_ptr { uint16_t limit; uint32_t base; } __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idt_p;

extern void load_idt(uint32_t idt_ptr);
extern void interrupt_handler_32();
extern void interrupt_handler_33();

void idt_set_gate(int n, uint32_t handler) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08;
    idt[n].zero        = 0;
    idt[n].type_attr   = 0x8E;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_install() {
    idt_p.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_p.base  = (uint32_t) &idt;
    for(int i = 0; i < 256; i++) idt_set_gate(i, 0);
    idt_set_gate(32, (uint32_t)interrupt_handler_32);
    idt_set_gate(33, (uint32_t)interrupt_handler_33);
    load_idt((uint32_t)&idt_p);
}

// Mapeamento simples de scancodes para caracteres ASCII
uint8_t keyboard_map[128] = { 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ' };
uint32_t cursor_pos = 80;

void interrupt_handler() {
    if (inb(0x64) & 0x01) { // Verifica se há dados no teclado
        uint8_t scancode = inb(0x60); 
        if (scancode < 0x80) { // Tecla pressionada
            uint8_t ascii = keyboard_map[scancode];
            if (ascii != 0) {
                fb_write_cell(cursor_pos++, ascii, 0x0A, 0);
            }
        }
        pic_acknowledge(33); // Reconhece interrupção do teclado
    } else {
        pic_acknowledge(32); // Reconhece interrupção do timer
    }
}
