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
    case SYS_FB_WRITE:
        fb_write_cell(a, (char)b, (uint8_t)c, (uint8_t)d);
        return 0;
    case SYS_FB_CLEAR:
        fb_clear();
        return 0;
    case SYS_SERIAL_CHAR:
        serial_write_byte((char)a);
        return 0;
    case SYS_SERIAL_STR:
        write_serial_str((char *)a);
        return 0;
    case SYS_FB_PUTC:
        fb_putc((char)a);
        return 0;
    case SYS_FB_PUTS:
        fb_puts((const char *)a);
        return 0;
    case SYS_KEYBOARD_READ:
        if (inb(0x64) & 0x01) {
            uint8_t sc = inb(0x60);
            outb(0x20, 0x20); /* EOI para o PIC mestre (IRQ1) */
            if (sc < 0x80) {
                return keyboard_map[sc];
            }
        }
        return 0;

    /* --- RAMFS --- */
    case SYS_RAMFS_INIT:
        return (uint32_t)ramfs_init_root();
    case SYS_RAMFS_MKDIR:
        return (uint32_t)ramfs_mkdir((ramfs_node_t *)a, (const char *)b);
    case SYS_RAMFS_CREATE:
        return (uint32_t)ramfs_create_file((ramfs_node_t *)a, (const char *)b);
    case SYS_RAMFS_FIND:
        return (uint32_t)ramfs_find_child((ramfs_node_t *)a, (const char *)b);
    case SYS_RAMFS_WRITE:
        return (uint32_t)ramfs_write((ramfs_node_t *)a, (const uint8_t *)b, c);
    case SYS_RAMFS_READ:
        return ramfs_read((ramfs_node_t *)a, (uint8_t *)b, c, d);
    case SYS_RAMFS_REMOVE:
        return (uint32_t)ramfs_remove((ramfs_node_t *)a);
    case SYS_RAMFS_PATH:
        ramfs_get_path((ramfs_node_t *)a, (char *)b, c);
        return 0;
    case SYS_RAMFS_TREE:
        ramfs_print_tree((ramfs_node_t *)a, (int)b);
        return 0;
    case SYS_RAMFS_PARENT: {
        ramfs_node_t *n = (ramfs_node_t *)a;
        return (n && n->parent) ? (uint32_t)n->parent : 0;
    }
    case SYS_RAMFS_MOVE:
        return (uint32_t)ramfs_move((ramfs_node_t *)a, (ramfs_node_t *)b, (const char *)c);
    case SYS_RAMFS_NAME:
        ramfs_node_name((ramfs_node_t *)a, (char *)b, c);
        return 0;
    case SYS_RAMFS_CHILDREN:
        return (uint32_t)ramfs_first_child((ramfs_node_t *)a);
    case SYS_RAMFS_SIBLING:
        return (uint32_t)ramfs_next_sibling((ramfs_node_t *)a);
    case SYS_RAMFS_MODE:
        return ((ramfs_node_t *)a)->flags;

    /* --- TARFS (read-only) --- */
    case SYS_TARFS_FIND:
        return (uint32_t)tarfs_find_file((const char *)a);

    /* --- VFS generico --- */
    case SYS_VFS_READ:
        return vfs_read((fs_node_t *)a, b, c, (uint8_t *)d);
    case SYS_VFS_WRITE:
        return vfs_write((fs_node_t *)a, b, c, (uint8_t *)d);
    }

    return 0;
}