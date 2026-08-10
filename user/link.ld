/* ============================================================================
 * user/link.ld — Script de link do programa de usuário
 * ----------------------------------------------------------------------------
 * O programa de usuário é linkado como um binário CRU (sem cabeçalho ELF),
 * carregado direto em memória pelo kernel. Ele começa no endereço 0, e o
 * `_start` (start.s) é obrigatoriamente a primeira coisa, porque o kernel
 * faz o salto pro endereço 0. Por isso a regra no .text: coloca o objeto
 * `obj/start_s.o` antes de tudo.
 * ============================================================================ */

OUTPUT_FORMAT("binary")

ENTRY(_start)
SECTIONS
{
    . = 0x00000000;               /* o binário é carregado no endereço 0 */

    .text ALIGN(4) :
    {
        obj/start_s.o(.text)  /* _start precisa estar estritamente no topo */
        *(.text)
    }
    .data ALIGN(4) :
    {
        *(.data)
    }
    .rodata ALIGN(4) :
    {
        *(.rodata*)
    }
    .bss ALIGN(4) :
    {
        *(.bss)
        *(COMMON)
    }
}