AS = i686-elf-as
GCC = i686-elf-gcc

# Kernel compilation flags
KERN_GCCFLAGS = -ffreestanding -O2 -Wall -Wextra -I include -I libc/include -D__is_libk -g -mgeneral-regs-only
# LIBC compilation flags
LIBC_GCCFLAGS = -ffreestanding -O2 -Wall -Wextra

# C files in root folder
KERN_CFILES = $(shell find ./kernel -name "*.c")
# Object files 
KERN_OFILES = $(patsubst ./%, $(OUTDIR)/%, $(KERN_CFILES:.c=.o))

# LIBC C files
LIBC_CFILES = $(shell find ./libc -name "*.c")
# LIBC Object files
LIBC_OFILES = $(patsubst ./%, $(OUTDIR)/%, $(LIBC_CFILES:.c=.o))

# output directory
OUTDIR = out

all: setup kernel

setup:
	mkdir -p $(OUTDIR)
	@find ./boot -type d -exec mkdir -p $(OUTDIR)/{} \;
	@find ./kernel -type d -exec mkdir -p $(OUTDIR)/{} \;
	@find ./libc -type d -exec mkdir -p $(OUTDIR)/{} \;

# compile C files into object (.o) files without linking
$(OUTDIR)/%.o: %.c
	$(GCC) -c $< -o $@ $(KERN_GCCFLAGS)

# assemble boot.S into boot.o
$(OUTDIR)/boot/boot.o: boot/boot.S
	$(AS) $< -o $@

# @todo libc should be compiled separately and then linked to the kernel object files

# link object files together
kernel: $(OUTDIR)/boot/boot.o $(KERN_OFILES) $(LIBC_OFILES)
	$(GCC) -T boot/linker.ld -o $(OUTDIR)/learnixos.bin -ffreestanding -O2 -nostdlib $(OUTDIR)/boot/boot.o $(KERN_OFILES) $(LIBC_OFILES) -lgcc

qemu: setup kernel
	rm serial.log
	qemu-system-i386 -kernel $(OUTDIR)/learnixos.bin -serial file:serial.log

# in another terminal run gdb and then issue the command target remote localhost:1234
gdb: setup kernel
	qemu-system-i386 -kernel $(OUTDIR)/learnixos.bin -s -S

format:
	clang-format -i $(KERN_CFILES)
	clang-format -i $(LIBC_CFILES)

clean:
	rm -rf $(OUTDIR)
