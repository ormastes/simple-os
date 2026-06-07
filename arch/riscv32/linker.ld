/* RISC-V 32 Linker Script — QEMU virt machine (M-mode, no OpenSBI)
 *
 * Kernel loaded at 0x80000000 — direct M-mode boot with -bios none.
 * RAM: 0x80000000 .. 0x8FFFFFFF (256MB total with -m 256M).
 * No OpenSBI firmware; kernel starts at RAM base.
 *
 * Stack: 8 MB (Simple code uses large stack frames)
 * Heap: 64 MB (bump allocator for runtime objects)
 *
 * Output format: ELF32 little-endian RISC-V.
 */

OUTPUT_ARCH(riscv)
OUTPUT_FORMAT("elf32-littleriscv")
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 256M
}

SECTIONS {
    /* Executable code — entry point first */
    .text : ALIGN(4K) {
        KEEP(*(.text.entry))        /* _start entry point */
        *(.text.trap)               /* Trap vector (must be 4-byte aligned) */
        *(.text*)
    } > RAM

    /* Read-only data */
    .rodata : ALIGN(4K) {
        *(.rodata*)
        *(.srodata*)
    } > RAM

    /* Initialized read-write data */
    .data : ALIGN(4K) {
        *(.data*)
        *(.sdata*)
        PROVIDE(__global_pointer$ = . + 0x800);
    } > RAM

    /* Uninitialized data (zeroed by startup code) */
    .bss (NOLOAD) : ALIGN(4K) {
        _sbss = .;
        *(.bss*)
        *(.sbss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    /* Kernel stack (8MB — Simple code uses large stack frames) */
    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . += 8M;
        _stack_top = .;
    } > RAM

    /* Kernel heap (64MB for bump allocator) */
    .heap (NOLOAD) : ALIGN(4K) {
        __heap_start = .;
        . += 64M;
        __heap_end = .;
    } > RAM

    _kernel_end = .;

    /DISCARD/ : {
        *(.note*)
        *(.eh_frame*)
        *(.comment)
    }
}
