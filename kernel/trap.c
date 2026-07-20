/*
 * trap.c
 *
 * 例外および割り込みの捕捉とハンドリング
 */

#include "types.h"
#include "kernel.h"

static uint32_t r_scause(void);
static uint32_t r_stval(void);
static uint32_t r_sepc(void);
static void w_stvec(uint32_t v);

/* 例外発生時の汎用レジスタのスナップショット */
struct trapframe {
    uint32_t ra, gp, tp;                                        /* リターンアドレス, グローバルポインタ, スレッドポインタ */
    uint32_t t0, t1, t2, t3, t4, t5, t6;                        /* 一時レジスタ */
    uint32_t a0, a1;                                            /* 引数・戻り値レジスタ */
    uint32_t a2, a3, a4, a5, a6, a7;                            /* 引数レジスタ */
    uint32_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;  /* 保存レジスタ */
    uint32_t sp;                                                /* スタックポインタ */
} __attribute__((packed));         /* kernel_entry と構造体レイアウトを一致させるためパディングしない */

void kernel_entry(void);
void handle_trap(struct trapframe *f);

/* 例外ハンドラの有効化 */
void trap_init(void) {
    w_stvec((uint32_t) kernel_entry);
}

/* CPUが例外時にジャンプしてくるエントリーポイント */
__attribute__((naked))
__attribute__((aligned(4)))         /* stvecの下位2bitはフラグとして使われるため、0b100(=4)の倍数に配置 */
void kernel_entry(void) {           /* staticを付けると、アドレスが stvec 経由で CPU に使われていると認識されず、コンパイラが関数を消去しうる */
    __asm__ __volatile__(
        /* 汎用レジスタが使えないため、 sp を例外処理用のカーネルスタックの先頭アドレスが格納されている sscratch の値と入れ替え */
        "csrrw sp, sscratch, sp\n"

        /* そのカーネルスタック上に、例外発生時のレジスタを保存するための退避領域を確保 */
        "addi sp, sp, -4 * 31\n"

        /* 確保した退避領域に、例外発生時の汎用レジスタを保存 */
        "sw ra,  4 * 0(sp)\n"
        "sw gp,  4 * 1(sp)\n"
        "sw tp,  4 * 2(sp)\n"
        "sw t0,  4 * 3(sp)\n"
        "sw t1,  4 * 4(sp)\n"
        "sw t2,  4 * 5(sp)\n"
        "sw t3,  4 * 6(sp)\n"
        "sw t4,  4 * 7(sp)\n"
        "sw t5,  4 * 8(sp)\n"
        "sw t6,  4 * 9(sp)\n"
        "sw a0,  4 * 10(sp)\n"
        "sw a1,  4 * 11(sp)\n"
        "sw a2,  4 * 12(sp)\n"
        "sw a3,  4 * 13(sp)\n"
        "sw a4,  4 * 14(sp)\n"
        "sw a5,  4 * 15(sp)\n"
        "sw a6,  4 * 16(sp)\n"
        "sw a7,  4 * 17(sp)\n"
        "sw s0,  4 * 18(sp)\n"
        "sw s1,  4 * 19(sp)\n"
        "sw s2,  4 * 20(sp)\n"
        "sw s3,  4 * 21(sp)\n"
        "sw s4,  4 * 22(sp)\n"
        "sw s5,  4 * 23(sp)\n"
        "sw s6,  4 * 24(sp)\n"
        "sw s7,  4 * 25(sp)\n"
        "sw s8,  4 * 26(sp)\n"
        "sw s9,  4 * 27(sp)\n"
        "sw s10, 4 * 28(sp)\n"
        "sw s11, 4 * 29(sp)\n"
        /* 例外発生時の sp を sscratch から復元し、汎用レジスタ経由で退避領域に保存 */
        "csrr a0, sscratch\n"
        "sw a0, 4 * 30(sp)\n"

        /* 二重トラップに備え、例外処理用のカーネルスタックの先頭アドレスを再度計算し、汎用レジスタ経由で sscratch に再設定 */
        "addi a0, sp, 4 * 31\n"
        "csrw sscratch, a0\n"

        /* 完成した退避領域の先頭アドレス(sp)を第一引数 a0 に渡し、例外ハンドラを呼び出す */
        "mv a0, sp\n"
        "call handle_trap\n"

        /* 例外ハンドラから戻ったら、退避領域から全ての汎用レジスタを復元 */
        "lw ra,  4 * 0(sp)\n"
        "lw gp,  4 * 1(sp)\n"
        "lw tp,  4 * 2(sp)\n"
        "lw t0,  4 * 3(sp)\n"
        "lw t1,  4 * 4(sp)\n"
        "lw t2,  4 * 5(sp)\n"
        "lw t3,  4 * 6(sp)\n"
        "lw t4,  4 * 7(sp)\n"
        "lw t5,  4 * 8(sp)\n"
        "lw t6,  4 * 9(sp)\n"
        "lw a0,  4 * 10(sp)\n"
        "lw a1,  4 * 11(sp)\n"
        "lw a2,  4 * 12(sp)\n"
        "lw a3,  4 * 13(sp)\n"
        "lw a4,  4 * 14(sp)\n"
        "lw a5,  4 * 15(sp)\n"
        "lw a6,  4 * 16(sp)\n"
        "lw a7,  4 * 17(sp)\n"
        "lw s0,  4 * 18(sp)\n"
        "lw s1,  4 * 19(sp)\n"
        "lw s2,  4 * 20(sp)\n"
        "lw s3,  4 * 21(sp)\n"
        "lw s4,  4 * 22(sp)\n"
        "lw s5,  4 * 23(sp)\n"
        "lw s6,  4 * 24(sp)\n"
        "lw s7,  4 * 25(sp)\n"
        "lw s8,  4 * 26(sp)\n"
        "lw s9,  4 * 27(sp)\n"
        "lw s10, 4 * 28(sp)\n"
        "lw s11, 4 * 29(sp)\n"
        "lw sp,  4 * 30(sp)\n"

        /* 元の処理を再開するため、ハードウェアが保存していたアドレス(sepc)へ復帰 */
        "sret\n"
    );
}

/* 例外発生直後の汎用レジスタのスナップショットから、原因を表示する例外ハンドラ */
void handle_trap(struct trapframe *f) { /* staticを付けると、インラインアセンブリが関数名を解決できず、リンクエラーになる */
    uint32_t scause = r_scause();
    uint32_t stval  = r_stval();
    uint32_t sepc   = r_sepc();
    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, sepc);
}

/* 例外発生時のジャンプ先アドレスを設定する */
static void w_stvec(uint32_t v) {
    __asm__ __volatile__("csrw stvec, %0" ::"r"(v));
}

/* 例外の発生要因を返す */
static uint32_t r_scause(void) {
    uint32_t x;
    __asm__ __volatile__("csrr %0, scause" : "=r"(x));
    return x;
}

/* 例外に付随する補足情報を返す */
static uint32_t r_stval(void) {
    uint32_t x;
    __asm__ __volatile__("csrr %0, stval" : "=r"(x));
    return x;
}

/* 例外が発生した命令のアドレスを返す */
static uint32_t r_sepc(void) {
    uint32_t x;
    __asm__ __volatile__("csrr %0, sepc" : "=r"(x));
    return x;
}
