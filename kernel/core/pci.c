#include <pci.h>
#include <hal/console.h>
// 定义 PCI 配置端口
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC
// 内联汇编实现 I/O 操作
static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// PCI配置空间读取函数
inline uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset)
{
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

// PCI配置空间写入函数
inline void pci_write_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t value)
{
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}
// 驱动链表头指针
static pci_driver_t *driver_list = 0;

// 注册驱动函数（在各个驱动模块的初始化中调用）
void register_driver(pci_driver_t *driver)
{
    kprintf("register_driver\n");
    driver->next = driver_list;
    driver_list = driver;
}

// 扫描 PCI 设备并调用匹配的驱动初始化
void pci_scan()
{
    pci_device_t dev;
    uint32_t data;
    // 假设总线范围 0~255，槽位 0~31，功能 0~7
    for (uint16_t bus = 0; bus < 256; bus++)
    {
        for (uint8_t slot = 0; slot < 32; slot++)
        {
            // 功能 0：先读一次检查设备是否存在
            data = pci_read_config(bus, slot, 0, 0);
            if (data == 0xFFFFFFFF) {
                // 没有设备，跳过
                continue;
            }
            // 多功能设备需要扫描所有功能
            uint8_t func_limit = 1;
            if ((data >> 16) != 0xFFFF) {  // 如果第一个设备的 header type 表示多功能，可读取 header type 检查
                uint8_t header_type = (pci_read_config(bus, slot, 0, 0x0C) >> 16) & 0xFF;
                if (header_type & 0x80) {
                    func_limit = 8;
                }
            }
            for (uint8_t func = 0; func < func_limit; func++) {
                data = pci_read_config(bus, slot, func, 0);
                if (data == 0xFFFFFFFF)
                    continue;
                dev.bus = bus;
                dev.slot = slot;
                dev.func = func;
                dev.vendor_id = data & 0xFFFF;
                dev.device_id = (data >> 16) & 0xFFFF;
                // 遍历注册的驱动进行匹配
                pci_driver_t *drv = driver_list;
                while (drv) {
                    if (drv->vendor_id == dev.vendor_id && drv->device_id == dev.device_id) {
                        // 找到匹配驱动，调用初始化函数
                        drv->init(&dev);
                        break;
                    }
                    drv = drv->next;
                }
            }
        }
    }
    kprintf("pci_scan_done\n");
}
