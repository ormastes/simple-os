/*
 * x86_64 Bare-Metal Linker Script
 *
 * Target: x86_64 processors (QEMU, physical hardware)
 * Bootloader: Multiboot2 (GRUB2 compatible)
 *
 * Memory Layout:
 *   0x00100000 - Kernel load address (1MB mark)
 *   0x00200000 - Stack (grows down)
 *   Identity-mapped first 1GB via page tables
 *
 * Sections:
 *   .multiboot - Multiboot2 header (must be in first 8KB)
 *   .text      - Code and read-only data
 *   .rodata    - Read-only data (GDT, etc.)
 *   .data      - Initialized data
 *   .bss       - Uninitialized data (page tables, stack)
 *   .heap      - Dynamic memory allocation
 */

ENTRY(_start)

SECTIONS
{
    /* Kernel starts at 1MB (standard for multiboot) */
    . = 0x100000;

    /* Multiboot header must be in first 8KB */
    .multiboot ALIGN(8) :
    {
        KEEP(*(.multiboot))
    }

    /* Code section */
    .text ALIGN(4K) :
    {
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

    /* Heap (grows upward) */
    .heap ALIGN(4K) :
    {
        __heap_start = .;
        . = . + 16M;  /* 16MB heap */
        __heap_end = .;
    }

    /* Stack (grows downward) */
    .stack ALIGN(4K) :
    {
        . = . + 64K;  /* 64KB stack */
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
