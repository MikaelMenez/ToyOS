/* ============================================================================
 * link.ld — Script de link do kernel (cap. 9: paging e metade superior)
 * ----------------------------------------------------------------------------
 * Decide onde cada seção do kernel vai parar em memória. O jeitão é este:
 *
 *   - O kernel é LINKADO pra rodar na metade superior (0xC0100000+);
 *   - mas as seções são GRAVADAS na memória física correspondente
 *     (por isso o "AT(... - 0xC0000000)").
 *
 * Assim, enquanto as seções ocupam fatos no disco/ELO, o código tá pronto pra
 * viver em 0xC0000000 quando o loader.s ligar a paginação. Os símbolos
 * *_start/_end definidos aqui são usados pelo loader e pelo pmm.
 * ============================================================================ */

ENTRY(loader)

SECTIONS
{
    . = 0xC0100000;                       /* começa 1 MB acima do "marco" */

    kernel_virtual_start = .;
    kernel_physical_start = . - 0xC0000000;

    .text : AT(ADDR(.text) - 0xC0000000)  /* código, físico = virtual - 0xC0000000 */
    {
        *(.text)
    }

    .rodata : AT(ADDR(.rodata) - 0xC0000000) /* constantes e strings */
    {
        *(.rodata*)
    }

    .data : AT(ADDR(.data) - 0xC0000000)  /* variáveis inicializadas */
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - 0xC0000000)    /* variáveis não inicializadas */
    {
        *(COMMON)
        *(.bss)
    }

    kernel_virtual_end = .;
    kernel_physical_end = . - 0xC0000000;
    _kernel_p_end = . - 0xC0000000; /* fim físico do kernel (usado pelo PMM) */
}