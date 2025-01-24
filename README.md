# Learnix86 OS

> My goal is to learn OS development by writing a simple OS in C for x86 machines.

I decided to go with x86 because it's definitely easier than x64 and there are lots of useful articles, main sources:
- [OSDev Wiki](https://wiki.osdev.org/Bare_Bones#Building_a_Cross-Compiler)
- [MIT JOS Lab](https://pdos.csail.mit.edu/6.828/2016/labs/lab1/)

I was tempted to use Rust at first but I'm far more comfortable with C and thus prefer to learn it this way, maybe I'll rewrite it in Rust in the future.

## Features
The foundamental features that MUST be added:
- [x] Multiboot compatible bootloader
- [ ] VGA Terminal
    - [x] newline characther support
    - [ ] scrollable rows
- [ ] x86 Paging
- [ ] Scheduling Processes

Will consider more features if I'm able to complete the ones above
