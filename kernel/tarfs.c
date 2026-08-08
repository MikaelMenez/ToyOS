#include "tarfs.h"
#include "string.h"

#define MAX_FILES 32

// Array para guardar os "nós" dos arquivos encontrados no TAR
static fs_node_t tarfs_nodes[MAX_FILES];
static int tarfs_file_count = 0;

// O TAR salva o tamanho em texto Octal (base 8). Precisamos converter para decimal.
static uint32_t octal2uint(const char *str, int size) {
    uint32_t n = 0;
    while (*str == ' ') str++; // Ignora espaços em branco
    while (size-- > 0 && *str >= '0' && *str <= '7') {
        n = n * 8 + (*str - '0');
        str++;
    }
    return n;
}

// Função de leitura específica para o formato TAR
static uint32_t tarfs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (offset > node->length) return 0;
    if (offset + size > node->length) size = node->length - offset;
    
    // O node->inode guarda o endereço de memória de onde os DADOS do arquivo começam
    uint8_t *file_data = (uint8_t *)node->inode;
    
    // Copia os dados da memória (onde o arquivo está) para o buffer pedido
    memcpy(buffer, file_data + offset, size);
    
    return size;
}

void tarfs_init(uint32_t module_address) {
    tarfs_file_count = 0;
    uint32_t current_addr = module_address;
    
    while (1) {
        tar_header_t *header = (tar_header_t *)current_addr;
        
        // Se o nome for vazio, chegamos ao final do arquivo TAR
        if (header->filename[0] == '\0') {
            break;
        }
        
        uint32_t size = octal2uint(header->size, 11);
        
        // Cadastra o arquivo no VFS local
        if (tarfs_file_count < MAX_FILES && header->typeflag[0] != '5') { // '5' é diretório
            fs_node_t *node = &tarfs_nodes[tarfs_file_count];
            
            // Copia o nome do arquivo
            strncpy(node->name, header->filename, 100);
            node->name[99] = '\0';
            
            node->flags = FS_FILE;
            node->length = size;
            
            // O cabeçalho TAR tem 512 bytes. Os dados começam logo em seguida.
            node->inode = current_addr + 512; 
            node->read = tarfs_read; // Conecta a função de leitura!
            node->write = 0;         // É um file system Read-Only
            
            tarfs_file_count++;
        }
        
        // Pula para o próximo cabeçalho. 
        // Os dados ocupam 'size' bytes, mas o TAR alinha tudo em blocos de 512 bytes.
        uint32_t padding = (512 - (size % 512)) % 512;
        current_addr += 512 + size + padding;
    }
}

fs_node_t* tarfs_find_file(const char *name) {
    for (int i = 0; i < tarfs_file_count; i++) {
        // Usa uma função strcmp (você precisa ter ela implementada na sua lib standard do kernel)
        if (strcmp(tarfs_nodes[i].name, name) == 0) {
            return &tarfs_nodes[i];
        }
    }
    return 0; // Arquivo não encontrado
}
