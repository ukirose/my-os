/*
 * proc.c
 *
 * プロセスの管理とコンテキストスイッチ
 */

#include "kernel.h"
#include "string.h"

#define PROCS_MAX     8            /* 同時に管理できる最大プロセス数 */

/* プロセス状態 */
#define PROC_UNUSED   0            /* 非アクティブ状態 */
#define PROC_RUNNABLE 1            /* 実行可能状態 */

/* プロセス切り替えによる実行再開に必要なレジスタのスナップショット */
struct context {
    uint32_t ra;                                                /* リターンアドレス */
    uint32_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;  /* 保存レジスタ */

    /* RISC-V Calling Conventionによりスタックポインタを常に16バイト境界に保つため */
    uint32_t padding[3];
};

/* プロセスの制御情報を保持するプロセス制御ブロック */
struct process {
    int pid;                            /* プロセス ID */
    int state;                          /* プロセス状態 */
    vaddr_t sp;                         /* カーネルモードスタックポインタ */
    uint8_t kstack[KSTACK_SIZE];         /* プロセス用カーネルスタック */
};

static struct process procs[PROCS_MAX]; /* 全プロセスの制御ブロックを保持するプロセステーブル */
static struct process *current_proc;    /* 現在実行中のプロセス */

#define idle_proc (&procs[0])           /* 常にプロセステーブルの先頭(PID 0)をアイドルプロセスとして扱う */

__attribute__((naked)) static void switch_context(uint32_t *prev_sp, uint32_t *next_sp);

/* アイドルプロセスの生成と初期化 */
void proc_init(void) {
    /* 最初に生成されるため、自動的にテーブル先頭の procs[0] (PID 0) に割り当てられる */
    create_process((uint32_t) NULL);
    current_proc = idle_proc;
}

/* 新規プロセスの生成および初期コンテキストの構築 */
struct process *create_process(uint32_t pc) {
    struct process *p = NULL;

    /* アイドルプロセスの場合は無駄な探索をせず、常に先頭スロットを使用する */
    if (pc == (uint32_t) NULL) {
        p = idle_proc;
    } else {
        /* 一般プロセスの場合は、 PID 1 以降から空きを探す */
        for (int pid = 1; pid < PROCS_MAX; pid++) {
            if (procs[pid].state == PROC_UNUSED) {
                p = &procs[pid];
                break;
            }
        }
    }
    if (!p) PANIC("no free process slots");

    /* カーネルスタックの末尾に初期コンテキスト（レジスタ退避領域）を配置する */
    uint32_t stack_top = (uint32_t) p->kstack + sizeof(p->kstack);
    struct context *ctx = (struct context *) (stack_top - sizeof(struct context));

    /* 予期せぬゴミデータを読まないよう、コンテキスト領域全体をゼロクリアする */
    memset(ctx, 0, sizeof(struct context));

    /* 初回のコンテキストスイッチ時に、ret命令でpcへジャンプするよう設定する */
    ctx->ra = pc;
    p->sp = (vaddr_t) ctx;

    p->pid = p - procs;     /* プロセステーブルのインデックスをそのままPIDとして割り当てる */
    p->state = PROC_RUNNABLE;

    return p;
}

/* 次に実行するプロセスを選択し、CPU実行権を譲渡する。 */
void yield(void) {
    /* 実行可能なプロセスがない場合にカーネルがクラッシュするのを防ぐため、安全網としてアイドルプロセスを選んでおく */
    struct process *next = idle_proc;

    /* すべてのプロセスを平等に扱うため、現在のプロセスの次からラウンドロビンで探索する */
    int pid = current_proc->pid;
    for (int i = 1; i < PROCS_MAX; i++) {
        pid++;
        /* 配列の末尾に達したら、アイドルプロセス(PID 0)を飛ばして探索をやり直す */
        if (pid >= PROCS_MAX) pid = 1;

        struct process *p = &procs[pid];
        if (p->state == PROC_RUNNABLE) {
            next = p;
            break;
        }
    }

    /* 次プロセスが自身と同じ場合はコンテキストスイッチをスキップ */
    if (next == current_proc) return;

    /* トラップ発生時に kernel_entry で安全なスタックを読み込めるよう、あらかじめ sscratch に次のカーネルスタックのトップを準備しておく */
    uint32_t next_stack_top = (uint32_t) next->kstack + sizeof(next->kstack);
    __asm__ __volatile__(
        "csrw sscratch, %0\n"
        :
        : "r" (next_stack_top)
    );

    /* 実行中のプロセスを更新し、CPUコンテキスト(レジスタ群)を入れ替える */
    struct process *prev = current_proc;
    current_proc = next;
    switch_context(&prev->sp, &next->sp);
}

/* 実行プロセス間におけるCPUコンテキストの切り替え */
__attribute__((naked)) static void switch_context(uint32_t *prev_sp, uint32_t *next_sp) {
    __asm__ __volatile__(
        /* 実行中プロセスのスタックに struct context 分の退避領域を確保する */
        "addi sp, sp, -4 * 16\n"

        /* 呼び出し先保存レジスタをスタックに退避 */
        "sw ra,  4 * 0(sp)\n"
        "sw s0,  4 * 1(sp)\n"
        "sw s1,  4 * 2(sp)\n"
        "sw s2,  4 * 3(sp)\n"
        "sw s3,  4 * 4(sp)\n"
        "sw s4,  4 * 5(sp)\n"
        "sw s5,  4 * 6(sp)\n"
        "sw s6,  4 * 7(sp)\n"
        "sw s7,  4 * 8(sp)\n"
        "sw s8,  4 * 9(sp)\n"
        "sw s9,  4 * 10(sp)\n"
        "sw s10, 4 * 11(sp)\n"
        "sw s11, 4 * 12(sp)\n"

        /* スタックポインタの切り替え */
        "sw sp, (a0)\n"             /* prev_sp に現在の sp を保存 */
        "lw sp, (a1)\n"             /* next_sp から次プロセスの sp を復元 */

        /* 保存した汎用レジスタを復元 */
        "lw ra,  4 * 0(sp)\n"
        "lw s0,  4 * 1(sp)\n"
        "lw s1,  4 * 2(sp)\n"
        "lw s2,  4 * 3(sp)\n"
        "lw s3,  4 * 4(sp)\n"
        "lw s4,  4 * 5(sp)\n"
        "lw s5,  4 * 6(sp)\n"
        "lw s6,  4 * 7(sp)\n"
        "lw s7,  4 * 8(sp)\n"
        "lw s8,  4 * 9(sp)\n"
        "lw s9,  4 * 10(sp)\n"
        "lw s10, 4 * 11(sp)\n"
        "lw s11, 4 * 12(sp)\n"

        /* コンテキストスイッチ前の状態へスタックポインタを戻す */
        "addi sp, sp, 4 * 16\n"

        /* 復元された ra へジャンプし、次プロセスの実行再開 */
        "ret\n"
    );
}
