#include "ramfs.h"
#include "console.h"

/* ============================================================================
 * ALOCADOR DE MEMÓRIA INTERNO DO RAMFS
 * Pool estático de 16KB para alocação rápida de nós (diretórios/arquivos) 
 * e buffers de dados sem depender de um kmalloc complexo.
 * ============================================================================ */
static uint8_t heap_pool[16384];
static uint32_t heap_offset = 0;

/**
 * @brief Aloca um bloco de memória contíguo dentro do pool do RAMFS.
 * @param size Tamanho em bytes a ser alocado.
 * @return Ponteiro para a memória alocada ou 0 (NULL) se estourar o pool.
 */
static void *ramfs_alloc(uint32_t size) {
    if (heap_offset + size > sizeof(heap_pool)) {
        return 0; // Out of memory
    }
    void *ptr = &heap_pool[heap_offset];
    heap_offset += (size + 3) & ~3; // Alinhamento de 4 bytes (32-bit friendly)
    return ptr;
}

static ramfs_node_t root_node;

/* ============================================================================
 * INICIALIZAÇÃO E UTILITÁRIOS
 * ============================================================================ */

/**
 * @brief Inicializa o nó raiz ('/') do sistema de arquivos RAMFS.
 * @note Deve ser chamada uma única vez na inicialização do Kernel.
 * @return Ponteiro para o nó raiz global (root_node).
 */
ramfs_node_t *ramfs_init_root(void) {
    /* Se a raiz já foi inicializada, não destrói a árvore existente */
    if (root_node.name[0] == '/') {
        return &root_node;
    }
    for (int i = 0; i < 128; i++) {
        root_node.name[i] = 0;
    }
    root_node.name[0] = '/';
    root_node.name[1] = '\0';
    root_node.flags = RAMFS_DIRECTORY;
    root_node.length = 0;
    root_node.parent = 0;
    root_node.children = 0;
    root_node.next = 0;
    root_node.data = 0;
    root_node.capacity = 0;
    return &root_node;
}

/**
 * @brief Comparação simples de strings (equivalente ao strcmp da libc).
 */
static int ramfs_strcmp(const char *s1, const char *s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        i++;
    }
    return s1[i] - s2[i];
}

/* ============================================================================
 * BUSCA E NAVEGAÇÃO
 * ============================================================================ */

/**
 * @brief Procura por um filho direto dentro de um diretório pai pelo nome.
 * @param parent_dir Nó do diretório onde a busca será feita.
 * @param name Nome exato da pasta ou arquivo procurado.
 * @return Ponteiro para o nó encontrado ou 0 (NULL) se não existir.
 * 
 * @note UTILIDADE NO SHELL: Use para navegar pasta por pasta (ex: buscar 'docs' dentro do '/').
 */
ramfs_node_t *ramfs_find_child(ramfs_node_t *parent_dir, const char *name) {
    if (!parent_dir || !(parent_dir->flags & RAMFS_DIRECTORY)) return 0;

    ramfs_node_t *curr = parent_dir->children;
    while (curr != 0) {
        if (ramfs_strcmp(curr->name, name) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return 0;
}

/* ============================================================================
 * CRIAÇÃO DE ESTRUTURAS (DIRETÓRIOS E ARQUIVOS)
 * ============================================================================ */

/**
 * @brief Cria um novo subdiretório dentro de um diretório pai.
 * @param parent_dir Diretório onde a nova pasta será inserida.
 * @param name Nome do novo diretório.
 * @return Ponteiro para o novo nó criado ou 0 (NULL) se falhar/já existir.
 * 
 * @note UTILIDADE NO SHELL: Chamada base para implementar o comando 'mkdir <nome>'.
 */
ramfs_node_t *ramfs_mkdir(ramfs_node_t *parent_dir, const char *name) {
    if (!parent_dir || !(parent_dir->flags & RAMFS_DIRECTORY)) return 0;
    if (ramfs_find_child(parent_dir, name) != 0) return 0; // Já existe com esse nome

    ramfs_node_t *new_dir = (ramfs_node_t *) ramfs_alloc(sizeof(ramfs_node_t));
    if (!new_dir) return 0;

    int i = 0;
    while (name[i] != '\0' && i < 127) {
        new_dir->name[i] = name[i];
        i++;
    }
    new_dir->name[i] = '\0';

    new_dir->flags = RAMFS_DIRECTORY;
    new_dir->length = 0;
    new_dir->parent = parent_dir;
    new_dir->children = 0;
    new_dir->next = 0;
    new_dir->data = 0;
    new_dir->capacity = 0;

    // Insere no final da lista encadeada de filhos do pai
    if (parent_dir->children == 0) {
        parent_dir->children = new_dir;
    } else {
        ramfs_node_t *temp = parent_dir->children;
        while (temp->next != 0) temp = temp->next;
        temp->next = new_dir;
    }

    return new_dir;
}

/**
 * @brief Cria um novo arquivo vazio dentro de um diretório pai.
 * @param parent_dir Diretório onde o arquivo será criado.
 * @param name Nome do novo arquivo.
 * @return Ponteiro para o novo nó do arquivo ou 0 (NULL) se falhar/já existir.
 * 
 * @note UTILIDADE NO SHELL: Chamada base para o comando 'touch <nome>'.
 */
ramfs_node_t *ramfs_create_file(ramfs_node_t *parent_dir, const char *name) {
    if (!parent_dir || !(parent_dir->flags & RAMFS_DIRECTORY)) return 0;
    if (ramfs_find_child(parent_dir, name) != 0) return 0;

    ramfs_node_t *new_file = (ramfs_node_t *) ramfs_alloc(sizeof(ramfs_node_t));
    if (!new_file) return 0;

    int i = 0;
    while (name[i] != '\0' && i < 127) {
        new_file->name[i] = name[i];
        i++;
    }
    new_file->name[i] = '\0';

    new_file->flags = RAMFS_FILE;
    new_file->length = 0;
    new_file->parent = parent_dir;
    new_file->children = 0;
    new_file->next = 0;
    new_file->data = 0;
    new_file->capacity = 0;

    if (parent_dir->children == 0) {
        parent_dir->children = new_file;
    } else {
        ramfs_node_t *temp = parent_dir->children;
        while (temp->next != 0) temp = temp->next;
        temp->next = new_file;
    }

    return new_file;
}

/* ============================================================================
 * LEITURA E ESCRITA EM ARQUIVOS
 * ============================================================================ */

/**
 * @brief Escreve dados em um arquivo existente.
 * @param file_node Ponteiro para o nó do arquivo (RAMFS_FILE).
 * @param src Ponteiro para o buffer com os dados a serem gravados.
 * @param size Quantidade de bytes a gravar.
 * @return Número de bytes gravados com sucesso ou -1 em caso de erro.
 * 
 * @note UTILIDADE NO SHELL: Usado para editores de texto ou redirecionadores (ex: echo "texto" > arquivo).
 */
int ramfs_write(ramfs_node_t *file_node, const uint8_t *src, uint32_t size) {
    if (!file_node || !(file_node->flags & RAMFS_FILE)) return -1;

    // Redimensiona/aloca buffer do arquivo se a capacidade atual for menor
    if (file_node->capacity < size) {
        file_node->data = (uint8_t *) ramfs_alloc(size);
        if (!file_node->data) return -1;
        file_node->capacity = size;
    }

    for (uint32_t i = 0; i < size; i++) {
        file_node->data[i] = src[i];
    }
    file_node->length = size;
    return (int)size;
}

/**
 * @brief Lê o conteúdo de um arquivo a partir de um offset.
 * @param file_node Ponteiro para o nó do arquivo (RAMFS_FILE).
 * @param dest Buffer de destino onde os bytes lidos serão armazenados.
 * @param offset Posição inicial de leitura dentro do arquivo (em bytes).
 * @param size Quantidade máxima de bytes a ler.
 * @return Quantidade real de bytes lidos e copiados para 'dest'.
 * 
 * @note UTILIDADE NO SHELL: Chamada base para o comando 'cat <arquivo>'.
 */
uint32_t ramfs_read(ramfs_node_t *file_node, uint8_t *dest, uint32_t offset, uint32_t size) {
    if (!file_node || !(file_node->flags & RAMFS_FILE) || !file_node->data) return 0;
    if (offset >= file_node->length) return 0;

    uint32_t bytes_to_read = size;
    if (offset + bytes_to_read > file_node->length) {
        bytes_to_read = file_node->length - offset;
    }

    for (uint32_t i = 0; i < bytes_to_read; i++) {
        dest[i] = file_node->data[offset + i];
    }
    return bytes_to_read;
}

/* ============================================================================
 * REMOÇÃO E CAMINHO ABSOLUTO
 * ============================================================================ */

/**
 * @brief Remove um nó (arquivo ou diretório vazio) da árvore.
 * @param target_node Nó a ser removido.
 * @return 0 em caso de sucesso, -1 se falhar ou se for um diretório não-vazio.
 * 
 * @note UTILIDADE NO SHELL: Chamada base para o comando 'rm <arquivo>' ou 'rmdir <pasta>'.
 */
int ramfs_remove(ramfs_node_t *target_node) {
    if (!target_node || target_node == &root_node) return -1;
    if (target_node->children != 0) return -1; // Não deleta pastas com conteúdo

    ramfs_node_t *parent = target_node->parent;
    if (!parent) return -1;

    // Desconecta o nó da lista encadeada do pai
    if (parent->children == target_node) {
        parent->children = target_node->next;
    } else {
        ramfs_node_t *curr = parent->children;
        while (curr->next != 0 && curr->next != target_node) {
            curr = curr->next;
        }
        if (curr->next == target_node) {
            curr->next = target_node->next;
        }
    }
    return 0;
}

/**
 * @brief Move um nó para outro diretório e/ou renomeia ele (comando 'mv').
 * Reaproveita o nó já alocado, apenas relocando-o na árvore.
 * @return 0 em caso de sucesso, -1 em caso de erro.
 */
int ramfs_move(ramfs_node_t *target_node, ramfs_node_t *new_parent, const char *new_name) {
    if (!target_node || target_node == &root_node) return -1;
    if (!new_parent || !(new_parent->flags & RAMFS_DIRECTORY)) return -1;

    const char *name = (new_name && new_name[0] != '\0') ? new_name : target_node->name;

    // Confere se já existe um filho com o (futuro) nome no destino
    ramfs_node_t *existing = ramfs_find_child(new_parent, name);
    if (existing != 0 && existing != target_node) return -1;

    // Desconecta da lista do pai atual
    ramfs_node_t *old_parent = target_node->parent;
    if (old_parent != 0) {
        if (old_parent->children == target_node) {
            old_parent->children = target_node->next;
        } else {
            ramfs_node_t *curr = old_parent->children;
            while (curr != 0 && curr->next != target_node) {
                curr = curr->next;
            }
            if (curr != 0 && curr->next == target_node) {
                curr->next = target_node->next;
            }
        }
    }

    target_node->next = 0;
    target_node->parent = new_parent;

    // Aplica o nome novo, se houver
    if (name != target_node->name) {
        int i = 0;
        while (name[i] != '\0' && i < 127) {
            target_node->name[i] = name[i];
            i++;
        }
        target_node->name[i] = '\0';
    }

    // Insere no final da lista de filhos do destino
    if (new_parent->children == 0) {
        new_parent->children = target_node;
    } else {
        ramfs_node_t *curr = new_parent->children;
        while (curr->next != 0) curr = curr->next;
        curr->next = target_node;
    }

    return 0;
}

uint32_t ramfs_node_name(ramfs_node_t *node, char *buf, uint32_t max_len) {
    if (!node || !buf || max_len == 0) return 0;
    uint32_t i = 0;
    while (node->name[i] != '\0' && i < max_len - 1) {
        buf[i] = node->name[i];
        i++;
    }
    buf[i] = '\0';
    return i;
}

ramfs_node_t *ramfs_first_child(ramfs_node_t *dir) {
    if (!dir || !(dir->flags & RAMFS_DIRECTORY)) return 0;
    return dir->children;
}

ramfs_node_t *ramfs_next_sibling(ramfs_node_t *node) {
    if (!node) return 0;
    return node->next;
}

/**
 * @brief Reconstrói o caminho absoluto em string de um nó (subindo até o '/').
 * @param node Nó de origem.
 * @param buffer Buffer de texto onde o caminho completo será gravado (ex: "/docs/nota.txt").
 * @param max_len Tamanho máximo suportado pelo buffer.
 * 
 * @note UTILIDADE NO SHELL: Chamada base para o comando 'pwd' ou exibir o prompt de comando.
 */
void ramfs_get_path(ramfs_node_t *node, char *buffer, uint32_t max_len) {
    if (!node || max_len == 0) return;

    if (node == &root_node) {
        if (max_len > 1) {
            buffer[0] = '/';
            buffer[1] = '\0';
        } else {
            buffer[0] = '\0';
        }
        return;
    }

    char path_stack[32][128];
    int depth = 0;
    ramfs_node_t *curr = node;

    // Empilha os nomes dos diretórios subindo pela árvore via ->parent
    while (curr != &root_node && curr != 0 && depth < 32) {
        int i = 0;
        while (curr->name[i] != '\0' && i < 127) {
            path_stack[depth][i] = curr->name[i];
            i++;
        }
        path_stack[depth][i] = '\0';
        depth++;
        curr = curr->parent;
    }

    // Desempilha os nomes montando a string final no buffer
    buffer[0] = '\0';
    uint32_t buf_idx = 0;

    for (int i = depth - 1; i >= 0; i--) {
        if (buf_idx < max_len - 1) {
            buffer[buf_idx++] = '/';
        }
        int j = 0;
        while (path_stack[i][j] != '\0' && buf_idx < max_len - 1) {
            buffer[buf_idx++] = path_stack[i][j];
            j++;
        }
    }
    buffer[buf_idx] = '\0';
}

/* ============================================================================
 * EXIBIÇÃO DA ÁRVORE HIERÁRQUICA
 * ============================================================================ */

/**
 * @brief Imprime recursivamente a estrutura visual da árvore de diretórios via Porta Serial COM1.
 * @param node Nó inicial a ser impresso (geralmente a raiz '/').
 * @param indent Nível de indentação atual (usado internamente na recursão).
 * 
 * @note UTILIDADE NO SHELL: Pode ser usado diretamente no comando 'ls' ou 'tree'.
 */
void ramfs_print_tree(ramfs_node_t *node, int indent) {
    if (!node) return;

    ramfs_node_t *child = node->children;
    while (child != 0) {
        // Imprime os espaços de indentação
        for (int i = 0; i < indent; i++) {
            write_serial_str("  ");
        }

        // Formata visualmente diretórios vs arquivos
        if (child->flags & RAMFS_DIRECTORY) {
            write_serial_str("├── [DIR] /");
            write_serial_str(child->name);
            write_serial_str("\n");
            ramfs_print_tree(child, indent + 1); // Chamada recursiva para subpastas
        } else {
            write_serial_str("├── [FILE] ");
            write_serial_str(child->name);
            write_serial_str("\n");
        }

        child = child->next;
    }
}
