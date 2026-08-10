; ============================================================================
; syscall_s.s — O "túnel" da system call (cap. 13)
; ----------------------------------------------------------------------------
; Quando um programa em Ring 3 executa `int $0x80`, a CPU:
;   1. troca pra pilha do kernel (graças à TSS.esp0 que configuramos);
;   2. empilha automaticamente EIP, CS, EFLAGS, ESP e SS do usuário;
;   3. pula pra cá, no anel 0.
;
; Nossa convenção (definida no syscall.h) é:
;   eax = número da chamada, ebx = a, ecx = b, edx = c, esi = d
;
; Aqui só preservamos os registradores, chamamos o dispatcher em C
; (syscall_handler_c) e guardamos o retorno lá no lugar do eax.
; ============================================================================

global syscall_entry
extern syscall_handler_c

syscall_entry:
    ; Preserva os registradores que o usuário pode estar usando
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax

    ; Monta a chamada em C (convenção cdecl):
    ; syscall_handler_c(num, a, b, c, d)
    push esi       ; d
    push edx       ; c
    push ecx       ; b
    push ebx       ; a
    push eax       ; num
    call syscall_handler_c
    add  esp, 20   ; desempilha os 5 argumentos

    ; Guarda o retorno da função no lugar do eax salvo e restaura tudo
    mov [esp], eax
    pop eax
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp
    iret   ; volta pro Ring 3 com o resultado em eax