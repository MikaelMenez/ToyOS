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
