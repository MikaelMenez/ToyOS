#include "stdint.h"
#include "fb.h"
#include "idt.h"
#include "pic.h"
#include "serial.h"
#include "pmm.h"

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint32_t vbe_mode;
    uint32_t vbe_interface_seg;
    uint32_t vbe_interface_off;
    uint32_t vbe_interface_len;
} __attribute__((packed)) multiboot_info_t;

typedef struct {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t cmdline;
    uint32_t reserved;
} __attribute__((packed)) multiboot_module_t;

void write_serial_str(char *s) {
    while (*s) {
        serial_write_byte(*s++);
    }
}

void kmain(uint32_t ebx, uint32_t kernel_v_start, uint32_t kernel_v_end, uint32_t kernel_p_start, uint32_t kernel_p_end) {
    (void)kernel_v_start;
    (void)kernel_v_end;
    (void)kernel_p_start;
    (void)kernel_p_end;

    serial_init();
    fb_clear();
    pic_remap();
    idt_install();
    
    char *msg = "Bem vindo ao ToyOS - Cap 10";
    for (uint32_t i = 0; msg[i] != '\0'; i++) {
        fb_write_cell(i, msg[i], 0x0A, 0x00); 
    }

    multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;

    uint32_t safe_end = kernel_p_end;
    if ((mbinfo->flags & 0x008) != 0 && mbinfo->mods_count > 0) {
        multiboot_module_t *modules = (multiboot_module_t *) (mbinfo->mods_addr + 0xC0000000);
        if (modules[0].mod_end > safe_end) {
            safe_end = modules[0].mod_end;
        }
    }

    if ((mbinfo->flags & 0x40) != 0) {
        pmm_init(mbinfo->mmap_addr, mbinfo->mmap_length, mbinfo->mem_lower, mbinfo->mem_upper, safe_end);
        write_serial_str("PMM Inicializado com sucesso!\n");
    }

    asm volatile("sti");
    write_serial_str("Kernel inicializado na Metade Superior...\n");

    if ((mbinfo->flags & 0x008) != 0 && mbinfo->mods_count > 0) {
        write_serial_str("Modulo carregado e pronto para execucao...\n");
        multiboot_module_t *modules = (multiboot_module_t *) (mbinfo->mods_addr + 0xC0000000);
        uint32_t module_start = modules[0].mod_start + 0xC0000000;
        
        void (*start_program)(void) = (void (*)(void)) module_start;
        
        write_serial_str("Saltando para o codigo do modulo externo...\n");
        start_program(); 
        
        write_serial_str("Modulo executado e retornou com sucesso!\n");
    }

    write_serial_str("Kernel travado estavelmente em loop infinito.\n");
    while(1) { 
        asm volatile("hlt"); 
    }
}
