/*
 * vm.c
 *
 * ページテーブルと仮想メモリの管理
 */

#include "stdint.h"
#include "types.h"
#include "kernel.h"
#include "mmu.h"

/* ページテーブルに仮想アドレスと物理アドレスの対応エントリを登録する */
void map_page(pagetable_t pt1, uint32_t va, paddr_t pa, uint32_t flags)
{
    /* アドレス変換はページ単位で行われるため、ページ境界に揃っていないアドレスの登録を禁止する */
    if (!IS_ALIGNED(va, PAGE_SIZE)) PANIC("unaligned va %x", va);
    if (!IS_ALIGNED(pa, PAGE_SIZE)) PANIC("unaligned pa %x", pa);

    /* 1段目ページテーブル(pt1)から該当エントリを特定する */
    uint32_t vpn1 = VPN1(va);
    uint32_t *pte1 = &pt1[vpn1];

    /* 1段目エントリが有効化されていない場合、2段目ページテーブル用の物理ページを新設し、 PTE形式に変換してリンクする */
    if ((*pte1 & PTE_V) == 0) {
        paddr_t new_pt = alloc_pages(1);
        *pte1 = pa2pte(new_pt) | PTE_V;
    }

    /* 1段目エントリからフラグ情報を削ぎ落とし、 2段目ページテーブルの純粋な物理アドレスを復元する */
    pagetable_t pt0 = (pagetable_t)pte2pa(*pte1);

    /* 2段目ページテーブル(pt0)の該当エントリを特定する */
    uint32_t vpn0 = VPN0(va);
    uint32_t *pte0 = &pt0[vpn0];

    /* 呼び出し元によるフラグの付け忘れを防ぐため、マッピングの存在証明(PTE_V)は関数内で強制付与して登録する */
    *pte0 = pa2pte(pa) | flags | PTE_V;
}
