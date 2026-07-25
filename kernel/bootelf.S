/*
 * bootelf.S
 *
 * ディスクからファイルを読み込む機能がないため、起動時にOSと一緒にメモリへ展開されるように、
 * アプリケーションのバイナリをカーネルのデータ区画（.boot_elf）としてリンカに直接結合する
 */

.section .boot_elf
.incbin "../build/shell/shell.elf"
