global gdtFlush

gdtFlush:
    mov eax, [esp+4]    ; pega o endereço da struct GdtPtr (parâmetro na pilha, cdecl)
    lgdt [eax]          ; carrega a GDT no processador

    mov ax, 0x10        ; 0x10 = offset do segmento de dados na GDT
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    jmp 0x08:.flush_cs   ; far jump: recarrega cs com o offset do segmento
                         ; de código (0x08); não existe "mov cs, ..."
.flush_cs:
    ret