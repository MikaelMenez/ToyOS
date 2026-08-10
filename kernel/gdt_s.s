; ============================================================================
; gdt_s.s — A parte em assembly da GDT
; ----------------------------------------------------------------------------
; A instrução LGDT é o único jeito de ligar a tabela que montamos no gdt.c.
; Depois de carregá-la, precisamos "recarregar" os registradores de segmento
; com os novos valores e dar um pulo pra dentro do novo segmento de código
; (isso é obrigatório, porque CS só muda saltando).
; ============================================================================

global load_gdt

load_gdt:
    mov eax, [esp+4]   ; Pega o endereço da estrutura gdt_ptr
    lgdt [eax]         ; Carrega a GDT na CPU

    mov ax, 0x10       ; 0x10 é o offset para o segmento de dados do kernel
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    jmp 0x08:flush_cs  ; 0x08 é o offset para o segmento de código do kernel;
                       ; o far jump deixa o CS apontando pro segmento certo

flush_cs:
    ret