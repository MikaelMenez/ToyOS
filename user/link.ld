OUTPUT_FORMAT("binary")

ENTRY(_start)
SECTIONS
{
    . = 0x00000000;

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
