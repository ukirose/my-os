/*
 * riscv.h
 *
 * RISC-Vアーキテクチャ固有の仕様
 */

#pragma once

/*
 * satpレジスタ     (Supervisor Address Translation and Protection) 32bit
 * MMUに対してページテーブルの位置とアドレス変換モードを指示する
 * - M       : (MODE) 1=Sv32ページング有効, 0=Bare(ページング無効)
 * - ASID    : TLB内でプロセスを識別するアドレス空間ID
 * - PPN     : 1段目ページテーブルの物理ページ番号
 */

#define SATP_SV32 (1u << 31)    /* satpレジスタ: Sv32ページング有効化フラグ(MODE=1) */
