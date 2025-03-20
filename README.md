# Learnix86-OS

Learnix86-OS is a hobbyist x86 operating system developed to gain a deeper understanding of kernel and OS internals. The project is primarily for educational purposes, but contributions are welcome!

## Build Instructions

### Prerequisites

Ensure you have the followinig dependencies installed:
- [qemu](https://www.qemu.org/)
- [i686-gcc cross compiler + binutils](https://wiki.osdev.org/GCC_Cross-Compiler)
- `make`
- `gdb` (optionally, for debugging purposes)

### Building the kernel

Clone the repository and then launch `make qemu` to automatically load the compiled OS image in a qemu vm: 

```bash
git clone https://github.com/0xlrivo/learnix86-os
cd Learnix86-OS
make qemu
```

## Debugging

Launch `make gdb`, which pauses the vm and allows an external `gcc` process to connect, use the `gdb.sh` utility script:

```bash
make gdb

./gdb.sh    # <- on another shell
```

## Notes

### Virtual Memory
- This is an higher-half kernel, meaning that it is virtually mapped starting from 0xC0000000 (3 GB)

You need to rewrite kernel_main to activate paging IMMEDIATELY before doing anything else
You will:
- map kernel's code pages from 0xC0000000 (3 GB)
- identity map kernel's code pages which contains vm setup code to their physical addresses
    - this is NECESSARY otherwise you page fault after paging is enabled
- load the kernel's page directory into CR3
- enable paging in CR0
- jump to a predefined VIRTUAL MEMORY ENTRYPOINT... aka the first instruction to execute after paging is enabled
    - after this point, EIP will contain VIRTUAL ADDRESSES, not physical ones
    - such function should unmap the identity mapped pages because they aren't needed anymore