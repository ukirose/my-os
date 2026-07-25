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

/*
 * sstatusレジスタ : S-modeが管轄する特権レベルや割り込み許可状態を制御する
 * - SPIE (bit 5) : 遷移先での割り込み許可
 * - SPP  (bit 8) : 遷移先の特権レベル (0=U-mode, 1=S-mode)
 * - SUM  (bit 18): カーネルからのユーザーメモリアクセス許可
 */
#define SSTATUS_SPIE  (1 << 5)
#define SSTATUS_SPP_U (0 << 8)   /* 遷移先をユーザーモードに指定 */
#define SSTATUS_SPP_S (1 << 8)   /* 遷移先をカーネルモードに指定 */
#define SSTATUS_SUM   (1 << 18)
