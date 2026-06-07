/*
 * crt0.s -- x86_32 (i686) Multiboot1 header + 32-bit protected mode entry
 *
 * Provides:
 *   1. Multiboot1 header in .text.entry
 *   2. _entry32: sets up stack, zeroes BSS, calls _start
 *
 * Multiboot1 drops us in 32-bit protected mode with:
 *   EAX = 0x2BADB002 (magic)   EBX = multiboot info pointer
 *   Paging disabled, A20 enabled, GDT loaded (flat 32-bit)
 *
 * No long mode transition needed -- stays in 32-bit protected mode.
 */

/* ==================================================================
 * Multiboot1 header -- must be in first 8 KiB, 4-byte aligned
 * ================================================================== */
.section .text.entry, "ax"
.code32
.align 4

.set MB_MAGIC, 0x1BADB002
.set MB_FLAGS, 0x00000003

.global _multiboot_header
_multiboot_header:
    .long MB_MAGIC
    .long MB_FLAGS
    .long 0xE4524FFB          /* checksum: -(magic + flags) & 0xFFFFFFFF */

/* ==================================================================
 * 32-bit entry point
 * ================================================================== */
.global _entry32
_entry32:
    /* Disable interrupts */
    cli

    /* Preserve Multiboot handoff registers across BSS clearing. */
    movl %eax, %edx
    movl %ebx, %esi

    /* Set up 32-bit stack */
    movl $_stack_top, %esp

    /* Zero BSS */
    movl $__bss_start, %edi
    movl $__bss_end, %ecx
    subl %edi, %ecx
    shrl $2, %ecx
    xorl %eax, %eax
    rep stosl

    /* Call C _start(multiboot_magic, multiboot_info) */
    pushl %esi
    pushl %edx
    call _start

    /* Halt if it returns */
.halt:
    cli
    hlt
    jmp .halt

/* ==================================================================
 * BSS: stack
 * ================================================================== */
.section .bss
.align 4096
