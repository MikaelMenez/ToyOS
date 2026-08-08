/* user/shell.c
 * Shell simples em Ring 3. Usa apenas syscalls ("int $0x80"):
 * navegacao (cd, ls, pwd, tree) e operacoes sobre o sistema de
 * arquivos (mkdir, touch, rm, mv, write, read/cat) e busca (grep).
 */

#include "stdint.h"
#include "syscall.h"

#define MAX_LINE  256
#define MAX_ARGV  16
#define PATH_MAX  256

#define F_DIR  0x02   /* RAMFS_DIRECTORY */

static uint32_t g_root = 0;
static uint32_t g_cwd  = 0;

static char g_line[MAX_LINE];

/* ------------------------------------------------------------------ */

static int  s_strlen(const char *s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static int  s_strcmp(const char *a, const char *b)
{
    while (*a && (*a == *b)) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

static uint32_t resolve_path(const char *path);

static void sys_print_uint(uint32_t v)
{
    char buf[12];
    int i = 11;
    buf[i] = '\0';
    if (v == 0) buf[--i] = '0';
    while (v) {
        buf[--i] = '0' + (v % 10);
        v /= 10;
    }
    sys_print(&buf[i]);
}

static void print_prompt(void)
{
    char p[PATH_MAX];
    sys_ramfs_path(g_cwd, p, PATH_MAX);
    sys_print(p);
    sys_print("> ");
}

/* Le uma linha de comando do teclado (eco + backspace via syscalls) */
static int readline(void)
{
    int len = 0;
    while (1) {
        char c = sys_read_key();
        if (!c) continue;

        if (c == '\n') {
            sys_print("\n");
            g_line[len] = '\0';
            return len;
        }
        if (c == '\b') {
            if (len > 0) {
                len--;
                sys_print("\b \b");
            }
            continue;
        }
        if (c == 27) {                 /* ESC: apaga a linha */
            while (len > 0) { len--; sys_print("\b \b"); }
            continue;
        }
        if (len < MAX_LINE - 1) {
            g_line[len++] = c;
            sys_putc(c);
        }
    }
}

static int tokenize(char **argv, int max)
{
    int n = 0;
    char *p = g_line;
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        argv[n++] = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        if (*p) *p++ = '\0';
        if (n >= max) break;
    }
    return n;
}

/* Dir-pai + nome-base de um caminho ("/a/b.txt" -> "/a" , "b.txt") */
static void split_path(const char *path, char *dir_out, char *base_out)
{
    const char *slash = 0;
    for (const char *p = path; *p; p++) {
        if (*p == '/') slash = p;
    }
    int i;
    if (slash) {
        int len = (int)(slash - path);
        for (i = 0; i < len && i < 255; i++) dir_out[i] = path[i];
        dir_out[i] = '\0';
        for (i = 0; slash[1 + i] && i < 255; i++) base_out[i] = slash[1 + i];
        base_out[i] = '\0';
    } else {
        dir_out[0] = '\0';
        for (i = 0; path[i] && i < 255; i++) base_out[i] = path[i];
        base_out[i] = '\0';
    }
}

/* Diretorio-pai de um caminho ("" -> cwd se relativo, root se absoluto) */
static uint32_t parent_of_path(const char *path)
{
    char dir[256], base[64];
    split_path(path, dir, base);
    (void)base;
    if (dir[0] == '\0') {
        return (path[0] == '/') ? g_root : g_cwd;
    }
    return resolve_path(dir);
}

/* Resolve um caminho em um handle (suporta /, .., . e relativo) */
static uint32_t resolve_path(const char *path)
{
    uint32_t node;
    const char *p = path;

    if (!p || p[0] == '\0') return g_cwd;
    if (p[0] == '/') { node = g_root; p++; }
    else              node = g_cwd;

    while (*p) {
        while (*p == '/') p++;
        if (!*p) break;

        char seg[128];
        int i = 0;
        while (*p && *p != '/' && i < 127) seg[i++] = *p++;
        seg[i] = '\0';

        if (seg[0] == '.' && seg[1] == '\0') continue;

        if (seg[0] == '.' && seg[1] == '.' && seg[2] == '\0') {
            if (node != g_root && (sys_ramfs_mode(node) & F_DIR)) {
                uint32_t par = sys_ramfs_parent(node);
                if (par) node = par;
            }
            continue;
        }

        node = sys_ramfs_find(node, seg);
        if (!node) return 0;
    }
    return node;
}

/* ---------------------------------------------------------------- */

static void cmd_ls(int argc, char **argv)
{
    uint32_t d = (argc > 1) ? resolve_path(argv[1]) : g_cwd;
    if (!d) { sys_print("ls: caminho invalido\n"); return; }

    uint32_t n = sys_ramfs_children(d);
    while (n) {
        char name[128];
        sys_ramfs_name(n, name, 128);
        if (sys_ramfs_mode(n) & F_DIR) sys_print("[d] ");
        else                            sys_print("[f] ");
        sys_print(name);
        sys_print("\n");
        n = sys_ramfs_sibling(n);
    }
}

static void cmd_tree(void)
{
    sys_ramfs_tree(g_root, 1);
}

static void cmd_cd(int argc, char **argv)
{
    if (argc < 2) { g_cwd = g_root; return; }
    uint32_t n = resolve_path(argv[1]);
    if (!n) { sys_print("cd: caminho invalido\n"); return; }
    if (!(sys_ramfs_mode(n) & F_DIR)) { sys_print("cd: nao e diretorio\n"); return; }
    g_cwd = n;
}

static void cmd_pwd(void)
{
    char p[PATH_MAX];
    sys_ramfs_path(g_cwd, p, PATH_MAX);
    sys_print(p);
    sys_print("\n");
}

static void cmd_mkdir(int argc, char **argv)
{
    if (argc < 2) { sys_print("use: mkdir <caminho>\n"); return; }
    char dir[256], base[64];
    split_path(argv[1], dir, base);
    uint32_t parent = parent_of_path(argv[1]);
    if (!parent) { sys_print("mkdir: diretorio-pai invalido\n"); return; }
    if (sys_ramfs_mkdir(parent, base)) sys_print("mkdir: feito\n");
    else                               sys_print("mkdir: erro / ja existe\n");
}

static void cmd_touch(int argc, char **argv)
{
    if (argc < 2) { sys_print("use: touch <caminho>\n"); return; }
    char dir[256], base[64];
    split_path(argv[1], dir, base);
    uint32_t parent = parent_of_path(argv[1]);
    if (!parent) { sys_print("touch: diretorio-pai invalido\n"); return; }
    if (sys_ramfs_create(parent, base)) sys_print("touch: criado\n");
    else                                sys_print("touch: erro / ja existe\n");
}

static void cmd_rm(int argc, char **argv)
{
    if (argc < 2) { sys_print("use: rm <caminho>\n"); return; }
    uint32_t n = resolve_path(argv[1]);
    if (!n) { sys_print("rm: nao encontrado\n"); return; }
    if (sys_ramfs_remove(n) == 0) sys_print("rm: removido\n");
    else sys_print("rm: erro (diretorio nao-vazio ou invalido)\n");
}

static void cmd_mv(int argc, char **argv)
{
    if (argc < 3) { sys_print("use: mv <origem> <destino>\n"); return; }
    uint32_t src = resolve_path(argv[1]);
    if (!src) { sys_print("mv: origem nao encontrada\n"); return; }

    uint32_t dst = resolve_path(argv[2]);
    if (dst) {
        if (!(sys_ramfs_mode(dst) & F_DIR)) {
            sys_print("mv: destino ja existe e nao e diretorio\n");
            return;
        }
        if (sys_ramfs_move(src, dst, 0) == 0) sys_print("mv: feito\n");
        else                                  sys_print("mv: erro\n");
        return;
    }

    /* destino nao existe: é um novo nome (possivelmente em outra pasta) */
    uint32_t parent = parent_of_path(argv[2]);
    char dir[256], base[64];
    split_path(argv[2], dir, base);
    if (!parent) { sys_print("mv: diretorio de destino invalido\n"); return; }
    if (sys_ramfs_move(src, parent, base) == 0) sys_print("mv: feito\n");
    else                                        sys_print("mv: erro\n");
}

static void cmd_write(int argc, char **argv)
{
    if (argc < 3) { sys_print("use: write <caminho> <texto...>\n"); return; }
    uint32_t n = resolve_path(argv[1]);
    if (!n) { sys_print("write: arquivo nao encontrado\n"); return; }

    char content[512];
    int o = 0;
    for (int i = 2; i < argc; i++) {
        if (i > 2 && o < 511) content[o++] = ' ';
        for (int j = 0; argv[i][j] && o < 511; j++) content[o++] = argv[i][j];
    }

    uint32_t w = sys_ramfs_write(n, content, (uint32_t)o);
    sys_print("write: gravado ");
    sys_print_uint(w);
    sys_print(" bytes\n");
}

static void cmd_read(int argc, char **argv)
{
    if (argc < 2) { sys_print("use: read <caminho>\n"); return; }
    uint32_t n = resolve_path(argv[1]);
    if (!n) { sys_print("read: nao encontrado\n"); return; }

    char buf[512];
    uint32_t lido = sys_ramfs_read(n, buf, 0, 511);
    buf[lido] = '\0';
    sys_print(buf);
    if (lido > 0 && buf[lido - 1] != '\n') sys_print("\n");
    if (lido == 0) sys_print("(arquivo vazio)\n");
}

static void cmd_grep(int argc, char **argv)
{
    if (argc < 3) { sys_print("use: grep <padrao> <caminho>\n"); return; }
    uint32_t n = resolve_path(argv[2]);
    if (!n) { sys_print("grep: nao encontrado\n"); return; }

    char buf[1024];
    uint32_t total = sys_ramfs_read(n, buf, 0, 1023);
    buf[total] = '\0';

    const char *needle = argv[1];
    char *line = buf;
    while (line && *line) {
        char *nl = line;
        while (*nl && *nl != '\n') nl++;
        if (*nl == '\n') { *nl = '\0'; nl++; }
        else nl = 0;

        int j = 0;
        while (needle[j] && line[j] && needle[j] == line[j]) j++;
        if (needle[j] == '\0') {
            sys_print(line);
            sys_print("\n");
        }
        line = nl;
    }
}

static void cmd_help(void)
{
    sys_print("Comandos:\n");
    sys_print("  pwd               diretorio atual\n");
    sys_print("  ls [caminho]      lista arquivos/pastas\n");
    sys_print("  cd <caminho>      navega (/, .., relativo)\n");
    sys_print("  tree              mostra a arvore completa\n");
    sys_print("  mkdir <caminho>   cria pasta\n");
    sys_print("  touch <caminho>   cria arquivo vazio\n");
    sys_print("  rm <caminho>      remove arquivo/pasta vazia\n");
    sys_print("  mv <orig> <dest>  move ou renomeia\n");
    sys_print("  write <caminho> <texto...>\n");
    sys_print("  read <caminho>    mostra conteudo (alias: cat)\n");
    sys_print("  grep <padra> <caminho>\n");
    sys_print("  help\n");
}

static void handle(int argc, char **argv)
{
    const char *c = argv[0];
    if      (s_strcmp(c, "help")  == 0) cmd_help();
    else if (s_strcmp(c, "ls")    == 0) cmd_ls(argc, argv);
    else if (s_strcmp(c, "tree")  == 0) cmd_tree();
    else if (s_strcmp(c, "cd")    == 0) cmd_cd(argc, argv);
    else if (s_strcmp(c, "pwd")   == 0) cmd_pwd();
    else if (s_strcmp(c, "mkdir") == 0) cmd_mkdir(argc, argv);
    else if (s_strcmp(c, "touch") == 0) cmd_touch(argc, argv);
    else if (s_strcmp(c, "rm")    == 0) cmd_rm(argc, argv);
    else if (s_strcmp(c, "mv")    == 0) cmd_mv(argc, argv);
    else if (s_strcmp(c, "write") == 0) cmd_write(argc, argv);
    else if (s_strcmp(c, "read")  == 0 || s_strcmp(c, "cat") == 0) cmd_read(argc, argv);
    else if (s_strcmp(c, "grep")  == 0) cmd_grep(argc, argv);
    else {
        sys_print("desconhecido: ");
        sys_print((char *)c);
        sys_print("  (digite help)\n");
    }
}

int main(void)
{
    g_root = sys_ramfs_init();
    g_cwd  = g_root;

    sys_print("ToyOS Shell - Ring 3\n");

    while (1) {
        print_prompt();
        if (!readline()) continue;

        char *argv[MAX_ARGV];
        int argc = tokenize(argv, MAX_ARGV);
        if (argc == 0) continue;

        handle(argc, argv);
    }

    return 0;
}