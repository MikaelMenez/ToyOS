global loader
extern kmain

MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x00000001
CHECKSUM     equ -(MAGIC_NUMBER + FLAGS)

section .text
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

KERNEL_STACK_SIZE equ 4096

section .bss
align 4
kernel_stack:
    resb KERNEL_STACK_SIZE

section .text
loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    push eax
    push ebx

    call kmain

.loop:
    jmp .loop
