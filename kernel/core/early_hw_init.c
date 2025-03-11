#include "defs.h"
typedef void (*init_func_t)(void);
extern init_func_t __init_array_start[];
extern init_func_t __init_array_end[];

void call_constructors(void)
{
    for (init_func_t *func = __init_array_start; func < __init_array_end; func++)
    {
        (*func)();
    }
}
void early_hw_init()
{
    // 早期硬件初始化代码
    call_constructors();
    console_init();
    console_set_color(CONSOLE_CYAN, CONSOLE_BLACK);
    kprintf("early_hw_init_done\n");
}