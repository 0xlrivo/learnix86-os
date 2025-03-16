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