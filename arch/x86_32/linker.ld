/*
 * x86_32 (i686) Bare-Metal Linker Script
 *
 * Target: x86_32 processors (QEMU i386, physical hardware)
 * Bootloader: Multiboot1 compatible (QEMU -kernel)
 *
 * Memory Layout:
 *   0x00100000 - Kernel load address (1MB mark)
 *   .heap      - 4MB heap (bump allocator)
 *   .stack     - 64KB stack (grows down)
 *
 * Sections:
 *   .text.entry - Multiboot1 header + _entry32 (must be in first 8KB)
 *   .text       - Code
 *   .rodata     - Read-only data
 *   .data       - Initialized data
 *   .bss        - Uninitialized data
 *   .heap       - Dynamic memory allocation
 */

ENTRY(_entry32)

SECTIONS
{
    /* Kernel starts at 1MB (standard for multiboot) */
    . = 0x100000;

    /* Code section -- .text.entry first for Multiboot header */
    .text ALIGN(4K) :
    {
        KEEP(*(.text.entry))
        KEEP(*(.multiboot))
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data */
    .bss ALIGN(4K) :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        __bss_end = .;
    }

    /* Heap (grows upward) -- 4MB for 32-bit */
    .heap ALIGN(4K) :
    {
        __heap_start = .;
        . = . + 4M;
        __heap_end = .;
    }

    /* Stack (grows downward) */
    .stack ALIGN(4K) :
    {
        . = . + 64K;
        _stack_top = .;
    }

    /* End of kernel */
    _kernel_end = .;

    /* Debug sections */
    .debug_info 0 : { *(.debug_info) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_line 0 : { *(.debug_line) }
    .debug_str 0 : { *(.debug_str) }
    .debug_frame 0 : { *(.debug_frame) }
    .debug_loc 0 : { *(.debug_loc) }
    .debug_ranges 0 : { *(.debug_ranges) }

    /* Discard unwanted sections */
    /DISCARD/ :
    {
        *(.note*)
        *(.eh_frame*)
        *(.comment)
    }
}
