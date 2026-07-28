global enter_usermode

; Offsets dos segmentos de usuario criados no capitulo 11.1 (Segments for
; User Mode). O "| 0x3" no final e o RPL (Requested Privilege Level = 3).
USER_CODE_SEG   equ 0x18
USER_DATA_SEG   equ 0x20

; Ponto de entrada e topo da pilha do processo de usuario, dentro da
; regiao fisica de 4MB que o usermode_setup() (capitulo 11.2) preparou.
USER_ENTRY_EIP  equ 0x00000000
USER_ESP        equ 0x003FFFFB

; enter_usermode - salta para o modo usuario (PL3) via iret.
; stack: [esp + 4] endereco FISICO do page directory do usuario
;        [esp    ] endereco de retorno (nunca sera usado - nao voltamos)
;
; Nao retorna: uma vez executado o iret, a CPU passa a rodar o codigo do
; processo de usuario, que nao tem como "dar ret" de volta pro kernel.
enter_usermode:
    cli                              ; desabilita interrupcoes: ainda nao
                                      ; temos uma TSS (capitulo 13), entao
                                      ; uma interrupcao em PL3 agora quebraria

    mov eax, [esp+4]                 ; pega o page directory fisico do usuario
    mov cr3, eax                     ; troca o espaco de enderecamento

    mov ax, (USER_DATA_SEG | 0x3)    ; seletor de dados do usuario, RPL=3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    pushfd                           ; pega o eflags atual
    pop eax
    and eax, 0xFFFFFDFF              ; zera o bit IF (interrupcoes off)

    push dword (USER_DATA_SEG | 0x3) ; ss do usuario
    push dword USER_ESP              ; esp do usuario
    push eax                         ; eflags (com IF=0)
    push dword (USER_CODE_SEG | 0x3) ; cs do usuario
    push dword USER_ENTRY_EIP        ; eip do usuario (inicio do programa)

    iret                             ; salta para PL3