[BITS 32]
align   4
section .text

extern  kmain

global  multiboot_header
multiboot_header:
    %define MAGIC 0x1badb002
    %define FLAGS 0
    dd MAGIC
    dd FLAGS
    dd -(MAGIC + FLAGS)

global _start
_start:
    mov  eax, 0x8000 ; 将数据的地址（0x8000）加载到 eax 寄存器
    push eax         ; 将地址压入栈中
    call kmain
