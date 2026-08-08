#include "usermode.h"
#include "serial.h"
#include "stdint.h"

static void usermode_log(const char *s)
{
    while (*s) {
        serial_write_byte(*s++);
    }
}

extern uint32_t page_directory[1024];

#define USER_PHYS_BASE           0x00400000
#define KERNEL_TEMP_WINDOW_VIRT   0xC0400000
#define KERNEL_TEMP_WINDOW_PDE    769

static uint32_t user_page_directory[1024] __attribute__((aligned(4096)));

#define PDE_KERNEL_ONLY_4MB   0x83
#define PDE_USER_4MB          0x87

uint32_t usermode_setup(uint32_t module_start_phys, uint32_t module_end_phys)
{
    usermode_log("Cap 11.2: preparando ambiente de modo usuario...\n");

    page_directory[KERNEL_TEMP_WINDOW_PDE] = USER_PHYS_BASE | PDE_KERNEL_ONLY_4MB;
    asm volatile ("invlpg (%0)" :: "r"(KERNEL_TEMP_WINDOW_VIRT) : "memory");

    uint8_t *src = (uint8_t *) (module_start_phys + 0xC0000000);
    uint8_t *dst = (uint8_t *) KERNEL_TEMP_WINDOW_VIRT;
    uint32_t size = module_end_phys - module_start_phys;

    for (uint32_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }

    usermode_log("Cap 11.2: binario do modulo copiado para 0x00400000\n");

    page_directory[KERNEL_TEMP_WINDOW_PDE] = 0;
    asm volatile ("invlpg (%0)" :: "r"(KERNEL_TEMP_WINDOW_VIRT) : "memory");

    for (int i = 0; i < 1024; i++) {
        user_page_directory[i] = 0;
    }

    user_page_directory[0] = USER_PHYS_BASE | PDE_USER_4MB;

    for (int i = 768; i < 1024; i++) {
        user_page_directory[i] = page_directory[i];
    }

    usermode_log("Cap 11.2: page directory do usuario montado com espelho do kernel\n");

    return ((uint32_t) user_page_directory) - 0xC0000000;
}
