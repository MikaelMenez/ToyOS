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
    
    ; O diretório de páginas foi linkado em 0xC0100000, mas a paginação ainda está desligada!
    ; Precisamos do endereço físico real para a CPU, então subtraímos 0xC0000000.
    mov ecx, (page_directory - 0xC0000000)

    ; Mapeamento de Identidade (Primeiros 4MB físicos -> Primeiros 4MB virtuais)
    ; Flags: Presente (bit 0) | R/W (bit 1) | Page Size 4MB (bit 7) = 0x83
    mov dword [ecx], 0x00000083

    ; Mapeamento da Metade Superior (Primeiros 4MB físicos -> 3GB virtuais)
    ; 0xC0000000 no diretório cai no índice 768 (768 * 4 bytes = 3072)
    mov dword [ecx + 3072], 0x00000083

    ; Carrega o endereço físico do diretório no registrador CR3
    mov cr3, ecx

    ; Habilita o Page Size Extension (PSE) no CR4 para suportar páginas de 4MB
    mov ecx, cr4
    or ecx, 0x00000010
    mov cr4, ecx

    ; Habilita a Paginação ativando o bit PG no CR0
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    
    ; Pega o endereço virtual absoluto do rótulo e pula para lá
    lea ecx, [higher_half]
    jmp ecx

higher_half:
    
    mov dword [page_directory + 0], 0
    invlpg [0]

    ; Configura a pilha (usando o endereço virtual agora)
    mov esp, kernel_stack + 4096
    
    
    ; Somamos 0xC0000000 para que o kmain consiga ler as informações na metade superior
    add ebx, 0xC0000000
    push ebx            
    
    call gdt_install
    call kmain

.loop:
    jmp .loop

section .bss
align 4096
page_directory:
    resb 4096 ; Reserva 4KB para o diretório de páginas

kernel_stack:
    resb 4096 ; Reserva 4KB para a pilha do kernel
