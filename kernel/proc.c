/*
 * proc.c
 *
 * プロセスの管理とコンテキストスイッチ
 */

#include "stdint.h"
#include "types.h"
#include "kernel.h"
#include "mmu.h"
#include "riscv.h"
#include "stddef.h"
#include "string.h"
#include "elf.h"

#define PROCS_MAX     8       /* 同時に管理できる最大プロセス数 */

/* プロセス状態 */
#define PROC_UNUSED   0       /* 非アクティブ状態 */
#define PROC_RUNNABLE 1       /* 実行可能状態 */

/* プロセス切り替えによる実行再開に必要なレジスタのスナップショット */
struct context {
    uint32_t ra;                                                /* リターンアドレス */
    uint32_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;  /* 保存レジスタ */

    /* RISC-V Calling Conventionによりスタックポインタを常に16バイト境界に保つため */
    uint32_t padding[3];
};

/* プロセス制御ブロック (PCB) */
struct process {
    int pid;                            /* プロセス ID */
    int state;                          /* プロセス状態 */
    vaddr_t sp;                         /* カーネルモードスタックポインタ */
    uint32_t user_pc;                   /* ユーザーモードにおける実行開始アドレス */
    pagetable_t page_table;             /* プロセスごとの1段目ページテーブル */
    uint8_t kstack[KSTACK_SIZE];        /* プロセス用カーネルスタック */
};

static struct process procs[PROCS_MAX]; /* 全プロセスの制御ブロックを保持するプロセステーブル */
static struct process *current_proc;    /* 現在実行中のプロセス */

#define idle_proc (&procs[0])           /* 常にプロセステーブルの先頭(PID 0)をアイドルプロセスとして扱う */

__attribute__((naked)) static void switch_context(uint32_t *prev_sp, uint32_t *next_sp);
__attribute__((naked)) static void user_entry(void);

/* アイドルプロセスの生成と初期化 */
void proc_init(void) {
    /* 最初に生成されるため、自動的にテーブル先頭の procs[0] (PID 0) に割り当てられる */
    create_process((uint32_t) NULL);
    current_proc = idle_proc;
}

/* 特権モードで動作する新規カーネルプロセスを生成する */
struct process *create_process(uint32_t pc) {
    struct process *new_proc = NULL;

    /* アイドルプロセスの場合は無駄な探索をせず、常に先頭スロットを使用する */
    if (pc == (uint32_t) NULL) {
        new_proc = idle_proc;
    } else {
        /* 一般プロセスの場合は、 PID 1 以降から空きを探す */
        for (int pid = 1; pid < PROCS_MAX; pid++) {
            if (procs[pid].state == PROC_UNUSED) {
                new_proc = &procs[pid];
                break;
            }
        }
    }
    if (!new_proc) PANIC("no free process slots");

    /* カーネルスタックの末尾に初期レジスタの退避領域を確保する */
    uint32_t stack_top = (uint32_t) new_proc->kstack + sizeof(new_proc->kstack);
    struct context *ctx = (struct context *) (stack_top - sizeof(struct context));

    /* 予期せぬゴミデータを読まないよう、コンテキスト領域全体をゼロクリアする */
    memset(ctx, 0, sizeof(struct context));

    /* 初回のコンテキストスイッチ時に、ret命令でプログラムカウンタへジャンプするよう設定する */
    ctx->ra = pc;
    new_proc->sp = (vaddr_t) ctx;

    new_proc->pid = new_proc - procs;     /* プロセステーブルのインデックスをそのままPIDとして割り当てる */
    new_proc->state = PROC_RUNNABLE;

    /* 1段目のページテーブルを物理メモリから割り当てる */
    new_proc->page_table = (pagetable_t)alloc_pages(1);

    /* ページング有効化後もカーネルの命令を読み込めるよう、カーネル領域を物理アドレスと同一の仮想アドレスで登録する */
    for (paddr_t paddr = (paddr_t)__kernel_base; paddr < PHYSTOP; paddr += PAGE_SIZE) {
        map_page(new_proc->page_table, paddr, paddr, PTE_R | PTE_W | PTE_X);
    }

    return new_proc;
}

/* ユーザーモードで動作する新規プロセスを生成する */
struct process *create_user_process(const uint8_t *elf_data) {
    struct process *user_proc = create_process((uint32_t) user_entry);
    user_proc->user_pc = load_elf(user_proc->page_table, elf_data);
    return user_proc;
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

        struct process *candidate = &procs[pid];
        if (candidate->state == PROC_RUNNABLE) {
            next = candidate;
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

    /* 実行中のプロセスを更新し、CPUコンテキストを入れ替える */
    struct process *prev = current_proc;
    current_proc = next;

    /* 次プロセスの仮想メモリ空間へ切り替える */
    uint32_t next_ppn = (uint32_t)next->page_table / PAGE_SIZE;
    uint32_t next_satp = SATP_SV32 | next_ppn;
    __asm__ __volatile__(
        "sfence.vma\n"  /* メモリアクセスの順序を制御し、先行するメモリ操作の完了を保証する */
        "csrw satp, %0\n"
        "sfence.vma\n"  /* 誤参照を防ぐため、古い変換キャッシュ(TLB)を全消去する */
        :
        : "r" (next_satp)
        : "memory"
    );

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

/* カーネルモードからユーザーモードへの特権レベルの切り替え */
__attribute__((naked)) static void user_entry(void) {
    __asm__ __volatile__(
        /* sepc にユーザープロセスの実行開始アドレスを設定 */
        "csrw sepc, %[sepc]\n"
        /* 特権レベルの遷移先をユーザーモードに指定し、割り込みとユーザーメモリアクセスを許可 */
        "csrw sstatus, %[sstatus]\n"

        /* カーネルの情報漏洩を防ぐため全汎用レジスタをゼロクリア */
        "mv x1, zero\n"
        "mv x2, zero\n"
        "mv x3, zero\n"
        "mv x4, zero\n"
        "mv x5, zero\n"
        "mv x6, zero\n"
        "mv x7, zero\n"
        "mv x8, zero\n"
        "mv x9, zero\n"
        "mv x10, zero\n"
        "mv x11, zero\n"
        "mv x12, zero\n"
        "mv x13, zero\n"
        "mv x14, zero\n"
        "mv x15, zero\n"
        "mv x16, zero\n"
        "mv x17, zero\n"
        "mv x18, zero\n"
        "mv x19, zero\n"
        "mv x20, zero\n"
        "mv x21, zero\n"
        "mv x22, zero\n"
        "mv x23, zero\n"
        "mv x24, zero\n"
        "mv x25, zero\n"
        "mv x26, zero\n"
        "mv x27, zero\n"
        "mv x28, zero\n"
        "mv x29, zero\n"
        "mv x30, zero\n"
        "mv x31, zero\n"

        /* ユーザーモードへ遷移し、プロセスの実行開始 */
        "sret\n"
        :
        : [sepc] "r" (current_proc->user_pc),
          [sstatus] "r" (SSTATUS_SPP_U | SSTATUS_SPIE | SSTATUS_SUM)
    );
}

