/* ============================================================================
 * vfs.c — A camada Virtual File System (VFS)
 * ----------------------------------------------------------------------------
 * É a "interface" que esconde QUAL sistema de arquivos está por baixo.
 * Em vez de cada parte do kernel conhecer RAMFS ou TARFS, ela conversa com
 * nós genéricos (fs_node_t). Cada nó guarda ponteiros de função (read/write);
 * o VFS só entrega a chamada pro sistema certo. Assim, amanhã se aparecer um
 * ext2, é só criar um nó que implemente read/write e o resto continua igual.
 * ============================================================================ */

#include "vfs.h"

fs_node_t *fs_root = 0; // Raiz do sistema de arquivos (começa nula)

uint32_t vfs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    // Se o nó tem uma função de leitura, chamamos ela
    if (node->read != 0) {
        return node->read(node, offset, size, buffer);
    } else {
        return 0; // esse nó não sabe ler, devolve "zero bytes lidos"
    }
}

uint32_t vfs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    // Se o nó tem uma função de escrita, chamamos ela
    if (node->write != 0) {
        return node->write(node, offset, size, buffer);
    } else {
        return 0; // nó sem escrita (ex: um TARFS é somente leitura)
    }
}