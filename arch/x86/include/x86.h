#ifndef __X86_H_
#define __X86_H_

#include "types.h"

static inline uint8_t inb(uint16_t port)
{
  uint8_t data;
  __asm__ __volatile__("inb %1,%0"
                   : "=a"(data)
                   : "d"(port));
  return data;
}

static inline void outb(uint16_t port, uint8_t data)
{
  __asm__ __volatile__("outb %0,%1" : : "a"(data), "d"(port));
}

static inline void outl(uint16_t port, uint32_t value)
{
  __asm__ __volatile__("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port)
{
  uint32_t result;
  __asm__ __volatile__("inl %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

static inline uint8_t in(uint16_t port)
{
  uint8_t result;
  __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
  return result;
}

static inline void out(uint16_t port, uint8_t data)
{
  __asm__ __volatile__("out %%al, %%dx" : : "a"(data), "d"(port));
}

static inline void insl(int port, void *addr, int cnt)
{
  __asm__ __volatile__("cld; rep insl" : "=D"(addr), "=c"(cnt) : "d"(port), "0"(addr), "1"(cnt) : "memory", "cc");
}

#endif
