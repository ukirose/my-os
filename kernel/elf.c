/*
 * elf.c
 *
 * ELFフォーマットのパースおよびメモリ展開
 */

#include "elf.h"
#include "kernel.h"
#include "string.h"
#include "mmu.h"

/* ELFファイルを仮想メモリ空間に展開し、実行開始アドレスを返す */
uint32_t load_elf(pagetable_t page_table, const uint8_t *elf_data) {
    /* 不正なデータの実行を防ぐため、正しいELFファイルであるかをチェック */
    struct elfhdr *elf = (struct elfhdr *)elf_data;
    if (elf->magic != ELF_MAGIC) {
        PANIC("invalid ELF magic: expected %x, got %x", ELF_MAGIC, elf->magic);
    }

    /* プログラムヘッダを読み取りメモリへ展開 */
    struct proghdr *ph = (struct proghdr *)(elf_data + elf->phoff);
    for (int i = 0; i < elf->phnum; i++) {
        /* ロード対象のセグメントのみ処理する */
        if (ph[i].type == ELF_PROG_LOAD) {
            uint32_t vaddr = ph[i].vaddr;
            uint32_t memsz = ph[i].memsz;
            uint32_t filesz = ph[i].filesz;
            uint32_t offset = ph[i].offset;

            /* 必要なサイズの物理ページを確保し、セグメントのデータをコピー */
            uint32_t pages = DIV_ROUND_UP(memsz, PAGE_SIZE);
            paddr_t paddr = alloc_pages(pages);
            memcpy((void *)paddr, elf_data + offset, filesz);

            /* ファイルサイズを超過した、余分なメモリ領域をゼロクリア */
            if (memsz > filesz) memset((void *)(paddr + filesz), 0, memsz - filesz);


            /* 確保した物理ページをページテーブルにマッピング */
            uint32_t flags = PTE_R | PTE_W | PTE_X | PTE_U;
            for (uint32_t j = 0; j < pages; j++) {
                uint32_t page_vaddr = vaddr + j * PAGE_SIZE;
                paddr_t page_paddr = paddr + j * PAGE_SIZE;
                map_page(page_table, page_vaddr, page_paddr, flags);
            }
        }
    }

    kprintf("ELF parsed successfully: entry point = %x\n", elf->entry);

    /* 実行開始アドレスを返す */
    return elf->entry;
}
