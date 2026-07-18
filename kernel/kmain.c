#include "fb.h"
#include "idt.h"
#include "pic.h"
#include "serial.h"

// Estrutura do Multiboot passada pelo GRUB via EBX
typedef struct {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    unsigned int syms[4];
    unsigned int mmap_length;
    unsigned int mmap_addr;
    unsigned int drives_length;
    unsigned int drives_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
    unsigned int apm_table;
    unsigned int vbe_control_info;
    unsigned int vbe_mode_info;
    unsigned int vbe_mode;
    unsigned int vbe_interface_seg;
    unsigned int vbe_interface_off;
    unsigned int vbe_interface_len;
} __attribute__((packed)) multiboot_info_t;

// Estrutura que guarda as informações de cada módulo carregado
typedef struct {
    unsigned int mod_start;
    unsigned int mod_end;
    unsigned int cmdline;
    unsigned int reserved;
} __attribute__((packed)) multiboot_module_t;

void write_serial_str(char *s) {
    while (*s != '\0') {
        serial_write_byte(*s);
        s++;
    }
}

void kmain(unsigned int ebx) {
    serial_init();
    fb_clear();
    pic_remap();
    idt_install();

    write_serial_str("Kernel inicializado. Detectando modulos...\n");

    multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;

    if ((mbinfo->flags & 0x008) != 0) {
        if (mbinfo->mods_count > 0) {
            write_serial_str("Mods:1\n");
            
            // 1. Aponta para a lista de módulos
            multiboot_module_t *modules = (multiboot_module_t *) mbinfo->mods_addr;
            
            // 2. Pega o endereço inicial do PRIMEIRO módulo carregado
            unsigned int module_start = modules[0].mod_start;
            
            // 3. Salta para o programa real
            void (*start_program)(void) = (void (*)(void)) module_start;
            start_program(); 
        }
    }

    // Loop infinito caso o salto falhe
    asm volatile("sti");
    while(1) { }
}
