global loader
extern kmain
extern gdt_install

extern kernel_virtual_start
extern kernel_virtual_end
extern kernel_physical_start
extern kernel_physical_end

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
    mov ecx, (page_directory - 0xC0000000)
    mov dword [ecx], 0x00000083
    mov dword [ecx + 3072], 0x00000083
    mov cr3, ecx

    mov ecx, cr4
    or ecx, 0x00000010
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    lea ecx, [higher_half]
    jmp ecx

higher_half:
    mov dword [page_directory + 0], 0
    invlpg [0]

    mov esp, kernel_stack + 4096

    add ebx, 0xC0000000

    push kernel_physical_end
    push kernel_physical_start
    push kernel_virtual_end
    push kernel_virtual_start
    push ebx            

    call gdt_install
    call kmain

.loop:
    cli
    hlt
    jmp .loop

section .bss
align 4096
page_directory:
    resb 4096 
kernel_stack:
    resb 4096

global loader
global page_directory    ;  nova linha - é necessário acessar isso
extern kmain
