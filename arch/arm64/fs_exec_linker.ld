/* ARM64 SimpleOS filesystem execution acceptance linker script.
 *
 * QEMU virt places its generated device tree at the start of RAM when using
 * the generic loader device. Keep this acceptance kernel above that reserved
 * area and let the loader set the CPU PC to ENTRY(_start).
 */

OUTPUT_ARCH(aarch64)
OUTPUT_FORMAT("elf64-littleaarch64")
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x40200000, LENGTH = 254M
}

SECTIONS {
    .text : ALIGN(4K) {
        KEEP(*(.text.entry))
        . = ALIGN(2K);
        *(.text.vectors)
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
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . += 8M;
        _stack_top = .;
    } > RAM

    .heap (NOLOAD) : ALIGN(4K) {
        __heap_start = .;
        . += 64M;
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
