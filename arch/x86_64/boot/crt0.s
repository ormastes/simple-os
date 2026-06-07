/*
 * crt0.s -- x86_64 Multiboot1 header + 32→64 bit mode switch
 *
 * Provides:
 *   1. Multiboot1 header (with framebuffer request) in .text.entry
 *   2. _start entry: sets up identity-mapped page tables, enables
 *      long mode (64-bit), loads a 64-bit GDT, jumps to spl_start
 *
 * Multiboot1 drops us in 32-bit protected mode with:
 *   EAX = 0x2BADB002 (magic)   EBX = multiboot info pointer
 *   Paging disabled, A20 enabled, GDT loaded (flat 32-bit)
 *
 * We identity-map the first 2 GiB with 2 MiB pages so the kernel
 * (loaded at 1 MiB) and all MMIO regions are accessible.
 */

/* ==================================================================
 * Multiboot1 header -- must be in first 8 KiB, 4-byte aligned
 * ================================================================== */
.section .text.entry, "ax"
.code32
.align 4

.set MB_MAGIC, 0x1BADB002
.set MB_FLAGS, 0x00000007

.global _multiboot_header
_multiboot_header:
    .long MB_MAGIC
    .long MB_FLAGS
    .long 0xE4524FF7          /* checksum: -(magic + flags) & 0xFFFFFFFF */
    .long 0                   /* video mode: linear graphics */
    .long 1024                /* framebuffer width */
    .long 768                 /* framebuffer height */
    .long 32                  /* framebuffer depth */

/* ==================================================================
 * 32-bit entry point
 * ================================================================== */
.global _entry32
_entry32:
    /* Disable interrupts */
    cli

    /* Save multiboot info (EBX) on the stack later -- preserve in ESI */
    movl %ebx, %esi

    /* Set up a temporary 32-bit stack */
    movl $_stack_top, %esp

    call boot32_serial_init
    movl $boot32_entry_msg, %esi
    call boot32_serial_puts

    /* Clear BSS */
    movl $__bss_start, %edi
    movl $__bss_end, %ecx
    subl %edi, %ecx
    shrl $2, %ecx
    xorl %eax, %eax
    rep stosl

    /* ------------------------------------------------------------------
     * Build identity-mapped page tables (2 MiB pages, first 2 GiB)
     *
     * PML4[0] -> PDPT
     * PDPT[0..3] -> PD[0..3]  (4 GiB identity map)
     * PD[0..2047] -> 2 MiB identity pages
     * PML4[1]/PDPT[256] -> high PD for OVMF/q35 MMIO near 0xC000000000
     * Maps full 4 GiB to cover VGA framebuffer at 0xFD000000.
     * ------------------------------------------------------------------ */

    /* Zero the page-table area (PML4 4K + PDPT 4K + PD 16K + high PDPT/PD 8K = 32 KiB) */
    movl $boot_pml4, %edi
    movl $8192, %ecx          /* 32768 / 4 = 8192 dwords */
    xorl %eax, %eax
    rep stosl

    /* PML4[0] -> PDPT | present | writable */
    movl $boot_pml4, %edi
    movl $boot_pdpt, %eax
    orl  $0x03, %eax           /* P | RW */
    movl %eax, (%edi)

    /* PDPT[0] -> PD+0x0000 (0-1 GiB) */
    movl $boot_pdpt, %edi
    movl $boot_pd, %eax
    orl  $0x03, %eax
    movl %eax, (%edi)

    /* PDPT[1] -> PD+0x1000 (1-2 GiB) */
    movl $boot_pd, %eax
    addl $0x1000, %eax
    orl  $0x03, %eax
    movl %eax, 8(%edi)

    /* PDPT[2] -> PD+0x2000 (2-3 GiB) */
    movl $boot_pd, %eax
    addl $0x2000, %eax
    orl  $0x03, %eax
    movl %eax, 16(%edi)

    /* PDPT[3] -> PD+0x3000 (3-4 GiB) */
    movl $boot_pd, %eax
    addl $0x3000, %eax
    orl  $0x03, %eax
    movl %eax, 24(%edi)

    /* Fill PD[0..2047] with 2 MiB identity pages (4 GiB total)
     * Each entry: addr | PS (bit 7) | RW | P = addr | 0x83 */
    movl $boot_pd, %edi
    movl $0x00000083, %eax     /* 0 MiB | PS | RW | P */
    movl $2048, %ecx
.fill_pd:
    movl %eax, (%edi)
    movl $0, 4(%edi)           /* high 32 bits = 0 */
    addl $0x200000, %eax       /* next 2 MiB */
    addl $8, %edi
    decl %ecx
    jnz .fill_pd

    /* OVMF on q35 can assign PCIe MMIO BARs above 4 GiB. Identity-map
     * 0xC000000000..0xC03FFFFFFF so the early NVMe C path can touch BAR0
     * before the full VMM installs dynamic MMIO mappings. */
    movl $boot_pml4, %edi
    movl $boot_high_pdpt, %eax
    orl  $0x03, %eax
    movl %eax, 8(%edi)         /* PML4[1] */

    movl $boot_high_pdpt, %edi
    movl $boot_high_pd, %eax
    orl  $0x03, %eax
    movl %eax, 2048(%edi)      /* PDPT[256] */

    movl $boot_high_pd, %edi
    movl $0x00000083, %eax     /* low 32 bits: 0xC000000000 | PS | RW | P */
    movl $0x000000c0, %edx     /* high 32 bits */
    movl $512, %ecx
.fill_high_pd:
    movl %eax, (%edi)
    movl %edx, 4(%edi)
    addl $0x200000, %eax
    addl $8, %edi
    decl %ecx
    jnz .fill_high_pd

    /* ------------------------------------------------------------------
     * Enable long mode
     * ------------------------------------------------------------------ */

    /* Load PML4 into CR3 */
    movl $boot_pml4, %eax
    movl %eax, %cr3

    /* Enable PAE (CR4.PAE = bit 5) */
    movl %cr4, %eax
    orl  $0x20, %eax
    movl %eax, %cr4

    /* Set LME (Long Mode Enable) in IA32_EFER MSR (0xC0000080) bit 8 */
    movl $0xC0000080, %ecx
    rdmsr
    orl  $0x100, %eax
    wrmsr

    /* Enable paging (CR0.PG = bit 31) + write protect (CR0.WP = bit 16) */
    movl %cr0, %eax
    orl  $0x80010000, %eax
    movl %eax, %cr0

    /* Now in 32-bit compatibility mode (long mode but CS is still 32-bit).
     * Load 64-bit GDT and far-jump to 64-bit code segment. */
    lgdt (gdt64_ptr)
    ljmp $0x08, $long_mode_entry

boot32_serial_init:
    movw $0x3f9, %dx
    xorb %al, %al
    outb %al, %dx
    movw $0x3fb, %dx
    movb $0x80, %al
    outb %al, %dx
    movw $0x3f8, %dx
    movb $0x03, %al
    outb %al, %dx
    movw $0x3f9, %dx
    xorb %al, %al
    outb %al, %dx
    movw $0x3fb, %dx
    movb $0x03, %al
    outb %al, %dx
    movw $0x3fa, %dx
    movb $0xc7, %al
    outb %al, %dx
    movw $0x3fc, %dx
    movb $0x0b, %al
    outb %al, %dx
    ret

boot32_serial_puts:
    lodsb
    testb %al, %al
    jz 2f
1:
    movw $0x3fd, %dx
    inb %dx, %al
    testb $0x20, %al
    jz 1b
    movw $0x3f8, %dx
    movb -1(%esi), %al
    outb %al, %dx
    jmp boot32_serial_puts
2:
    ret

boot32_entry_msg:
    .asciz "[BOOT32] entry\r\n"
boot64_entry_msg:
    .asciz "[BOOT64] entry\r\n"
boot64_idt_msg:
    .asciz "[BOOT64] idt\r\n"
boot64_start_msg:
    .asciz "[BOOT64] call _start\r\n"

/* ==================================================================
 * 64-bit code
 * ================================================================== */
.code64
boot64_serial_puts:
    lodsb
    testb %al, %al
    jz 2f
1:
    movw $0x3fd, %dx
.boot64_wait:
    inb %dx, %al
    testb $0x20, %al
    jz .boot64_wait
    movw $0x3f8, %dx
    movb -1(%rsi), %al
    outb %al, %dx
    jmp boot64_serial_puts
2:
    ret

long_mode_entry:
    movq $boot64_entry_msg, %rsi
    call boot64_serial_puts

    /* Reload data segments with 64-bit data selector */
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    /* Enable SSE/SSE2 — Cranelift generates xmm instructions (movq, xorpd, etc.)
     * Without this, any SSE instruction triggers #UD or #NM fault.
     *   CR0: clear EM (bit 2), set MP (bit 1)
     *   CR4: set OSFXSR (bit 9), set OSXMMEXCPT (bit 10) */
    movq %cr0, %rax
    andq $~(1 << 2), %rax          /* clear CR0.EM */
    orq  $(1 << 1), %rax           /* set CR0.MP   */
    movq %rax, %cr0
    movq %cr4, %rax
    orq  $((1 << 9) | (1 << 10)), %rax  /* set CR4.OSFXSR | CR4.OSXMMEXCPT */
    movq %rax, %cr4

    /* Set up 64-bit stack */
    movq $_stack_top, %rsp

    /* Install minimal IDT — catches faults from stubbed function calls.
     * All 256 vectors point to _fault_handler which returns 0 in RAX.
     */
    leaq _idt(%rip), %rdi
    leaq _fault_handler(%rip), %rsi
    movl $256, %ecx
.fill_idt:
    movq %rsi, %rax
    movw %ax, (%rdi)              /* offset 15:0  */
    movw $0x08, 2(%rdi)           /* selector: code64 */
    movb $0, 4(%rdi)              /* IST = 0 */
    movb $0x8E, 5(%rdi)           /* type: interrupt gate, DPL=0, present */
    shrq $16, %rax
    movw %ax, 6(%rdi)             /* offset 31:16 */
    shrq $16, %rax
    movl %eax, 8(%rdi)            /* offset 63:32 */
    movl $0, 12(%rdi)             /* reserved */
    addq $16, %rdi
    decl %ecx
    jnz .fill_idt

    leaq _idt(%rip), %rax
    movw $4095, _idt_ptr(%rip)    /* limit: 256*16-1 */
    movq %rax, _idt_ptr+2(%rip)   /* base */
    lidt _idt_ptr(%rip)
    movq $boot64_idt_msg, %rsi
    call boot64_serial_puts

    /* Run Simple module-global initializers before the entry point.
     * Freestanding builds have no C main wrapper to call this, so do it here.
     * Weak: skip if the linker didn't provide an aggregator. Preserve the
     * multiboot info pointer (ESI) in RBX (callee-saved) across the call. */
    .weak __simple_call_module_inits
    movl %esi, %ebx
    leaq __simple_call_module_inits(%rip), %rax
    testq %rax, %rax
    jz .skip_module_inits
    call __simple_call_module_inits
.skip_module_inits:
    movl %ebx, %esi

    /* Pass multiboot info pointer as first arg (rdi) -- zero-extend ESI */
    movl %esi, %edi

    /* Call Simple compiler entry point */
    movq $boot64_start_msg, %rsi
    call boot64_serial_puts
    call _start

    /* Halt if it returns */
.halt64:
    cli
    hlt
    jmp .halt64

/* Recoverable fault handler: prints fault address and returns RAX=0x3 (nil).
 *
 * Prints "FAULT @ 0x<RIP>\r\n" to COM1 (first 32 faults), then recovers by
 * setting RAX=0x3 (tagged nil) and advancing RIP past the faulting insn.
 * This lets the test suite continue past stubbed/unresolved function calls.
 *
 * Stack on entry (no error code): [RIP, CS, RFLAGS, RSP, SS]
 * Stack on entry (with error code): [errcode, RIP, CS, RFLAGS, RSP, SS]
 * We detect error code by checking if value at +8(%rsp) == 0x08 (CS).
 */
.align 16
_fault_handler:
    /* Count faults; only print first 32 to keep output manageable */
    pushq %rax
    leaq _fault_count(%rip), %rax
    lock incq (%rax)
    cmpq $32, (%rax)
    popq %rax
    ja .fault_recover_silent

    /* Print "FAULT @ 0x" to COM1 (port 0x3F8) */
    pushq %rax
    pushq %rdx
    pushq %rcx
    pushq %rsi

    /* Wait for UART TX ready then send char — inline for each byte */
    movw $0x3FD, %dx
.fw0: inb %dx, %al
    testb $0x20, %al
    jz .fw0
    movw $0x3F8, %dx
    movb $'F', %al
    outb %al, %dx
    movw $0x3FD, %dx
.fw1: inb %dx, %al
    testb $0x20, %al
    jz .fw1
    movw $0x3F8, %dx
    movb $'A', %al
    outb %al, %dx
    movw $0x3FD, %dx
.fw2: inb %dx, %al
    testb $0x20, %al
    jz .fw2
    movw $0x3F8, %dx
    movb $'U', %al
    outb %al, %dx
    movw $0x3FD, %dx
.fw3: inb %dx, %al
    testb $0x20, %al
    jz .fw3
    movw $0x3F8, %dx
    movb $'L', %al
    outb %al, %dx
    movw $0x3FD, %dx
.fw4: inb %dx, %al
    testb $0x20, %al
    jz .fw4
    movw $0x3F8, %dx
    movb $'T', %al
    outb %al, %dx
    movw $0x3FD, %dx
.fw5: inb %dx, %al
    testb $0x20, %al
    jz .fw5
    movw $0x3F8, %dx
    movb $' ', %al
    outb %al, %dx
    movw $0x3FD, %dx
.fw6: inb %dx, %al
    testb $0x20, %al
    jz .fw6
    movw $0x3F8, %dx
    movb $'@', %al
    outb %al, %dx
    movw $0x3FD, %dx
.fw7: inb %dx, %al
    testb $0x20, %al
    jz .fw7
    movw $0x3F8, %dx
    movb $' ', %al
    outb %al, %dx
    movw $0x3FD, %dx
.fw8: inb %dx, %al
    testb $0x20, %al
    jz .fw8
    movw $0x3F8, %dx
    movb $'0', %al
    outb %al, %dx
    movw $0x3FD, %dx
.fw9: inb %dx, %al
    testb $0x20, %al
    jz .fw9
    movw $0x3F8, %dx
    movb $'x', %al
    outb %al, %dx

    /* Get faulting RIP: check for error code.
     * Stack: [saved rsi, saved rcx, saved rdx, saved rax, RIP-or-errcode, ...]
     * +32(%rsp) = first frame value. If +40(%rsp) == 0x08, no error code.
     * Otherwise +40(%rsp) is errcode and +48(%rsp) has CS. */
    cmpq $0x08, 40(%rsp)
    je .fh_no_errcode
    movq 40(%rsp), %rsi           /* RIP (after error code) */
    jmp .fh_print_rip
.fh_no_errcode:
    movq 32(%rsp), %rsi           /* RIP (no error code) */

.fh_print_rip:
    /* Print RSI as 16 hex digits to COM1 */
    movl $16, %ecx
.fh_hex_loop:
    rolq $4, %rsi                 /* rotate left 4 bits (MSB first) */
    movq %rsi, %rax
    andq $0x0F, %rax
    cmpb $10, %al
    jb .fh_digit
    addb $('a' - 10), %al
    jmp .fh_send
.fh_digit:
    addb $'0', %al
.fh_send:
    movw $0x3FD, %dx
.fh_wait: inb %dx, %al
    testb $0x20, %al
    jz .fh_wait
    movw $0x3F8, %dx
    /* Recalculate the digit (inb clobbered %al) */
    movq %rsi, %rax
    andq $0x0F, %rax
    cmpb $10, %al
    jb .fh_digit2
    addb $('a' - 10), %al
    jmp .fh_out
.fh_digit2:
    addb $'0', %al
.fh_out:
    outb %al, %dx
    decl %ecx
    jnz .fh_hex_loop

    /* Print \r\n */
    movw $0x3FD, %dx
.fhcr: inb %dx, %al
    testb $0x20, %al
    jz .fhcr
    movw $0x3F8, %dx
    movb $'\r', %al
    outb %al, %dx
    movw $0x3FD, %dx
.fhlf: inb %dx, %al
    testb $0x20, %al
    jz .fhlf
    movw $0x3F8, %dx
    movb $'\n', %al
    outb %al, %dx

    popq %rsi
    popq %rcx
    popq %rdx
    popq %rax

    /* Fall through to recovery */

.fault_recover_silent:
    /* Recovery: set RAX=0x3 (nil tagged value) and advance RIP.
     * For ud2 (0x0F 0x0B), advance by 2 bytes.
     * For other faults, advance by 2 (best effort).
     * Determine if error code was pushed (check CS at expected position). */
    cmpq $0x08, 8(%rsp)
    je .fr_no_errcode

    /* Error code present: stack is [errcode, RIP, CS, RFLAGS, RSP, SS] */
    addq $2, 8(%rsp)                /* advance RIP by 2 */
    movq $0x3, %rax                 /* RAX = nil (tagged special 0x3) */
    addq $8, %rsp                   /* pop error code */
    iretq

.fr_no_errcode:
    /* No error code: stack is [RIP, CS, RFLAGS, RSP, SS] */
    addq $2, (%rsp)                 /* advance RIP by 2 */
    movq $0x3, %rax                 /* RAX = nil (tagged special 0x3) */
    iretq

/* Runtime W^X probe.
 *
 * Converts the first 2 MiB identity huge-page mapping into 4 KiB pages, clears
 * RW on this probe's code page, attempts a write to that page, then restores
 * RW. The existing recoverable #PF handler advances past the two-byte faulting
 * store, so success is measured by the fault counter increasing.
 */
.global rt_harden_text_write_trap_probe
.type rt_harden_text_write_trap_probe, @function
rt_harden_text_write_trap_probe:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12

    movq _fault_count(%rip), %r8

    leaq boot_text_pt(%rip), %rdi
    xorq %rcx, %rcx
    xorq %rax, %rax
.harden_fill_pt:
    movq %rax, %rdx
    orq $0x3, %rdx                 /* P | RW */
    movq %rdx, (%rdi,%rcx,8)
    addq $0x1000, %rax
    incq %rcx
    cmpq $512, %rcx
    jne .harden_fill_pt

    leaq .harden_text_probe_target(%rip), %rax
    movq %rax, %r12
    andq $0x1fffff, %r12
    shrq $12, %r12                 /* PT index within first 2 MiB */

    movq (%rdi,%r12,8), %rbx
    andq $~0x2, %rbx               /* clear RW for the code page */
    movq %rbx, (%rdi,%r12,8)

    leaq boot_text_pt(%rip), %rbx
    orq $0x3, %rbx
    movq %rbx, boot_pd(%rip)       /* replace first huge PDE with PT */
    invlpg (%rax)

.harden_text_probe_target:
    movb %al, (%rax)               /* 2-byte write; #PF should skip it */

    movq (%rdi,%r12,8), %rbx
    orq $0x2, %rbx
    movq %rbx, (%rdi,%r12,8)
    invlpg (%rax)

    xorq %rax, %rax
    cmpq %r8, _fault_count(%rip)
    jbe .harden_probe_done
    movq $1, %rax
.harden_probe_done:
    popq %r12
    popq %rbx
    popq %rbp
    ret

/* ==================================================================
 * 64-bit GDT (5 entries for SYSCALL/SYSRET support)
 *
 * Selector layout required by MSR_STAR (Intel SDM Vol 3A §5.8.8):
 *   0x00: null
 *   0x08: kernel CS  — SYSCALL loads CS=0x08, SS=0x10
 *   0x10: kernel SS/DS
 *   0x18: user CS 32-bit compat — SYSRET compat loads CS=0x1B (0x18|3)
 *   0x20: user SS/DS            — SYSRET loads SS=0x23 (0x20|3)
 *   0x28: user CS 64-bit        — SYSRET 64-bit loads CS=0x2B (0x28|3)
 *
 * MSR_STAR value: (USER_CS_32 | 3) << 48 | KERNEL_CS << 32
 *              = 0x001B_0008_0000_0000
 * ================================================================== */
.section .rodata
.align 16
gdt64:
    .quad 0                    /* 0x00: null descriptor */

    /* 0x08: kernel CS — 64-bit code segment, DPL=0 */
    .word 0xFFFF               /* limit low */
    .word 0x0000               /* base low */
    .byte 0x00                 /* base mid */
    .byte 0x9A                 /* access: present, DPL=0, code, exec/read */
    .byte 0xAF                 /* flags: G=1, L=1 (64-bit); limit hi=0xF */
    .byte 0x00                 /* base high */

    /* 0x10: kernel SS/DS — 64-bit data segment, DPL=0 */
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0x92                 /* access: present, DPL=0, data, read/write */
    .byte 0xCF                 /* flags: G=1, D=1 (ignored in 64-bit mode) */
    .byte 0x00

    /* 0x18: user CS 32-bit compat — SYSRET compat target, DPL=3 */
    .word 0xFFFF               /* limit low */
    .word 0x0000               /* base low */
    .byte 0x00                 /* base mid */
    .byte 0xFA                 /* access: present, DPL=3, code, exec/read */
    .byte 0xCF                 /* flags: G=1, D=1, L=0 (32-bit compat) */
    .byte 0x00                 /* base high */

    /* 0x20: user SS/DS — user data segment, DPL=3 */
    .word 0xFFFF               /* limit low */
    .word 0x0000               /* base low */
    .byte 0x00                 /* base mid */
    .byte 0xF2                 /* access: present, DPL=3, data, read/write */
    .byte 0xCF                 /* flags: G=1, D=1 */
    .byte 0x00                 /* base high */

    /* 0x28: user CS 64-bit — SYSRET 64-bit target, DPL=3 */
    .word 0xFFFF               /* limit low */
    .word 0x0000               /* base low */
    .byte 0x00                 /* base mid */
    .byte 0xFA                 /* access: present, DPL=3, code, exec/read */
    .byte 0xAF                 /* flags: G=1, L=1, D=0 (64-bit) */
    .byte 0x00                 /* base high */

gdt64_end:

.global gdt64_ptr
gdt64_ptr:
    .word gdt64_end - gdt64 - 1  /* limit */
    .long gdt64                   /* base (32-bit address, fine for < 4 GiB) */

/* ==================================================================
 * IDT — 256 entries * 16 bytes = 4 KiB, in BSS
 * ================================================================== */
.section .bss
.align 8
_fault_count:
    .quad 0

.align 4096
_idt:
    .space 4096

.section .data
.align 16
_idt_ptr:
    .word 0        /* limit (filled at runtime) */
    .quad 0        /* base  (filled at runtime) */

/* ==================================================================
 * Page tables -- 4K-aligned, in BSS
 * ================================================================== */
.section .bss
.align 4096
.global boot_pml4
boot_pml4:
    .space 4096
boot_pdpt:
    .space 4096
boot_pd:
    .space 16384   /* 2048 entries * 8 bytes = 16 KiB for 4 GiB identity map */
boot_high_pdpt:
    .space 4096
boot_high_pd:
    .space 4096    /* 512 entries * 8 bytes = 4 KiB for 1 GiB high-MMIO map */
boot_text_pt:
    .space 4096    /* 512 4 KiB entries for runtime W^X text probe */
