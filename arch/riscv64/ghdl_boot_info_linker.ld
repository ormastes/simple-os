/* RV64 GHDL boot-info proof linker script.
 *
 * Keeps the payload origin at 0x80200000, uses the normal RV64 boot entry
 * shape, and reserves a disjoint proof mailbox page at 0x80FF1000.
 */
OUTPUT_ARCH(riscv)
OUTPUT_FORMAT("elf64-littleriscv")
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x80200000, LENGTH = 126M
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

    .proof_mailbox 0x80FF1000 (NOLOAD) : ALIGN(4K) {
        __ghdl_boot_info_proof_start = .;
        . += 4K;
        __ghdl_boot_info_proof_end = .;
    } > RAM

    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . += 64K;
        _stack_top = .;
    } > RAM

    .heap (NOLOAD) : ALIGN(4K) {
        __heap_start = .;
        . += 1M;
        __heap_end = .;
    } > RAM

    _kernel_end = .;

    /DISCARD/ : {
        *(.note*)
        *(.eh_frame*)
        *(.comment)
    }
}
