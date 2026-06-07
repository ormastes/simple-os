#ifndef SIMPLEOS_BAREMETAL_PL011_SERIAL_H
#define SIMPLEOS_BAREMETAL_PL011_SERIAL_H

#ifndef PL011_BASE
#define PL011_BASE 0x09000000U
#endif

#define PL011_DR     0x000U
#define PL011_FR     0x018U
#define PL011_IBRD   0x024U
#define PL011_FBRD   0x028U
#define PL011_LCRH   0x02CU
#define PL011_CR     0x030U
#define PL011_IMSC   0x038U
#define PL011_ICR    0x044U
#define PL011_FR_TXFF (1U << 5)

static inline volatile uint32_t *pl011_reg(uint32_t offset){return (volatile uint32_t *)((uintptr_t)PL011_BASE+(uintptr_t)offset);}

#define UART_DR   (*pl011_reg(PL011_DR))
#define UART_FR   (*pl011_reg(PL011_FR))
#define UART_IBRD (*pl011_reg(PL011_IBRD))
#define UART_FBRD (*pl011_reg(PL011_FBRD))
#define UART_LCRH (*pl011_reg(PL011_LCRH))
#define UART_CR   (*pl011_reg(PL011_CR))
#define UART_ICR  (*pl011_reg(PL011_ICR))

static void serial_putchar(char c){
    for (uint32_t spin = 0; spin < 100000; spin++) {
        if ((*pl011_reg(PL011_FR) & PL011_FR_TXFF) == 0) break;
    }
    *pl011_reg(PL011_DR) = (uint32_t)(unsigned char)c;
}

static void serial_puts(const char *s){
    while (*s) {
        if (*s == '\n') serial_putchar('\r');
        serial_putchar(*s++);
    }
}

#ifdef BAREMETAL_PL011_ENABLE_DIRECT_PUTS
static void serial_puts_direct(const char *s){
    while (*s) {
        *pl011_reg(PL011_DR) = (uint32_t)(unsigned char)(*s++);
    }
}
#endif

#endif
