#include <stdint.h>
#include <stddef.h>

#define UART_BASE 0x10000000UL
#define UART_THR 0x00UL
#define UART_LSR 0x05UL
#define UART_LSR_THRE 0x20u

void serial_putchar(char c){
    volatile uint8_t *uart = (volatile uint8_t *)UART_BASE;
    for (uint32_t spin = 0; spin < 100000u; spin++) {
        if ((uart[UART_LSR] & UART_LSR_THRE) != 0u) break;
    }
    uart[UART_THR] = (uint8_t)c;
}

#include "baremetal_min_stdout.h"

extern void spl_start(void) __attribute__((weak));
extern char _stack_top[];

__attribute__((naked, section(".text.entry"))) void _start(void){
    __asm__ volatile(
        "la sp, _stack_top\n"
        "la t0, spl_start\n"
        "beqz t0, 1f\n"
        "jalr t0\n"
        "1: wfi\n"
        "j 1b\n"
    );
}
