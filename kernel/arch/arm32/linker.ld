/* ARM32 SimpleOS Linker Script
 *
 * Target: QEMU virt machine (arm, Cortex-A15)
 * RAM starts at 0x40000000, kernel loaded by QEMU -kernel flag.
 *
 * Memory layout:
 *   .text     — code (entry point first, then exception vectors)
 *   .rodata   — read-only data
 *   .data     — initialized read-write data
 *   .bss      — zero-initialized data
 *   .stack    — kernel stack (32KB)
 *   .heap     — kernel heap (8MB)
 *
 * Entry point: _start (in .text.entry section)
 * Exception vectors: .text.vectors (must be 32-byte aligned for VBAR)
 */

OUTPUT_ARCH(arm)
OUTPUT_FORMAT("elf32-littlearm")
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 128M
}

SECTIONS {
    /* Code section — entry point must be first */
    .text : ALIGN(4K) {
        KEEP(*(.text.entry))        /* Boot entry point (_start) */
        . = ALIGN(32);              /* VBAR requires 32-byte alignment */
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
    .stack (NOLOAD) : ALIGN(8) {
        _stack_bottom = .;
        . += 32K;
        _stack_top = .;
    } > RAM

    /* Kernel heap */
    .heap (NOLOAD) : ALIGN(4K) {
        __heap_start = .;
        . += 8M;
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
