> [!NOTE]
> This is an educational programming language implementation.
> Do not expect anything robust, fast or something you could use
> in production.

# Juls

Juls is a programming language plus compiler. The goal of this
project is to learn more about programming languages, compilers and
other surrounding tools. That means that most parts
of the compiler are implemented from scratch and kept very simple.

The compiler is implemented in C99 and dependencies are kept to a
minimum to ease porting to different platforms.

Currently the compiler supports generating code and binary files
for Aarch64 and x86_64 on Android, Linux and macOs. Windows support
is planned.

## References

### Syscalls

- Linux Syscalls: [https://syscall.sh](https://syscall.sh)

### x86-64

- x86 and amd64 instruction reference: [https://www.felixcloutier.com/x86/](https://www.felixcloutier.com/x86/)
- Online x86 and x64 Intel Instruction Assembler: [https://defuse.ca/online-x86-assembler.htm](https://defuse.ca/online-x86-assembler.htm)

### ARM

- Arm Architecture Reference Manual for A-profile architecture: [https://developer.arm.com/documentation/ddi0487/ja/](https://developer.arm.com/documentation/ddi0487/ja/)
  C6.2 has a list of all the base instructions with their respective encodings.

### ELF

- Executable and Linking Format (ELF) Specification, Version 1.2: [https://refspecs.linuxbase.org/elf/elf.pdf](https://refspecs.linuxbase.org/elf/elf.pdf)

### Mach-O

- OS X ABI Mach-O File Format Reference: [https://github.com/aidansteele/osx-abi-macho-file-format-reference](https://github.com/aidansteele/osx-abi-macho-file-format-reference)
- Mach-O linker in Zig: linking in the era of Apple Silicon: [https://archive.fosdem.org/2021/schedule/event/zig_macho/](https://archive.fosdem.org/2021/schedule/event/zig_macho/)
