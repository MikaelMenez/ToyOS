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