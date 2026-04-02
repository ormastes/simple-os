/* RISC-V 32-bit Linker Script
 *
 * Memory layout for QEMU virt machine (RV32).
 * RAM starts at 0x80000000 with 128MB available.
 *
 * Sections:
 *   .text    - Code (read-execute)
 *   .rodata  - Read-only data
 *   .data    - Initialized data
 *   .bss     - Zero-initialized data
 *   .stack   - Stack allocation
 *
 * Symbols provided:
 *   _stext, _etext     - Text section bounds
 *   _srodata, _erodata - Read-only data bounds
 *   _sdata, _edata     - Data section bounds
 *   _sbss, _ebss       - BSS section bounds
 *   _stack_top          - Top of stack (grows downward)
 *   _heap_start         - Start of heap (grows upward)
 */

OUTPUT_ARCH(riscv)
OUTPUT_FORMAT("elf32-littleriscv")
ENTRY(_start)

/* Memory regions for QEMU virt machine */
MEMORY
{
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
}

/* Stack and heap sizes */
_stack_size = 64K;
_heap_size  = 1M;

SECTIONS
{
    /* Code section */
    .text : ALIGN(4)
    {
        _stext = .;
        KEEP(*(.text.entry))       /* Entry point first */
        *(.text.trap_vector)       /* Trap vector (aligned) */
        *(.text)
        *(.text.*)
        _etext = .;
    } > RAM

    /* Read-only data */
    .rodata : ALIGN(4)
    {
        _srodata = .;
        *(.rodata)
        *(.rodata.*)
        *(.srodata)
        *(.srodata.*)
        _erodata = .;
    } > RAM

    /* Initialized data */
    .data : ALIGN(4)
    {
        _sdata = .;
        *(.data)
        *(.data.*)
        *(.sdata)
        *(.sdata.*)

        /* Global pointer (gp) - optimizes access to small data */
        . = ALIGN(4);
        PROVIDE(__global_pointer$ = . + 0x800);

        _edata = .;
    } > RAM

    /* Uninitialized data (zeroed by startup code) */
    .bss (NOLOAD) : ALIGN(4)
    {
        _sbss = .;
        *(.bss)
        *(.bss.*)
        *(.sbss)
        *(.sbss.*)
        *(COMMON)
        _ebss = .;
    } > RAM

    /* Stack (grows downward from _stack_top) */
    .stack (NOLOAD) : ALIGN(16)
    {
        _stack_bottom = .;
        . += _stack_size;
        _stack_top = .;
    } > RAM

    /* Heap (grows upward from _heap_start) */
    .heap (NOLOAD) : ALIGN(4)
    {
        _heap_start = .;
        . += _heap_size;
        _heap_end = .;
    } > RAM

    /* Discard unnecessary sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }

    /* End of used RAM */
    _end = .;
}
