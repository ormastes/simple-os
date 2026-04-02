/* ARM Cortex-M Linker Script
 *
 * Memory Layout (STM32F407 Discovery):
 *   FLASH: 0x08000000 - 0x080FFFFF (1MB)
 *   SRAM:  0x20000000 - 0x2001FFFF (128KB)
 *   CCM:   0x10000000 - 0x1000FFFF (64KB, Cortex-M4 only)
 *
 * Adjust memory sizes for your specific MCU.
 */

ENTRY(reset_handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    /* Vector table at start of flash */
    .vector_table :
    {
        KEEP(*(.vector_table))
        . = ALIGN(4);
    } > FLASH

    /* Read-only code */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
    } > FLASH

    /* ARM exception tables */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH

    /* Initialized data (copied from flash to RAM at startup) */
    _sidata = LOADADDR(.data);

    .data : AT(_sidata)
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM

    /* Uninitialized data (zeroed at startup) */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    /* Heap (grows upward) */
    .heap :
    {
        . = ALIGN(8);
        _heap_start = .;
        . = . + 16K;  /* 16KB heap */
        _heap_end = .;
    } > SRAM

    /* Stack (grows downward from top of RAM) */
    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        _stack_bottom = .;
        . = ORIGIN(SRAM) + LENGTH(SRAM);
        _stack_top = .;
    } > SRAM

    /* Discard debugging sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.ARM.attributes)
    }
}

/* Provide stack pointer value to reset handler */
PROVIDE(_initial_sp = _stack_top);
