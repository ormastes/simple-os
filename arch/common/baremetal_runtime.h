/*
 * SimpleOS Shared Baremetal Runtime — Header
 *
 * Architecture-independent type definitions, tagged-value macros,
 * and forward declarations for the shared baremetal runtime.
 *
 * USAGE:
 *   1. #include <stdint.h> and <stddef.h> in the arch-specific file
 *   2. Define the following arch-specific functions BEFORE including this header:
 *        static void serial_putchar(char c);
 *        static void serial_puts(const char *s);
 *        static void serial_put_hex(uint64_t v);
 *        static void serial_put_dec(int64_t v);
 *        static void arch_halt_forever(void);
 *        static void arch_pause(void);
 *   3. #include "baremetal_runtime.h"
 *   4. #include "baremetal_runtime.c"
 *
 * The arch-specific file provides _start(), serial I/O, port I/O,
 * MMIO, PCI, NVMe, framebuffer, syscall dispatch, and CPU control.
 */

#ifndef BAREMETAL_RUNTIME_H
#define BAREMETAL_RUNTIME_H

#include <stdint.h>
#include <stddef.h>

/* ===================================================================
 * RuntimeValue — tagged 64-bit value
 * =================================================================== */

typedef int64_t RuntimeValue;

/* ===================================================================
 * Tag encoding
 * =================================================================== */

#define TAG_MASK    0x7ULL
#define TAG_INT     0x0ULL
#define TAG_HEAP    0x1ULL
#define TAG_FLOAT   0x2ULL
#define TAG_SPECIAL 0x3ULL

#define ENCODE_INT(v)  ((RuntimeValue)(((uint64_t)(int64_t)(v) << 3) | TAG_INT))
#define DECODE_INT(v)  ((int64_t)((uint64_t)(v) >> 3))

#define ENCODE_PTR(p)  ((RuntimeValue)((uint64_t)(uintptr_t)(p) | TAG_HEAP))
#define DECODE_PTR(v)  ((void*)((uint64_t)(v) & ~TAG_MASK))

#define IS_INT(v)      (((uint64_t)(v) & TAG_MASK) == TAG_INT)
#define IS_HEAP(v)     (((uint64_t)(v) & TAG_MASK) == TAG_HEAP)
#define IS_FLOAT(v)    (((uint64_t)(v) & TAG_MASK) == TAG_FLOAT)
#define IS_SPECIAL(v)  (((uint64_t)(v) & TAG_MASK) == TAG_SPECIAL)
#define IS_NIL(v)      ((v) == (RuntimeValue)TAG_SPECIAL)

#define NIL_VALUE      ((RuntimeValue)TAG_SPECIAL)
#define TRUE_VALUE     ENCODE_INT(1)
#define FALSE_VALUE    ENCODE_INT(0)

/* ===================================================================
 * Heap object types
 * =================================================================== */

#define HEAP_STRING 1
#define HEAP_ARRAY  2
#define HEAP_MAP    3
#define HEAP_OBJECT 4

typedef struct {
    uint32_t type;
    uint32_t size;
} HeapHeader;

typedef struct {
    HeapHeader hdr;
    uint32_t   len;
    char       data[];
} RuntimeString;

typedef struct {
    HeapHeader   hdr;
    uint32_t     len;
    uint32_t     cap;
    RuntimeValue items[];
} RuntimeArray;

typedef struct {
    HeapHeader    hdr;
    uint32_t      len;
    uint32_t      cap;
    RuntimeValue *keys;
    RuntimeValue *values;
} RuntimeMap;

/* ===================================================================
 * Forward declarations for functions used before definition
 * =================================================================== */

RuntimeValue rt_map_clone(RuntimeValue map);
RuntimeValue rt_map_new(void);
RuntimeValue rt_map_set(RuntimeValue map, RuntimeValue key, RuntimeValue value);
RuntimeValue rt_map_get(RuntimeValue map, RuntimeValue key);
RuntimeValue rt_array_new(RuntimeValue cap_val);
int8_t rt_array_push(RuntimeValue arr, RuntimeValue val);
RuntimeValue rt_string_concat(RuntimeValue a, RuntimeValue b);
RuntimeValue rt_string_from_cstr(const char *cstr);
RuntimeValue rt_string_new(RuntimeValue data, RuntimeValue len_val);
RuntimeValue rt_native_eq(RuntimeValue a, RuntimeValue b);
RuntimeValue rt_value_to_string(RuntimeValue val);
RuntimeValue rt_value_format_string(RuntimeValue val, RuntimeValue fmt_ptr, RuntimeValue fmt_len);
RuntimeValue rt_string_format(RuntimeValue fmt, RuntimeValue val);
void rt_print_value(RuntimeValue val);

/* ===================================================================
 * Arch-specific function contracts (must be defined before
 * #include "baremetal_runtime.c"):
 *
 *   static void serial_putchar(char c);
 *   static void serial_puts(const char *s);
 *   static void serial_put_hex(uint64_t v);
 *   static void serial_put_dec(int64_t v);
 *   static void arch_halt_forever(void);
 *   static void arch_pause(void);
 * =================================================================== */

#endif /* BAREMETAL_RUNTIME_H */
