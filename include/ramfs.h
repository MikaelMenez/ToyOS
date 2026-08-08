#ifndef RAMFS_H
#define RAMFS_H

#include "stdint.h"

#define RAMFS_FILE      0x01
#define RAMFS_DIRECTORY 0x02

typedef struct ramfs_node {
    char name[128];
    uint32_t flags;          // RAMFS_FILE ou RAMFS_DIRECTORY
    uint32_t length;         // Tamanho do arquivo em bytes
    
    struct ramfs_node *parent;   // Diretório pai
    struct ramfs_node *children; // Primeiro filho (se for diretório)
    struct ramfs_node *next;     // Próximo irmão na mesma pasta
    
    uint8_t *data;            // Ponteiro para o conteúdo (se for arquivo)
    uint32_t capacity;        // Capacidade alocada para os dados
} ramfs_node_t;

// API do RAMFS
ramfs_node_t *ramfs_init_root(void);
ramfs_node_t *ramfs_mkdir(ramfs_node_t *parent_dir, const char *name);
ramfs_node_t *ramfs_create_file(ramfs_node_t *parent_dir, const char *name);
ramfs_node_t *ramfs_find_child(ramfs_node_t *parent_dir, const char *name);

int ramfs_write(ramfs_node_t *file_node, const uint8_t *src, uint32_t size);
uint32_t ramfs_read(ramfs_node_t *file_node, uint8_t *dest, uint32_t offset, uint32_t size);
int ramfs_remove(ramfs_node_t *target_node);
void ramfs_get_path(ramfs_node_t *node, char *buffer, uint32_t max_len);
void ramfs_print_tree(ramfs_node_t *node, int indent);

#endif
