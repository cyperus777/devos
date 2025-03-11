#include "elf.h"
#include "x86.h"

#define SECTSIZE 512            // 每个扇区的大小为512字节
#define BASEOFFSET 4 * SECTSIZE // ELF文件的起始偏移
// 读取指定偏移处的指定数量的段到物理地址pa
void readseg(uint8_t *pa, uint32_t count, uint32_t offset);

void bootmain(void)
{
  struct elfhdr *elf;       // ELF文件头指针
  struct proghdr *ph, *eph; // 程序头表的起始和结束指针
  void (*entry)(void);      // 入口点函数指针
  uint8_t *pa;

  elf = (struct elfhdr *)0x10000; // 将ELF头放在0x10000处

  // 读取ELF头的前4096字节
  readseg((uint8_t *)elf, 4096, 0);

  // 检查ELF magic是否正确，判断是否是一个有效的ELF文件
  if (elf->magic != ELF_MAGIC)
    return;
  // 读取程序头表
  ph = (struct proghdr *)((uint8_t *)elf + elf->phoff);
  eph = ph + elf->phnum;
  for (; ph < eph; ph++)
  {
    pa = (uint8_t *)ph->paddr;                       // 加载段的物理地址
    readseg(pa, ph->filesz, ph->off);                // 读取段的文件部分
    for (int i = 0; i < ph->memsz - ph->filesz; i++) // 将未初始化的部分置零
    {
      *((char *)ph->paddr + ph->filesz + i) = 0;
    }
  }
  // 跳转到ELF文件的入口点，开始执行加载的程序
  entry = (void (*)(void))(elf->entry);
  entry();
}

// 等待磁盘准备好
void waitdisk(void)
{
  while ((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// 从磁盘的指定扇区读取数据到dst
void readsect(void *dst, uint32_t offset)
{
  waitdisk();                         // 等待磁盘就绪
  outb(0x1F2, 1);                     // 设置读取的扇区数
  outb(0x1F3, offset);                // 设置扇区号低8位
  outb(0x1F4, offset >> 8);           // 设置扇区号中间8位
  outb(0x1F5, offset >> 16);          // 设置扇区号高8位
  outb(0x1F6, (offset >> 24) | 0xE0); // 设置驱动器号
  outb(0x1F7, 0x20);                  // 发送读命令

  waitdisk();                     // 再次等待磁盘准备好
  insl(0x1F0, dst, SECTSIZE / 4); // 从磁盘读取数据到dst
}

// 从磁盘读取指定数量的字节到内存
void readseg(uint8_t *pa, uint32_t count, uint32_t offset)
{
  uint8_t *epa;
  epa = pa + count; // 计算结束地址

  offset += BASEOFFSET;             // 计算实际的偏移
  pa -= offset % SECTSIZE;          // 对齐到扇区边界
  offset = (offset / SECTSIZE) + 1; // 计算起始扇区号

  for (; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset); // 逐个扇区读取数据
}
