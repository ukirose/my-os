/*
 * kernel.h
 *
 * カーネル全体で共有する型定義と関数プロトタイプを提供する
 */

#pragma once

/*
 * リンカスクリプトが出力するメモリ境界シンボル。
 * ベアメタル環境にはCRTが存在せずメモリレイアウトを自力で管理する必要があるため直接参照する。
 * 配列として宣言することで識別子がアドレスそのものを指す。ポインタ宣言だと中身を読みに行ってしまう
 */
extern char __bss[], __bss_end[], __stack_top[];

void kprintf(const char *fmt, ...);
void putchar(char c);
