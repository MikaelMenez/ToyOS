#ifndef INCLUDE_PMM_H
#define INCLUDE_PMM_H

#include "stdint.h"

void pmm_init(uint32_t mmap_addr, uint32_t mmap_length, uint32_t mem_lower, uint32_t mem_upper, uint32_t kernel_phys_end);
uint32_t pmm_alloc_frame();
void pmm_free_frame(uint32_t frame_addr);

#endif
