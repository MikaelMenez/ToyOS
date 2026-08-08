/* user/program.c
 * Roda em Ring 3. Todo acesso ao hardware e ao sistema de arquivos
 * acontece atraves de syscalls ("int $0x80"), reutilizando as funcoes
 * que ja existem no kernel -- o programa nao mexe em nenhuma porta I/O.
 */

#include "stdint.h"
#include "syscall.h"

int main(void)
{
    sys_print("[User Mode] Testando syscalls de E/S e do sistema de arquivos...\n");

    /* ---- Sistema de arquivos: cria /apps/log.txt via RAMFS ---- */
    uint32_t root = sys_ramfs_init();
    uint32_t dir = sys_ramfs_mkdir(root, "apps");
    if (dir == 0) {
        sys_print("  [ramfs] falha ao criar 'apps'\n");
    }

    uint32_t file = sys_ramfs_create(dir, "log.txt");
    const char *texto = "escrito pelo processo em Ring 3!\n";
    if (file != 0) {
        sys_ramfs_write(file, texto, 33);
        sys_ramfs_tree(root, 1);
    }

    /* ---- Le o arquivo de volta ---- */
    char buffer[128];
    if (file != 0) {
        uint32_t lido = sys_ramfs_read(file, buffer, 0, 127);
        buffer[lido] = '\0';
        sys_print("  [ramfs] leu de volta: ");
        sys_print(buffer);
    }

    /* ---- Le o 'teste.txt' do TARFS atraves do VFS (generico) ---- */
    uint32_t tar_node = sys_tarfs_find("teste.txt");
    if (tar_node != 0) {
        char buf2[256];
        uint32_t n = sys_vfs_read(tar_node, 0, 200, buf2);
        buf2[n] = '\0';
        sys_print("  [tarfs] teste.txt via vfs_read: ");
        sys_print(buf2);
    }

    /* ---- Agora ecoa o teclado via syscall ---- */
    sys_print("\nDigite algo (sera ecoado): ");
    while (1) {
        char c = sys_read_key();
        if (c) {
            sys_serial_char(c);
        }
    }

    return 0;
}