/*
 * mmu.h
 *
 * RISC-V のメモリ管理ユニット (MMU) に関するハードウェア定数とフラグ
 */

#pragma once

#include "types.h"

/*
 * Sv32 仮想アドレス: VPN[1] 10bit / VPN[0] 10bit / ページ内オフセット 12bit
 *
 * Sv32 物理アドレス: 34bit 幅を持ち、PPN が上位 22bit を占める
 *
 * Sv32 ページテーブル・エントリ(PTE)の構造 : 32ビット幅
 *  31                     20 19          10 9   8 7 6 5 4 3 2 1 0
 * +-------------------------+--------------+-----+-+-+-+-+-+-+-+-+
 * |         PPN[1]          |    PPN[0]    | RSW |D|A|G|U|X|W|R|V|
 * |        (12 bit)         |   (10 bit)   | (2) | | | | | | | | |
 * +-------------------------+--------------+-----+-+-+-+-+-+-+-+-+
 * 1つのページ(4KB)には、4バイトのPTEが 1024個(10ビット幅) 収まる。
 */

/* RISC-V MMU (Sv32) ページサイズとメモリレイアウト */
#define PAGE_SIZE 4096                  /* メモリ管理の最小単位 */
#define PHYSTOP ((paddr_t)0x88000000)   /* 物理メモリ領域の上限 */

/*
 * 仮想アドレス(VA)からページテーブル・インデックス(VPN)を抽出する
 */
static inline uint32_t vpn1(uint32_t va) {
    return (va >> 22) & 0x3ff;
}

static inline uint32_t vpn0(uint32_t va) {
    return (va >> 12) & 0x3ff;
}

/*
 * 物理アドレス(PA)とページテーブル・エントリ(PTE)の相互変換
 */
static inline uint32_t pa2pte(paddr_t pa) {
    uint32_t ppn = pa / PAGE_SIZE;
    uint32_t pte = ppn << 10;
    return pte;
}

static inline paddr_t pte2pa(uint32_t pte_val) {
    uint32_t ppn = pte_val >> 10;
    paddr_t pa = ppn * PAGE_SIZE;
    return pa;
}

/*
 * ページテーブル・エントリ (PTE) フラグ
 *  R, W, X が全て0の場合         : このPTEは「次のレベルのページテーブル」を指す（非終端）
 *  R, W, X のいずれかが1の場合    : このPTEは「実際の物理メモリ」を指す（終端）
 * ※ 1段目のPTEで終端となった場合は、4MBのメガページとしてマッピングされる
 */
#define PTE_V (1 << 0)  /* 有効化フラグ */
#define PTE_R (1 << 1)  /* 読み取り可能 */
#define PTE_W (1 << 2)  /* 書き込み可能 */
#define PTE_X (1 << 3)  /* 実行可能 */
#define PTE_U (1 << 4)  /* ユーザーモードでアクセス可能 */
