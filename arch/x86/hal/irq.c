#include "hal/irq.h"
#include "hal/console.h"
#include "x86.h"
#include <types.h>
#define IDT_ENTRIES 256
/* Segment selectors */
#define KERNEL_CS 0x08
/*                      I/O port */
#define PIC_1 0x20 /* IO base address for master PIC */
#define PIC_2 0xA0 /* IO base address for slave PIC */
#define PIC_1_COMMAND PIC_1
#define PIC_1_DATA (PIC_1 + 1)
#define PIC_2_COMMAND PIC_2
#define PIC_2_DATA (PIC_2 + 1)

#define PIC_1_OFFSET 0x20
#define PIC_2_OFFSET 0x28
#define PIC_2_END PIC_2_OFFSET + 7

#define PIC_1_COMMAND_PORT 0x20
#define PIC_2_COMMAND_PORT 0xA0
#define PIC_ACKNOWLEDGE 0x20

#define PIC_ICW1_ICW4 0x01      /* ICW4 (not) needed */
#define PIC_ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define PIC_ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define PIC_ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define PIC_ICW1_INIT 0x10      /* Initialization - required! */

#define PIC_ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define PIC_ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define PIC_ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define PIC_ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define PIC_ICW4_SFNM 0x10       /* Special fully nested (not) */

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

/* How every interrupt gate (handler) is defined */
typedef struct
{
    uint16_t low_offset; /* Lower 16 bits of handler function address */
    uint16_t sel;        /* Kernel segment selector */
    uint8_t always0;
    /* First byte
     * Bit 7: "Interrupt is present"
     * Bits 6-5: Privilege level of caller (0=kernel..3=user)
     * Bit 4: Set to 0 for interrupt gates
     * Bits 3-0: bits 1110 = decimal 14 = "32 bit interrupt gate" */
    uint8_t flags;
    uint16_t high_offset; /* Higher 16 bits of handler function address */
} __attribute__((packed)) idt_gate_t;
/* A pointer to the array of interrupt handlers.
 * Assembly instruction 'lidt' will read it */
typedef struct
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_register_t;

#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

isr_t interrupt_handlers[256];

extern uint32_t vectors[];
idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;

void pic_remap(int offset1, int offset2)
{
    out(PIC_1_COMMAND, PIC_ICW1_INIT + PIC_ICW1_ICW4); // starts the initialization sequence (in cascade mode)
    out(PIC_2_COMMAND, PIC_ICW1_INIT + PIC_ICW1_ICW4);
    out(PIC_1_DATA, offset1); // ICW2: Master PIC vector offset
    out(PIC_2_DATA, offset2); // ICW2: Slave PIC vector offset
    out(PIC_1_DATA, 4);       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    out(PIC_2_DATA, 2);       // ICW3: tell Slave PIC its cascade identity (0000 0010)

    out(PIC_1_DATA, PIC_ICW4_8086);
    out(PIC_2_DATA, PIC_ICW4_8086);

    // Setup Interrupt Mask Register (IMR)
    out(PIC_1_DATA, 0x00);
    out(PIC_2_DATA, 0x00);
}

void set_idt_gate(int n, uint32_t handler)
{
    idt[n].low_offset = low_16(handler);
    idt[n].sel = KERNEL_CS;
    idt[n].always0 = 0;
    idt[n].flags = 0x8E;
    idt[n].high_offset = high_16(handler);
}

void set_idt()
{
    idt_reg.base = (uint32_t)&idt;
    idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
    __asm__ volatile("lidtl (%0)" : : "r"(&idt_reg));
}
void register_interrupt_handler(uint8_t n, isr_t handler);
void keyboard_interrupt_handler(registers_t *regs)
{
    // 处理键盘中断
    // 清除中断标志
    uint8_t scancode = in(0x60);
    kprintf("Keyboard scancode: %02x\n", scancode);
}
static void x86_irq_init(void)
{
    // Remap the PIC
    pic_remap(0x20, 0x28);
    // 设置IRQ中断处理程序
    for (int i = 0; i < 256; i++)
    {
        set_idt_gate(i, (uint32_t)vectors[i]);
    }
    set_idt(); // Load with ASM
    __asm__ volatile("sti");
    register_interrupt_handler(32 + 1, keyboard_interrupt_handler);
}
void common_handler(registers_t *regs)
{
    if (regs->int_no < 32)
    {
        kprintf("irq interrupt: %d\n", regs->int_no);
        //  处理ISR
        switch (regs->int_no)
        {
        case 0:
            // 处理除零错误
            break;
        case 14:
            // 处理页面错误
            break;
        // 其他ISR处理
        default:
            // 未知ISR
            break;
        }
    }
    else
    {
        // 处理IRQ
        // 发送结束信号给PICs
        if (regs->int_no >= 40)
            out(0xA0, 0x20); /* slave */
        out(0x20, 0x20);     /* master */

        if (interrupt_handlers[regs->int_no] != 0)
        {
            isr_t handler = interrupt_handlers[regs->int_no];
            handler(regs);
        }
    }
}

void register_interrupt_handler(uint8_t n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}
// 导出平台实现的接口
const hal_irq_ops_t platform_irq_ops = {
    .init = x86_irq_init,
};