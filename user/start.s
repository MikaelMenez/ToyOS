; ============================================================================
; user/start.s — O ponto de entrada do programa de usuário (Ring 3)
; ----------------------------------------------------------------------------
; Depois que o enter_usermode.s termina o IRET, a CPU começa a executar aqui
; no endereço 0. A única missão deste arquivo é chamar a função `main` do
; programa em C e, se ela por acaso retornar, segurar a execução pra sempre
; (não existe "deletar processo" por aqui ainda — seria o cap. 14!).
; ============================================================================

; user/start.s
[BITS 32]
global _start
extern main

section .text
_start:
    ; Chama a função main do nosso programa em C
    call main

    ; Se por acaso o main retornar, fica travado aqui para não crashar
.loop:
    jmp .loop