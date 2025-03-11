#ifndef __MM_H_
#define __MM_H_
#include <types.h>
// 内存映射表项结构
#pragma pack(push, 1)
typedef struct
{
  uint32_t size;
  uint64_t base_addr;
  uint64_t length;
  uint32_t type;
} memory_map_t;
#pragma pack(pop)

void mm_init(memory_map_t *mmap);

#endif