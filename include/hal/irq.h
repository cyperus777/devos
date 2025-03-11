#ifndef HAL_IRQ_H
#define HAL_IRQ_H
#include <types.h>

typedef struct registers
{
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;
typedef void (*isr_t)(registers_t *);
void irq_init();
void register_interrupt_handler(uint8_t n, isr_t handler);

// 控制台硬件抽象接口
typedef struct {
    void (*init)(void);                    // 初始化控制台
} hal_irq_ops_t;
// 平台需要实现的接口结构体
extern const hal_irq_ops_t platform_irq_ops;
#endif