# my-os

RISC-V 32bit (rv32i) 向けのベアメタル OS を、QEMU 上でゼロから実装するプロジェクト。

## ビルドと実行

```bash
$ make run
```

必要なもの: LLVM (clang, lld) と qemu-system-riscv32

## 構成

| ディレクトリ | 動作モード | 内容 |
| :--- | :--- | :--- |
| `kernel/` | S-mode | カーネル本体。メモリ管理、プロセス管理、例外処理 |
| `shell/` | U-mode | ユーザープロセス。ファイルシステムがないため ELF をカーネルに埋め込む |
| OpenSBI | M-mode | ファームウェア。コンソール出力を SBI 経由で提供する |

## 実行例

```text
$ make run

OpenSBI v1.5.1
...
Boot HART MEDELEG         : 0x00f0b509

Hello World!
ELF parsed successfully: entry point = 100001a
PANIC: trap.c:131: unexpected trap scause=f, stval=fffffffc, sepc=100001c
```

U-mode への遷移までは通っている。最後の PANIC はユーザープロセス側で発生した
Store ページフォルト (scause=15) で、ユーザー空間のスタックがまだ用意されていない。

## 実装状況

| 機能 | 状態 |
| :--- | :--- |
| ブートシーケンスと BSS のゼロクリア | 実装済み |
| SBI 経由のコンソール出力と `kprintf` | 実装済み |
| `PANIC` マクロ | 実装済み |
| 物理ページアロケータ | 実装済み (bump 方式。解放は未対応) |
| 例外ハンドラ | 実装済み |
| プロセス管理とコンテキストスイッチ | 実装済み |
| Sv32 ページテーブルとプロセスごとのアドレス空間 | 実装済み |
| ELF ローダと U-mode への遷移 | 実装済み |
| ユーザースタックの確保 | 未対応 |
| システムコール | 未着手 |
| ディスクドライバ、ファイルシステム | 未着手 |

## プロセスの状態

| 状態 | 説明 |
| :--- | :--- |
| `PROC_UNUSED` | 未使用のスロット |
| `PROC_RUNNABLE` | 実行可能。スケジューラが選択の対象にする |

## 起動シーケンス

```text
OpenSBI がハードウェアを初期化
  -> boot            kernel.ld により 0x80200000 へ配置される
  -> kernel_main
       BSS のゼロクリア
       例外ハンドラの登録        trap_init
       アイドルプロセスの生成    proc_init
       shell の ELF をロードしてユーザープロセスを生成
       yield でコンテキストスイッチ
  -> user_entry      sret で U-mode へ遷移する
```
