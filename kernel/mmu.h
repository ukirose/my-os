/*
 * mmu.h
 *
 * RISC-V のメモリ管理ユニットに関するハードウェア定数とフラグ
 */

#pragma once

#include "types.h"

/*
 * Sv32 仮想アドレス: VPN[1] 10bit / VPN[0] 10bit / ページ内オフセット 12bit
 *
 * Sv32 ページテーブルエントリ (PTE) : 32bit
 *  31                    20 19                10 9   8 7 6 5 4 3 2 1 0
 * +------------------------+--------------------+-----+-+-+-+-+-+-+-+-+
 * |         PPN[1]         |       PPN[0]       | RSW |D|A|G|U|X|W|R|V|
 * +------------------------+--------------------+-----+-+-+-+-+-+-+-+-+
 *            12                      10            2   1 1 1 1 1 1 1 1
 *
 * Sv32 物理アドレス: 34bit 幅を持ち、PPN が上位 22bit を占める
 */

/* ページサイズとレイアウト */
#define PAGE_SIZE 4096                  /* メモリ管理の最小単位(4KB) */
#define PHYSTOP ((paddr_t)0x88000000)   /* QEMU virtマシンのRAM終端アドレス(0x80000000 + 128MB) */

/* PTEフラグ */
#define PTE_D (1 << 7)  /* 書き込みが発生したことを示す */
#define PTE_A (1 << 6)  /* アクセスが発生したことを示す */

#define PTE_G (1 << 5)  /* 全プロセス共通のグローバルマッピング */
#define PTE_U (1 << 4)  /* ユーザーモードでアクセス可能 */

#define PTE_X (1 << 3)  /* 実行可能 */
#define PTE_W (1 << 2)  /* 書き込み可能 */
#define PTE_R (1 << 1)  /* 読み取り可能 */

#define PTE_V (1 << 0)  /* 有効化フラグ */

/* 仮想アドレスからページテーブルインデックス(VPN)を抽出するマクロ */
#define VPN1(va) (((va) >> 22) & 0x3ff)
#define VPN0(va) (((va) >> 12) & 0x3ff)

/* 物理アドレスとページテーブルエントリの相互変換 */
static inline uint32_t pa2pte(paddr_t pa) {
    uint32_t ppn = pa >> 12;
    uint32_t pte = ppn << 10;
    return pte;
}

static inline paddr_t pte2pa(uint32_t pte) {
    uint32_t ppn = pte >> 10;
    paddr_t pa = ppn << 12;
    return pa;
}
