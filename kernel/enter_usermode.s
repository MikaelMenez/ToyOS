global enter_usermode

USER_CODE_SEG   equ 0x18
USER_DATA_SEG   equ 0x20
USER_ENTRY_EIP  equ 0x00000000
USER_ESP        equ 0x003FFFFB

enter_usermode:
    cli

    mov eax, [esp+4]
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov ax, (USER_DATA_SEG | 0x3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    pushfd
    pop eax
    and eax, 0xFFFFFDFF       ; limpa IF (sem interrupções no modo usuário)

    push dword (USER_DATA_SEG | 0x3)
    push dword USER_ESP
    push eax
    push dword (USER_CODE_SEG | 0x3)
    push dword USER_ENTRY_EIP

    iret
