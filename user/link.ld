/* user/link.ld */
ENTRY(_start)

SECTIONS
{
    /* O programa de usuário começa no endereço virtual 0 */
    . = 0x00000000;

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}