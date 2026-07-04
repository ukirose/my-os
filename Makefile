.PHONY: all run clean

all:
	$(MAKE) -C kernel

run:
	$(MAKE) -C kernel run

clean:
	$(MAKE) -C kernel clean
