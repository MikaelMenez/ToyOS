#ifndef STDDEF_H
#define STDDEF_H

// Como estamos em 32 bits (-m32), o size_t é um unsigned int
typedef unsigned int size_t;

#define NULL ((void*)0)

#endif
