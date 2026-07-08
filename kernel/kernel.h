/*
 * kernel.h
 *
 * カーネル全体で共有する型定義と関数プロトタイプを提供する
 */

#pragma once

/* ベアメタル環境にはCRTが存在しないので、リンカスクリプトが出力したメモリ境界シンボルを直接参照する */
extern char __bss[], __bss_end[], __stack_top[];

/* SBI ecallの戻り値 */
struct sbiret {
    long error;
    long value;
};

struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid);
void kprintf(const char *fmt, ...);
void putchar(char c);
