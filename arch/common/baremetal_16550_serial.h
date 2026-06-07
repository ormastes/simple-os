#ifndef SIMPLEOS_BAREMETAL_16550_SERIAL_H
#define SIMPLEOS_BAREMETAL_16550_SERIAL_H

#ifndef UART_BASE
#define UART_BASE 0x10000000UL
#endif

#define UART_THR 0x00UL
#define UART_LSR 0x05UL
#define UART_LSR_THRE 0x20U

static void uart_putc(char c){
    volatile uint8_t *uart = (volatile uint8_t *)UART_BASE;
    for (uint32_t spin = 0; spin < 100000; spin++) {
        if ((uart[UART_LSR] & UART_LSR_THRE) != 0) break;
    }
    uart[UART_THR] = (uint8_t)c;
}

static void uart_puts(const char *s){
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

#endif
