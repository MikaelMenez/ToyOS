/* ============================================================================
 * gdt.c — A Global Descriptor Table (GDT) do ToyOS
 * ----------------------------------------------------------------------------
 * Em modo protegido a CPU "traduz" endereços usando segmentos definidos aqui.
 * A GDT é uma tabela de descritores; cada descritor define um segmento (suas
 * permissões, onde começa, até onde vai). Nosso kernel usa segmentação
 * "flat" (do 0 ao 0xFFFFFFFF), e a graça está nas permissões: temos segmentos
 * de CÓDIGO e DADOS tanto pro kernel (Ring 0) quanto pro usuário (Ring 3),
 * além de uma TSS, que é quem guarda a pilha do kernel quando o usuário chama
 * uma system call. Tudo isso o livro aborda no capítulo 5 (e 11, pro usuário).
 * ============================================================================ */

#include "stdint.h"

/* Descrição binária de um descritor de segmento (8 bytes "quebrados" em
 * campos). O processador tem um formato bem específico de como os bits se
 * organizam, então a gente usa um struct "packed" pra deixar na ordem certa. */
struct gdt_entry {
    uint16_t limit_low;   // primeiros 16 bits do limite
    uint16_t base_low;    // primeiros 16 bits da base
    uint8_t  base_middle; // próximos 8 bits da base
    uint8_t  access;      // flags de acesso (tipo, privilégio, presente...)
    uint8_t  granularity; // granularidade + bits altos do limite
    uint8_t  base_high;   // últimos 8 bits da base
} __attribute__((packed));

/* Estrutura que a instrução LGDT espera: tamanho e endereço da tabela. */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Formato da TSS no x86. A TSS guarda o contexto de troca de tarefas; pra nós
 * o mais importante é o `esp0`/`ss0`, que dizem onde fica a pilha do kernel
 * quando o usuário chama uma interrupção (int $0x80). */
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
static tss_entry_t ktss; // nossa TSS (uma só, do kernel)

extern void load_gdt(uint32_t gdt_ptr); // definida em gdt_s.s

/* Monta um descritor genérico a partir de base/limite/access/granularity.
 * É importante que a base caiba nos 32 bits "espalhados" pelo descritor. */
void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access      = access;
}

/* Configura e carrega a TSS. Recebe o stack pointer do kernel pra guardar no
 * esp0 — é pra lá que a CPU pula quando um processo de usuário interrompe. */
static void write_tss(int num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t) &ktss;
    uint32_t limit = sizeof(tss_entry_t) - 1;

    // Descritor 0x89 = presente, 32 bits, "disponível" (busy bit zerado)
    gdt_set_gate(num, base, limit, 0x89, 0x00);

    // Zera a TSS inteira antes de usá-la, pra não ter lixo de memória.
    for (uint32_t i = 0; i < sizeof(tss_entry_t); i++) {
        ((char*)&ktss)[i] = 0;
    }

    // Pilha do kernel usada quando o Ring 3 faz uma syscall.
    ktss.ss0 = ss0;
    ktss.esp0 = esp0;

    // Carrega a TSS (LTR). O seletor é o índice do descritor * 8.
    asm volatile("ltr %%ax" : : "a" (num * 8));
}

/* Instala a GDT completa e a ativa. É chamada uma única vez no boot. */
void gdt_install() {
    gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gp.base  = (uint32_t) &gdt;

    // 0: Descritor Nulo (0x00) — obrigatório, sempre com tudo zerado
    gdt_set_gate(0, 0, 0, 0, 0);

    // 1: Kernel Code Segment (0x08) — executável, privilégio 0
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 2: Kernel Data Segment (0x10) — dados, privilégio 0
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // 3: User Code Segment (0x18, DPL=3 -> 0x18 | 0x3 = 0x1B)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // 4: User Data Segment (0x20, DPL=3 -> 0x20 | 0x3 = 0x23)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // Carrega a GDT (LGDP + atualiza segmentos, em gdt_s.s)
    load_gdt((uint32_t)&gp);

    // 5: Inicializa e carrega a TSS no índice 5 (Seletor 0x28)
    uint32_t esp_atual;
    asm volatile("mov %%esp, %0" : "=r" (esp_atual)); // pega a pilha atual
    write_tss(5, 0x10, esp_atual);
}