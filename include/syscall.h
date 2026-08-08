#ifndef SYSCALL_H
#define SYSCALL_H

#include "stdint.h"

/* ============================================================
 * GATE I/O básico
 * ============================================================ */
#define SYS_FB_WRITE      0   /* a=celula, b=char, c=fg, d=bg      */
#define SYS_FB_CLEAR      1
#define SYS_SERIAL_CHAR   2   /* a=char                               */
#define SYS_SERIAL_STR    3   /* a: char*                            */
#define SYS_KEYBOARD_READ 4   /* retorna char lido (0 se nada)      */
#define SYS_FB_PUTC       5   /* a=char no console do framebuffer   */
#define SYS_FB_PUTS       6   /* a: char* no framebuffer            */

/* ============================================================
 * SISTEMA DE ARQUIVOS (RAMFS)
 * Todos os handles são ponteiros de kernel retornados como uint32.
 * ============================================================ */
#define SYS_RAMFS_INIT    10  /* () -> raiz '/'                       */
#define SYS_RAMFS_MKDIR   11  /* (parent, name) -> handle             */
#define SYS_RAMFS_CREATE  12  /* (parent, name) -> handle             */
#define SYS_RAMFS_FIND    13  /* (parent, name) -> handle             */
#define SYS_RAMFS_WRITE   14  /* (file, data, size) -> bytes          */
#define SYS_RAMFS_READ    15  /* (file, buf, offset, size) -> bytes   */
#define SYS_RAMFS_REMOVE  16  /* (node) -> 0 sucesso / -1 erro        */
#define SYS_RAMFS_PATH    17  /* (node, buf, maxlen)                  */
#define SYS_RAMFS_TREE    18  /* (node, indent)                       */
#define SYS_RAMFS_PARENT  19  /* (node) -> handle do pai / 0          */
#define SYS_RAMFS_MOVE    21  /* (src, dest_dir, new_name) -> 0/-1    */
#define SYS_RAMFS_NAME    22  /* (node, buf, maxlen)                  */
#define SYS_RAMFS_CHILDREN 23 /* (dir) -> primeiro filho (handle)     */
#define SYS_RAMFS_SIBLING 24  /* (node) -> proximo irmao (handle)     */
#define SYS_RAMFS_MODE    25  /* (node) -> flags (FILE/DIR)           */

#define SYS_TARFS_FIND    20  /* (name) -> fs_node_t* (read-only)     */

#define SYS_VFS_READ      30  /* (node, offset, size, buf) -> bytes   */
#define SYS_VFS_WRITE     31  /* (node, offset, size, buf) -> bytes   */

/* Dispatcher do syscall via int $0x80 (eax=n, ebx=a, ecx=b, edx=c, esi=d) */
static inline uint32_t syscall(uint32_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    uint32_t ret;
    __asm__ volatile("int $0x80" : "=a"(ret)
                     : "a"(n), "b"(a), "c"(b), "d"(c), "S"(d)
                     : "memory");
    return ret;
}

/* =============== wrappers bonitos =============== */

static inline void sys_fb_write(uint32_t cell, char ch, uint8_t fg, uint8_t bg) {
    syscall(SYS_FB_WRITE, cell, (uint32_t)ch, fg, bg);
}
static inline void sys_fb_clear(void) {
    syscall(SYS_FB_CLEAR, 0, 0, 0, 0);
}
static inline void sys_serial_char(char c) {
    syscall(SYS_SERIAL_CHAR, (uint32_t)c, 0, 0, 0);
}
static inline void sys_print(const char *s) {
    syscall(SYS_SERIAL_STR, (uint32_t)s, 0, 0, 0);
    syscall(SYS_FB_PUTS, (uint32_t)s, 0, 0, 0);
}
static inline void sys_fb_putc(char c) {
    syscall(SYS_FB_PUTC, (uint32_t)c, 0, 0, 0);
}
static inline void sys_putc(char c) {
    sys_serial_char(c);
    sys_fb_putc(c);
}
static inline char sys_read_key(void) {
    return (char)syscall(SYS_KEYBOARD_READ, 0, 0, 0, 0);
}

static inline uint32_t sys_ramfs_init(void) {
    return syscall(SYS_RAMFS_INIT, 0, 0, 0, 0);
}
static inline uint32_t sys_ramfs_mkdir(uint32_t parent, const char *name) {
    return syscall(SYS_RAMFS_MKDIR, parent, (uint32_t)name, 0, 0);
}
static inline uint32_t sys_ramfs_create(uint32_t parent, const char *name) {
    return syscall(SYS_RAMFS_CREATE, parent, (uint32_t)name, 0, 0);
}
static inline uint32_t sys_ramfs_find(uint32_t parent, const char *name) {
    return syscall(SYS_RAMFS_FIND, parent, (uint32_t)name, 0, 0);
}
static inline uint32_t sys_ramfs_write(uint32_t file, const char *data, uint32_t size) {
    return syscall(SYS_RAMFS_WRITE, file, (uint32_t)data, size, 0);
}
static inline uint32_t sys_ramfs_read(uint32_t file, char *buf, uint32_t offset, uint32_t size) {
    return syscall(SYS_RAMFS_READ, file, (uint32_t)buf, offset, size);
}
static inline int sys_ramfs_remove(uint32_t node) {
    return (int)syscall(SYS_RAMFS_REMOVE, node, 0, 0, 0);
}
static inline void sys_ramfs_path(uint32_t node, char *buf, uint32_t len) {
    syscall(SYS_RAMFS_PATH, node, (uint32_t)buf, len, 0);
}
static inline void sys_ramfs_tree(uint32_t node, int indent) {
    syscall(SYS_RAMFS_TREE, node, (uint32_t)indent, 0, 0);
}
static inline uint32_t sys_ramfs_parent(uint32_t node) {
    return syscall(SYS_RAMFS_PARENT, node, 0, 0, 0);
}
static inline int sys_ramfs_move(uint32_t src, uint32_t dst_dir, const char *new_name) {
    return (int)syscall(SYS_RAMFS_MOVE, src, dst_dir, new_name ? (uint32_t)new_name : 0, 0);
}
static inline void sys_ramfs_name(uint32_t node, char *buf, uint32_t maxlen) {
    syscall(SYS_RAMFS_NAME, node, (uint32_t)buf, maxlen, 0);
}
static inline uint32_t sys_ramfs_children(uint32_t dir) {
    return syscall(SYS_RAMFS_CHILDREN, dir, 0, 0, 0);
}
static inline uint32_t sys_ramfs_sibling(uint32_t node) {
    return syscall(SYS_RAMFS_SIBLING, node, 0, 0, 0);
}
static inline uint32_t sys_ramfs_mode(uint32_t node) {
    return syscall(SYS_RAMFS_MODE, node, 0, 0, 0);
}

static inline uint32_t sys_tarfs_find(const char *name) {
    return syscall(SYS_TARFS_FIND, (uint32_t)name, 0, 0, 0);
}
static inline uint32_t sys_vfs_read(uint32_t node, uint32_t offset, uint32_t size, char *buf) {
    return syscall(SYS_VFS_READ, node, offset, size, (uint32_t)buf);
}
static inline uint32_t sys_vfs_write(uint32_t node, uint32_t offset, uint32_t size, char *buf) {
    return syscall(SYS_VFS_WRITE, node, offset, size, (uint32_t)buf);
}

#endif