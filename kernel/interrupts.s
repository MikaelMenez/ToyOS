; ============================================================================
; interrupts.s — Os manuseadores "genéricos" de interrupção
; ----------------------------------------------------------------------------
; Quando uma interrupção acontece, a CPU empurra na pilha o número dela
; (e o código de erro, quando existe). Aqui usamos uma macro pra criar os
; handlers que: salvam os registradores, chamam o tratador em C e restauram
; tudo antes de voltar com IRET. Também embrulhamos a instrução LIDT pro C.
; ============================================================================

extern interrupt_handler
global load_idt
global interrupt_handler_32 ; Temporizador
global interrupt_handler_33 ; Teclado

; O livro usa uma macro para interrupções sem código de erro.
; Interrupções de hardware não têm código de erro, então a macro empilha um 0
; no lugar dele pra manter o formato da pilha sempre igual.
%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push    dword 0                     ; empilha 0 no lugar do código de erro
    push    dword %1                    ; empilha o número da interrupção
    jmp     common_interrupt_handler    ; pula pro tratador comum
%endmacro

common_interrupt_handler:
    ; Salva o contexto do código que foi interrompido. Assim o tratador em C
    ; pode usar os registradores à vontade sem estragar o que estava antes
    ; rodando.
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    push    ebp

    ; Chama o tratador em C (idt.c), que decide o que fazer (teclado, timer...)
    call    interrupt_handler

    ; Restaura os registradores na ordem inversa do push
    pop     ebp
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    ; Desce os dois valores que a macro empilhou (número + código de erro)
    add     esp, 8

    ; Volta pro código que tinha sido interrompido
    iret

; Cria os manipuladores das interrupções de hardware (Timer = 32, Teclado = 33)
no_error_code_interrupt_handler 32
no_error_code_interrupt_handler 33

; Embrulha a instrução 'lidt' pra ser usada pelo C
load_idt:
    mov     eax, [esp+4]    ; pega o endereço da estrutura idt_ptr
    lidt    [eax]           ; carrega a IDT
    ret                     ; volta pra quem chamou