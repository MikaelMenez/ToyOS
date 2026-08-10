; ============================================================================
; enter_usermode.s — O pulo final pro modo usuário (capítulo 11.3)
; ----------------------------------------------------------------------------
; Recebe o endereço físico do page directory do usuário, troca o CR3 pra ele,
; garante que a paginação segue ligada, ajusta os segmentos de dados pro modo
; Ring 3 e, por fim, usa um IRET "artificial" pra saltar com privilégio de
; usuário. O IRET é a única forma de trocar de privilégio numa CPU x86 limpa:
; ele lê da pilha um frame com SS, ESP, EFLAGS, CS e EIP, e a partir disso a
; CPU entende que estamos indo pra outro anel de proteção.
; ============================================================================

global enter_usermode

; Seletores dos segmentos de usuário (definidos no gdt.c):
; 0x18 = user code, 0x20 = user data. O "| 0x3" seta os bits de privilégio
; (RPL = 3) pra CPU aceitar a troca de anel.
USER_CODE_SEG   equ 0x18
USER_DATA_SEG   equ 0x20
USER_ENTRY_EIP  equ 0x00000000     ; o programa começa no endereço 0
USER_ESP        equ 0x003FFFFB     ; topo da pilha de usuário (fim da página)

enter_usermode:
    cli                     ; desligamos interrupções durante a troca de anel

    mov eax, [esp+4]        ; primeiro argumento: endereço físico do CR3
    mov cr3, eax            ; troca pro page directory do usuário

    mov eax, cr0
    or eax, 0x80000000      ; confirma que a paginação continua ligada
    mov cr0, eax

    ; Recarrega os segmentos de dados agora já no "mundo" do usuário
    mov ax, (USER_DATA_SEG | 0x3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Pega o EFLAGS atual e limpa a flag IF. Assim o Ring 3 começa sem
    ; interrupções até que o kernel determine que pode reabilitá-las.
    pushfd
    pop eax
    and eax, 0xFFFFFDFF       ; limpa IF

    ; Empilha o frame que o IRET vai consumir, de cima pra baixo:
    push dword (USER_DATA_SEG | 0x3)   ; SS do usuário
    push dword USER_ESP                ; ESP do usuário
    push eax                           ; EFLAGS (sem interrupções por enquanto)
    push dword (USER_CODE_SEG | 0x3)   ; CS do usuário
    push dword USER_ENTRY_EIP          ; EIP = começa a executar o programa

    iret                                ; agora estamos em Ring 3! 🎉