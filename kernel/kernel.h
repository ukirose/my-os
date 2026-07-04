/*
 * kernel.h
 *
 * カーネル全体で共有する型定義と関数プロトタイプを提供する
 */

#pragma once

/* ベアメタル環境にはCRTが存在しないので、リンカスクリプトが出力したメモリ境界シンボルを直接参照する */
extern char __bss[], __bss_end[], __stack_top[];

void kprintf(const char *fmt, ...);
void putchar(char c);
