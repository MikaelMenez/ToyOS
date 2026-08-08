#include "stdint.h"
#include "fb.h"
#include "idt.h"
#include "pic.h"
#include "serial.h"
#include "pmm.h"
#include "usermode.h"
#include "vfs.h"   // Cabeçalho do VFS
#include "tarfs.h" // Cabeçalho do TARFS
#include "ramfs.h" // Cabeçalho do RAMFS

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

void kmain(uint32_t ebx) {
    extern uint32_t kernel_p_end;

    serial_init();
    fb_clear();
    pic_remap();
    idt_install();
    
    char *msg = "Bem vindo ao ToyOS - Cap 12 (VFS & RAMFS)";
    for (uint32_t i = 0; msg[i] != '\0'; i++) {
        fb_write_cell(i, msg[i], 0x0A, 0x00); 
    }

    multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;

    uint32_t safe_end = (uint32_t)&kernel_p_end;
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
        write_serial_str("Modulo encontrado. Lendo Initrd/TARFS...\n");
        
        multiboot_module_t *modules = (multiboot_module_t *) (mbinfo->mods_addr + 0xC0000000);
        uint32_t initrd_virtual_addr = modules[0].mod_start + 0xC0000000;
        
        // 1. Inicializa o Sistema de Arquivos TAR
        tarfs_init(initrd_virtual_addr);
        
        // 2. Leitura do "teste.txt"
        fs_node_t *arquivo1 = tarfs_find_file("teste.txt");
        if (arquivo1 != 0) {
            char buffer1[256];
            uint32_t bytes_lidos = vfs_read(arquivo1, 0, arquivo1->length, (uint8_t*)buffer1);
            buffer1[bytes_lidos] = '\0';
            write_serial_str("Lido do teste.txt: ");
            write_serial_str(buffer1);
            write_serial_str("\n");
        }

        // 2b. Leitura do "teste2.txt"
        fs_node_t *arquivo2 = tarfs_find_file("teste2.txt");
        if (arquivo2 != 0) {
            char buffer2[256];
            uint32_t bytes_lidos2 = vfs_read(arquivo2, 0, arquivo2->length, (uint8_t*)buffer2);
            buffer2[bytes_lidos2] = '\0';
            write_serial_str("Lido do teste2.txt: ");
            write_serial_str(buffer2);
            write_serial_str("\n");
        }
        
        /* COMENTADO TEMPORARIAMENTE: 
            Como o módulo 0 agora é o initrd.tar, se tentarmos pular para ele como Ring 3,
            o sistema vai tentar executar texto plano e vai dar Crash/Page Fault!
        */
        /*
        uint32_t user_pdt_phys = usermode_setup(modules[0].mod_start, modules[0].mod_end);
        write_serial_str("Saltando para o modo usuario (PL3) via IRET...\n");
        enter_usermode(user_pdt_phys); 
        */
    }

    // 3. Testa a criação de diretórios e imprime a árvore visual na serial
    write_serial_str("\n=== ESTRUTURA DO RAMFS (/) ===\n");
    write_serial_str("/ (Raiz)\n");
    
    ramfs_node_t *ramfs_root = ramfs_init_root();
    
    // Criando estrutura de teste
    ramfs_node_t *dir_docs = ramfs_mkdir(ramfs_root, "documentos");
    ramfs_mkdir(ramfs_root, "bin");
    ramfs_mkdir(ramfs_root, "drivers");
    
    if (dir_docs != 0) {
        ramfs_mkdir(dir_docs, "projetos");
        ramfs_node_t *file = ramfs_create_file(dir_docs, "nota.txt");
        if (file != 0) {
            const char *conteudo = "Exemplo de conteudo no RAMFS";
            ramfs_write(file, (const uint8_t *)conteudo, 28);
        }
    }

    // Imprime a árvore hierárquica no log serial
    ramfs_print_tree(ramfs_root, 1);
    write_serial_str("==============================\n\n");

    write_serial_str("Kernel aguardando...\n");
    while(1) { 
        asm volatile("hlt"); 
    }
}
