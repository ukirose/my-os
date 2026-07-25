# ユーザープログラム（シェル）のビルド設定
# 
# ファイルシステム未実装のため、システムコールテスト用のELFをカーネルに直接埋め込む

# コンパイラ
CC ?= clang

# 標準ライブラリを排除し（freestanding/nostdlib）、純粋な命令のみでコンパイルする設定
CFLAGS = -target riscv32 -mabi=ilp32 -mcmodel=medany -std=c11 -ffreestanding -fno-common -nostdlib -fuse-ld=lld

# 出力先ディレクトリと最終生成物（ELF）の定義
BUILD_DIR = ../build/shell
SHELL_ELF = $(BUILD_DIR)/shell.elf

# コンパイル対象のソースファイルと、対応するオブジェクトファイルの定義
SRCS = shell.c user.c
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

.PHONY: all clean

all: $(SHELL_ELF)

# 各Cファイルをオブジェクトファイル（中間部品）に変換する汎用ルール
$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# オブジェクトファイルを合体し、エントリポイント（_start）と仮想アドレス（0x1000000）を強制してリンクする
$(SHELL_ELF): $(OBJS)
	$(CC) $(CFLAGS) -Wl,-e,_start -Wl,--image-base=0x1000000 -Wl,-Ttext=0x1000000 $(OBJS) -o $@

# ビルド生成物を削除する
clean:
	rm -rf $(BUILD_DIR)
