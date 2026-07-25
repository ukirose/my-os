/*
 * main.c
 *
 * カーネルの起動処理とメインループ
 */

#include "stdint.h"
#include "kernel.h"
#include "string.h"

/* BSS領域とスタックの境界アドレス(kernel.ldで定義) */
extern char _bss[], _bss_end[], _stack_top[];

/* shellアプリのELFバイナリ(bootelf.Sによって埋め込まれる) */
extern char _boot_elf_start[];

#define DELAY_LOOPS 30000000  /* プロセス間の実行状態を観察可能にするための遅延ループ回数 */

static void delay(void);

/* カーネルのエントリーポイント(OpenSBIからジャンプされる) */
__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__(
        /* 初期スタックポインタを設定し、カーネルのメイン処理へ移行 */
        "mv sp, %0\n"
        "j kernel_main\n"
        :
        : "r" (_stack_top)
    );
}

/* カーネルの初期化処理とマルチタスクの開始 */
void kernel_main(void) {
    memset(_bss, 0, (size_t) _bss_end - (size_t) _bss);

    trap_init();

    kprintf("\n\nHello %s!\n", "World");

    proc_init();

    create_user_process((const uint8_t *)_boot_elf_start);

    /* 最初のプロセスへコンテキストスイッチ */
    yield();

    PANIC("switched to idle process");
}

/* プロセス間の実行状態を観察可能にするための遅延ループ */
static void delay(void) {
    for (int i = 0; i < DELAY_LOOPS; i++) {
        __asm__ __volatile__("nop");   /* 何もしない命令 */
    }
}
