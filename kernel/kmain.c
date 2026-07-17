#include "fb.h"
#include "idt.h"
#include "pic.h"
#include "serial.h" 

void kmain() {
    fb_clear();
    serial_init();  
    pic_remap();
    idt_install();
    
    asm volatile("sti");
    while(1) { }
}
