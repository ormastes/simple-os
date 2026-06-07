/*
 * ap_trampoline.s -- x86_64 AP startup trampoline template.
 *
 * The BSP copies this template to SIMPLEOS_AP_TRAMPOLINE_PHYS (0x8000) before
 * sending SIPI vector 0x08. The code is written for that fixed physical
 * address so real-mode and protected-mode absolute operands remain valid after
 * the copy. Runtime patch slots for the AP-local GDT descriptor and PML4
 * physical address are filled by rt_x86_prepare_ap_startup() in
 * baremetal_stubs.c.
 */

    .set SIMPLEOS_AP_TRAMPOLINE_PHYS, 0x8000

    .section .text.ap_trampoline, "ax"
    .globl simpleos_ap_trampoline_template_start
    .globl simpleos_ap_trampoline_template_end
    .globl simpleos_ap_trampoline_gdt_desc
    .globl simpleos_ap_trampoline_gdt
    .globl simpleos_ap_trampoline_gdt_end
    .globl simpleos_ap_trampoline_pml4_phys_slot
    .type simpleos_ap_trampoline_template_start, @function

    .code16
    .align 16
simpleos_ap_trampoline_template_start:
    cli
    xorw    %ax, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    $0x7c00, %sp

    lgdt    SIMPLEOS_AP_TRAMPOLINE_PHYS + (simpleos_ap_trampoline_gdt_desc - simpleos_ap_trampoline_template_start)

    movl    %cr0, %eax
    orl     $0x1, %eax
    movl    %eax, %cr0
    ljmpl   $0x08, $SIMPLEOS_AP_TRAMPOLINE_PHYS + (simpleos_ap_trampoline_pm32 - simpleos_ap_trampoline_template_start)

    .code32
simpleos_ap_trampoline_pm32:
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

    movl    SIMPLEOS_AP_TRAMPOLINE_PHYS + (simpleos_ap_trampoline_pml4_phys_slot - simpleos_ap_trampoline_template_start), %eax
    movl    %eax, %cr3

    movl    %cr4, %eax
    orl     $0x20, %eax
    movl    %eax, %cr4

    movl    $0xC0000080, %ecx
    rdmsr
    orl     $0x100, %eax
    wrmsr

    movl    %cr0, %eax
    orl     $0x80010000, %eax
    movl    %eax, %cr0
    ljmpl   $0x18, $SIMPLEOS_AP_TRAMPOLINE_PHYS + (simpleos_ap_trampoline_lm64 - simpleos_ap_trampoline_template_start)

    .code64
simpleos_ap_trampoline_lm64:
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

    movq    %cr0, %rax
    andq    $~(1 << 2), %rax
    orq     $(1 << 1), %rax
    movq    %rax, %cr0
    movq    %cr4, %rax
    orq     $((1 << 9) | (1 << 10)), %rax
    movq    %rax, %cr4

    movabsq $simpleos_ap_boot_stack_top, %rax
    movq    (%rax), %rsp
    movabsq $simpleos_ap_entry64, %rax
    callq   *%rax

simpleos_ap_halt:
    cli
    hlt
    jmp     simpleos_ap_halt

    .align 8
simpleos_ap_trampoline_gdt_desc:
    .word   0
    .long   0

    .align 4
simpleos_ap_trampoline_pml4_phys_slot:
    .long   0

    .align 8
simpleos_ap_trampoline_gdt:
    .quad   0x0000000000000000  /* 0x00: null */
    .quad   0x00cf9a000000ffff  /* 0x08: 32-bit kernel code */
    .quad   0x00cf92000000ffff  /* 0x10: 32-bit/64-bit kernel data */
    .quad   0x00af9a000000ffff  /* 0x18: 64-bit kernel code */
simpleos_ap_trampoline_gdt_end:

    .align 16
simpleos_ap_trampoline_template_end:
    .size simpleos_ap_trampoline_template_start, . - simpleos_ap_trampoline_template_start

    .section .note.GNU-stack,"",@progbits
