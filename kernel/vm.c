/*
 * vm.c
 *
 * ページテーブルと仮想メモリの管理
 */

#include "stdint.h"
#include "types.h"
#include "kernel.h"
#include "mmu.h"

#define is_aligned(value, align) (((value) % (align)) == 0)

/* 仮想アドレスから物理アドレスへのマッピングを設定する */
void map_page(pagetable_t table1, uint32_t vaddr, paddr_t paddr, uint32_t flags) {
    if (!is_aligned(vaddr, PAGE_SIZE)) PANIC("unaligned vaddr %x", vaddr);
    if (!is_aligned(paddr, PAGE_SIZE)) PANIC("unaligned paddr %x", paddr);

    uint32_t vpn1_idx = vpn1(vaddr);

    if ((table1[vpn1_idx] & PTE_V) == 0) {
        /* 2段目が未割り当てなら新規ページを確保してリンクする */
        paddr_t pt_paddr = alloc_pages(1);
        table1[vpn1_idx] = pa2pte(pt_paddr) | PTE_V;
    }

    uint32_t vpn0_idx = vpn0(vaddr);
    pagetable_t table0 = (pagetable_t)pte2pa(table1[vpn1_idx]);

    /* 2段目PTEへの終端マッピング */
    table0[vpn0_idx] = pa2pte(paddr) | flags | PTE_V;
}
