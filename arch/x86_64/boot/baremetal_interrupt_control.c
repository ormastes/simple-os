#define X86_INSN_FN(name, insn) void name(void){__asm__ volatile(insn::: "memory");}
X86_INSN_FN(simpleos_x86_interrupt_disable, "cli")
X86_INSN_FN(simpleos_x86_interrupt_enable, "sti")
X86_INSN_FN(simpleos_x86_halt_until_interrupt, "hlt")

void simpleos_x86_pic_mask_all(void){__asm__ volatile("outb %0,%1;outb %0,%2"::"a"((unsigned char)0xFF),"Nd"((unsigned short)0x21),"Nd"((unsigned short)0xA1):"memory");}
