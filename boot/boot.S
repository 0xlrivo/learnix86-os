# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reserve a stack for the initial thread.
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/* DATA SECTION */
.section .data
.align 8

/* Setup a Global Descriptor table that maps the entire 4GB as both code and data R/W segments */
gdt:
    .quad 0x0000000000000000  # Null descriptor
    .quad 0x00CF9A000000FFFF  # Code segment: base=0, limit=4GB, type=0x9A
    .quad 0x00CF92000000FFFF  # Data segment: base=0, limit=4GB, type=0x92

gdt_end:

gdtr:
	.word gdt_end - gdt - 1  # Limit (GDT size - 1)
	.long gdt				 # Base  (address at which the GDT exists)

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
    # disable interrupts
    # the kernel will enable them later after setup
    cli

    # setup C stack
	movl $stack_top, %esp

    # multiboot info are calling parameters for kernel_main
    pushl %ebx
    pushl %eax

	# load the GDT
	mov $gdtr, %eax
	lgdt (%eax)

	# set registers to use the GDT
	mov $0x10, %ax		# Data Segment Selector (2)
    mov %ax, %ds		# DS: Data Segment
    mov %ax, %es		# ES: Extra Segment
    mov %ax, %fs		# FS
    mov %ax, %gs		# GS
    mov %ax, %ss		# SS: Stack Segment

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b

.size _start, . - _start

.global irq1_wrapper
.type irq1_wrapper, @function
irq1_wrapper:
	pushal
    cld
    call irq1_handler   /* C code that does the actual work */
    popal
    iret