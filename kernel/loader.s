; ============================================================================
; loader.s — O ponto de partida do ToyOS
; ----------------------------------------------------------------------------
; É aqui que a vida começa: o GRUB carrega o nosso kernel e pula pra cá.
; A primeira coisa que a gente precisa fazer é construir uma "ponte" entre o
; modo como a CPU está rodando (flat, identidade) e o modo onde o kernel quer
; viver (na metade superior, acima de 0xC0000000). O Resumo da ópera:
;
;   1. Montar uma tabela de páginas de 4 MB para mapear a memória toda.
;   2. Ligar a paginação na CPU (CR0.PG).
;   3. Pular pro código "alto" (higher half) que já está linkado pra rodar lá.
;   4. Organizar as coisas (pilha, ponteiro do GRUB) e chamar o C.
; ============================================================================

global loader
extern kmain
extern gdt_install

; Símbolos que o linker script (link.ld) define pra gente:
; o começo/fim do kernel, tanto em endereço virtual quanto físico.
extern kernel_virtual_start
extern kernel_virtual_end
extern kernel_physical_start
extern kernel_physical_end

; ----------------------------------------------------------------------------
; Cabeçalho Multiboot: é o "RG" que o GRUB exige para aceitar nosso kernel.
; ----------------------------------------------------------------------------
MAGIC_NUMBER    equ 0x1BADB002      ; número mágico reconhecido pelo GRUB
ALIGN_MODULES   equ 0x00000001      ; pedimos que os módulos venham alinhados
FLAGS           equ ALIGN_MODULES
CHECKSUM        equ -(MAGIC_NUMBER + FLAGS)   ; soma dá zero, GRUB aceita

section .text
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

; ----------------------------------------------------------------------------
; loader: o primeiro código de verdade que roda
; ----------------------------------------------------------------------------
loader:
    ; Vamos construir uma página de 4 MB que mapeia a memória física inteira.
    ; A tabela de páginas vive no próprio kernel (section .bss, no fim do arquivo).
    ; CUIDADO: enquanto a paginação não estiver ligada, a gente acha a tabela
    ; pelo endereço FÍSICO dela, por isso o "- 0xC0000000".
    mov ecx, (page_directory - 0xC0000000)
    mov dword [ecx], 0x00000083          ; entrada 0: 4MB, presente, escrita, supervisor
    mov dword [ecx + 3072], 0x00000083   ; entrada 768 (0xC0000000/4MB): idem, pro kernel
    mov cr3, ecx                         ; carrega o endereço da tabela no CR3

    ; Habilita o suporte a páginas grandes de 4 MB (bit PSE no CR4).
    mov ecx, cr4
    or ecx, 0x00000010
    mov cr4, ecx

    ; Liga a paginação de vez (bit PG do CR0).
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    ; Agora que tudo está mapeado, saltamos para o código "de cima".
    ; O lea carrega o endereço VIRTUAL do rótulo higher_half, que só existe
    ; após pularmos a fronteira dos 4 MB.
    lea ecx, [higher_half]
    jmp ecx

higher_half:
    ; Aqui já estamos rodando na metade superior. Apaga o mapeamento de
    ; identidade da entrada 0 pra ninguém mais acessar a memória baixa
    ; como se fosse "física". Só o kernel (em 0xC...) continua mapeado.
    mov dword [page_directory + 0], 0
    invlpg [0]

    ; Define uma pilha pra gente poder chamar funções em C.
    mov esp, kernel_stack + 4096

    ; O GRUB nos passa em ebx um ponteiro pra estrutura multiboot.
    ; Como vamos rodar acima de 4 MB, o ponteiro precisa ser ajustado pra
    ; apontar pra mesma estrutura na visão superior.
    add ebx, 0xC0000000

    ; Prepara os argumentos para as rotinas de inicialização em C
    ; (o kmain espera ebx; os limites do kernel são usados pelo link.ld).
    push kernel_physical_end
    push kernel_physical_start
    push kernel_virtual_end
    push kernel_virtual_start
    push ebx

    ; Deixa o mundo do C por conta da casa: instala a GDT e chama o kmain.
    call gdt_install
    call kmain

.loop:
    ; Se o kmain "terminar", nunca mais deixamos o kernel desistir — trava aí.
    cli
    hlt
    jmp .loop

; ----------------------------------------------------------------------------
; Área de dados do loader
; ----------------------------------------------------------------------------
section .bss
align 4096
page_directory:
    resb 4096 ; tabela de páginas do kernel (alinhada a 4 KB)
kernel_stack:
    resb 4096 ; pilha do kernel (4 KB de sobra)

global loader
global page_directory    ; exposto pro usermode.c preparar a tabela do usuário