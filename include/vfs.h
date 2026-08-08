#ifndef VFS_H
#define VFS_H

#include "stdint.h"
#include "stddef.h"

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02

struct fs_node;

/* Tipos para os ponteiros de função das operações de arquivo */
typedef uint32_t (*read_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);

/* Estrutura de um "nó" do sistema de arquivos (representa um arquivo ou diretório) */
typedef struct fs_node {
    char name[128];     // Nome do arquivo
    uint32_t flags;     // Arquivo (0x01) ou Diretório (0x02)
    uint32_t length;    // Tamanho do arquivo
    uint32_t inode;     // ID interno (útil para mapear o arquivo na memória)
    
    read_type_t read;   // Ponteiro para a função de leitura
    write_type_t write; // Ponteiro para a função de escrita
} fs_node_t;

extern fs_node_t *fs_root; // Raiz do sistema de arquivos VFS

/* Funções padrão do VFS */
uint32_t vfs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t vfs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);

#endif
