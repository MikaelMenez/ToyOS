#ifndef STRING_H
#define STRING_H

#include "stddef.h"
#include "stdint.h"

void *memcpy(void *dest, const void *src, size_t n);
int strcmp(const char *s1, const char *s2);
char *strncpy(char *dest, const char *src, size_t n);

#endif
