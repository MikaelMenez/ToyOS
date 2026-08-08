#include "ramfs.h"

extern void write_serial_str(char *s);

// Pool estático de memória de 16KB para alocação simples no kernel
static uint8_t heap_pool[16384];
static uint32_t heap_offset = 0;

static void *ramfs_alloc(uint32_t size) {
    if (heap_offset + size > sizeof(heap_pool)) {
        return 0;
    }
    void *ptr = &heap_pool[heap_offset];
    heap_offset += (size + 3) & ~3; // Alinhamento de 4 bytes
    return ptr;
}

static ramfs_node_t root_node;

ramfs_node_t *ramfs_init_root(void) {
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

static int ramfs_strcmp(const char *s1, const char *s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        i++;
    }
    return s1[i] - s2[i];
}

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

ramfs_node_t *ramfs_mkdir(ramfs_node_t *parent_dir, const char *name) {
    if (!parent_dir || !(parent_dir->flags & RAMFS_DIRECTORY)) return 0;
    if (ramfs_find_child(parent_dir, name) != 0) return 0;

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

    if (parent_dir->children == 0) {
        parent_dir->children = new_dir;
    } else {
        ramfs_node_t *temp = parent_dir->children;
        while (temp->next != 0) temp = temp->next;
        temp->next = new_dir;
    }

    return new_dir;
}

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

int ramfs_write(ramfs_node_t *file_node, const uint8_t *src, uint32_t size) {
    if (!file_node || !(file_node->flags & RAMFS_FILE)) return -1;

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

int ramfs_remove(ramfs_node_t *target_node) {
    if (!target_node || target_node == &root_node) return -1;
    if (target_node->children != 0) return -1;

    ramfs_node_t *parent = target_node->parent;
    if (!parent) return -1;

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

void ramfs_print_tree(ramfs_node_t *node, int indent) {
    if (!node) return;

    ramfs_node_t *child = node->children;
    while (child != 0) {
        for (int i = 0; i < indent; i++) {
            write_serial_str("  ");
        }

        if (child->flags & RAMFS_DIRECTORY) {
            write_serial_str("├── [DIR] /");
            write_serial_str(child->name);
            write_serial_str("\n");
            ramfs_print_tree(child, indent + 1);
        } else {
            write_serial_str("├── [FILE] ");
            write_serial_str(child->name);
            write_serial_str("\n");
        }

        child = child->next;
    }
}
