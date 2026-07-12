/*
 * kernel.h
 *
 * カーネル全体で共有する型定義と関数プロトタイプを提供する
 */

#pragma once

/*
 * 続行不能なエラー時に呼出元のファイル名・行番号とメッセージを出力して停止する。
 * do{}while(0)で複文を1文にまとめ、呼出側の ; と合わせて通常の関数呼出しと同じ構文にする。
 * __FILE__, __LINE__ : マクロ展開時に呼出元のファイル、行に置き換わる。そのため関数ではなくマクロで定義する。
 * ##__VA_ARGS__      : 可変長引数が空のとき、直前の , を消す。
 */
#define PANIC(fmt, ...)                                                          \
    do {                                                                         \
        kprintf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);   \
        while (1) {}    /* 無限ループでCPUを停止 */                                  \
    } while (0)

/* 標準ライブラリに依存しないため、可変長引数はコンパイラが提供するbuiltinを利用する  */
#define va_list  __builtin_va_list
#define va_start __builtin_va_start
#define va_end   __builtin_va_end
#define va_arg   __builtin_va_arg

/*
 * リンカスクリプトが出力する BSS 領域の先頭・末尾とスタック先頭のアドレス。
 * ポインタではなく配列として宣言することで、シンボル名がそのままアドレスを指す。
 */
extern char __bss[], __bss_end[], __stack_top[];

void kprintf(const char *fmt, ...);
void putchar(char c);
