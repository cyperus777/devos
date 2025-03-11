#include <pci.h>
#include <hal/console.h>
#include <hal/irq.h>
#include <utils.h>
// MMIO基地址
volatile uint32_t *e1000_mmio;

// e1000寄存器偏移量
#define E1000_REG_CTRL 0x0000
#define E1000_REG_STATUS 0x0008
#define E1000_REG_EERD 0x0014
#define E1000_REG_ICR 0x00C0
#define E1000_REG_IMS 0x00D0
#define E1000_REG_RCTL 0x0100
#define E1000_REG_TCTL 0x0400
#define E1000_REG_RDBAL 0x2800
#define E1000_REG_RDBAH 0x2804
#define E1000_REG_RDLEN 0x2808
#define E1000_REG_RDH 0x2810
#define E1000_REG_RDT 0x2818
#define E1000_REG_TDBAL 0x3800
#define E1000_REG_TDBAH 0x3804
#define E1000_REG_TDLEN 0x3808
#define E1000_REG_TDH 0x3810
#define E1000_REG_TDT 0x3818

// 接收和发送描述符的数量
#define RX_DESC_COUNT 128
#define TX_DESC_COUNT 128

// 接收和发送缓冲区的大小
#define RX_BUFFER_SIZE 2048
#define TX_BUFFER_SIZE 2048

// 接收和发送描述符结构
typedef struct
{
    uint64_t buffer_addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} e1000_rx_desc;

typedef struct
{
    uint64_t buffer_addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} e1000_tx_desc;

// 接收和发送缓冲区
uint8_t rx_buffer[RX_DESC_COUNT][RX_BUFFER_SIZE];
uint8_t tx_buffer[TX_DESC_COUNT][TX_BUFFER_SIZE];

// 接收和发送描述符环
e1000_rx_desc rx_ring[RX_DESC_COUNT] __attribute__((aligned(16)));
e1000_tx_desc tx_ring[TX_DESC_COUNT] __attribute__((aligned(16)));

// MMIO基地址
volatile uint32_t *e1000_mmio;

// 读取MMIO寄存器
uint32_t e1000_read_reg(uint16_t reg)
{
    return e1000_mmio[reg / 4];
}

// 写入MMIO寄存器
void e1000_write_reg(uint16_t reg, uint32_t value)
{
    e1000_mmio[reg / 4] = value;
}

// 初始化接收描述符环
void e1000_init_rx_ring()
{
    for (int i = 0; i < RX_DESC_COUNT; i++)
    {
        rx_ring[i].buffer_addr = (uint64_t)&rx_buffer[i];
        rx_ring[i].length = RX_BUFFER_SIZE;
        rx_ring[i].status = 0;
    }

    e1000_write_reg(E1000_REG_RDBAL, (uint32_t)((uint64_t)rx_ring & 0xFFFFFFFF));
    e1000_write_reg(E1000_REG_RDBAH, (uint32_t)(((uint64_t)rx_ring >> 32) & 0xFFFFFFFF));
    e1000_write_reg(E1000_REG_RDLEN, RX_DESC_COUNT * sizeof(e1000_rx_desc));
    e1000_write_reg(E1000_REG_RDH, 0);
    e1000_write_reg(E1000_REG_RDT, RX_DESC_COUNT - 1);
}

// 初始化发送描述符环
void e1000_init_tx_ring()
{
    for (int i = 0; i < TX_DESC_COUNT; i++)
    {
        tx_ring[i].buffer_addr = (uint64_t)&tx_buffer[i];
        tx_ring[i].length = TX_BUFFER_SIZE;
        tx_ring[i].status = 0;
    }

    e1000_write_reg(E1000_REG_TDBAL, (uint32_t)((uint64_t)tx_ring & 0xFFFFFFFF));
    e1000_write_reg(E1000_REG_TDBAH, (uint32_t)(((uint64_t)tx_ring >> 32) & 0xFFFFFFFF));
    e1000_write_reg(E1000_REG_TDLEN, TX_DESC_COUNT * sizeof(e1000_tx_desc));
    e1000_write_reg(E1000_REG_TDH, 0);
    e1000_write_reg(E1000_REG_TDT, 0);
}
#define RCTL_EN       (1 << 1)   // Enable receiver
#define RCTL_SBP      (1 << 3)   // Allow bad packets (for debugging, usually 0)
#define RCTL_UPE      (1 << 4)   // Unicast promiscuous enable
#define RCTL_MPE      (1 << 5)   // Multicast promiscuous enable
#define RCTL_LPE      (1 << 24)  // Long Packet Enable
#define RCTL_BAM      (1 << 15)  // Broadcast Accept Mode
#define RCTL_SECRC    (1 << 26)  // Strip Ethernet CRC

// Buffer size definitions (BSIZE fields)
#define RCTL_BSIZE_2048  (0x0 << 16) // 2048 bytes per buffer
#define RCTL_BSIZE_1024  (0x1 << 16) // 1024 bytes per buffer
#define RCTL_BSIZE_512   (0x2 << 16) // 512 bytes per buffer
#define RCTL_BSIZE_256   (0x3 << 16) // 256 bytes per buffer

void e1000_enable_rx() {
    uint32_t rctl = 0;

    // Enable receiver
    rctl |= RCTL_EN;

    // Allow broadcast packets
    rctl |= RCTL_BAM;

    // Enable CRC stripping
    rctl |= RCTL_SECRC;

    // Set buffer size to 2048 bytes
    rctl |= RCTL_BSIZE_2048;

    // Allow long packets if needed (e.g., jumbo frames)
    rctl |= RCTL_LPE;

    // Write to the RCTL register (assuming REG_RCTL is the address of the RCTL register)
    e1000_write_reg(E1000_REG_RCTL, rctl);
}
// 启用接收功能
void e1000_enable_rx2()
{
    uint32_t rctl = e1000_read_reg(E1000_REG_RCTL);
    rctl |= 0x02; // 启用接收功能
    e1000_write_reg(E1000_REG_RCTL, rctl);
}

// 启用发送功能
void e1000_enable_tx()
{
    uint32_t tctl = e1000_read_reg(E1000_REG_TCTL);
    tctl |= 0x02; // 启用发送功能
    e1000_write_reg(E1000_REG_TCTL, tctl);
}
////////////////////////////////////////////////////

// e1000寄存器偏移量
#define E1000_REG_ICR 0x00C0
#define E1000_REG_IMS 0x00D0
#define E1000_REG_IMC 0x00D8

// 中断原因标志
#define E1000_ICR_TXDW (1 << 0) // 发送描述符写回
#define E1000_ICR_TXQE (1 << 1) // 发送队列空
#define E1000_ICR_LSC (1 << 2)  // 链路状态改变
#define E1000_ICR_RXO (1 << 6)  // 接收溢出
#define E1000_ICR_RXT0 (1 << 7) // 接收定时器中断

// 启用中断
void e1000_enable_interrupts()
{
    // 启用接收和发送中断
    e1000_write_reg(E1000_REG_IMS, E1000_ICR_TXDW | E1000_ICR_RXT0);
}

// 禁用中断
void e1000_disable_interrupts()
{
    e1000_write_reg(E1000_REG_IMC, 0xFFFFFFFF);
}

// 接收数据包的最大长度
#define MAX_PACKET_SIZE 2048

// 接收数据包的回调函数类型
typedef void (*packet_received_callback)(uint8_t *packet, size_t length);

// 全局变量：接收数据包的回调函数
packet_received_callback on_packet_received = NULL;

// 设置接收数据包的回调函数
void e1000_set_packet_received_callback(packet_received_callback callback)
{
    on_packet_received = callback;
}

// 中断处理函数
void e1000_interrupt_handler(registers_t *regs)
{
    // 读取中断原因
    uint32_t icr = e1000_read_reg(E1000_REG_ICR);

    kprintf("e1000 irq, icr:%d\n", icr);
    // 处理发送中断
    if (icr & E1000_ICR_TXDW)
    {
        // 发送描述符写回，表示数据包已发送完成
        // 可以在这里释放发送缓冲区或处理发送完成事件
    }

    // 处理接收中断
    if (icr & E1000_ICR_RXT0)
    {
        // 接收定时器中断，表示有数据包到达
        uint16_t rdt = e1000_read_reg(E1000_REG_RDT);
        uint16_t next_rdt = (rdt + 1) % RX_DESC_COUNT;

        // 检查接收描述符状态
        if (rx_ring[next_rdt].status & 0x01)
        { // DD (Descriptor Done) 位
            // 读取数据包长度
            size_t length = rx_ring[next_rdt].length;

            // 检查数据包长度是否有效
            if (length > 0 && length <= MAX_PACKET_SIZE)
            {
                // 读取数据包
                uint8_t packet[MAX_PACKET_SIZE];
                memcpy(packet, (void *)rx_ring[next_rdt].buffer_addr, length);

                // 调用回调函数处理数据包
                if (on_packet_received)
                {
                    on_packet_received(packet, length);
                }
            }

            // 重置描述符状态
            rx_ring[next_rdt].status = 0;

            // 更新RDT
            e1000_write_reg(E1000_REG_RDT, next_rdt);
        }
    }

    // 处理链路状态改变中断
    if (icr & E1000_ICR_LSC)
    {
        // 链路状态改变，可以重新协商链路或更新状态
    }

    // 处理接收溢出中断
    if (icr & E1000_ICR_RXO)
    {
        // 接收缓冲区溢出，需要调整接收缓冲区大小或处理溢出
    }

    // 清除中断标志
    e1000_write_reg(E1000_REG_ICR, icr);
}
////////////////////////////////////////////////////

// 接收数据包
int e1000_receive_packet(uint8_t *buffer, size_t *length)
{
    uint16_t rdt = e1000_read_reg(E1000_REG_RDT);
    uint16_t next_rdt = (rdt + 1) % RX_DESC_COUNT;

    if (!(rx_ring[next_rdt].status & 0x01))
    {
        // 没有新的数据包
        return -1;
    }

    // 读取数据包
    *length = rx_ring[next_rdt].length;
    memcpy(buffer, (void *)rx_ring[next_rdt].buffer_addr, *length);

    // 更新描述符状态
    rx_ring[next_rdt].status = 0;

    // 更新RDT
    e1000_write_reg(E1000_REG_RDT, next_rdt);

    return 0;
}

// 发送数据包
int e1000_send_packet(const uint8_t *buffer, size_t length)
{
    uint16_t tdt = e1000_read_reg(E1000_REG_TDT);
    uint16_t next_tdt = (tdt + 1) % TX_DESC_COUNT;

    if (tx_ring[tdt].status & 0x01)
    {
        // 发送描述符忙，无法发送
        return -1;
    }

    // 复制数据包到发送缓冲区
    memcpy((void *)tx_ring[tdt].buffer_addr, buffer, length);

    // 设置描述符
    tx_ring[tdt].length = length;
    tx_ring[tdt].cmd = 0x01 | 0x08; // 设置EOP（End of Packet）和RS（Report Status）
    tx_ring[tdt].status = 0;

    // 更新TDT
    e1000_write_reg(E1000_REG_TDT, next_tdt);

    return 0;
}
////////////////////////////////////////////////////
// 示例回调函数：处理接收到的数据包
void handle_packet(uint8_t *packet, size_t length)
{
    // 打印数据包内容（仅用于调试）
    kprintf("Received packet (length: %d):\n", length);
    for (size_t i = 0; i < length; i++)
    {
        kprintf("%02x ", packet[i]);
        if ((i + 1) % 16 == 0)
        {
            kprintf("\n");
        }
    }
    kprintf("\n");

    // 在这里可以进一步解析数据包（例如ARP、IP等）
}
////////////////////////////////////////////////////
// 初始化e1000网卡
void e1000_init(uint32_t mmio_base)
{
    e1000_mmio = (volatile uint32_t *)mmio_base;
    // 初始化接收描述符环
    e1000_init_rx_ring();

    // 初始化发送描述符环
    e1000_init_tx_ring();
    // 启用接收功能
    e1000_enable_rx();

    // 启用发送功能
    e1000_enable_tx();

    // 启用中断
    e1000_enable_interrupts();

    // 注册中断处理函数
    register_interrupt_handler(32 + 11, e1000_interrupt_handler); 
    // 网卡的MAC地址和IP地址
    uint8_t src_mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
    uint32_t src_ip = 0xC0A80101;    // 192.168.1.1
    uint32_t target_ip = 0xC0A80102; // 192.168.1.2

    // 发送ARP请求
    //e1000_send_arp_request(src_mac, htonl(src_ip), htonl(target_ip));
    e1000_set_packet_received_callback(handle_packet);
}
void example_driver_init(pci_device_t *dev) {
    kprintf("Initializing device at bus %d, slot %d, func %d\n", dev->bus, dev->slot, dev->func);
    // 读取BAR0寄存器，获取MMIO基地址
    uint32_t bar0 = pci_read_config(dev->bus, dev->slot, dev->func, 0x10);
    if ((bar0 & 0x01) == 0)
    {
        // BAR0是MMIO地址
        uint32_t mmio_base = bar0 & ~0xF;

        // 启用MMIO和Bus Master
        uint32_t command = pci_read_config(dev->bus, dev->slot, dev->func, 0x04);
        command |= 0x02 | 0x04; // 启用Bus Master和MMIO
        pci_write_config(dev->bus, dev->slot, dev->func, 0x04, command);

        e1000_init(mmio_base);
    }
}

// 声明并注册示例驱动
REGISTER_PCI_DRIVER(0x8086, 0x100E, example_driver_init)