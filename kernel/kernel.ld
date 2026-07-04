/*
 * kernel.ld
 *
 * ブートコードや変数を指定のメモリアドレスへ配置するリンカスクリプト
 */

/* カーネルの実行を boot 関数から開始するように指定 */
ENTRY(boot)

SECTIONS {
    /* OpenSBI がカーネルをロードする固定アドレス */
    . = 0x80200000;

    .text :{
        /* boot 関数をバイナリ先頭に固定。リンカ最適化で削除されないよう KEEP で保護 */
        KEEP(*(.text.boot));
        *(.text .text.*);
    }

    .rodata : ALIGN(4) { *(.rodata .rodata.*); }
    .data   : ALIGN(4) { *(.data .data.*); }

    .bss : ALIGN(4) {
        __bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        __bss_end = .;
    }

    /* C 呼出規約が要求するスタック領域を 128KB 確保 */
    . = ALIGN(4);
    . += 128 * 1024;
    __stack_top = .;
}
