ENTRY(loader)

SECTIONS {
    /* O kernel será carregado em 1MB na memória física */
    . = 0x00100000;

    /* Força a seção .text a alinhar em blocos de 4096 bytes (4KB) */
    .text ALIGN(4K) :
    {
        /* O cabeçalho Multiboot PRECISA ir primeiro, sem nenhuma outra seção antes */
        *(.multiboot)
        *(.text)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata*)
    }

    .data ALIGN(4K) :
    {
        *(.data)
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }
}
