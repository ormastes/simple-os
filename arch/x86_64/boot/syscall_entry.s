/*
 * syscall_entry.s — SimpleOS x86_64 SYSCALL kernel trampoline.
 *
 * Hardware state on SYSCALL entry:
 *   rcx = caller RIP (saved by hardware)
 *   r11 = caller RFLAGS (saved by hardware)
 *   rax = syscall number
 *   rdi, rsi, rdx, r10, r8, r9 = syscall args 0..5
 *     (NOTE: SYSCALL uses r10 instead of rcx for arg3 because rcx holds
 *      the saved caller RIP.)
 *   CS loaded from MSR_STAR[47:32]  (kernel CS)
 *   SS loaded from MSR_STAR[47:32]+8
 *   IF cleared via MSR_SFMASK
 *
 * This trampoline is the C-callable bridge from the SYSCALL instruction
 * to the Simple-side kernel syscall dispatcher. It:
 *
 *   1. Swaps to a global kernel stack (SMP-unsafe but SimpleOS is single-
 *      CPU today; per-CPU GS-base scratch can be wired in later without
 *      changing the C-side ABI).
 *   2. Adapts the SYSCALL register convention to the System V C ABI.
 *   3. Calls rt_syscall_dispatch(num, a0..a5) which returns int64_t in rax.
 *   4. Returns via SYSRETQ for ring-3 callers, or plain jmp+popfq for
 *      ring-0 callers (see "Ring-0-safe return" below).
 *
 * Ring-0-safe return:
 *   Kernel-internal code issues `syscall` from ring 0 (e.g. desktop_e2e's
 *   launcher path → posix_spawn → syscall(13, ...)). Hardware does not
 *   record the caller's CPL, so the trampoline tells ring-0 callers from
 *   ring-3 callers by comparing the saved caller RIP (rcx) against the
 *   kernel image bounds [0x100000, _kernel_end). A caller RIP inside that
 *   window is ring 0 by construction — ring-3 user text lives outside the
 *   kernel image. Ring-0 callers return via `jmp *%rcx` so execution stays
 *   in ring 0; ring-3 callers get the standard `sysretq` that transitions
 *   back to CPL=3.
 *
 *   This is a single-CPU, single-address-space heuristic. With SMP or
 *   multiple user VAs colliding with the kernel image range, replace the
 *   check with a proper CS save (e.g. push CS into a per-CPU slot before
 *   SYSCALL-facing code on the kernel side, or swap to an iret-return
 *   trampoline that carries CS in its stack frame).
 *
 * Symbols exported:
 *   kernel_syscall_entry_asm      - the trampoline itself (goes in LSTAR)
 *   get_kernel_syscall_entry_addr - C-callable helper returning the
 *                                   address of the trampoline
 *
 * Symbols referenced:
 *   rt_syscall_dispatch           - C-side scalar dispatcher
 *                                   (baremetal_stubs.c)
 *   _kernel_syscall_stack_top     - top of the global kernel stack
 *   _kernel_syscall_scratch_rsp   - scratch slot for caller rsp
 *   _kernel_end                   - linker-provided top of kernel image
 */

    .section .text
    .globl kernel_syscall_entry_asm
    .type kernel_syscall_entry_asm, @function
    .align 16
kernel_syscall_entry_asm:
    /* Save caller rsp and switch to the global kernel syscall stack.
     * Using a global stack is safe on single-CPU SimpleOS. When SMP
     * lands, replace with per-CPU GS-base scratch (see comment at top). */
    movq    %rsp, _kernel_syscall_scratch_rsp(%rip)
    movq    _kernel_syscall_stack_top(%rip), %rsp

    /* Preserve SYSRET state and call the scalar C dispatcher:
     *   rt_syscall_dispatch(num, a0, a1, a2, a3, a4, a5)
     * Hardware gives us:
     *   rax=num, rdi=a0, rsi=a1, rdx=a2, r10=a3, r8=a4, r9=a5
     * System V C wants:
     *   rdi=num, rsi=a0, rdx=a1, rcx=a2, r8=a3, r9=a4, stack=a5
     */
    pushq   %rcx            /* caller RIP */
    pushq   %r11            /* caller RFLAGS */
    subq    $8, %rsp        /* keep 16-byte alignment with one stack arg */
    pushq   %r9             /* C arg6: a5 */
    movq    %r8, %r9        /* C arg5: a4 */
    movq    %r10, %r8       /* C arg4: a3 */
    movq    %rdx, %rcx      /* C arg3: a2 */
    movq    %rsi, %rdx      /* C arg2: a1 */
    movq    %rdi, %rsi      /* C arg1: a0 */
    movq    %rax, %rdi      /* C arg0: syscall number */
    call    rt_syscall_dispatch
    /* rax now holds the int64_t return value from the dispatcher. */

    addq    $16, %rsp       /* discard a5 and alignment pad */
    popq    %r11            /* restore caller RFLAGS */
    popq    %rcx            /* restore caller RIP */

    /* Decide whether to return via sysretq (ring-3 caller) or plain jmp
     * (ring-0 caller). Test the saved caller RIP against the kernel image
     * bounds [0x100000, _kernel_end). Hardware does not record CPL, but
     * ring-3 user code cannot share the kernel image range, so a simple
     * RIP-range check is correct on single-CPU SimpleOS.
     *
     * rax holds the dispatcher return value and must be preserved through
     * both return paths — use rdx as the scratch comparison register. */
    movq    $0x100000, %rdx
    cmpq    %rdx, %rcx
    jb      .Lkse_ring3_return      /* rcx < 1MB  -> ring 3 */
    leaq    _kernel_end(%rip), %rdx
    cmpq    %rdx, %rcx
    jae     .Lkse_ring3_return      /* rcx >= _kernel_end -> ring 3 */

    /* Ring-0 return: restore RFLAGS from r11, restore the caller's RSP,
     * and jump to the saved RIP. Keeps CPL=0; sysretq would forcibly
     * transition to CPL=3 into a kernel-text address and crash.
     *
     * popfq on a value pushed from r11 is safe — the kernel's SFMASK
     * cleared IF before entry, but the caller's saved r11 has the
     * original IF bit. */
    pushq   %r11
    popfq
    movq    _kernel_syscall_scratch_rsp(%rip), %rsp
    jmpq    *%rcx

.Lkse_ring3_return:
    /* Standard ring-3 return path: hardware restores CS/SS from STAR,
     * RFLAGS from r11, RIP from rcx. */
    movq    _kernel_syscall_scratch_rsp(%rip), %rsp
    sysretq
    .size kernel_syscall_entry_asm, . - kernel_syscall_entry_asm

/* C-callable helper so Simple code can fetch the trampoline address
 * without needing raw symbol-to-u64 coercion. */
    .globl get_kernel_syscall_entry_addr
    .type get_kernel_syscall_entry_addr, @function
    .align 16
get_kernel_syscall_entry_addr:
    leaq    kernel_syscall_entry_asm(%rip), %rax
    ret
    .size get_kernel_syscall_entry_addr, . - get_kernel_syscall_entry_addr

/* Global kernel syscall stack (8 KiB). SMP-unsafe but single-CPU SimpleOS.
 * _kernel_syscall_stack_top (in .data) points at the high end. */
    .section .bss
    .align 16
    .globl _kernel_syscall_stack_base
_kernel_syscall_stack_base:
    .skip 8192
    .globl _kernel_syscall_stack_end
_kernel_syscall_stack_end:

    .align 8
    .globl _kernel_syscall_scratch_rsp
_kernel_syscall_scratch_rsp:
    .skip 8

    .section .data
    .align 8
    .globl _kernel_syscall_stack_top
_kernel_syscall_stack_top:
    .quad _kernel_syscall_stack_end

    .section .note.GNU-stack,"",@progbits
