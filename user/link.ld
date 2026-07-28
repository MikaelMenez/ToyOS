/* user/link.ld */
OUTPUT_FORMAT("binary")   /* ADICIONAR ESTA LINHA */

ENTRY(_start)
SECTIONS
{
    /* O programa de usuário começa no endereço virtual 0 */
    . = 0x00000000;

    .text ALIGN(4) :
    {
        obj/start_s.o(.text)   /* garante que _start (start.s) vem primeiro */
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
