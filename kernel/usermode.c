#include "usermode.h"
#include "serial.h"

/* write_serial_str e' privada do kmain.c (nao existe no serial.h),
 * entao criamos aqui a mesma logica, usando serial_write_byte que
 * essa sim e' publica no serial.h. */
static void usermode_log(const char *s)
{
    while (*s) {
        serial_write_byte(*s++);
    }
}

/* Page directory do KERNEL, definido em loader.s (agora exportado com
 * "global page_directory"). Precisamos dele só para abrir uma janela
 * temporária de escrita na região física nova, já que o kernel hoje só
 * enxerga fisicamente os primeiros 4 MB (0x000000 - 0x3FFFFF). */
extern uint32_t page_directory[1024];

/* Região física reservada para o processo de usuário: o bloco de 4 MB
 * logo depois do bloco onde o kernel vive (0x000000 - 0x3FFFFF).
 * Precisa ser múltiplo de 4 MB, porque é o base address de uma page
 * de 4 MB (PSE). */
#define USER_PHYS_BASE   0x00400000

/* Janela temporária que o KERNEL usa para escrever nessa região física
 * nova. Fica em 0xC0400000 (logo depois da faixa 0xC0000000 que o
 * kernel já usa para os primeiros 4 MB). Só o kernel usa isso (U/S=0). */
#define KERNEL_TEMP_WINDOW_VIRT   0xC0400000
#define KERNEL_TEMP_WINDOW_PDE    769   /* 0xC0400000 >> 22 = 769 */

/* Page directory do processo de usuário. Precisa ficar alinhado em
 * 4096 bytes (requisito do hardware para o page directory). */
static uint32_t user_page_directory[1024] __attribute__((aligned(4096)));

/* Flags usadas nas entradas do page directory (todas páginas de 4 MB):
 * bit 0 = Present, bit 1 = Read/Write, bit 2 = User/Supervisor,
 * bit 7 = Page Size (1 = 4 MB) */
#define PDE_KERNEL_ONLY_4MB   0x83   /* P=1, RW=1, US=0, PS=1 */
#define PDE_USER_4MB          0x87   /* P=1, RW=1, US=1, PS=1 */

uint32_t usermode_setup(uint32_t module_start_phys, uint32_t module_end_phys)
{
    usermode_log("Cap 11.2: preparando ambiente de modo usuario...\n");

    /* 1. Abre uma janela temporaria no page directory do KERNEL para
     *    conseguir ESCREVER na regiao fisica nova (0x00400000+), que
     *    ainda nao esta mapeada em lugar nenhum. */
    page_directory[KERNEL_TEMP_WINDOW_PDE] = USER_PHYS_BASE | PDE_KERNEL_ONLY_4MB;
    asm volatile ("invlpg (%0)" :: "r"(KERNEL_TEMP_WINDOW_VIRT) : "memory");

    /* 2. Copia o binario do modulo (codigo+dados) para o inicio da
     *    regiao fisica nova, atraves dessa janela temporaria. */
    uint8_t *src = (uint8_t *) (module_start_phys + 0xC0000000);
    uint8_t *dst = (uint8_t *) KERNEL_TEMP_WINDOW_VIRT;
    uint32_t size = module_end_phys - module_start_phys;

    for (uint32_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }

    usermode_log("Cap 11.2: binario do modulo copiado para 0x00400000\n");

    /* 3. Fecha a janela temporaria - nao precisamos mais dela. */
    page_directory[KERNEL_TEMP_WINDOW_PDE] = 0;
    asm volatile ("invlpg (%0)" :: "r"(KERNEL_TEMP_WINDOW_VIRT) : "memory");

    /* 4. Monta o page directory do PROCESSO DE USUARIO:
     *    - entrada 0: mapeia o virtual 0x00000000-0x003FFFFF para a
     *      regiao fisica nova, com U/S=1 (acessivel em PL3). O codigo
     *      do programa fica no INICIO dessa faixa (virtual 0x0), e a
     *      pilha pode usar o TOPO da mesma faixa (perto de 0x003FFFFF).
     *    - entrada 768: igual a do kernel, para o kernel continuar
     *      acessivel (com U/S=0, entao o processo de usuario nao
     *      consegue ler/escrever essa faixa mesmo ela estando no
     *      diretorio). */
    for (int i = 0; i < 1024; i++) {
        user_page_directory[i] = 0;
    }
    user_page_directory[0]   = USER_PHYS_BASE | PDE_USER_4MB;
    user_page_directory[768] = 0x00000000 | PDE_KERNEL_ONLY_4MB;

    usermode_log("Cap 11.2: page directory do usuario montado\n");

    /* Retorna o endereco FISICO do page directory (cr3 precisa do
     * endereco fisico, nao virtual). */
    return ((uint32_t) user_page_directory) - 0xC0000000;
}