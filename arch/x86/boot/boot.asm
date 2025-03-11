[org 0x7c00]
[bits 16]
KERNEL_OFFSET equ 0x1000
global _start
_start:

    mov [BOOT_DRIVE], dl
    mov bp,           0x9000
    mov sp,           bp

    call load_kernel

    mov ax, 0x0000
    mov ds, ax

    jmp 0x0000:0x1000

%include "disk.asm"

[bits 16]
load_kernel:
    mov  bx, KERNEL_OFFSET
    mov  dh, 4
    mov  dl, [BOOT_DRIVE]
    call disk_load
    ret

BOOT_DRIVE db 0

times 510 - ($-$$) db 0
dw 0xaa55