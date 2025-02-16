# Learnix86-OS

An hobbyst x86 operating system that I'm developing to understand more about the topic.

## Debugging

Simply run make gdb and, in another terminal, run gdb.sh to automatically launch a remote GDB session with loaded symbols

## Features Timeline

- [x] Multiboot support (to use GRUB)
- [x] GDT (Global Descriptor Table) installed
- [x] IDT (Interrupt Descriptor Table) installed
- [ ] Minimal Keyboard Driver
    - [x] IRQ1 (Keyboard)
    - [x] Basic Scancode conversion
    - [x] Basic SHIFT and CAPSLOCK support
- [x] GDB attachment
- [ ] Exception Handling
    - [ ] Division by zero
    - [ ] Null-Pointer dereference
- [ ] Kernel Monitor
- [ ] x86 Multi-Level Paging for Virtual Memory
    - [ ] Page-Fault Exception Handler
- [ ] File System (vsfs)
- [ ] Userland