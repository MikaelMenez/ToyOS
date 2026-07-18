global loader
extern kmain
extern gdt_install

MAGIC_NUMBER    equ 0x1BADB002
ALIGN_MODULES   equ 0x00000001
FLAGS           equ ALIGN_MODULES
CHECKSUM        equ -(MAGIC_NUMBER + FLAGS)

section .text
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

loader:
    mov esp, kernel_stack + 4096
    
    ; Passa o ponteiro da estrutura Multiboot (ebx) para o kmain
    push ebx            
    
    call gdt_install
    call kmain

.loop:
    jmp .loop

section .bss
align 4
kernel_stack:
    resb 4096
