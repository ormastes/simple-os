/*
 * crt0.s -- ARM32 (ARMv7) startup for QEMU virt
 *
 * Sets up stack, zeroes BSS, then calls _start (C entry point).
 * Runs in Supervisor mode (SVC).
 */

    .section .text.entry, "ax", %progbits
    .arm
    .globl _entry_asm
    .type _entry_asm, %function

_entry_asm:
    /* Early UART marker before stack/BSS setup. */
    ldr r0, =.Lcrt0_banner
    bl .Lserial_puts_early

    /* Disable interrupts (IRQ + FIQ) */
    cpsid if

    /* Set up stack pointer from linker symbol */
    ldr sp, =_stack_top

    /* Zero BSS section */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
1:
    cmp r0, r1
    strlt r2, [r0], #4
    blt 1b

    /* Call C _start */
    bl _start

    /* Halt: infinite WFE loop */
2:
    wfe
    b 2b

    .size _entry_asm, . - _entry_asm

.Lserial_puts_early:
    ldr r1, =0x09000000
.Lserial_puts_early_loop:
    ldrb r2, [r0], #1
    cmp r2, #0
    beq .Lserial_puts_early_done
    str r2, [r1]
    b .Lserial_puts_early_loop
.Lserial_puts_early_done:
    bx lr

.Lcrt0_banner:
    .asciz "[BOOT] ARM32 crt0 entered\r\n"
