/*
 * enter_user_first.s — SimpleOS x86_64 first-dispatch ring-3 entry helper.
 *
 * FR-SOS-024 Phase 3. Called by the kernel after `create_user_task` builds a
 * user-mode TCB and its address space. Swaps to the per-task PML4 and iretq's
 * into the user's entry point at CPL 3.
 *
 * C-ABI signature (extern fn from Simple), 6 arguments in Sys V AMD64 order:
 *
 *   void rt_x86_enter_user_first(
 *       uint64_t rip,       // rdi
 *       uint64_t rsp,       // rsi
 *       uint64_t cs,        // rdx  (must have RPL=3 for ring-3 target)
 *       uint64_t ss,        // rcx  (must have RPL=3)
 *       uint64_t rflags,    // r8
 *       uint64_t cr3_phys)  // r9
 *
 * All user GPRs are zeroed before the iretq — the user task's entry
 * convention (SysV AMD64 for _start) reads argc/argv/envp from the stack
 * top (`rsp` points at argc).
 *
 * Caveats (MVP):
 *   - Does NOT touch TSS.RSP0. Syscalls from CPL 3 go through the LSTAR
 *     trampoline which swaps to `_kernel_syscall_stack_top` directly.
 *     TSS.RSP0 is only needed for hardware interrupts / page faults from
 *     CPL 3, which this MVP does not exercise.
 *   - Does NOT return. If the user calls exit (syscall 0) the baremetal
 *     dispatcher exits QEMU via isa-debug-exit.
 *   - Assumes cr3_phys maps the kernel image at its HHDM/identity range,
 *     which `create_user_address_space()` already arranges.
 */

    .section .text
    .globl rt_x86_enter_user_first
    .type rt_x86_enter_user_first, @function
    .align 16
rt_x86_enter_user_first:
    /* Stash CR3 into a callee-saved register before any stack push so the
     * stack writes happen before we swap address spaces. The pushes below
     * write to the kernel stack, which must still be mapped after cr3 load
     * (and it is — create_user_address_space copies the kernel mappings). */
    movq    %r9, %rax               /* cr3 */

    /* Build the iret frame on the current (kernel) stack.
     * Hardware pops in order RIP, CS, RFLAGS, RSP, SS — so we push SS first. */
    pushq   %rcx                    /* SS   (arg4) */
    pushq   %rsi                    /* RSP  (arg2) */
    pushq   %r8                     /* RFLAGS (arg5) */
    pushq   %rdx                    /* CS   (arg3) */
    pushq   %rdi                    /* RIP  (arg1) */

    /* Swap to the user address space. Kernel is mapped in the user PML4
     * (clone of kernel range), so execution continues past this. */
    movq    %rax, %cr3

    /* Zero GPRs so the user starts with a clean register file. */
    xorl    %eax, %eax
    xorl    %ebx, %ebx
    xorl    %ecx, %ecx
    xorl    %edx, %edx
    xorl    %esi, %esi
    xorl    %edi, %edi
    xorl    %ebp, %ebp
    xorl    %r8d,  %r8d
    xorl    %r9d,  %r9d
    xorl    %r10d, %r10d
    xorl    %r11d, %r11d
    xorl    %r12d, %r12d
    xorl    %r13d, %r13d
    xorl    %r14d, %r14d
    xorl    %r15d, %r15d

    iretq
    .size rt_x86_enter_user_first, . - rt_x86_enter_user_first

    .section .note.GNU-stack,"",@progbits
