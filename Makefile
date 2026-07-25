.PHONY: all run clean

# kernel は shell の ELF を .incbin で取り込むため、shell を先にビルドする
all:
	$(MAKE) -C shell
	$(MAKE) -C kernel

run: all
	$(MAKE) -C kernel run

clean:
	$(MAKE) -C shell clean
	$(MAKE) -C kernel clean
