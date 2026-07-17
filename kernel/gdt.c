struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gp;

// Declarando a função externa que criaremos no assembly
extern void load_gdt(unsigned int gdt_ptr);

void gdt_set_gate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access      = access;
}

void gdt_install() {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base  = (unsigned int) &gdt;

    // Descritor Nulo
    gdt_set_gate(0, 0, 0, 0, 0);
    // Segmento de Código (0x9A = Executável/Legível, 0xCF = 4KB granularidade, 32-bit)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    // Segmento de Dados (0x92 = Gravável, 0xCF = 4KB granularidade, 32-bit)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    load_gdt((unsigned int)&gp);
}
