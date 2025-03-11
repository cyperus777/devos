#include <hal/irq.h>
#include <hal/console.h>
void irq_init(void)
{
    platform_irq_ops.init();
    kprintf("irq_init_done\n");
}