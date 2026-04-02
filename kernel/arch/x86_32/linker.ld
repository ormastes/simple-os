/* x86_32 Linker Script for Multiboot 1 Kernel
 *
 * OUTPUT_FORMAT: elf32-i386 (32-bit x86 ELF)
 * ENTRY: _start (Multiboot entry point)
 * Load address: 1MB (0x100000) — Multiboot standard
 *
 * The .multiboot section must appear within the first 8KB of the
 * binary for GRUB to find the Multiboot header.
 *
 * Memory layout:
 *   0x00100000  .multiboot  (Multiboot header, must be early)
 *   0x00101000  .text       (kernel code)
 *   ...         .rodata     (read-only data)
 *   ...         .data       (initialized data)
 *   ...         .bss        (uninitialized data)
 *   ...         .stack      (32KB kernel stack)
 *   ...         .heap       (8MB kernel heap)
 */

OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    /* Load at 1MB (standard for Multiboot kernels) */
    . = 0x100000;

    /* Provide kernel start symbol */
    _kernel_start = .;

    /* Multiboot header — must be within first 8KB of binary */
    .multiboot ALIGN(4) :
    {
        *(.multiboot)
    }

    /* Code section */
    .text ALIGN(4K) :
    {
        *(.text .text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) :
    {
        *(.rodata .rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4K) :
    {
        *(.data .data.*)
    }

    /* BSS (uninitialized data) */
    .bss ALIGN(4K) :
    {
        _bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        _bss_end = .;
    }

    /* Kernel stack (32KB) */
    .stack ALIGN(4K) :
    {
        _stack_bottom = .;
        . += 32K;
        _stack_top = .;
    }

    /* Kernel heap (8MB) */
    .heap ALIGN(4K) :
    {
        _heap_start = .;
        . += 8M;
        _heap_end = .;
    }

    /* Kernel end symbol */
    _kernel_end = .;

    /* Discard unnecessary sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame*)
    }
}
