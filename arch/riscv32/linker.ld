/* RISC-V 32 Linker Script — QEMU virt machine
 *
 * Kernel loaded at 0x80400000 by OpenSBI (4MB into RAM).
 * RAM: 0x80000000 .. 0x87FFFFFF (128MB total).
 * OpenSBI occupies 0x80000000 .. 0x803FFFFF.
 *
 * Output format: ELF32 little-endian RISC-V.
 */

OUTPUT_ARCH(riscv)
OUTPUT_FORMAT("elf32-littleriscv")
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x80400000, LENGTH = 124M
}

SECTIONS {
    .text : ALIGN(4K) {
        KEEP(*(.text.entry))
        *(.text.trap)
        *(.text*)
    } > RAM

    .rodata : ALIGN(4K) {
        *(.rodata*)
    } > RAM

    .data : ALIGN(4K) {
        *(.data*)
        *(.sdata*)
        PROVIDE(__global_pointer$ = . + 0x800);
    } > RAM

    .bss (NOLOAD) : ALIGN(4K) {
        _sbss = .;
        *(.bss*)
        *(.sbss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . += 32K;
        _stack_top = .;
    } > RAM

    .heap (NOLOAD) : ALIGN(4K) {
        __heap_start = .;
        . += 8M;
        __heap_end = .;
    } > RAM

    _kernel_end = .;

    /DISCARD/ : {
        *(.note*)
        *(.eh_frame*)
        *(.comment)
    }
}
