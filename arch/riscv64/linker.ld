/* RISC-V 64 SimpleOS Linker Script — S-mode with OpenSBI
 *
 * Kernel is loaded at 0x80200000 by OpenSBI as an S-mode payload.
 * OpenSBI occupies 0x80000000..0x801FFFFF (2MB).
 *
 * Memory layout (QEMU virt, 512MB RAM):
 *   0x80000000 - 0x801FFFFF : OpenSBI (2MB, reserved)
 *   0x80200000 - onwards    : Kernel image
 *
 * Stack: 8 MB (Simple code uses large stack frames)
 * Heap: 64 MB (bump allocator for runtime objects)
 *
 * Sections are 4KB-aligned for page table compatibility (Sv39).
 */
OUTPUT_ARCH(riscv)
OUTPUT_FORMAT("elf64-littleriscv")
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x80200000, LENGTH = 510M
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
