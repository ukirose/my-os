/*
 * kalloc.c
 *
 * Bumpアロケータによる物理メモリの割り当て
 */

#include "types.h"
#include "kernel.h"
#include "string.h"

#define PAGE_SIZE 4096                  /* メモリ管理の最小単位 */
#define PHYSTOP ((paddr_t)0x88000000)   /* 物理メモリ領域の上限 */

extern char _end[];

/* 次の割り当てで使われる、未使用メモリの先頭アドレス */
static paddr_t bump_alloc_ptr = (paddr_t)_end;

/* 要求された n ページ分の連続した物理メモリを割り当て、ゼロクリアして返す */
paddr_t alloc_pages(size_t n) {
    paddr_t paddr = bump_alloc_ptr;

    bump_alloc_ptr += n * PAGE_SIZE;
    if (bump_alloc_ptr > PHYSTOP) PANIC("out of memory: alloc_ptr=%x, PHYSTOP=%x", bump_alloc_ptr, PHYSTOP);

    memset((void *)paddr, 0, n * PAGE_SIZE);
    return paddr;
}
