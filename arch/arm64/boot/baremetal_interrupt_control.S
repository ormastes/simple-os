.macro A_DI; msr daifset, #0xf; .endm
.macro A_EI; msr daifclr, #0xf; .endm
.macro A_WI; wfe; .endm
.macro A_RT; ret; .endm
.include "examples/simple_os/arch/common/baremetal_arm_interrupt_control.S"
AIC simpleos_arm64_interrupt_disable, simpleos_arm64_interrupt_enable, simpleos_arm64_wait_for_interrupt
