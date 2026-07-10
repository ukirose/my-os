/*
 * string.c
 *
 * カーネル内で使うメモリ操作・文字列操作関数を提供する
 */

#include "types.h"

/* 指定されたメモリ領域を1バイトずつ指定された値で塗りつぶす */
void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;             /* あらゆる型を受け取るvoid*を1バイト単位で操作するためuint8_t*にキャスト */
    while (n--)
        *p++ = (uint8_t)c;
    return s;
}

/* srcからnバイト分をdstにコピーする */
void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *) dst;              /* コピー元・先の型を問わないvoid*を1バイト単位で扱う */
    const uint8_t *s = (const uint8_t *) src;
    while (n--)
        *d++ = *s++;
    return dst;
}

/* 文字列srcをdstにコピーする */
char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++) != '\0');
    return dst;
}

/* s1とs2を比較し、等しければ0、s1>s2なら正の値、s1<s2なら負の値を返す */
int
strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2) {
        if (*s1 != *s2)
            break;
        s1++;
        s2++;
    }

    return *(uint8_t *)s1 - *(uint8_t *)s2; /* charが符号付きの処理系でも正しく比較するため符号無しにキャスト */
}
