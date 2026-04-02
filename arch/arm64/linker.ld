/* ARM64 SimpleOS Linker Script
 *
 * Target: QEMU virt machine (aarch64)
 * RAM starts at 0x40000000, kernel loaded by QEMU -kernel flag.
 *
 * Memory layout:
 *   .text     — code (entry point first, then exception vectors)
 *   .rodata   — read-only data
 *   .data     — initialized read-write data
 *   .bss      — zero-initialized data
 *   .stack    — kernel stack (64KB)
 *   .heap     — kernel heap (16MB)
 *
 * Entry point: _start (in .text.entry section)
 * Exception vectors: .text.vectors (must be 2KB aligned for VBAR_EL1)
 */

OUTPUT_ARCH(aarch64)
OUTPUT_FORMAT("elf64-littleaarch64")
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 128M
}

SECTIONS {
    /* Code section — entry point must be first */
    .text : ALIGN(4K) {
        KEEP(*(.text.entry))        /* Boot entry point (_start) */
        . = ALIGN(2K);              /* VBAR_EL1 requires 2KB alignment */
        *(.text.vectors)            /* Exception vector table */
        *(.text*)                   /* All other code */
    } > RAM

    /* Read-only data */
    .rodata : ALIGN(4K) {
        *(.rodata*)
    } > RAM

    /* Initialized read-write data */
    .data : ALIGN(4K) {
        *(.data*)
    } > RAM

    /* Zero-initialized data */
    .bss (NOLOAD) : ALIGN(4K) {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    /* Kernel stack (grows downward, _stack_top is initial SP) */
    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . += 64K;
        _stack_top = .;
    } > RAM

    /* Kernel heap */
    .heap (NOLOAD) : ALIGN(4K) {
        __heap_start = .;
        . += 16M;
        __heap_end = .;
    } > RAM

    /* End of kernel image */
    _kernel_end = .;

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.note*)
        *(.eh_frame*)
        *(.comment)
        *(.ARM.attributes)
        *(.ARM.exidx*)
    }
}
