.syntax unified
.arm
.macro A_DI; cpsid if; .endm
.macro A_EI; cpsie if; .endm
.macro A_WI; wfe; .endm
.macro A_RT; bx lr; .endm
.include "examples/simple_os/arch/common/baremetal_arm_interrupt_control.S"
AIC simpleos_arm32_interrupt_disable, simpleos_arm32_interrupt_enable, simpleos_arm32_wait_for_interrupt
