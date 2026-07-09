/*
 * kernel.h
 *
 * カーネル全体で共有する型定義と関数プロトタイプを提供する
 */

#pragma once

/* 可変長引数はコンパイラが提供するbuiltinを利用する。標準ライブラリに依存しないため */
#define va_list  __builtin_va_list
#define va_start __builtin_va_start
#define va_end   __builtin_va_end
#define va_arg   __builtin_va_arg

/*
 * リンカスクリプトが出力するメモリ境界シンボル。
 * ベアメタル環境にはCRTが存在せずメモリレイアウトを自力で管理する必要があるため直接参照する。
 * 配列として宣言することで識別子がアドレスそのものを指す。ポインタ宣言だと中身を読みに行ってしまう
 */
extern char __bss[], __bss_end[], __stack_top[];

void kprintf(const char *fmt, ...);
void putchar(char c);
