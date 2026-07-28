[bits 32]
section .text
global start

start:
    ; O módulo agora não escreve mais nada na tela, apenas retorna o controle para o kernel
    ret
