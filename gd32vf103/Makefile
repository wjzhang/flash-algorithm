CROSS_COMPILE ?= riscv32-unknown-elf-

CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
OBJCOPY=$(CROSS_COMPILE)objcopy
OBJDUMP=$(CROSS_COMPILE)objdump
NM=$(CROSS_COMPILE)nm


CFLAGS  = -march=rv32i -mabi=ilp32 -static -nostartfiles -nostdlib -Os -fPIC -W
LDFLAGS = -static -nostdlib -T gd32vf103.ld

all: gd32vf103.bin gd32vf103.sym flashalgorithm.S

.PHONY: clean

flashalgorithm.o: flashalgorithm.c
	$(CC) $(CFLAGS) -c  $< -o  $@

flashalgorithm.S: flashalgorithm.o
	$(OBJDUMP) -d -S -l  --inlines  $<  > $@
    
start.o: start.S
	$(CC) $(CFLAGS) -c $< -o  $@
    
gd32vf103.elf: start.o flashalgorithm.o
	$(LD) $(LDFLAGS) $^ -o $@

gd32vf103.bin: gd32vf103.elf
	$(OBJCOPY) -Obinary $< $@

gd32vf103.sym: gd32vf103.elf
	$(NM) -n $<  > $@

clean:
	-rm -f *.elf *.o *.bin *.sym flashalgorithm.S