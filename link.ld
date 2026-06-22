ENTRY(loader)                /* O ponto de entrada do SO é o rótulo 'loader' */

SECTIONS {
    . = 0x00100000;          /* O código será carregado em 1 MB da memória RAM */

    .text ALIGN (0x1000) :   /* Seção de código alinhada */
    {
        *(.multiboot)        /* Garante que o cabeçalho multiboot fique no início */
        *(.text)             /* Todo o resto do código em Assembly/C */
    }

    .rodata ALIGN (0x1000) : /* Dados de apenas leitura (ex: strings constantes) */
    {
        *(.rodata*)
    }

    .data ALIGN (0x1000) :   /* Dados inicializados (ex: variáveis globais) */
    {
        *(.data)
    }

    .bss ALIGN (0x1000) :    /* Dados não-inicializados */
    {
        *(COMMON)
        *(.bss)
    }
}
