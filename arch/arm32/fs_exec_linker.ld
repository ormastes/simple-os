/* ARM32 SimpleOS filesystem execution acceptance linker script.
 *
 * QEMU virt reserves the start of RAM for its generated device tree when the
 * generic loader device is used. Link this acceptance kernel above that area.
 */

OUTPUT_ARCH(arm)
OUTPUT_FORMAT("elf32-littlearm")
ENTRY(_entry_asm)

MEMORY {
    RAM (rwx) : ORIGIN = 0x40200000, LENGTH = 126M
}

SECTIONS {
    .text : ALIGN(4K) {
        KEEP(*(.text.entry))
        *(.text*)
    } > RAM

    .rodata : ALIGN(4K) {
        *(.rodata*)
    } > RAM

    .simple_sandbox : ALIGN(4K) {
        __simple_sandbox_start = .;
        KEEP(*(.simple.sandbox*))
        KEEP(*(.simple.mpu*))
        __simple_sandbox_end = .;
    } > RAM

    .data : ALIGN(4K) {
        *(.data*)
    } > RAM

    .bss (NOLOAD) : ALIGN(4K) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > RAM

    .stack (NOLOAD) : ALIGN(8) {
        _stack_bottom = .;
        . += 64K;
        _stack_top = .;
    } > RAM

    .heap (NOLOAD) : ALIGN(4K) {
        __heap_start = .;
        . += 4M;
        __heap_end = .;
    } > RAM

    _kernel_end = .;

    /DISCARD/ : {
        *(.note*)
        *(.eh_frame*)
        *(.comment)
        *(.ARM.attributes)
        *(.ARM.exidx*)
    }
}
