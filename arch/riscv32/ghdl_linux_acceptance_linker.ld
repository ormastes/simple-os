/* RISC-V 32 linker script for the GHDL Linux acceptance payload.
 *
 * This lane does not need the large QEMU stack/heap reservation from the
 * general bare-metal linker. Keep the live image compact so the RV32 GHDL
 * testbench can model the full payload, stack, and probe window cheaply.
 */

OUTPUT_ARCH(riscv)
OUTPUT_FORMAT("elf32-littleriscv")
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 256M
}

SECTIONS {
    .text : ALIGN(4K) {
        KEEP(*(.text.entry))
        *(.text.trap)
        *(.text*)
    } > RAM

    .rodata : ALIGN(4K) {
        *(.rodata*)
        *(.srodata*)
    } > RAM

    .data : ALIGN(4K) {
        *(.data*)
        *(.sdata*)
        PROVIDE(__global_pointer$ = . + 0x800);
    } > RAM

    .bss (NOLOAD) : ALIGN(4K) {
        _sbss = .;
        *(.bss*)
        *(.sbss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . += 256K;
        _stack_top = .;
    } > RAM

    .heap (NOLOAD) : ALIGN(4K) {
        __heap_start = .;
        . += 512K;
        __heap_end = .;
    } > RAM

    _kernel_end = .;

    /DISCARD/ : {
        *(.note*)
        *(.eh_frame*)
        *(.comment)
    }
}
