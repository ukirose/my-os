/*
 * elf.h
 *
 * ELFフォーマットの構造体定義と定数
 */

#pragma once

#include "stdint.h"
#include "types.h"

#define ELF_MAGIC 0x464C457FU   /* リトルエンディアンにおける \x7FELF のマジックナンバー */

/* ELFファイルの全体ヘッダ */
struct elfhdr {
    uint32_t magic;             /* マジックナンバー (0x7F E L F) */
    uint8_t  elf[12];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;             /* プロセス実行時のエントリポイントとなる仮想アドレス */
    uint32_t phoff;             /* ファイル先頭からプログラムヘッダテーブルへのオフセット */
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;             /* プログラムヘッダのエントリ数 */
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} __attribute__((packed));

/* プログラムの各セグメントのロード情報 */
struct proghdr {
    uint32_t type;              /* セグメントの種類 */
    uint32_t offset;            /* ファイル先頭から該当セグメントデータへのオフセット */
    uint32_t vaddr;             /* メモリ上にロードする際の仮想アドレス */
    uint32_t paddr;
    uint32_t filesz;            /* ファイル上に実際に存在するサイズ */
    uint32_t memsz;             /* メモリ上で確保すべきサイズ */
    uint32_t flags;             /* 権限フラグ (1=Execute, 2=Write, 4=Read) */
    uint32_t align;             /* メモリアライメント要求 */
} __attribute__((packed));

#define ELF_PROG_LOAD 1         /* メモリへロードすべきセグメントを示す定数 */

/* ページテーブルにELFバイナリを展開し、エントリポイントの仮想アドレスを返す */
uint32_t load_elf(pagetable_t page_table, const uint8_t *elf_data);
