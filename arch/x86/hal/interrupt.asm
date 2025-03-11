section .data
global vectors
vectors:
    dd isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
    dd isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23, isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    dd irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7, irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
    dd irq16, irq17, irq18, irq19, irq20, irq21, irq22, irq23, irq24, irq25, irq26, irq27, irq28, irq29, irq30, irq31
    dd irq32, irq33, irq34, irq35, irq36, irq37, irq38, irq39, irq40, irq41, irq42, irq43, irq44, irq45, irq46, irq47
    dd irq48, irq49, irq50, irq51, irq52, irq53, irq54, irq55, irq56, irq57, irq58, irq59, irq60, irq61, irq62, irq63
    dd irq64, irq65, irq66, irq67, irq68, irq69, irq70, irq71, irq72, irq73, irq74, irq75, irq76, irq77, irq78, irq79
    dd irq80, irq81, irq82, irq83, irq84, irq85, irq86, irq87, irq88, irq89, irq90, irq91, irq92, irq93, irq94, irq95
    dd irq96, irq97, irq98, irq99, irq100, irq101, irq102, irq103, irq104, irq105, irq106, irq107, irq108, irq109, irq110, irq111
    dd irq112, irq113, irq114, irq115, irq116, irq117, irq118, irq119, irq120, irq121, irq122, irq123, irq124, irq125, irq126, irq127
    dd irq128, irq129, irq130, irq131, irq132, irq133, irq134, irq135, irq136, irq137, irq138, irq139, irq140, irq141, irq142, irq143
    dd irq144, irq145, irq146, irq147, irq148, irq149, irq150, irq151, irq152, irq153, irq154, irq155, irq156, irq157, irq158, irq159
    dd irq160, irq161, irq162, irq163, irq164, irq165, irq166, irq167, irq168, irq169, irq170, irq171, irq172, irq173, irq174, irq175
    dd irq176, irq177, irq178, irq179, irq180, irq181, irq182, irq183, irq184, irq185, irq186, irq187, irq188, irq189, irq190, irq191
    dd irq192, irq193, irq194, irq195, irq196, irq197, irq198, irq199, irq200, irq201, irq202, irq203, irq204, irq205, irq206, irq207
    dd irq208, irq209, irq210, irq211, irq212, irq213, irq214, irq215, irq216, irq217, irq218, irq219, irq220, irq221, irq222, irq223

section .text
[extern common_handler]

; 通用中断处理程序
common_stub:
    pusha                   ; 1. 保存 CPU 状态
    mov ax, ds
    push eax                ; 保存数据段描述符
    mov ax, 0x10            ; 内核数据段描述符
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp                ; registers_t *r
    cld                     ; 清除方向标志
    call common_handler     ; 调用通用处理程序
    pop eax
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8              ; 清理推送的错误代码和中断号
    iret                    ; 弹出 CS, EIP, EFLAGS, SS 和 ESP

; 定义所有中断处理程序
%macro ISR 1
global isr%1
isr%1:
    push dword 0
    push dword %1
    jmp common_stub
%endmacro

%macro IRQ 2
global irq%1
irq%1:
    push dword %1
    push dword %2
    jmp common_stub
%endmacro

%assign i 0
%rep 32
    ISR i
    %assign i i+1
%endrep

%assign i 0
%rep 224
    IRQ i, i+32
    %assign i i+1
%endrep

