/*
 * Como não estamos usando a biblioteca padrão do C (libC), 
 * precisamos definir nossos próprios tamanhos de tipos de dados.
 * Isso garante que um 'uint32_t' tenha exatamente 32 bits,
 * não importa onde ou como o código seja compilado.
 */

#ifndef _STDINT_H
#define _STDINT_H

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

#endif
