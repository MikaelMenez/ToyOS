global loader
extern kmain
extern gdt_install

MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x0
CHECKSUM     equ -(MAGIC_NUMBER + FLAGS)

section .text
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

loader:
    mov esp, kernel_stack + 4096 ; Define a pilha
    call gdt_install             ; Inicializa a GDT antes do C
    call kmain                   ; Chama o seu código C

.loop:
    jmp .loop

section .bss
align 4
kernel_stack:
    resb 4096
