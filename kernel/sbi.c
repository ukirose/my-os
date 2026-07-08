/*
 * sbi.c
 *
 * ecall命令でS-ModeからM-ModeのOpenSBIへハードウェア操作を委譲する
 */

#include "kernel.h"

/* SBI拡張ID */
#define SBI_CONSOLE_PUTCHAR 1

/* SBI ecallの戻り値 */
struct sbiret {
    long error;
    long value;
};

/*
 * ecallでS-ModeからM-ModeのOpenSBIに処理を委譲する。
 * SBIの呼出規約では、a0〜a5=引数、a6=fid(機能ID)、a7=eid(拡張ID)をレジスタに設定してからecallを発行する
 */
static struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                              long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
                         : "memory");
    return (struct sbiret){.error = a0, .value = a1};
}

void
putchar(char c)
{
    sbi_call(c, 0, 0, 0, 0, 0, 0, SBI_CONSOLE_PUTCHAR);
}
