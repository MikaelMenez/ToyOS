/* ============================================================================
 * usermode.c — Preparando o terreno pro modo usuário (Ring 3)
 * ----------------------------------------------------------------------------
 * Esse é o capítulo 11 do livro. A ideia é: o kernel carrega um programa
 * "de fora" (o módulo `program` que o GRUB entregou), copia ele pra uma área
 * de memória separada e monta um page directory próprio pro usuário, onde:
 *
 *   - a entrada 0 mapeia o programa em 0x00400000 com permissão de usuário;
 *   - as entradas 768+ (metade superior) são um "espelho" do kernel, assim o
 *     usuário não consegue ver o kernel, mas ele continua acessível internamente.
 *
 * No fim, devolvemos o endereço FÍSICO desse page directory, que o
 * enter_usermode.s coloca no CR3 antes do pulo final.
 * ============================================================================ */

#include "usermode.h"
#include "serial.h"
#include "stdint.h"

/* Escreve uma mensagem no log serial. */
static void usermode_log(const char *s)
{
    while (*s) {
        serial_write_byte(*s++);
    }
}

/* A tabela de páginas do KERNEL, criada no loader.s. */
extern uint32_t page_directory[1024];

/* Endereço físico onde o programa de usuário vai morar (4 MB).
 * Escolhido longe do kernel pra não se misturar com ele. */
#define USER_PHYS_BASE           0x00400000
/* "Janelinha" virtual que o kernel usa temporariamente pra enxergar o módulo
 * (ele ainda está na memória do GRUB, não na área do usuário). */
#define KERNEL_TEMP_WINDOW_VIRT   0xC0400000
#define KERNEL_TEMP_WINDOW_PDE    769 /* índice da PDE da janelinha (0xC0400000 / 4 MB) */

/* O page directory do usuário, alinhado a 4 KB pra ser válido no CR3. */
static uint32_t user_page_directory[1024] __attribute__((aligned(4096)));

/* Flags das entradas do page directory (bit 0 = presente, bit 1 = escrita,
 * bit 2 = userId (U/S), bit 7 = página de 4 MB). */
#define PDE_KERNEL_ONLY_4MB   0x83  /* 4MB supervisor (só kernel) */
#define PDE_USER_4MB          0x87  /* 4MB com acesso de usuário   */

/* Copia o binário do módulo pra 0x00400000 e monta o page directory acima. */
uint32_t usermode_setup(uint32_t module_start_phys, uint32_t module_end_phys)
{
    usermode_log("Cap 11.2: preparando ambiente de modo usuario...\n");

    /* 1) Cria uma "janela" temporária pra enxergar o módulo via página grande.
     *    O módulo ainda está na região que o GRUB carregou (endereço físico),
     *    e nós só enxergamos memória pelos endereços virtuais mapeados. */
    page_directory[KERNEL_TEMP_WINDOW_PDE] = USER_PHYS_BASE | PDE_KERNEL_ONLY_4MB;
    asm volatile ("invlpg (%0)" :: "r"(KERNEL_TEMP_WINDOW_VIRT) : "memory");

    /* 2) Copia o binário byte a byte pra área dedicada do usuário. */
    uint8_t *src = (uint8_t *) (module_start_phys + 0xC0000000);
    uint8_t *dst = (uint8_t *) KERNEL_TEMP_WINDOW_VIRT;
    uint32_t size = module_end_phys - module_start_phys;

    for (uint32_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }

    usermode_log("Cap 11.2: binario do modulo copiado para 0x00400000\n");

    /* 3) Fecha a janela — não precisamos mais dela. */
    page_directory[KERNEL_TEMP_WINDOW_PDE] = 0;
    asm volatile ("invlpg (%0)" :: "r"(KERNEL_TEMP_WINDOW_VIRT) : "memory");

    /* 4) Monta o page directory do usuário do zero. */
    for (int i = 0; i < 1024; i++) {
        user_page_directory[i] = 0;
    }

    /* Entrada 0: página única de 4 MB ligando a região virtual
     * 0x00000000–0x003FFFFF à física 0x00400000–0x007FFFFF, com acesso
     * de usuário. É ali que o programa vai rodar (o binário é linkado no
     * endereço 0, então o primeiro byte de código bate com o EIP 0). */
    user_page_directory[0] = USER_PHYS_BASE | PDE_USER_4MB;

    /* Entradas 768+: espelho do kernel (mesmas entradas do page directory
     * original), mas SEM o bit de usuário — Ring 3 não tem como mexer ali. */
    for (int i = 768; i < 1024; i++) {
        user_page_directory[i] = page_directory[i];
    }

    usermode_log("Cap 11.2: page directory do usuario montado com espelho do kernel\n");

    /* O CR3 precisa do endereço FÍSICO; como este array vive na metade
     * superior, descontamos o deslocamento de 0xC0000000. */
    return ((uint32_t) user_page_directory) - 0xC0000000;
}