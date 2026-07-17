extern interrupt_handler
global load_idt
global interrupt_handler_32 ; Temporizador
global interrupt_handler_33 ; Teclado

; O livro usa uma macro para interrupções sem código de erro
%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push    dword 0                     ; push 0 as error code
    push    dword %1                    ; push the interrupt number
    jmp     common_interrupt_handler    ; jump to the common handler
%endmacro

common_interrupt_handler:               ; the common parts of the generic interrupt handler
    ; save the registers
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    push    ebp

    ; call the C function
    call    interrupt_handler

    ; restore the registers
    pop     ebp
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    ; restore the esp
    add     esp, 8

    ; return to the code that got interrupted
    iret

; Cria os manipuladores para as interrupções de hardware (Timer = 32, Teclado = 33)
no_error_code_interrupt_handler 32       
no_error_code_interrupt_handler 33       

; Embrulha a instrução 'lidt' para ser usada pelo C
load_idt:
    mov     eax, [esp+4]    ; load the address of the IDT into register eax
    lidt    [eax]           ; load the IDT
    ret                     ; return to the calling function
