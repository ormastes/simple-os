/*
 * SimpleOS x86_64 minimal boot/stdout capsule.
 *
 * This is intentionally small: long-mode setup remains in crt0.s, policy lives
 * in pure Simple, and this C capsule only owns the platform instructions for
 * COM1, debug-exit, and halt. It is an auditable replacement candidate for the
 * boot/stdout subset currently buried in baremetal_stubs.c.
 */

#include <stdint.h>
#include <stddef.h>

#include "../../common/baremetal_runtime.h"

#define COM1_PORT 0x3F8u
#define ISA_DEBUG_EXIT_PORT 0xF4u

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void serial_init(void)
{
    outb(COM1_PORT + 1u, 0x00u);
    outb(COM1_PORT + 3u, 0x80u);
    outb(COM1_PORT + 0u, 0x03u);
    outb(COM1_PORT + 1u, 0x00u);
    outb(COM1_PORT + 3u, 0x03u);
    outb(COM1_PORT + 2u, 0xC7u);
    outb(COM1_PORT + 4u, 0x0Bu);
}

void serial_putchar(char c)
{
    while ((inb(COM1_PORT + 5u) & 0x20u) == 0u) {
        __asm__ volatile("pause");
    }
    outb(COM1_PORT, (uint8_t)c);
}

#include "../../common/baremetal_min_stdout.h"

RuntimeValue rt_port_outb(RuntimeValue port, RuntimeValue val)
{
    outb((uint16_t)(uint64_t)port, (uint8_t)(uint64_t)val);
    return ENCODE_INT(0);
}

RuntimeValue rt_port_inb(RuntimeValue port)
{
    return (RuntimeValue)(uint64_t)inb((uint16_t)(uint64_t)port);
}

__attribute__((noreturn)) static void halt_forever(void)
{
    __asm__ volatile("cli");
    for (;;) {
        __asm__ volatile("hlt");
    }
}

extern void spl_start(void) __attribute__((weak));
extern void __simple_call_module_inits(void) __attribute__((weak));

void _start(void)
{
    __asm__ volatile("cli");
    outb(0x21u, 0xFFu);
    outb(0xA1u, 0xFFu);
    serial_init();
    if (__simple_call_module_inits) __simple_call_module_inits();
    if (spl_start) spl_start();
    outb(ISA_DEBUG_EXIT_PORT, 0u);
    halt_forever();
}
