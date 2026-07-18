/*
 * kernel.h
 *
 * カーネルの各サブシステム間で共有される内部インターフェース
 */

#pragma once
#include "types.h"

/* メモリ管理 */
paddr_t alloc_pages(size_t n);



/* コンソール・ユーティリティ */
void kprintf(const char *fmt, ...);
void putchar(char c);

/*
 * 続行不能なエラー時に呼出元のファイル名・行番号とメッセージを出力して停止する
 * do{}while(0)で文を1文にまとめ、呼出側の ; と合わせて通常の関数呼出しと同じ構文にする
 * __FILE__, __LINE__ がマクロ展開時に呼出元のファイル、行に置き換わるため関数ではなくマクロで定義する
 * ##__VA_ARGS__      : 可変長引数が空のとき、直前の , を消す
 */
#define PANIC(fmt, ...)                                                          \
    do {                                                                         \
        kprintf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);   \
        while (1) {}    /* 無限ループでCPUを停止 */                                  \
    } while (0)


/* トラップ・例外処理 */
void trap_init(void);
