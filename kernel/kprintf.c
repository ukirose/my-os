/*
 * kprintf.c
 *
 * 内部バッファとputcharのみで完結する書式付き出力
 */

#include "types.h"
#include "kernel.h"

/* 標準ライブラリに依存しないため、可変長引数はコンパイラが提供するbuiltinを利用する */
#define va_list  __builtin_va_list
#define va_start __builtin_va_start
#define va_end   __builtin_va_end
#define va_arg   __builtin_va_arg

static void print_str(const char *s);
static void print_int(int value);
static void print_hex(unsigned value);

/* 書式付き文字列をシリアルコンソールに出力する */
void kprintf(const char *fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            putchar(*fmt++);
            continue;
        }

        fmt++;
        switch (*fmt) {
            case '\0':
                putchar('%');
                va_end(vargs);
                return;
            case '%':
                putchar('%');
                break;
            case 's':
                print_str(va_arg(vargs, const char *));
                break;
            case 'd':
                print_int(va_arg(vargs, int));
                break;
            case 'x':
                print_hex(va_arg(vargs, unsigned));
                break;
        }
        fmt++;
    }

    va_end(vargs);
}

static void print_str(const char *s) {
    while (*s) {
        putchar(*s++);
    }
}

static void print_int(int value) {
    if (value == 0) {
        putchar('0');
        return;
    }

    int negative = (value < 0);
    if (negative) value = -value;

    char num_buf[32];
    int digit_index = 0;

    while (value > 0) {
        num_buf[digit_index++] = (value % 10) + '0';
        value /= 10;
    }

    if (negative) num_buf[digit_index++] = '-';

    while (digit_index > 0) {
        putchar(num_buf[--digit_index]);
    }
}

static void print_hex(unsigned value) {
    if (value == 0) {
        putchar('0');
        return;
    }

    char num_buf[32];
    int digit_index = 0;

    while (value > 0) {
        unsigned nibble = value & 0xf;
        num_buf[digit_index++] = "0123456789abcdef"[nibble];
        value >>= 4;
    }

    while (digit_index > 0) {
        putchar(num_buf[--digit_index]);
    }
}
