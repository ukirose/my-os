# my-os

RISC-V 32bit (rv32i) 向けのベアメタル OS を、QEMU 上でゼロから実装するプロジェクト。

## ビルドと実行

```bash
$ make        # カーネルをビルドする
$ make run    # QEMU で起動する
```

必要なもの: LLVM (clang, lld) と qemu-system-riscv32

## 構成

- `kernel/` : カーネル本体
