; ============================================================================
; io.s — Acesso às portas de E/S do processador
; ----------------------------------------------------------------------------
; Toda conversa com o hardware em x86 acontece por portas de I/O (instruções
; `in` e `out`). O C não tem como fazer isso "de grana", então a gente expõe
; dois atalhos: outb (escreve um byte numa porta) e inb (lê um byte). São a
; base que o teclado, o serial, o VGA e o PIC usam.
; ============================================================================

; Acesso direto ao hardware via portas I/O
global outb
outb:
    mov al, [esp + 8] ; Carrega o byte de dados
    mov dx, [esp + 4] ; Carrega o endereço da porta
    out dx, al        ; Envia o byte
    ret

global inb
inb:
    mov dx, [esp + 4] ; Carrega a porta
    in al, dx         ; Lê o byte da porta
    ret               ; Retorna o valor em AL