#include "fb.h"
#include "idt.h"
#include "pic.h"

void kmain() {
    fb_clear(); // Limpa a tela para começar do zero
    
    pic_remap();    // Configura o PIC
    idt_install();  // Instala nossa tabela de interrupções
    
    asm volatile("sti"); // Abre a porta para as interrupções de hardware
    
    while(1) { 
        
    }
}
