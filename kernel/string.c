/*
 * string.c
 *
 * カーネル内で使うメモリ操作・文字列操作関数を提供する
 */

#include "types.h"

/* 指定されたメモリ領域を1バイトずつ指定された値で塗りつぶす */
void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--)
        *p++ = (unsigned char)c;
    return s;
}
