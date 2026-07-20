/*
 * types.h
 *
 * カーネル全体で共通して使用する基本型と定数の定義
 */

#pragma once

#include "stdint.h"

typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;
typedef uint32_t *pagetable_t;

