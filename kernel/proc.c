/*
 * proc.c
 *
 * プロセスの管理とコンテキストスイッチ
 */

#include "kernel.h"

#define PROCS_MAX     8            /* 同時に管理できる最大プロセス数 */

/* プロセス状態 */
#define PROC_UNUSED   0            /* 非アクティブ状態 */
#define PROC_RUNNABLE 1            /* 実行可能状態 */

/* プロセスのメタデータや実行コンテキストを管理するプロセス制御ブロック */
struct process {
    int pid;                        /* プロセス ID */
    int state;                      /* プロセス状態 */
    vaddr_t sp;                     /* カーネルモードスタックポインタ */
    uint8_t stack[8192];            /* プロセス用カーネルスタック */
};

static struct process procs[PROCS_MAX];
static struct process *current_proc;
static struct process *idle_proc;

__attribute__((naked)) static void switch_context(uint32_t *prev_sp, uint32_t *next_sp);

/* プロセス管理機構の初期化およびアイドルプロセスの設定 */
void proc_init(void) {
    idle_proc = create_process((uint32_t) NULL);
    idle_proc->pid = 0;

    current_proc = idle_proc;
}

/* 新規プロセスの生成および初期コンテキストの構築 */
struct process *create_process(uint32_t pc) {
    struct process *p = NULL;
    int i;
    for (i = 0; i < PROCS_MAX; i++) {
        if (procs[i].state == PROC_UNUSED) {
            p = &procs[i];
            break;
        }
    }

    if (!p)
        PANIC("no free process slots");

    /* コンテキストスイッチでの初回復帰に必要なレジスタ群をスタックに配置 */
    uint32_t *sp = (uint32_t *) &p->stack[sizeof(p->stack)];
    *--sp = 0;                      /* s11 */
    *--sp = 0;                      /* s10 */
    *--sp = 0;                      /* s9 */
    *--sp = 0;                      /* s8 */
    *--sp = 0;                      /* s7 */
    *--sp = 0;                      /* s6 */
    *--sp = 0;                      /* s5 */
    *--sp = 0;                      /* s4 */
    *--sp = 0;                      /* s3 */
    *--sp = 0;                      /* s2 */
    *--sp = 0;                      /* s1 */
    *--sp = 0;                      /* s0 */
    *--sp = (uint32_t) pc;          /* ra */

    p->pid = i + 1;
    p->state = PROC_RUNNABLE;
    p->sp = (uint32_t) sp;

    return p;
}

/* スケジューラによる他プロセスへのCPU実行権の譲渡 */
void yield(void) {
    struct process *next = idle_proc;
    for (int i = 0; i < PROCS_MAX; i++) {
        struct process *p = &procs[(current_proc->pid + i) % PROCS_MAX];
        if (p->state == PROC_RUNNABLE && p->pid > 0) {
            next = p;
            break;
        }
    }

    /* 現在実行中のプロセス以外に実行可能なプロセスがないため、戻って処理を続行する */
    if (next == current_proc)
        return;

    /* トラップハンドラが例外処理で使用する次プロセスのカーネルスタック領域の終端を sscratch に設定 */
    __asm__ __volatile__(
        "csrw sscratch, %[sscratch]\n"
        :
        : [sscratch] "r" ((uint32_t) &next->stack[sizeof(next->stack)])
    );

    struct process *prev = current_proc;
    current_proc = next;

    switch_context(&prev->sp, &next->sp);
}

/* 実行プロセス間におけるCPUコンテキストの切り替え */
__attribute__((naked)) static void switch_context(uint32_t *prev_sp, uint32_t *next_sp) {
    __asm__ __volatile__(
        /* 実行中プロセスのスタックへレジスタを保存 */
        "addi sp, sp, -13 * 4\n"
        "sw ra,  0  * 4(sp)\n"
        "sw s0,  1  * 4(sp)\n"
        "sw s1,  2  * 4(sp)\n"
        "sw s2,  3  * 4(sp)\n"
        "sw s3,  4  * 4(sp)\n"
        "sw s4,  5  * 4(sp)\n"
        "sw s5,  6  * 4(sp)\n"
        "sw s6,  7  * 4(sp)\n"
        "sw s7,  8  * 4(sp)\n"
        "sw s8,  9  * 4(sp)\n"
        "sw s9,  10 * 4(sp)\n"
        "sw s10, 11 * 4(sp)\n"
        "sw s11, 12 * 4(sp)\n"

        /* スタックポインタの切り替え */
        "sw sp, (a0)\n"
        "lw sp, (a1)\n"

        /* 次のプロセスのスタックからレジスタを復元 */
        "lw ra,  0  * 4(sp)\n"
        "lw s0,  1  * 4(sp)\n"
        "lw s1,  2  * 4(sp)\n"
        "lw s2,  3  * 4(sp)\n"
        "lw s3,  4  * 4(sp)\n"
        "lw s4,  5  * 4(sp)\n"
        "lw s5,  6  * 4(sp)\n"
        "lw s6,  7  * 4(sp)\n"
        "lw s7,  8  * 4(sp)\n"
        "lw s8,  9  * 4(sp)\n"
        "lw s9,  10 * 4(sp)\n"
        "lw s10, 11 * 4(sp)\n"
        "lw s11, 12 * 4(sp)\n"
        "addi sp, sp, 13 * 4\n"
        "ret\n"
    );
}
