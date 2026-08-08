#ifndef TARFS_H
#define TARFS_H

#include "vfs.h"

/* Estrutura do cabeçalho de 512 bytes padrão do formato TAR (USTAR) */
typedef struct {
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];     // Tamanho do arquivo em ASCII Octal
    char mtime[12];
    char chksum[8];
    char typeflag[1];
} __attribute__((packed)) tar_header_t;

/* Inicializa o sistema de arquivos a partir do endereço de memória onde o GRUB carregou o módulo */
void tarfs_init(uint32_t module_address);

/* Procura um arquivo pelo nome no TARFS */
fs_node_t* tarfs_find_file(const char *name);

#endif
