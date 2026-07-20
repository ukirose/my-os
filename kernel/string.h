/*
 * string.h
 *
 * 連続したデータ領域を操作するための公開インターフェース
 */

#pragma once
#include "stddef.h"

#include "types.h"

void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
