# Learnix86 OS

> My goal is to learn OS development by writing a simple OS in C for x86 machines.

I decided to go with x86 because it's definitely easier than x64 and there are lots of useful articles, main sources:
- [OSDev Wiki](https://wiki.osdev.org/Bare_Bones#Building_a_Cross-Compiler)
- [MIT JOS Lab](https://pdos.csail.mit.edu/6.828/2016/labs/lab1/)

I was tempted to use Rust at first but I'm far more comfortable with C and thus prefer to learn it this way, maybe I'll rewrite it in Rust in the future.

## Boot Process
To keep things simple I opted to use the Multiboot Standard instead of writing my own bootloader; look into kern/boot/boot.S.

An initial GDT (Global Descriptor Table) is setup to map the entire 4GB physical memory to avoid issues later.

## Features
The foundamental features that MUST be added:
- [x] Multiboot compatible bootloader
    - [x] GDT setup (maps the entire 4GB for both code and data with read and write permissions)
    - [ ] Figure out multiboot headers to tell the kernel the amount of physical memory avaiable for paging setup
- [ ] VGA Terminal
    - [x] newline characther support
    - [x] terminal scrolling
- [ ] Interrupts handling
    - [ ] keyboard input
- [ ] Virtual Memory (4KB Paging)

Will consider more features if I'm able to complete the ones above
