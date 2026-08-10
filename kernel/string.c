/* ============================================================================
 * string.c — Pequenas funções de string/memória pro kernel
 * ----------------------------------------------------------------------------
 * Como um kernel não pode usar a libc do sistema (rode o `make` e veja...),
 * precisamos de nossas próprias versões das funções mais básicas. Só tem o
 * essencial pro sistema de arquivos funcionar: copiar memória, comparar e
 * copiar strings.
 * ============================================================================ */

#include "string.h"

/* Copia n bytes de src pra dest (sem checar sobreposição). */
void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;
    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }
    return dest;
}

/* Compara duas strings; retorna 0 se iguais, ou a diferença do primeiro
 * caractere que divergiu. */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

/* Copia até n caracteres de src pra dest (preenchendo o resto com '\0'). */
char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for ( ; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}