/*
 * riscv.h
 *
 * RISC-VアーキテクチャのCPUレジスタや特権モードに関するハードウェア仕様
 */

#pragma once

/* RISC-Vアーキテクチャ固有のレジスタ仕様 */

/*
 * satp (Supervisor Address Translation and Protection) レジスタの構造 (32bit)
 * - M (MODE): 1=Sv32ページング有効, 0=Bare(ページング無効)
 * - ASID    : アドレス空間ID (今回は使用しないため0)
 * - PPN     : 1段目(ルート)ページテーブルの物理ページ番号
 */
#define SATP_SV32 (1u << 31) /* satpレジスタ: Sv32ページング有効化フラグ(MODE=1) */
