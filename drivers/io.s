; Aqui estão as instruções puras de Assembly para falar com as portas.
; 'outb' envia dados para uma porta; 'inb' lê dados.

global outb
outb:
    mov al, [esp + 8] ; Carrega o dado (segundo argumento)
    mov dx, [esp + 4] ; Carrega a porta (primeiro argumento)
    out dx, al        ; Instrução nativa do X86 para enviar
    ret

global inb
inb:
    mov dx, [esp + 4] ; Carrega a porta
    in al, dx         ; Instrução nativa do X86 para ler
    ret               ; O resultado fica em 'al' (retornado ao C)
