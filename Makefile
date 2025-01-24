AS = i686-elf-as
GCC = i686-elf-gcc
GCCFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra

CFILES = $(shell find ./kernel -name "*.c")
OFILES = $(patsubst ./%, $(OUTDIR)/%, $(CFILES:.c=.o))

OUTDIR = out

all: setup kernel

setup:
	mkdir -p $(OUTDIR)
	@find ./kernel -type d -exec mkdir -p $(OUTDIR)/{} \;

$(OUTDIR)/%.o: %.c
	$(GCC) -c $< -o $@ $(GCCFLAGS)

$(OUTDIR)/boot.o: boot/boot.S
	$(AS) $< -o $@

kernel: $(OUTDIR)/boot.o $(OFILES)
	$(GCC) -T linker.ld -o $(OUTDIR)/myos.bin -ffreestanding -O2 -nostdlib $(OUTDIR)/boot.o $(OFILES) -lgcc

qemu: kernel
	qemu-system-i386 -kernel $(OUTDIR)/myos.bin

clean:
	rm -rf $(OUTDIR)
