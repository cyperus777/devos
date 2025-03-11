; Declare external C function
extern bootmain

[BITS 16]
global _start
_start:
    call do_e820 ; 调用内存映射函数
    ; 禁用中断
    cli

    ; Enable A20
    in  al,   0x92
    or  al,   0x2
    out 0x92, al

    ; Load GDT
    lgdt [gdt_descriptor]
    
    ; Switch from real to protected mode
    mov eax, cr0
    or  eax, 0x1
    mov cr0, eax

    ; Jump into 32-bit protected mode
    jmp CODE_SEG:protected_mode

%include "e820.asm"
%include "gdt.asm"

[BITS 32]
protected_mode:
    mov ax,  DATA_SEG
    mov ds,  ax
    mov es,  ax
    mov fs,  ax
    mov gs,  ax
    mov ss,  ax
    mov esp, 0x9000

    call bootmain
spin:
    jmp spin
