/*
 * string.h
 *
 * カーネルに依存せず単体で使える最小限のメモリ操作・文字列操作を提供する
 */

#pragma once

#include "types.h"

void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
