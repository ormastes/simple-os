/*
 * context_switch.s — x86_64 context/FPU helpers for SimpleOS.
 *
 * Exported to Simple as:
 *   void rt_x86_64_context_switch(uint64_t from_ptr, uint64_t to_ptr)
 *   void rt_x86_64_fpu_save(uint64_t fpu_addr)
 *   void rt_x86_64_fpu_restore(uint64_t fpu_addr)
 *
 * X86_64Context offsets (u64 fields, repr C):
 *   rbx=8, rbp=48, rsp=56, r12=96, r13=104, r14=112, r15=120,
 *   rip=128, rflags=136
 */

    .section .text

    .globl rt_x86_64_context_switch
    .type rt_x86_64_context_switch, @function
    .align 16
rt_x86_64_context_switch:
    /* rdi=from_ptr, rsi=to_ptr */
    movq    %rdi, %rax

    movq    %rbx, 8(%rax)
    movq    %rbp, 48(%rax)
    movq    %rsp, 56(%rax)
    movq    %r12, 96(%rax)
    movq    %r13, 104(%rax)
    movq    %r14, 112(%rax)
    movq    %r15, 120(%rax)

    pushfq
    popq    136(%rax)

    leaq    .Lswitch_resume(%rip), %rcx
    movq    %rcx, 128(%rax)

    movq    %rsi, %rax
    movq    8(%rax), %rbx
    movq    48(%rax), %rbp
    movq    96(%rax), %r12
    movq    104(%rax), %r13
    movq    112(%rax), %r14
    movq    120(%rax), %r15

    pushq   136(%rax)
    popfq

    movq    56(%rax), %rsp
    jmp     *128(%rax)

.Lswitch_resume:
    ret
    .size rt_x86_64_context_switch, . - rt_x86_64_context_switch

    .globl rt_x86_64_fpu_save
    .type rt_x86_64_fpu_save, @function
    .align 16
rt_x86_64_fpu_save:
    fxsave  (%rdi)
    ret
    .size rt_x86_64_fpu_save, . - rt_x86_64_fpu_save

    .globl rt_x86_64_fpu_restore
    .type rt_x86_64_fpu_restore, @function
    .align 16
rt_x86_64_fpu_restore:
    fxrstor (%rdi)
    ret
    .size rt_x86_64_fpu_restore, . - rt_x86_64_fpu_restore

    .section .note.GNU-stack,"",@progbits
