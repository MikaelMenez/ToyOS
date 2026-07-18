global load_gdt

load_gdt:
    mov eax, [esp+4]   ; Pega o endereço da estrutura gdt_ptr
    lgdt [eax]         ; Carrega a GDT

    mov ax, 0x10       ; 0x10 é o offset para o segmento de dados na GDT
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    jmp 0x08:flush_cs  ; 0x08 é o offset para o segmento de código na GDT

flush_cs:
    ret
