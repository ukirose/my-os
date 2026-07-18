/*
 * types.h
 *
 * カーネル全体で共通して使用する基本型と定数の定義
 */

#pragma once

typedef unsigned char  uint8_t;
typedef unsigned int   uint32_t;
typedef uint32_t       size_t;
typedef uint32_t       paddr_t;
typedef uint32_t       vaddr_t;

#define NULL ((void *)0)
