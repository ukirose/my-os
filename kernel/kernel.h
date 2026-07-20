/*
 * kernel.h
 *
 * カーネルの各サブシステム間で共有される内部インターフェース
 */

#pragma once
#include "stdint.h"
#include "stddef.h"
#include "types.h"

extern char __kernel_base[]; /* リンカスクリプトで定義されるカーネル開始アドレス */

/* kalloc.c */
paddr_t alloc_pages(size_t n);

/* vm.c */
void map_page(pagetable_t table1, uint32_t vaddr, paddr_t paddr, uint32_t flags);

/* proc.c */
#define KSTACK_SIZE 8192

struct process;
void proc_init(void);
struct process *create_process(uint32_t pc);
void yield(void);

/* kprintf.c */
void kprintf(const char *fmt, ...);
void putchar(char c);

/*
 * 続行不能なエラー時に呼出元のファイル名・行番号を出力してカーネルをパニックさせる
 * - __FILE__, __LINE__ : マクロ展開時に、呼出元のファイル名と行番号に置換される
 * - ##__VA_ARGS__      : 可変長引数が空の場合、コンパイルエラーを防ぐため直前のカンマを消去する
 * - do {} while(0)     : 展開後のコードが単一の文として振る舞うことを保証し、if文等での意図せぬ構文崩壊を防ぐ安全機構
 */
#define PANIC(fmt, ...)                                                          \
    do {                                                                         \
        kprintf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);   \
        while (1) {}    /* 無限ループでCPUを停止 */                                  \
    } while (0)


/* trap.c */
void trap_init(void);
