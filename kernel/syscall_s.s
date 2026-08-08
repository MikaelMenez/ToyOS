; Stub do syscall: interrompido via "int $0x80" a partir do Ring 3.
; A CPU ja trocou para a pilha do kernel (TSS.esp0) e empilhou
; EIP, CS, EFLAGS, ESP, SS do usuario.
;
; Entrada:
;   eax = numero  ebx = a  ecx = b  edx = c  esi = d

global syscall_entry
extern syscall_handler_c

syscall_entry:
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax

    ; Argumentos (cdecl): syscall_handler_c(num, a, b, c, d)
    push esi
    push edx
    push ecx
    push ebx
    push eax
    call syscall_handler_c
    add  esp, 20

    ; Guarda o retorno no lugar do eax salvo e restaura os demais registradores
    mov [esp], eax
    pop eax
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp
    iret