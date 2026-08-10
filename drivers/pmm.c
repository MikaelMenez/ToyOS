/* ============================================================================
 * pmm.c — Page Frame Allocator (capítulo 10)
 * ----------------------------------------------------------------------------
 * O alocador de memória física. A memória é dividida em "frames" de 4 KB, e a
 * gente controla quais frames estão livres com um bitmap (1 bit por frame).
 *
 *   - bit = 1 → frame ocupado
 *   - bit = 0 → frame livre
 *
 * Na inicialização, literamos tudo como ocupado e depois liberamos (zeramos)
 * somente os frames que o mapa de memória do GRUB diz que estão livres.
 * pmm_alloc_frame() acha o primeiro bit zerado e o marca; pmm_free_frame()
 * devolve o frame de volta.
 * ============================================================================ */

#include "stdint.h"
#include "pmm.h"
#include "serial.h"

#define PMM_MAX_SIZE 0x08000000  // cuidamos de no máximo 128 MB de RAM
#define PAGE_SIZE 4096

static uint32_t memory_limit = 0;   // quantos bytes de memória gerenciamos
static uint32_t *memory_bitmap = 0; // o bitmap em si (1 bit = 1 frame)
static uint32_t bitmap_size = 0;    // tamanho do bitmap em BYTES

/* Marca um frame como ocupado. */
static void bitmap_set(uint32_t bit) {
    memory_bitmap[bit / 32] |= (1 << (bit % 32));
}

/* Marca um frame como livre. */
static void bitmap_unset(uint32_t bit) {
    memory_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

/* Pergunta se um frame está ocupado. */
static uint8_t bitmap_test(uint32_t bit) {
    return (memory_bitmap[bit / 32] & (1 << (bit % 32))) != 0;
}

/* Varre o bitmap procurando o primeiro frame livre (bit zerado). */
static int find_first_free() {
    for (uint32_t i = 0; i < bitmap_size * 8; i++) {
        if (!bitmap_test(i)) {
            return i;
        }
    }
    return -1; // memória cheia!
}

/* Entrada do mapa de memória do GRUB (multiboot). */
typedef struct {
    uint32_t size;
    uint32_t base_addr_low;
    uint32_t base_addr_high;
    uint32_t length_low;
    uint32_t length_high;
    uint32_t type; // 1 = disponível pra uso
} __attribute__((packed)) multiboot_memory_map_t;

/* Monta o bitmap a partir do mapa de memória que o GRUB entregou.
 * `safe_end` é o fim físico do kernel+módulos, que nunca podem ser liberados.
 * O próprio bitmap é criado logo depois do kernel (na memória livre). */
void pmm_init(uint32_t mmap_addr, uint32_t mmap_length, uint32_t mem_lower, uint32_t mem_upper, uint32_t safe_end) {
    (void)mem_lower;
    memory_limit = (mem_upper * 1024) + 0x100000; // RAM instalada (mem_upper vem em KB)
    if (memory_limit > PMM_MAX_SIZE) {
        memory_limit = PMM_MAX_SIZE; // não passamos do nosso teto
    }
    uint32_t total_frames = memory_limit / PAGE_SIZE;

    // CORRIGIDO: tamanho do bitmap em bytes = 1 bit por frame
    bitmap_size = total_frames / 8;

    // O bitmap mora bem no fim do kernel (primeiro lugar que pode ser usado),
    // na visão da memória superior.
    memory_bitmap = (uint32_t *) (safe_end + 0xC0000000);
    // Por via das dúvidas: começamos com TUDO ocupado.
    for (uint32_t i = 0; i < bitmap_size / 4; i++) {
        memory_bitmap[i] = 0xFFFFFFFF;
    }

    // Agora liberamos os frames que o GRUB diz que são utilizáveis.
    multiboot_memory_map_t *mmap = (multiboot_memory_map_t *) (mmap_addr + 0xC0000000);
    uint32_t mmap_end = mmap_addr + mmap_length + 0xC0000000;
    while ((uint32_t) mmap < mmap_end) {
        if (mmap->type == 1 && mmap->base_addr_low >= 0x100000) {
            uint32_t start_frame = mmap->base_addr_low / PAGE_SIZE;
            uint32_t num_frames = mmap->length_low / PAGE_SIZE;
            for (uint32_t i = 0; i < num_frames; i++) {
                uint32_t frame = start_frame + i;
                // DEFENSIVO: nunca escreve fora do bitmap alocado
                if (frame < total_frames) {
                    bitmap_unset(frame);
                }
            }
        }
        // Anda pro próximo registro do mapa (pulando o campo size)
        mmap = (multiboot_memory_map_t *) ((uint32_t) mmap + mmap->size + sizeof(mmap->size));
    }

    // O próprio bitmap não pode ser alocado: marca os frames dele como ocupados.
    uint32_t reserved_end = (uint32_t)memory_bitmap + bitmap_size;
    uint32_t reserved_frames = reserved_end / PAGE_SIZE;
    for (uint32_t i = 0; i <= reserved_frames; i++) {
        bitmap_set(i);
    }
}

/* Aloca um frame: devolve o endereço físico do primeiro frame livre. */
uint32_t pmm_alloc_frame() {
    int frame = find_first_free();
    if (frame == -1) {
        return 0; // sem memória
    }
    bitmap_set(frame);
    return frame * PAGE_SIZE;
}

/* Libera um frame, devolvendo o endereço físico. */
void pmm_free_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    bitmap_unset(frame);
}