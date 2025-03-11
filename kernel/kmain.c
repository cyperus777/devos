#include <hal/console.h>
#include <hal/irq.h>
#include <defs.h>
#include <pci.h>
int kmain(struct memory_map_t *mmap)
{
    early_hw_init();
    mm_init(mmap);
    irq_init();
    pci_scan();
    __asm__ volatile("hlt");
    while (1)
        ;
}