/*
 * main.c
 *
 * カーネルの起動処理と初期化完了後のメインループを提供する
 */

/* リンカスクリプトで定義されたシンボル */
extern char __bss[], __bss_end[], __stack_top[];

#include "kernel.h"
#include "string.h"

/* OpenSBIが0x80200000へ無条件ジャンプするので、カーネル先頭にbootを強制配置する */
__attribute__((section(".text.boot")))
/* スタックが使えない状態で呼び出されるので、コンパイラがプロローグでspを操作するのを抑止する */
__attribute__((naked))
void
boot(void)
{
    __asm__ __volatile__(
        /* Cの呼び出し規約がスタックを要求するので、最初にスタックポインタを設定する */
        "mv sp, %[stack_top]\n"
        "j kernel_main\n"
        :
        : [stack_top] "r" (__stack_top)
    );
}

/* bootからスタックを受け取り、BSSクリア・メインループを開始するカーネルのエントリポイント */
void
kernel_main(void)
{
    /* C言語の仕様では未初期化の静的変数は0でなければならないので、起動直後に自力でBSSをゼロクリアする */
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    kprintf("\n\nHello %s!\n", "World");

    PANIC("booted!");
}
