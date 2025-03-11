#ifndef __PCI_H_
#define __PCI_H_
#include <types.h>
// 定义 PCI 设备结构体
typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t vendor_id;
    uint16_t device_id;
    // 可根据需要扩展其他字段（如 class code、subclass 等）
} pci_device_t;

// 驱动注册结构体
typedef struct pci_driver {
    uint16_t vendor_id;
    uint16_t device_id;
    // 驱动初始化函数，传入扫描到的设备信息
    void (*init)(pci_device_t *dev);
    struct pci_driver *next;
} pci_driver_t;

void pci_scan();
void register_driver(pci_driver_t *driver);
// PCI配置空间读取函数
extern inline uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
// PCI配置空间写入函数
extern inline void pci_write_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t value);

#define REGISTER_PCI_DRIVER(vendor, device, init_func)                     \
    static pci_driver_t __pci_driver_##vendor##_##device = {                 \
        .vendor_id = (vendor),                                               \
        .device_id = (device),                                               \
        .init = (init_func),                                                 \
        .next = 0                                                            \
    };                                                                       \
    __attribute__((constructor)) static void register_##vendor##_##device(void) { \
        register_driver(&__pci_driver_##vendor##_##device);                  \
    }
#endif