/*
 * main.c
 *
 * カーネルの起動処理と初期化完了後のメインループ
 */

#include "kernel.h"
#include "string.h"

/* リンカスクリプトが出力する BSS 領域の先頭・末尾とスタック先頭のアドレス */
extern char _bss[], _bss_end[], _stack_top[];


/* OpenSBIが0x80200000へ無条件ジャンプするので、カーネル先頭にbootを強制配置する */
__attribute__((section(".text.boot")))
__attribute__((naked))  /* スタックが使えない状態で呼び出されるので、コンパイラがプロローグでspを操作するのを抑止する */
void boot(void) {
    __asm__ __volatile__(
        /* Cの呼び出し規約がスタックを要求するので、最初にスタックポインタを設定する */
        "mv sp, %[stack_top]\n"
        "j kernel_main\n"
        :
        : [stack_top] "r" (_stack_top)
    );
}

static void delay(void) {
    for (int i = 0; i < 30000000; i++)
        __asm__ __volatile__("nop"); /* 何もしない命令 */
}

static void proc_a_entry(void) {
    kprintf("starting process A\n");
    while (1) {
        putchar('A');
        yield();
        delay();
    }
}

static void proc_b_entry(void) {
    kprintf("starting process B\n");
    while (1) {
        putchar('B');
        yield();
        delay();
    }
}

/* bootからスタックを受け取り、BSSクリア・メインループを開始するカーネルのエントリポイント */
void kernel_main(void) {
    /* C言語の仕様では未初期化の静的変数は0でなければならないので、起動直後に自力でBSSをゼロクリアする */
    memset(_bss, 0, (size_t) _bss_end - (size_t) _bss);

    trap_init();

    kprintf("\n\nHello %s!\n", "World");

    proc_init();
    create_process((uint32_t) proc_a_entry);
    create_process((uint32_t) proc_b_entry);

    yield();

    PANIC("switched to idle process");
}
