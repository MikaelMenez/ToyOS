#include "stdint.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

/* 6 Entradas: 0=Nulo, 1=KCode, 2=KData, 3=UCode, 4=UData, 5=TSS */
struct gdt_entry gdt[6];
struct gdt_ptr gp;
static tss_entry_t ktss;

extern void load_gdt(uint32_t gdt_ptr);

void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access      = access;
}

static void write_tss(int num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t) &ktss;
    uint32_t limit = sizeof(tss_entry_t) - 1;

    gdt_set_gate(num, base, limit, 0x89, 0x00);

    for (uint32_t i = 0; i < sizeof(tss_entry_t); i++) {
        ((char*)&ktss)[i] = 0;
    }

    ktss.ss0 = ss0;
    ktss.esp0 = esp0;

    asm volatile("ltr %%ax" : : "a" (num * 8));
}

void gdt_install() {
    gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gp.base  = (uint32_t) &gdt;

    // 0: Descritor Nulo (0x00)
    gdt_set_gate(0, 0, 0, 0, 0);

    // 1: Kernel Code Segment (0x08)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 2: Kernel Data Segment (0x10)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // 3: User Code Segment (0x18, DPL=3 -> 0x18 | 0x3 = 0x1B)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // 4: User Data Segment (0x20, DPL=3 -> 0x20 | 0x3 = 0x23)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    load_gdt((uint32_t)&gp);

    // 5: Inicializa e carrega a TSS no índice 5 (Seletor 0x28)
    uint32_t esp_atual;
    asm volatile("mov %%esp, %0" : "=r" (esp_atual));
    write_tss(5, 0x10, esp_atual);
}
