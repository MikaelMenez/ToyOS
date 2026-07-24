ENTRY(loader)                

SECTIONS {
    /* Mudamos a base virtual para 3GB + 1MB */
    . = 0xC0100000;          

    /* O comando AT(...) diz ao linker o endereço físico de carregamento (1MB) */
    .text ALIGN (0x1000) : AT(ADDR(.text) - 0xC0000000)   
    {
        *(.multiboot)        
        *(.text)             
    }

    .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - 0xC0000000) 
    {
        *(.rodata*)
    }

    .data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000)   
    {
        *(.data)
    }

    .bss ALIGN (0x1000) : AT(ADDR(.bss) - 0xC0000000)    
    {
        *(COMMON)
        *(.bss)
    }
}
