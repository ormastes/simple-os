/*
 * crt0.S -- ARM64 (AArch64) startup for QEMU virt
 *
 * Sets up stack, zeroes BSS, installs exception vector table,
 * then calls _start in C (baremetal_stubs.c) which initializes
 * PL011 UART and calls spl_start (Simple entry point).
 *
 * Runs at EL1 (or EL2 if QEMU drops us there — we don't switch).
 */

    .section .text.entry, "ax", %progbits
    .globl _start
    .type _start, @function

_start:
    /* Early UART marker before stack/BSS setup. This keeps ARM64 QEMU smoke
     * failures diagnosable even if the C runtime handoff faults.
     */
    adr     x0, .Lcrt0_banner
    bl      .Lserial_puts_early

    /* Check current EL — if EL2, drop to EL1 */
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #2
    b.ne    .Lel1_setup

    /* --- Running at EL2: configure EL1 and drop down --- */
    /* HCR_EL2: RW=1 (AArch64 at EL1), no traps */
    mov     x0, #(1 << 31)     /* RW bit */
    msr     hcr_el2, x0

    /* CPTR_EL2: do not trap EL1 FP/SIMD instructions. */
    msr     cptr_el2, xzr

    /* SCTLR_EL1: disable MMU, caches, alignment check */
    mov     x0, #0
    msr     sctlr_el1, x0

    /* Set SPSR_EL2: return to EL1h (EL1 with SP_EL1) */
    mov     x0, #5              /* M[3:0]=0b0101 = EL1h */
    msr     spsr_el2, x0

    /* Set ELR_EL2 to continue at .Lel1_setup */
    adr     x0, .Lel1_setup
    msr     elr_el2, x0
    eret

.Lel1_setup:
    /* Disable MMU, caches, and alignment check in SCTLR_EL1 */
    mrs     x0, sctlr_el1
    bic     x0, x0, #(1 << 0)  /* Clear M bit (MMU off) */
    bic     x0, x0, #(1 << 1)  /* Clear A bit (alignment check off) */
    bic     x0, x0, #(1 << 2)  /* Clear C bit (data cache off) */
    bic     x0, x0, #(1 << 12) /* Clear I bit (instruction cache off) */
    msr     sctlr_el1, x0
    isb
    adr     x0, .Lcrt0_sctlr
    bl      .Lserial_puts_early

    /* LLVM may emit SIMD loads/stores for struct initialization even in
     * freestanding builds. Enable EL1 FP/SIMD access before C/Simple code.
     */
    mrs     x0, cpacr_el1
    orr     x0, x0, #(3 << 20)
    msr     cpacr_el1, x0
    isb

    /* Set up stack pointer from linker symbol */
    adrp    x0, _stack_top
    add     sp, x0, :lo12:_stack_top
    adr     x0, .Lcrt0_stack
    bl      .Lserial_puts_early

    /* Zero BSS section */
    adrp    x0, _sbss
    add     x0, x0, :lo12:_sbss
    adrp    x1, _ebss
    add     x1, x1, :lo12:_ebss
.Lbss_loop:
    cmp     x0, x1
    b.ge    .Lbss_done
    str     xzr, [x0], #8
    b       .Lbss_loop
.Lbss_done:
    adr     x0, .Lcrt0_bss
    bl      .Lserial_puts_early

    /* Install exception vector table */
    adrp    x0, _vectors
    add     x0, x0, :lo12:_vectors
    msr     vbar_el1, x0
    isb
    adr     x0, .Lcrt0_vectors
    bl      .Lserial_puts_early

    /* Call C _start (PL011 init + spl_start) */
    bl      _c_start

    /* Halt: infinite WFE loop */
.Lhalt:
    wfe
    b       .Lhalt

    .size _start, . - _start

.Lserial_puts_early:
    ldr     x1, =0x09000000
.Lserial_puts_early_loop:
    ldrb    w2, [x0], #1
    cbz     w2, .Lserial_puts_early_done
    str     w2, [x1]
    b       .Lserial_puts_early_loop
.Lserial_puts_early_done:
    ret

.Lcrt0_banner:
    .asciz  "[BOOT] ARM64 crt0 entered\r\n"
.Lcrt0_sctlr:
    .asciz  "[BOOT] ARM64 sctlr ok\r\n"
.Lcrt0_stack:
    .asciz  "[BOOT] ARM64 stack ok\r\n"
.Lcrt0_bss:
    .asciz  "[BOOT] ARM64 bss ok\r\n"
.Lcrt0_vectors:
    .asciz  "[BOOT] ARM64 vectors ok\r\n"

/* ===================================================================
 * Exception Vector Table — 16 entries, 128 bytes each, 2KB aligned
 *
 * On any exception, prints "FAULT @ <ELR_EL1>" to PL011 serial
 * then halts with WFE loop.
 * =================================================================== */

    .section .text.vectors, "ax", %progbits
    .balign 2048
    .globl _vectors
_vectors:
    /* Current EL with SP0 — Synchronous */
    b       _alignment_handler
    .balign 128
    /* Current EL with SP0 — IRQ */
    b       _fault_handler
    .balign 128
    /* Current EL with SP0 — FIQ */
    b       _fault_handler
    .balign 128
    /* Current EL with SP0 — SError */
    b       _fault_handler
    .balign 128

    /* Current EL with SPx — Synchronous */
    b       _alignment_handler
    .balign 128
    /* Current EL with SPx — IRQ */
    b       _fault_handler
    .balign 128
    /* Current EL with SPx — FIQ */
    b       _fault_handler
    .balign 128
    /* Current EL with SPx — SError */
    b       _fault_handler
    .balign 128

    /* Lower EL using AArch64 — Synchronous */
    b       _lower_el_aarch64_sync_handler
    .balign 128
    /* Lower EL using AArch64 — IRQ */
    b       _fault_handler
    .balign 128
    /* Lower EL using AArch64 — FIQ */
    b       _fault_handler
    .balign 128
    /* Lower EL using AArch64 — SError */
    b       _fault_handler
    .balign 128

    /* Lower EL using AArch32 — Synchronous */
    b       _fault_handler
    .balign 128
    /* Lower EL using AArch32 — IRQ */
    b       _fault_handler
    .balign 128
    /* Lower EL using AArch32 — FIQ */
    b       _fault_handler
    .balign 128
    /* Lower EL using AArch32 — SError */
    b       _fault_handler

/* ===================================================================
 * Alignment fault emulator — handles unaligned LDR Xt,[literal]
 *
 * Cranelift may emit 64-bit literal loads from 4-byte aligned
 * addresses. ARMv8 requires 8-byte alignment for LDR Xt.
 * This handler detects alignment faults from LDR (literal),
 * performs two 32-bit reads, combines them, writes to the
 * destination register, and returns.
 * =================================================================== */

_alignment_handler:
    /* Save ALL general-purpose registers to stack frame (33 * 8 = 264 bytes) */
    sub     sp, sp, #272        /* 34 * 8, 16-byte aligned */
    stp     x0,  x1,  [sp, #0]
    stp     x2,  x3,  [sp, #16]
    stp     x4,  x5,  [sp, #32]
    stp     x6,  x7,  [sp, #48]
    stp     x8,  x9,  [sp, #64]
    stp     x10, x11, [sp, #80]
    stp     x12, x13, [sp, #96]
    stp     x14, x15, [sp, #112]
    stp     x16, x17, [sp, #128]
    stp     x18, x19, [sp, #144]
    stp     x20, x21, [sp, #160]
    stp     x22, x23, [sp, #176]
    stp     x24, x25, [sp, #192]
    stp     x26, x27, [sp, #208]
    stp     x28, x29, [sp, #224]
    str     x30,       [sp, #240]

    /* Check if this is an alignment fault */
    mrs     x0, esr_el1
    and     x1, x0, #0x3F      /* DFSC */
    cmp     x1, #0x21           /* alignment fault */
    b.ne    .Lnot_align_fault

    /* Check WnR bit 6: 0=read, 1=write */
    tst     x0, #(1 << 6)
    b.ne    .Lalign_write

    /* --- READ alignment fault emulation --- */

    /* Get faulting instruction */
    mrs     x2, elr_el1
    ldr     w3, [x2]            /* fetch the 32-bit instruction */

    /* Use the Fault Address Register to get the exact address */
    mrs     x11, far_el1           /* faulting data address */

    /* Perform an unaligned 64-bit load using two 32-bit loads.
     * The faulting address in FAR_EL1 is the data address.
     * For ANY 64-bit load that faults with alignment, we can:
     * - Read the target address (FAR_EL1)
     * - Load 8 bytes using byte-by-byte reads
     * - Put the result into the destination register
     * - Advance PC past the faulting instruction
     */

    /* Byte-by-byte read of 8 bytes from the faulting address.
     * This avoids any alignment requirement. */
    ldrb    w12, [x11, #0]
    ldrb    w13, [x11, #1]
    orr     w12, w12, w13, lsl #8
    ldrb    w13, [x11, #2]
    orr     w12, w12, w13, lsl #16
    ldrb    w13, [x11, #3]
    orr     w12, w12, w13, lsl #24
    /* w12 now has low 32 bits */
    mov     x14, x12               /* save low word in x14 */

    ldrb    w12, [x11, #4]
    ldrb    w13, [x11, #5]
    orr     w12, w12, w13, lsl #8
    ldrb    w13, [x11, #6]
    orr     w12, w12, w13, lsl #16
    ldrb    w13, [x11, #7]
    orr     w12, w12, w13, lsl #24
    /* w12 now has high 32 bits */
    orr     x12, x14, x12, lsl #32 /* combined 64-bit value in x12 */

    /* Decode the faulting instruction to find Rt */
    ldr     w3, [x2]               /* re-read instruction */
    and     w10, w3, #0x1F         /* Rt = bits [4:0] */

    /* Write the loaded value into the saved register frame
     * x10 = Rt, x12 = value, saved regs at sp+0 = x0, sp+8 = x1, etc. */
    cmp     w10, #30
    b.hi    .Lnot_align_fault      /* Rt > 30 invalid */

    /* Store value at sp + Rt*8 */
    lsl     x10, x10, #3           /* byte offset = Rt * 8 */
    str     x12, [sp, x10]         /* write into saved register slot */

    /* Advance ELR past the faulting instruction */
    add     x2, x2, #4
    msr     elr_el1, x2

    /* Restore ALL registers from frame (including the one we just wrote) */
    ldp     x0,  x1,  [sp, #0]
    ldp     x2,  x3,  [sp, #16]
    ldp     x4,  x5,  [sp, #32]
    ldp     x6,  x7,  [sp, #48]
    ldp     x8,  x9,  [sp, #64]
    ldp     x10, x11, [sp, #80]
    ldp     x12, x13, [sp, #96]
    ldp     x14, x15, [sp, #112]
    ldp     x16, x17, [sp, #128]
    ldp     x18, x19, [sp, #144]
    ldp     x20, x21, [sp, #160]
    ldp     x22, x23, [sp, #176]
    ldp     x24, x25, [sp, #192]
    ldp     x26, x27, [sp, #208]
    ldp     x28, x29, [sp, #224]
    ldr     x30,       [sp, #240]
    add     sp, sp, #272
    eret

.Lalign_write:
    /* --- WRITE alignment fault emulation --- */
    mrs     x2, elr_el1
    ldr     w3, [x2]               /* faulting instruction */
    mrs     x11, far_el1           /* faulting data address */

    /* Get Rt (source register) = bits [4:0] */
    and     w10, w3, #0x1F

    /* Read the value from saved register frame */
    cmp     w10, #30
    b.hi    .Lnot_align_fault
    lsl     x10, x10, #3
    ldr     x12, [sp, x10]        /* value to store */

    /* Byte-by-byte write of 8 bytes to the faulting address */
    strb    w12, [x11, #0]
    lsr     x13, x12, #8
    strb    w13, [x11, #1]
    lsr     x13, x12, #16
    strb    w13, [x11, #2]
    lsr     x13, x12, #24
    strb    w13, [x11, #3]
    lsr     x13, x12, #32
    strb    w13, [x11, #4]
    lsr     x13, x12, #40
    strb    w13, [x11, #5]
    lsr     x13, x12, #48
    strb    w13, [x11, #6]
    lsr     x13, x12, #56
    strb    w13, [x11, #7]

    /* Advance ELR past the faulting instruction */
    add     x2, x2, #4
    msr     elr_el1, x2

    /* Restore ALL registers */
    ldp     x0,  x1,  [sp, #0]
    ldp     x2,  x3,  [sp, #16]
    ldp     x4,  x5,  [sp, #32]
    ldp     x6,  x7,  [sp, #48]
    ldp     x8,  x9,  [sp, #64]
    ldp     x10, x11, [sp, #80]
    ldp     x12, x13, [sp, #96]
    ldp     x14, x15, [sp, #112]
    ldp     x16, x17, [sp, #128]
    ldp     x18, x19, [sp, #144]
    ldp     x20, x21, [sp, #160]
    ldp     x22, x23, [sp, #176]
    ldp     x24, x25, [sp, #192]
    ldp     x26, x27, [sp, #208]
    ldp     x28, x29, [sp, #224]
    ldr     x30,       [sp, #240]
    add     sp, sp, #272
    eret

.Lnot_align_fault:
    /* Restore registers and fall through to fault handler */
    ldp     x0,  x1,  [sp, #0]
    ldp     x2,  x3,  [sp, #16]
    ldp     x4,  x5,  [sp, #32]
    ldp     x6,  x7,  [sp, #48]
    ldp     x8,  x9,  [sp, #64]
    ldp     x10, x11, [sp, #80]
    ldp     x12, x13, [sp, #96]
    ldp     x14, x15, [sp, #112]
    ldp     x16, x17, [sp, #128]
    ldp     x18, x19, [sp, #144]
    ldp     x20, x21, [sp, #160]
    ldp     x22, x23, [sp, #176]
    ldp     x24, x25, [sp, #192]
    ldp     x26, x27, [sp, #208]
    ldp     x28, x29, [sp, #224]
    ldr     x30,       [sp, #240]
    add     sp, sp, #272
    /* Fall through to print FAULT */

/* ===================================================================
 * Lower EL AArch64 synchronous handler
 *
 * Handles lower-EL AArch64 SVC traps. The handler saves a minimal complete
 * GPR frame, dispatches through the C syscall shim, writes the returned value
 * back to the saved user x0, advances ELR past the SVC instruction, and erets.
 * Other lower-EL synchronous exceptions still fall through to the fatal
 * diagnostic handler.
 * =================================================================== */

    .globl _lower_el_aarch64_sync_handler
_lower_el_aarch64_sync_handler:
    sub     sp, sp, #272
    stp     x0,  x1,  [sp, #0]
    stp     x2,  x3,  [sp, #16]
    stp     x4,  x5,  [sp, #32]
    stp     x6,  x7,  [sp, #48]
    stp     x8,  x9,  [sp, #64]
    stp     x10, x11, [sp, #80]
    stp     x12, x13, [sp, #96]
    stp     x14, x15, [sp, #112]
    stp     x16, x17, [sp, #128]
    stp     x18, x19, [sp, #144]
    stp     x20, x21, [sp, #160]
    stp     x22, x23, [sp, #176]
    stp     x24, x25, [sp, #192]
    stp     x26, x27, [sp, #208]
    stp     x28, x29, [sp, #224]
    str     x30,       [sp, #240]

    mrs     x7, esr_el1
    lsr     x9, x7, #26
    cmp     x9, #0x15              /* EC=0x15: SVC from AArch64 */
    b.ne    _fault_handler

    mrs     x6, elr_el1
    mov     x5, x4                 /* syscall arg4 */
    mov     x4, x3                 /* syscall arg3 */
    mov     x3, x2                 /* syscall arg2 */
    mov     x2, x1                 /* syscall arg1 */
    mov     x1, x0                 /* syscall arg0 */
    mov     x0, x8                 /* syscall id */
    bl      rt_arm64_handle_user_svc
    str     x0, [sp, #0]           /* syscall return value -> user x0 */

    mrs     x9, elr_el1
    add     x9, x9, #4
    msr     elr_el1, x9
    isb

    ldp     x0,  x1,  [sp, #0]
    ldp     x2,  x3,  [sp, #16]
    ldp     x4,  x5,  [sp, #32]
    ldp     x6,  x7,  [sp, #48]
    ldp     x8,  x9,  [sp, #64]
    ldp     x10, x11, [sp, #80]
    ldp     x12, x13, [sp, #96]
    ldp     x14, x15, [sp, #112]
    ldp     x16, x17, [sp, #128]
    ldp     x18, x19, [sp, #144]
    ldp     x20, x21, [sp, #160]
    ldp     x22, x23, [sp, #176]
    ldp     x24, x25, [sp, #192]
    ldp     x26, x27, [sp, #208]
    ldp     x28, x29, [sp, #224]
    ldr     x30,       [sp, #240]
    add     sp, sp, #272
    eret

/* ===================================================================
 * Fault handler — prints "FAULT @ 0x<ELR_EL1>" to PL011 then halts
 * =================================================================== */

_fault_handler:
    /* Print "FAULT @ 0x" */
    ldr     x1, =0x09000000
    mov     w0, #'F'
    str     w0, [x1]
    mov     w0, #'A'
    str     w0, [x1]
    mov     w0, #'U'
    str     w0, [x1]
    mov     w0, #'L'
    str     w0, [x1]
    mov     w0, #'T'
    str     w0, [x1]
    mov     w0, #' '
    str     w0, [x1]
    mov     w0, #'@'
    str     w0, [x1]
    mov     w0, #' '
    str     w0, [x1]
    mov     w0, #'0'
    str     w0, [x1]
    mov     w0, #'x'
    str     w0, [x1]

    /* Print ELR_EL1 as 16-digit hex */
    mrs     x2, elr_el1
    mov     x3, #60
.Lhex_loop:
    lsr     x4, x2, x3
    and     x4, x4, #0xF
    cmp     x4, #10
    b.lt    .Lhex_digit
    add     w4, w4, #('a' - 10)
    b       .Lhex_emit
.Lhex_digit:
    add     w4, w4, #'0'
.Lhex_emit:
    str     w4, [x1]
    subs    x3, x3, #4
    b.ge    .Lhex_loop

    /* Print CR+LF */
    mov     w0, #'\r'
    str     w0, [x1]
    mov     w0, #'\n'
    str     w0, [x1]

    /* Also print ESR_EL1 for diagnostics */
    mov     w0, #'E'
    str     w0, [x1]
    mov     w0, #'S'
    str     w0, [x1]
    mov     w0, #'R'
    str     w0, [x1]
    mov     w0, #'='
    str     w0, [x1]
    mov     w0, #'0'
    str     w0, [x1]
    mov     w0, #'x'
    str     w0, [x1]

    mrs     x2, esr_el1
    mov     x3, #28
.Lesr_loop:
    lsr     x4, x2, x3
    and     x4, x4, #0xF
    cmp     x4, #10
    b.lt    .Lesr_digit
    add     w4, w4, #('a' - 10)
    b       .Lesr_emit
.Lesr_digit:
    add     w4, w4, #'0'
.Lesr_emit:
    str     w4, [x1]
    subs    x3, x3, #4
    b.ge    .Lesr_loop

    mov     w0, #'\r'
    str     w0, [x1]
    mov     w0, #'\n'
    str     w0, [x1]

.Lfault_halt:
    wfe
    b       .Lfault_halt
