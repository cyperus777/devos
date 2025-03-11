#ifndef UTILS_H
#define UTILS_H

#include "types.h"
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void memory_copy(char *source, char *dest, int nbytes);
void int_to_ascii(int n, char str[]);
void hex_to_ascii(int n, char str[]);
#endif
