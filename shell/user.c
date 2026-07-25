/*
 * user.c
 *
 * ユーザープロセスのエントリポイント
 */

extern int main(int argc, char **argv);

/*
 * カーネルの user_entry から sret で飛び込んでくる最初の関数。
 * a0 に argc, a1 に argv がセットされた状態で実行される。
 */
__attribute__((noreturn)) void _start(int argc, char **argv) {
    main(argc, argv);

    /* main が終了してもクラッシュしないように無限ループで安全に停止 */
    while (1) {
    }
}
