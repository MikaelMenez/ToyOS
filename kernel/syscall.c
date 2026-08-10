/* ============================================================================
 * syscall.c — O "balcão de atendimento" das system calls
 * ----------------------------------------------------------------------------
 * Quando um programa em Ring 3 dispara `int $0x80`, a CPU troca pro kernel
 * e essa função (chamada pelo stub em syscall_s.s) decide o que fazer com
 * base no número da chamada. Os programas de usuário usam os wrappers que
 * estão no syscall.h, então nunca precisam chamar isso diretamente.
 *
 * Vale notar: o kernel NUNCA confia no usuário — se um endereço for inválido,
 * azar do chamador (ainda sem proteção completa, mas já dá pra sentir o gosto).
 * ============================================================================ */

#include "stdint.h"
#include "syscall.h"
#include "fb.h"
#include "serial.h"
#include "io.h"
#include "ramfs.h"
#include "tarfs.h"
#include "vfs.h"
#include "console.h"

/* Tabela de scancodes que o kernel ja usa no idt.c */
extern uint8_t keyboard_map[128];

/* ------------------------------------------------------------------- */
/* Dispatcher: chamado pelo stub em syscall_s.s com
 *   eax = numero, ebx = a, ecx = b, edx = c, esi = d
 * Retorno vai para eax do modo usuario.
 * ------------------------------------------------------------------- */
uint32_t syscall_handler_c(uint32_t num, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    (void)b; (void)c; (void)d;

    switch (num) {
    /* --- I/O basico --- */
    case SYS_FB_WRITE:        // escreve uma célula do framebuffer
        fb_write_cell(a, (char)b, (uint8_t)c, (uint8_t)d);
        return 0;
    case SYS_FB_CLEAR:        // limpa a tela
        fb_clear();
        return 0;
    case SYS_SERIAL_CHAR:     // manda um char pro log serial
        serial_write_byte((char)a);
        return 0;
    case SYS_SERIAL_STR:      // manda uma string pro log serial
        write_serial_str((char *)a);
        return 0;
    case SYS_FB_PUTC:         // imprime um char no console do framebuffer
        fb_putc((char)a);
        return 0;
    case SYS_FB_PUTS:         // imprime uma string no framebuffer
        fb_puts((const char *)a);
        return 0;
    case SYS_KEYBOARD_READ:   // lê uma tecla (0 se ainda não há nada)
        if (inb(0x64) & 0x01) {
            uint8_t sc = inb(0x60);
            outb(0x20, 0x20); /* EOI para o PIC mestre (IRQ1) */
            if (sc < 0x80) {
                return keyboard_map[sc];
            }
        }
        return 0;

    /* --- RAMFS (sistema de arquivos em memória, cap. 12) --- */
    case SYS_RAMFS_INIT:      // retorna o handle da raiz '/'
        return (uint32_t)ramfs_init_root();
    case SYS_RAMFS_MKDIR:     // cria pasta
        return (uint32_t)ramfs_mkdir((ramfs_node_t *)a, (const char *)b);
    case SYS_RAMFS_CREATE:    // cria arquivo vazio
        return (uint32_t)ramfs_create_file((ramfs_node_t *)a, (const char *)b);
    case SYS_RAMFS_FIND:      // procura um filho pelo nome
        return (uint32_t)ramfs_find_child((ramfs_node_t *)a, (const char *)b);
    case SYS_RAMFS_WRITE:     // escreve dados num arquivo
        return (uint32_t)ramfs_write((ramfs_node_t *)a, (const uint8_t *)b, c);
    case SYS_RAMFS_READ:      // lê dados de um arquivo
        return ramfs_read((ramfs_node_t *)a, (uint8_t *)b, c, d);
    case SYS_RAMFS_REMOVE:    // remove um nó da árvore
        return (uint32_t)ramfs_remove((ramfs_node_t *)a);
    case SYS_RAMFS_PATH:      // monta o caminho absoluto de um nó
        ramfs_get_path((ramfs_node_t *)a, (char *)b, c);
        return 0;
    case SYS_RAMFS_TREE:      // imprime a árvore no log serial
        ramfs_print_tree((ramfs_node_t *)a, (int)b);
        return 0;
    case SYS_RAMFS_PARENT: {  // retorna o diretório pai
        ramfs_node_t *n = (ramfs_node_t *)a;
        return (n && n->parent) ? (uint32_t)n->parent : 0;
    }
    case SYS_RAMFS_MOVE:      // move ou renomeia um nó
        return (uint32_t)ramfs_move((ramfs_node_t *)a, (ramfs_node_t *)b, (const char *)c);
    case SYS_RAMFS_NAME:      // copia o nome do nó pro buffer
        ramfs_node_name((ramfs_node_t *)a, (char *)b, c);
        return 0;
    case SYS_RAMFS_CHILDREN:  // primeiro filho de um diretório
        return (uint32_t)ramfs_first_child((ramfs_node_t *)a);
    case SYS_RAMFS_SIBLING:   // próximo irmão de um nó
        return (uint32_t)ramfs_next_sibling((ramfs_node_t *)a);
    case SYS_RAMFS_MODE:      // flags do nó (arquivo ou diretório)
        return ((ramfs_node_t *)a)->flags;

    /* --- TARFS (read-only, o initrd do GRUB) --- */
    case SYS_TARFS_FIND:      // procura arquivo dentro do TAR
        return (uint32_t)tarfs_find_file((const char *)a);

    /* --- VFS generico (camada abstrata por cima dos dois) --- */
    case SYS_VFS_READ:        // lê via ponteiro de função do nó
        return vfs_read((fs_node_t *)a, b, c, (uint8_t *)d);
    case SYS_VFS_WRITE:       // escreve via ponteiro de função do nó
        return vfs_write((fs_node_t *)a, b, c, (uint8_t *)d);
    }

    return 0; // número desconhecido? Faz de conta que deu tudo certo...
}