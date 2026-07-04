/*
 * string.h
 *
 * カーネルに依存せず単体で使える最小限のメモリ操作を提供する
 */

#pragma once
#include "types.h"
void *memset(void *s, int c, size_t n);
