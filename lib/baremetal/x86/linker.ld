/* x86 Linker Script for Multiboot-compliant kernel
 *
 * Memory Layout:
 *   0x00100000 - Kernel load address (1MB)
 *   0x00110000 - Kernel code/data
 *   0x???????? - Heap/Stack
 *   0x000B8000 - VGA text buffer
 */

ENTRY(_start)

SECTIONS
{
    /* Kernel is loaded at 1MB by bootloader */
    . = 0x00100000;

    .multiboot ALIGN(4K) :
    {
        *(.multiboot)
        . = ALIGN(4K);
    }

    /* Read-only code */
    .text ALIGN(4K) :
    {
        *(.text.boot)
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Read-write data (initialized) */
    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    /* Read-write data (uninitialized) */
    .bss ALIGN(4K) :
    {
        _sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        _ebss = .;
    }

    /* Stack */
    .stack ALIGN(16) :
    {
        _stack_bottom = .;
        . += 64K;  /* 64KB stack */
        _stack_top = .;
    }

    /* Discard sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.*)
    }
}
