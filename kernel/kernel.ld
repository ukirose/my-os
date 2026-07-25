/*
 * kernel.ld
 *
 * ブートコードや変数を指定のメモリアドレスへ配置するリンカスクリプト
 */

/* カーネルの実行を boot 関数から開始するように指定 */
ENTRY(boot)

/* カーネルバイナリの全メモリレイアウト */
SECTIONS {
    /* OpenSBI がカーネルをロードするアドレス */
    . = 0x80200000;
    __kernel_base = .;

    /* 機械語命令 */
    .text :{
        KEEP(*(.text.boot));    /* boot 関数をバイナリ先頭に固定し、リンカ最適化で削除されないよう KEEP で保護 */
        *(.text .text.*);
    }
    /* 読み取り専用データ */
    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);

        /* bootelf.S で埋め込まれたELFバイナリの配置とポインタの定義 */
        . = ALIGN(4);
        _boot_elf_start = .;
        KEEP(*(.boot_elf));
    }
    /* 初期値ありグローバル変数 */
    .data   : ALIGN(4) { *(.data .data.*); }
    /* 初期値なしグローバル変数 (ゼロクリア) */
    .bss : ALIGN(4) {
        _bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        _bss_end = .;
    }

    /* RISC-V Calling Conventionの要件に準拠し、 sp は16バイト境界にアライン */
    . = ALIGN(16);
    /* ブート用スタック */
    . += 128 * 1024;
    _stack_top = .;

    /* ページ単位で物理アドレスを扱うため、4KB境界にアライン */
    . = ALIGN(4096);
    /* カーネル終端 （＝物理メモリのフリー領域の始点） */
    _end = .;
}
