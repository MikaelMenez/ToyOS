#include "stdint.h"
#include "pmm.h"
#include "serial.h"

#define PMM_MAX_SIZE 0x08000000
#define PAGE_SIZE 4096

static uint32_t memory_limit = 0;
static uint32_t *memory_bitmap = 0;
static uint32_t bitmap_size = 0;

static void bitmap_set(uint32_t bit) {
    memory_bitmap[bit / 32] |= (1 << (bit % 32));
}

static void bitmap_unset(uint32_t bit) {
    memory_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static uint8_t bitmap_test(uint32_t bit) {
    return (memory_bitmap[bit / 32] & (1 << (bit % 32))) != 0;
}

static int find_first_free() {
    for (uint32_t i = 0; i < bitmap_size * 8; i++) {
        if (!bitmap_test(i)) {
            return i;
        }
    }
    return -1;
}

typedef struct {
    uint32_t size;
    uint32_t base_addr_low;
    uint32_t base_addr_high;
    uint32_t length_low;
    uint32_t length_high;
    uint32_t type;
} __attribute__((packed)) multiboot_memory_map_t;

void pmm_init(uint32_t mmap_addr, uint32_t mmap_length, uint32_t mem_lower, uint32_t mem_upper, uint32_t safe_end) {
    (void)mem_lower;
    memory_limit = (mem_upper * 1024) + 0x100000;
    if (memory_limit > PMM_MAX_SIZE) {
        memory_limit = PMM_MAX_SIZE;
    }
    uint32_t total_frames = memory_limit / PAGE_SIZE;

    // CORRIGIDO: tamanho do bitmap em bytes = 1 bit por frame
    bitmap_size = total_frames / 8;

    memory_bitmap = (uint32_t *) (safe_end + 0xC0000000);
    for (uint32_t i = 0; i < bitmap_size / 4; i++) {
        memory_bitmap[i] = 0xFFFFFFFF;
    }

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
        mmap = (multiboot_memory_map_t *) ((uint32_t) mmap + mmap->size + sizeof(mmap->size));
    }

    uint32_t reserved_end = (uint32_t)memory_bitmap + bitmap_size;
    uint32_t reserved_frames = reserved_end / PAGE_SIZE;
    for (uint32_t i = 0; i <= reserved_frames; i++) {
        bitmap_set(i);
    }
}

uint32_t pmm_alloc_frame() {
    int frame = find_first_free();
    if (frame == -1) {
        return 0;
    }
    bitmap_set(frame);
    return frame * PAGE_SIZE;
}

void pmm_free_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    bitmap_unset(frame);
}
