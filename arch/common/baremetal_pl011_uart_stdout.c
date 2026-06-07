#include <stdint.h>
#include <stddef.h>

#define PL011_DR   0x000u
#define PL011_FR   0x018u
#define PL011_IBRD 0x024u
#define PL011_FBRD 0x028u
#define PL011_LCRH 0x02Cu
#define PL011_CR   0x030u
#define PL011_ICR  0x044u
#define PL011_TXFF (1u << 5)
#define PL011_REG(offset) (*(volatile uint32_t *)(PL011_BASE + (offset)))

static void pl011_init(void){
    PL011_REG(PL011_CR) = 0u;
    PL011_REG(PL011_ICR) = 0x7FFu;
    PL011_REG(PL011_IBRD) = PL011_IBRD_VALUE;
    PL011_REG(PL011_FBRD) = PL011_FBRD_VALUE;
    PL011_REG(PL011_LCRH) = (3u << 5) | (1u << 4);
    PL011_REG(PL011_CR) = (1u << 0) | (1u << 8) | (1u << 9);
}

void serial_putchar(char c){
    for (uint32_t spin = 0; spin < 100000u; spin++) {
        if ((PL011_REG(PL011_FR) & PL011_TXFF) == 0u) break;
    }
    PL011_REG(PL011_DR) = (uint32_t)(uint8_t)c;
}

#include "baremetal_min_stdout.h"

extern void spl_start(void) __attribute__((weak));
#if SIMPLEOS_CALL_MODULE_INITS
extern void __simple_call_module_inits(void) __attribute__((weak));
#endif

void SIMPLEOS_PL011_ENTRY(void){
    pl011_init();
#if SIMPLEOS_CALL_MODULE_INITS
    if (__simple_call_module_inits) __simple_call_module_inits();
#endif
    if (spl_start) spl_start();
    for (;;) __asm__ volatile(SIMPLEOS_PL011_HALT);
}
