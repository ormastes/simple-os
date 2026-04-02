/* RISC-V Linker Script
 *
 * Memory Layout (QEMU virt machine):
 *   RAM: 0x80000000 - 0x87FFFFFF (128MB default)
 *
 * For real hardware (e.g., SiFive HiFive1):
 *   FLASH: 0x20000000 - 0x203FFFFF (4MB)
 *   RAM:   0x80000000 - 0x8000FFFF (64KB)
 *
 * Adjust memory regions for your target.
 */

OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY
{
    /* QEMU virt: All RAM */
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 128M

    /* For real hardware with flash:
     * FLASH (rx) : ORIGIN = 0x20000000, LENGTH = 4M
     * RAM (rwx)  : ORIGIN = 0x80000000, LENGTH = 64K
     */
}

SECTIONS
{
    /* Boot code starts at RAM base */
    . = 0x80000000;

    /* Entry point and boot code */
    .text.boot :
    {
        KEEP(*(.text.boot))
        . = ALIGN(4);
    } > RAM

    /* Read-only code */
    .text :
    {
        *(.text)
        *(.text.*)
        . = ALIGN(4);
    } > RAM

    /* Read-only data */
    .rodata :
    {
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
    } > RAM

    /* Initialized data */
    .data :
    {
        . = ALIGN(8);
        _sdata = .;
        *(.data)
        *(.data.*)
        *(.sdata)
        *(.sdata.*)
        . = ALIGN(8);
        _edata = .;
    } > RAM

    /* Uninitialized data */
    .bss :
    {
        . = ALIGN(8);
        _sbss = .;
        *(.bss)
        *(.bss.*)
        *(.sbss)
        *(.sbss.*)
        *(COMMON)
        . = ALIGN(8);
        _ebss = .;
    } > RAM

    /* Heap (grows upward) */
    .heap :
    {
        . = ALIGN(16);
        _heap_start = .;
        . = . + 1M;  /* 1MB heap */
        _heap_end = .;
    } > RAM

    /* Stack (per-hart, grows downward) */
    .stack (NOLOAD) :
    {
        . = ALIGN(16);
        _stack_bottom = .;
        . = . + 256K;  /* 256KB total (64KB per hart × 4 harts) */
        _stack_top = .;
    } > RAM

    /* Discard unwanted sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.*)
        *(.riscv.attributes)
    }
}

/* Symbols for startup code */
PROVIDE(_ram_start = ORIGIN(RAM));
PROVIDE(_ram_size = LENGTH(RAM));
