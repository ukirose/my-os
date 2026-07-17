/*
 * kprintf.c
 *
 * 動的メモリ割り当てやファイルI/Oに依存できないため、書式変換を内部バッファとputcharのみで完結させる
 */

#include "kernel.h"

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

/* 文字列を1文字ずつ出力する */
static void print_str(const char *s) {
    while (*s) {
        putchar(*s++);
    }
}

/* 数値を10進数で出力する */
static void print_int(int value) {
    if (value == 0) {
        putchar('0');
        return;
    }

    int sign = value;
    if (sign < 0) value = -value;

    char num_buf[32];
    int digit_index = 0;

    while (value > 0) {
        num_buf[digit_index++] = (value % 10) + '0';
        value /= 10;
    }

    if (sign < 0) num_buf[digit_index++] = '-';

    while (digit_index > 0) {
        putchar(num_buf[--digit_index]);
    }
}

/* 数値を16進数で出力する */
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
