OUTPUT_FORMAT("binary")

SECTIONS
{
    . = 0;                 /* Começa no endereço virtual 0 */

    .text ALIGN(4):
    {
        obj/user_start.o(.text) /* Aponta corretamente para a pasta obj/ */
        *(.text)
    }

    .data ALIGN(4):
    {
        *(.data)
    }

    .rodata ALIGN(4):
    {
        *(.rodata*)
    }
}
