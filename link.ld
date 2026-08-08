ENTRY(loader)

SECTIONS
{
    . = 0xC0100000;

    kernel_virtual_start = .;
    kernel_physical_start = . - 0xC0000000;

    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)
    }

    .rodata : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata*)
    }

    .data : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
    }

    kernel_virtual_end = .;
    kernel_physical_end = . - 0xC0000000;
    _kernel_p_end = . - 0xC0000000;
}
