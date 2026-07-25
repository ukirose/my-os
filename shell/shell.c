/*
 * shell.c
 *
 * 初期のユーザープロセス（テスト用ダミーシェル）
 */

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    /* U-mode 遷移のテストとして、意図的に無効なメモリアクセス(0x0)を行いページフォルトを発生させる */
    *((volatile int *)0) = 0;

    while (1) {
    }

    return 0;
}
