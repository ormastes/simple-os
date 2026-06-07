/*
 * ARM32 (ARMv7) SimpleOS Linker Script
 *
 * Target: QEMU virt machine (arm, 32-bit)
 * RAM starts at 0x40000000, kernel loaded by QEMU -kernel flag.
 *
 * Memory layout:
 *   .text     - code (entry point first)
 *   .rodata   - read-only data
 *   .data     - initialized read-write data
 *   .bss      - zero-initialized data
 *   .stack    - kernel stack (64KB)
 *   .heap     - kernel heap (4MB)
 *
 * Entry point: _entry_asm (in .text.entry section)
 */

OUTPUT_ARCH(arm)
OUTPUT_FORMAT("elf32-littlearm")
ENTRY(_entry_asm)

MEMORY {
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 128M
}

SECTIONS {
    /* Code section -- entry point must be first */
    .text : ALIGN(4K) {
        KEEP(*(.text.entry))        /* Boot entry point (_entry_asm) */
        *(.text*)                   /* All other code */
    } > RAM

    /* Read-only data */
    .rodata : ALIGN(4K) {
        *(.rodata*)
    } > RAM

    .simple_sandbox : ALIGN(4K) {
        __simple_sandbox_start = .;
        KEEP(*(.simple.sandbox*))
        KEEP(*(.simple.mpu*))
        __simple_sandbox_end = .;
    } > RAM

    /* Initialized read-write data */
    .data : ALIGN(4K) {
        *(.data*)
    } > RAM

    /* Zero-initialized data */
    .bss (NOLOAD) : ALIGN(4K) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > RAM

    /* Kernel stack (grows downward, _stack_top is initial SP) */
    .stack (NOLOAD) : ALIGN(8) {
        _stack_bottom = .;
        . += 64K;
        _stack_top = .;
    } > RAM

    /* Kernel heap -- 4MB for 32-bit */
    .heap (NOLOAD) : ALIGN(4K) {
        __heap_start = .;
        . += 4M;
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
