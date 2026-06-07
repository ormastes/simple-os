/*
 * SimpleOS x86_64 Baremetal Runtime — Extra Functions
 *
 * This file provides implementations (or safe stubs) for ~1,554 rt_*
 * symbols that the Simple compiler references but are NOT yet in
 * baremetal_stubs.c.
 *
 * Organisation:
 *   1. Shared types, macros, forward declarations
 *   2. GENUINELY IMPLEMENTED — functions useful on bare metal
 *      (value ops, volatile, ptr, tuple, error, random, math, closure,
 *       generator, async, atomic, object, shared/unique/weak, debug, etc.)
 *   3. HOST-ONLY NO-OP STUBS — functions that need a host OS and return NIL
 *      (file I/O, network, GPU, ML, database, multimedia, crypto, etc.)
 *   4. WRONG-ARCH STUBS — ARM32, ARM64, RISC-V instructions (no-op on x86)
 *
 * Link order: this file is compiled alongside baremetal_stubs.c.
 * auto_stubs.c provides weak fallbacks; these strong definitions win.
 *
 * Compile:
 *   clang --target=x86_64-unknown-elf -c -ffreestanding -nostdlib \
 *         -fno-pie -mno-red-zone -o /tmp/test_rt.o rt_extras.c
 */

#include <stdint.h>
#include <stddef.h>
#include "../../common/baremetal_runtime.h"

/* ===================================================================
 * 1. Shared types, macros, forward declarations
 * =================================================================== */

/* Encode/decode float as tagged RuntimeValue */
static inline RuntimeValue ENCODE_FLOAT(double f) {
    uint64_t bits;
    __builtin_memcpy(&bits, &f, 8);
    return (RuntimeValue)((bits & ~TAG_MASK) | TAG_FLOAT);
}
static inline double DECODE_FLOAT(RuntimeValue v) {
    uint64_t bits = ((uint64_t)v & ~TAG_MASK);
    double f;
    __builtin_memcpy(&f, &bits, 8);
    return f;
}

typedef struct {
    HeapHeader   hdr;
    uint32_t     enum_id;
    uint32_t     discriminant;
    RuntimeValue payload;
} RuntimeEnum;

#define HEAP_CLOSURE 5
#define HEAP_MODULE  6
#define HEAP_ENUM    7

/* Forward declarations — defined in baremetal_stubs.c */
extern void *malloc(size_t sz);
extern void  free(void *p);
extern void *memcpy(void *dst, const void *src, size_t n);
extern void *memset(void *dst, int c, size_t n);
extern size_t strlen(const char *s);

/* Serial output (defined in baremetal_stubs.c) */
extern void serial_puts(const char *s);
extern void serial_putchar(char c);

/* serial_puthex is `static` in baremetal_stubs.c and not linkable from here.
 * The rt_tuple_/rt_closure_ diagnostic prints below are debug-only, so stub
 * them out locally rather than teaching the other TU to export the helper.
 * Silently dropping this .c from the boot link (which was Agent E's
 * x64-desktop-test fault cause) is worse than losing debug spew. */
#define serial_puthex(x) ((void)(x))

/* Functions defined in baremetal_stubs.c that we delegate to */
extern RuntimeValue rt_string_from_cstr(const char *cstr);
extern RuntimeValue rt_string_new(RuntimeValue data, RuntimeValue len_val);
extern RuntimeValue rt_string_concat(RuntimeValue a, RuntimeValue b);
extern RuntimeValue rt_string_len(RuntimeValue str);
extern RuntimeValue rt_value_to_string(RuntimeValue val);
extern RuntimeValue rt_array_new(RuntimeValue cap);
extern int8_t rt_array_push(RuntimeValue arr, RuntimeValue val);
extern RuntimeValue rt_array_get(RuntimeValue arr, RuntimeValue idx);
extern RuntimeValue rt_array_len(RuntimeValue arr);
extern RuntimeValue rt_native_eq(RuntimeValue a, RuntimeValue b);
extern RuntimeValue rt_enum_new(RuntimeValue eid, RuntimeValue disc, RuntimeValue payload);
extern RuntimeValue rt_print(RuntimeValue val);

/* Helper: decode to RuntimeString */
static RuntimeString *_decode_str(RuntimeValue v) {
    if (!IS_HEAP(v)) return (RuntimeString *)0;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(v);
    if (!s || s->hdr.type != HEAP_STRING) return (RuntimeString *)0;
    return s;
}

/* Helper: decode to RuntimeArray */
static RuntimeArray *_decode_arr(RuntimeValue v) {
    if (!IS_HEAP(v)) return (RuntimeArray *)0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(v);
    if (!a || a->hdr.type != HEAP_ARRAY) return (RuntimeArray *)0;
    return a;
}


/* ===================================================================
 * 2. GENUINELY IMPLEMENTED — value ops, volatile, ptr, tuple, etc.
 * =================================================================== */

/* ---- rt_value_* (Value boxing/unboxing) ---- */

RuntimeValue rt_value_int(RuntimeValue i) {
    return ENCODE_INT(i);
}

RuntimeValue rt_value_float(RuntimeValue f_raw) {
    /* f_raw is a raw double bit-cast to i64 */
    return (RuntimeValue)(((uint64_t)f_raw & ~TAG_MASK) | TAG_FLOAT);
}

RuntimeValue rt_value_bool(RuntimeValue b) {
    return b ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue rt_value_nil(void) {
    return NIL_VALUE;
}

RuntimeValue rt_value_as_int(RuntimeValue v) {
    if (IS_INT(v)) return DECODE_INT(v);
    return 0;
}

RuntimeValue rt_value_as_float(RuntimeValue v) {
    if (IS_FLOAT(v)) {
        double f = DECODE_FLOAT(v);
        RuntimeValue r;
        __builtin_memcpy(&r, &f, 8);
        return r;
    }
    return 0;
}

RuntimeValue rt_value_as_bool(RuntimeValue v) {
    if (IS_INT(v)) return DECODE_INT(v) ? 1 : 0;
    if (IS_NIL(v)) return 0;
    return 1;
}

RuntimeValue rt_value_as_string(RuntimeValue v) {
    return rt_value_to_string(v);
}

RuntimeValue rt_value_truthy(RuntimeValue v) {
    if (IS_NIL(v)) return 0;
    if (IS_INT(v)) return DECODE_INT(v) ? 1 : 0;
    return 1;
}

RuntimeValue rt_value_is_nil(RuntimeValue v) {
    return IS_NIL(v) ? 1 : 0;
}

RuntimeValue rt_value_is_int(RuntimeValue v) {
    return IS_INT(v) ? 1 : 0;
}

RuntimeValue rt_value_is_float(RuntimeValue v) {
    return IS_FLOAT(v) ? 1 : 0;
}

RuntimeValue rt_value_is_bool(RuntimeValue v) {
    if (!IS_INT(v)) return 0;
    int64_t n = DECODE_INT(v);
    return (n == 0 || n == 1) ? 1 : 0;
}

RuntimeValue rt_value_is_string(RuntimeValue v) {
    if (!IS_HEAP(v)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
    return (h && h->type == HEAP_STRING) ? 1 : 0;
}

RuntimeValue rt_value_is_array(RuntimeValue v) {
    if (!IS_HEAP(v)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
    return (h && h->type == HEAP_ARRAY) ? 1 : 0;
}

RuntimeValue rt_value_is_dict(RuntimeValue v) {
    if (!IS_HEAP(v)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
    return (h && h->type == HEAP_MAP) ? 1 : 0;
}

RuntimeValue rt_value_type(RuntimeValue v) {
    if (IS_NIL(v))   return rt_string_from_cstr("nil");
    if (IS_INT(v))   return rt_string_from_cstr("int");
    if (IS_FLOAT(v)) return rt_string_from_cstr("float");
    if (IS_HEAP(v)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
        if (h) {
            if (h->type == HEAP_STRING) return rt_string_from_cstr("string");
            if (h->type == HEAP_ARRAY)  return rt_string_from_cstr("array");
            if (h->type == HEAP_MAP)    return rt_string_from_cstr("map");
            if (h->type == HEAP_OBJECT) return rt_string_from_cstr("object");
        }
    }
    return rt_string_from_cstr("unknown");
}

RuntimeValue rt_value_string(RuntimeValue v) {
    return rt_value_to_string(v);
}

RuntimeValue rt_value_clone(RuntimeValue v) {
    /* Simple value clone — primitives are value types, heap objects shallow copy */
    if (!IS_HEAP(v)) return v;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
    if (!h) return v;
    if (h->type == HEAP_STRING) {
        RuntimeString *s = (RuntimeString *)h;
        return rt_string_new((RuntimeValue)(uintptr_t)s->data, (RuntimeValue)s->len);
    }
    if (h->type == HEAP_ARRAY) {
        RuntimeArray *a = (RuntimeArray *)h;
        RuntimeValue new_arr = rt_array_new(ENCODE_INT(a->cap));
        for (uint32_t i = 0; i < a->len; i++) {
            rt_array_push(new_arr, a->items[i]);
        }
        return new_arr;
    }
    return v;
}

RuntimeValue rt_value_free(RuntimeValue v) {
    (void)v; /* bump allocator — no-op */
    return NIL_VALUE;
}

RuntimeValue rt_value_print(RuntimeValue v) {
    rt_print(v);
    return NIL_VALUE;
}

RuntimeValue rt_value_println(RuntimeValue v) {
    rt_print(v);
    serial_puts("\r\n");
    return NIL_VALUE;
}

RuntimeValue rt_value_eq(RuntimeValue a, RuntimeValue b) {
    return rt_native_eq(a, b);
}

RuntimeValue rt_value_lt(RuntimeValue a, RuntimeValue b) {
    return (DECODE_INT(a) < DECODE_INT(b)) ? 1 : 0;
}

RuntimeValue rt_value_add(RuntimeValue a, RuntimeValue b) {
    if (IS_INT(a) && IS_INT(b))
        return ENCODE_INT(DECODE_INT(a) + DECODE_INT(b));
    return rt_string_concat(a, b);
}

RuntimeValue rt_value_sub(RuntimeValue a, RuntimeValue b) {
    return ENCODE_INT(DECODE_INT(a) - DECODE_INT(b));
}

RuntimeValue rt_value_mul(RuntimeValue a, RuntimeValue b) {
    return ENCODE_INT(DECODE_INT(a) * DECODE_INT(b));
}

RuntimeValue rt_value_div(RuntimeValue a, RuntimeValue b) {
    int64_t d = DECODE_INT(b);
    if (d == 0) return ENCODE_INT(0);
    return ENCODE_INT(DECODE_INT(a) / d);
}

RuntimeValue rt_value_array_new(RuntimeValue cap) {
    return rt_array_new(cap);
}

RuntimeValue rt_value_dict_new(void) {
    /* Delegate to baremetal_stubs.c rt_dict_new if available, else NIL */
    return NIL_VALUE;
}

RuntimeValue rt_value_to_ptr(RuntimeValue v) {
    if (IS_HEAP(v)) return (RuntimeValue)(uintptr_t)DECODE_PTR(v);
    return (RuntimeValue)v;
}


/* ---- rt_volatile_* (volatile memory access — essential for MMIO) ---- */

RuntimeValue rt_volatile_read_u8(RuntimeValue addr) {
    return (RuntimeValue)(uint64_t)*(volatile uint8_t *)(uintptr_t)(uint64_t)addr;
}

RuntimeValue rt_volatile_read_u16(RuntimeValue addr) {
    return (RuntimeValue)(uint64_t)*(volatile uint16_t *)(uintptr_t)(uint64_t)addr;
}

RuntimeValue rt_volatile_read_u32(RuntimeValue addr) {
    return (RuntimeValue)(uint64_t)*(volatile uint32_t *)(uintptr_t)(uint64_t)addr;
}

RuntimeValue rt_volatile_read_u64(RuntimeValue addr) {
    return (RuntimeValue)*(volatile uint64_t *)(uintptr_t)(uint64_t)addr;
}

RuntimeValue rt_volatile_write_u8(RuntimeValue addr, RuntimeValue val) {
    *(volatile uint8_t *)(uintptr_t)(uint64_t)addr = (uint8_t)(uint64_t)val;
    return NIL_VALUE;
}

RuntimeValue rt_volatile_write_u16(RuntimeValue addr, RuntimeValue val) {
    *(volatile uint16_t *)(uintptr_t)(uint64_t)addr = (uint16_t)(uint64_t)val;
    return NIL_VALUE;
}

RuntimeValue rt_volatile_write_u32(RuntimeValue addr, RuntimeValue val) {
    *(volatile uint32_t *)(uintptr_t)(uint64_t)addr = (uint32_t)(uint64_t)val;
    return NIL_VALUE;
}

RuntimeValue rt_volatile_write_u64(RuntimeValue addr, RuntimeValue val) {
    *(volatile uint64_t *)(uintptr_t)(uint64_t)addr = (uint64_t)val;
    return NIL_VALUE;
}


/* ---- rt_ptr_* (raw pointer read/write — for unsafe OS code) ---- */

RuntimeValue rt_ptr_read_i32(RuntimeValue addr, RuntimeValue offset) {
    int32_t *p = (int32_t *)((uintptr_t)addr + (uintptr_t)offset);
    return ENCODE_INT((int64_t)*p);
}

RuntimeValue rt_ptr_read_i16(RuntimeValue addr, RuntimeValue offset) {
    int16_t *p = (int16_t *)((uintptr_t)addr + (uintptr_t)offset);
    return ENCODE_INT((int64_t)*p);
}

RuntimeValue rt_ptr_read_i64(RuntimeValue addr, RuntimeValue offset) {
    int64_t *p = (int64_t *)((uintptr_t)addr + (uintptr_t)offset);
    return ENCODE_INT(*p);
}

RuntimeValue rt_ptr_write_i16(RuntimeValue addr, RuntimeValue offset, RuntimeValue value) {
    int16_t *p = (int16_t *)((uintptr_t)addr + (uintptr_t)offset);
    *p = (int16_t)DECODE_INT(value);
    return NIL_VALUE;
}

RuntimeValue rt_ptr_write_i32(RuntimeValue addr, RuntimeValue offset, RuntimeValue value) {
    int32_t *p = (int32_t *)((uintptr_t)addr + (uintptr_t)offset);
    *p = (int32_t)DECODE_INT(value);
    return NIL_VALUE;
}

RuntimeValue rt_ptr_write_i64(RuntimeValue addr, RuntimeValue offset, RuntimeValue value) {
    int64_t *p = (int64_t *)((uintptr_t)addr + (uintptr_t)offset);
    *p = DECODE_INT(value);
    return NIL_VALUE;
}

RuntimeValue rt_ptr_to_value(RuntimeValue ptr) {
    return ENCODE_PTR((void *)(uintptr_t)ptr);
}


/* ---- rt_tuple_* (tuple as thin wrapper around array) ---- */

RuntimeValue rt_tuple_new(RuntimeValue len_rv) {
    int64_t len = (int64_t)len_rv;
    serial_puts("[tup] new len="); serial_puthex((uint8_t)(len & 0xFF)); serial_puts("\r\n");
    if (len <= 0) len = 0;
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    a->len = (uint32_t)len;
    a->cap = (uint32_t)len;
    for (uint32_t i = 0; i < (uint32_t)len; i++) a->items[i] = NIL_VALUE;
    RuntimeValue result = ENCODE_PTR(a);
    serial_puts("[tup] new -> 0x"); serial_puthex((uint8_t)((result >> 8) & 0xFF)); serial_puthex((uint8_t)(result & 0xFF)); serial_puts("\r\n");
    return result;
}

RuntimeValue rt_tuple_get(RuntimeValue tuple, RuntimeValue index) {
    serial_puts("[tup] get tuple=0x"); serial_puthex((uint8_t)((tuple >> 8) & 0xFF)); serial_puthex((uint8_t)(tuple & 0xFF));
    serial_puts(" idx="); serial_puthex((uint8_t)(index & 0xFF)); serial_puts("\r\n");
    RuntimeArray *a = _decode_arr(tuple);
    int64_t i = (int64_t)index;
    if (!a || i < 0 || (uint32_t)i >= a->len) {
        serial_puts("[tup] get -> NIL (a="); serial_puthex(a ? 1 : 0); serial_puts(")\r\n");
        return NIL_VALUE;
    }
    RuntimeValue val = a->items[i];
    serial_puts("[tup] get -> 0x"); serial_puthex((uint8_t)((val >> 8) & 0xFF)); serial_puthex((uint8_t)(val & 0xFF)); serial_puts("\r\n");
    return val;
}

RuntimeValue rt_tuple_set(RuntimeValue tuple, RuntimeValue index, RuntimeValue value) {
    serial_puts("[tup] set idx="); serial_puthex((uint8_t)(index & 0xFF));
    serial_puts(" val=0x"); serial_puthex((uint8_t)((value >> 8) & 0xFF)); serial_puthex((uint8_t)(value & 0xFF)); serial_puts("\r\n");
    RuntimeArray *a = _decode_arr(tuple);
    int64_t i = (int64_t)index;
    if (!a || i < 0 || (uint32_t)i >= a->len) return 0;
    a->items[i] = value;
    return 1;
}

RuntimeValue rt_tuple_len(RuntimeValue tuple) {
    RuntimeArray *a = _decode_arr(tuple);
    return a ? (RuntimeValue)(int64_t)a->len : 0;
}


/* ---- rt_error_* (error constructors — return tagged enum or NIL) ---- */

RuntimeValue rt_error_throw(RuntimeValue msg) {
    serial_puts("RUNTIME ERROR: ");
    if (IS_HEAP(msg)) {
        RuntimeString *s = _decode_str(msg);
        if (s) {
            for (uint32_t i = 0; i < s->len; i++) serial_putchar(s->data[i]);
        }
    }
    serial_puts("\r\n");
    /* On baremetal, errors are fatal — halt */
    for (;;) {
#if defined(__x86_64__) || defined(__i386__)
        __asm__ volatile("cli; hlt");
#endif
    }
    return NIL_VALUE;
}

RuntimeValue rt_error_division_by_zero(void) {
    return rt_error_throw(rt_string_from_cstr("division by zero"));
}

RuntimeValue rt_error_index_oob(RuntimeValue index) {
    (void)index;
    return rt_error_throw(rt_string_from_cstr("index out of bounds"));
}

RuntimeValue rt_error_type_mismatch(RuntimeValue expected, RuntimeValue got) {
    (void)expected; (void)got;
    return rt_error_throw(rt_string_from_cstr("type mismatch"));
}

RuntimeValue rt_error_arg_count(RuntimeValue expected, RuntimeValue got) {
    (void)expected; (void)got;
    return rt_error_throw(rt_string_from_cstr("wrong argument count"));
}

RuntimeValue rt_error_undefined_var(RuntimeValue name) {
    (void)name;
    return rt_error_throw(rt_string_from_cstr("undefined variable"));
}

RuntimeValue rt_error_semantic(RuntimeValue msg) {
    return rt_error_throw(msg);
}

RuntimeValue rt_error_free(RuntimeValue err) {
    (void)err;
    return NIL_VALUE;
}


/* ---- rt_math_* (hyperbolic trig — the only 3 missing from baremetal_stubs) ---- */
/* Approximations using exponential series since we have no libm on baremetal */

static double _exp_approx(double x) {
    /* exp(x) via Taylor series — 20 terms gives good precision for |x| < 20 */
    if (x > 700.0) return 1e308;  /* prevent overflow */
    if (x < -700.0) return 0.0;
    double result = 1.0;
    double term = 1.0;
    for (int i = 1; i <= 25; i++) {
        term *= x / (double)i;
        result += term;
    }
    return result;
}

RuntimeValue rt_math_sinh(RuntimeValue x_raw) {
    double x;
    __builtin_memcpy(&x, &x_raw, 8);
    double ep = _exp_approx(x);
    double em = _exp_approx(-x);
    double result = (ep - em) / 2.0;
    RuntimeValue r;
    __builtin_memcpy(&r, &result, 8);
    return r;
}

RuntimeValue rt_math_cosh(RuntimeValue x_raw) {
    double x;
    __builtin_memcpy(&x, &x_raw, 8);
    double ep = _exp_approx(x);
    double em = _exp_approx(-x);
    double result = (ep + em) / 2.0;
    RuntimeValue r;
    __builtin_memcpy(&r, &result, 8);
    return r;
}

RuntimeValue rt_math_tanh(RuntimeValue x_raw) {
    double x;
    __builtin_memcpy(&x, &x_raw, 8);
    double ep = _exp_approx(x);
    double em = _exp_approx(-x);
    double result = (ep - em) / (ep + em);
    RuntimeValue r;
    __builtin_memcpy(&r, &result, 8);
    return r;
}


/* ---- rt_object_field_* (struct field access by index) ---- */

RuntimeValue rt_object_field_get(RuntimeValue object, RuntimeValue field_index) {
    if (!IS_HEAP(object)) return NIL_VALUE;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(object);
    if (!h || h->type != HEAP_OBJECT) return NIL_VALUE;
    /* Object layout: HeapHeader + uint32_t field_count + RuntimeValue fields[] */
    uint32_t *count_ptr = (uint32_t *)(h + 1);
    uint32_t count = *count_ptr;
    RuntimeValue *fields = (RuntimeValue *)(count_ptr + 1);
    /* Also handle alignment padding: fields may be at 8-byte boundary */
    uint32_t idx = (uint32_t)(int32_t)field_index;
    if (idx >= count) return NIL_VALUE;
    return fields[idx];
}

RuntimeValue rt_object_field_set(RuntimeValue object, RuntimeValue field_index, RuntimeValue value) {
    if (!IS_HEAP(object)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(object);
    if (!h || h->type != HEAP_OBJECT) return 0;
    uint32_t *count_ptr = (uint32_t *)(h + 1);
    uint32_t count = *count_ptr;
    RuntimeValue *fields = (RuntimeValue *)(count_ptr + 1);
    uint32_t idx = (uint32_t)(int32_t)field_index;
    if (idx >= count) return 0;
    fields[idx] = value;
    return 1;
}

RuntimeValue rt_field_set(RuntimeValue object, RuntimeValue field_index, RuntimeValue value) {
    return rt_object_field_set(object, field_index, value);
}


/* ---- rt_closure_* (closure introspection) ---- */

RuntimeValue rt_closure_func_ptr(RuntimeValue closure) {
    if (!IS_HEAP(closure)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(closure);
    if (!h || h->type != HEAP_CLOSURE) return 0;
    /* Closure layout: HeapHeader + func_ptr(i64) + capture_count(u32) + captures[] */
    int64_t *func = (int64_t *)(h + 1);
    return (RuntimeValue)*func;
}

RuntimeValue rt_closure_get_capture(RuntimeValue closure, RuntimeValue index) {
    if (!IS_HEAP(closure)) return NIL_VALUE;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(closure);
    if (!h || h->type != HEAP_CLOSURE) return NIL_VALUE;
    /* func_ptr(8 bytes) + capture_count(4 bytes) + padding(4 bytes) + captures[] */
    uint8_t *base = (uint8_t *)(h + 1);
    uint32_t cap_count = *(uint32_t *)(base + 8);
    RuntimeValue *captures = (RuntimeValue *)(base + 16);
    uint32_t idx = (uint32_t)(int64_t)index;
    if (idx >= cap_count) return NIL_VALUE;
    return captures[idx];
}

RuntimeValue rt_closure_set_capture(RuntimeValue closure, RuntimeValue index, RuntimeValue value) {
    if (!IS_HEAP(closure)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(closure);
    if (!h || h->type != HEAP_CLOSURE) return 0;
    uint8_t *base = (uint8_t *)(h + 1);
    uint32_t cap_count = *(uint32_t *)(base + 8);
    RuntimeValue *captures = (RuntimeValue *)(base + 16);
    uint32_t idx = (uint32_t)(int64_t)index;
    if (idx >= cap_count) return 0;
    captures[idx] = value;
    return 1;
}


/* ---- rt_generator_* (coroutine state machine) ---- */
/* Generator layout: HeapHeader + state(i64) + ctx(RV) + slot_count(u32) + slots[] */
#define HEAP_GENERATOR 8

typedef struct {
    HeapHeader   hdr;
    int64_t      state;
    RuntimeValue ctx;
    uint32_t     slot_count;
    uint32_t     _pad;
    RuntimeValue slots[];
} RuntimeGenerator;

RuntimeValue rt_generator_new(RuntimeValue slot_count) {
    uint32_t sc = (uint32_t)(int64_t)slot_count;
    RuntimeGenerator *g = (RuntimeGenerator *)malloc(sizeof(RuntimeGenerator) + sc * sizeof(RuntimeValue));
    if (!g) return NIL_VALUE;
    g->hdr.type = HEAP_GENERATOR;
    g->hdr.size = (uint32_t)(sizeof(RuntimeGenerator) + sc * sizeof(RuntimeValue));
    g->state = 0;
    g->ctx = NIL_VALUE;
    g->slot_count = sc;
    g->_pad = 0;
    for (uint32_t i = 0; i < sc; i++) g->slots[i] = NIL_VALUE;
    return ENCODE_PTR(g);
}

static RuntimeGenerator *_decode_gen(RuntimeValue v) {
    if (!IS_HEAP(v)) return (RuntimeGenerator *)0;
    RuntimeGenerator *g = (RuntimeGenerator *)DECODE_PTR(v);
    if (!g || g->hdr.type != HEAP_GENERATOR) return (RuntimeGenerator *)0;
    return g;
}

RuntimeValue rt_generator_get_state(RuntimeValue gen) {
    RuntimeGenerator *g = _decode_gen(gen);
    return g ? (RuntimeValue)g->state : -1;
}

RuntimeValue rt_generator_set_state(RuntimeValue gen, RuntimeValue state) {
    RuntimeGenerator *g = _decode_gen(gen);
    if (g) g->state = (int64_t)state;
    return NIL_VALUE;
}

RuntimeValue rt_generator_get_ctx(RuntimeValue gen) {
    RuntimeGenerator *g = _decode_gen(gen);
    return g ? g->ctx : NIL_VALUE;
}

RuntimeValue rt_generator_mark_done(RuntimeValue gen) {
    RuntimeGenerator *g = _decode_gen(gen);
    if (g) g->state = -1;
    return NIL_VALUE;
}

RuntimeValue rt_generator_load_slot(RuntimeValue gen, RuntimeValue index) {
    RuntimeGenerator *g = _decode_gen(gen);
    uint32_t i = (uint32_t)(int64_t)index;
    if (!g || i >= g->slot_count) return NIL_VALUE;
    return g->slots[i];
}

RuntimeValue rt_generator_store_slot(RuntimeValue gen, RuntimeValue index, RuntimeValue val) {
    RuntimeGenerator *g = _decode_gen(gen);
    uint32_t i = (uint32_t)(int64_t)index;
    if (!g || i >= g->slot_count) return NIL_VALUE;
    g->slots[i] = val;
    return NIL_VALUE;
}

RuntimeValue rt_generator_next(RuntimeValue gen) {
    /* The actual resumption is done by compiled code; this just checks state */
    RuntimeGenerator *g = _decode_gen(gen);
    if (!g || g->state == -1) return NIL_VALUE; /* done */
    return g->ctx;
}


/* ---- rt_async_* (coroutine-like async state machine) ---- */
/* Same layout as generator for simple baremetal async */

RuntimeValue rt_async_get_ctx(RuntimeValue future) {
    return rt_generator_get_ctx(future);
}

RuntimeValue rt_async_get_state(RuntimeValue future) {
    return rt_generator_get_state(future);
}

RuntimeValue rt_async_set_state(RuntimeValue future, RuntimeValue state) {
    return rt_generator_set_state(future, state);
}

RuntimeValue rt_async_mark_done(RuntimeValue future) {
    return rt_generator_mark_done(future);
}

/* These are no-ops on single-core baremetal — no real async scheduler */
RuntimeValue rt_async_spawn_task(RuntimeValue func) { (void)func; return NIL_VALUE; }
RuntimeValue rt_async_poll_tasks(void) { return NIL_VALUE; }
RuntimeValue rt_async_run_until_complete(RuntimeValue future) { (void)future; return NIL_VALUE; }
RuntimeValue rt_async_schedule_await(RuntimeValue future, RuntimeValue callback) { (void)future; (void)callback; return NIL_VALUE; }


/* ---- rt_atomic_* (single-core baremetal — no actual contention) ---- */

/* Atomic int: on single-core baremetal, just a boxed i64 */
RuntimeValue rt_atomic_int_new(RuntimeValue initial) {
    int64_t *p = (int64_t *)malloc(sizeof(int64_t));
    if (!p) return 0;
    *p = (int64_t)initial;
    return (RuntimeValue)(uintptr_t)p;
}

RuntimeValue rt_atomic_int_load(RuntimeValue handle) {
    int64_t *p = (int64_t *)(uintptr_t)handle;
    return p ? (RuntimeValue)*p : 0;
}

RuntimeValue rt_atomic_int_store(RuntimeValue handle, RuntimeValue value) {
    int64_t *p = (int64_t *)(uintptr_t)handle;
    if (p) *p = (int64_t)value;
    return NIL_VALUE;
}

RuntimeValue rt_atomic_int_swap(RuntimeValue handle, RuntimeValue value) {
    int64_t *p = (int64_t *)(uintptr_t)handle;
    if (!p) return 0;
    int64_t old = *p;
    *p = (int64_t)value;
    return (RuntimeValue)old;
}

RuntimeValue rt_atomic_int_compare_exchange(RuntimeValue handle, RuntimeValue current, RuntimeValue new_val) {
    int64_t *p = (int64_t *)(uintptr_t)handle;
    if (!p) return 0;
    if (*p == (int64_t)current) {
        *p = (int64_t)new_val;
        return 1;
    }
    return 0;
}

RuntimeValue rt_atomic_int_fetch_add(RuntimeValue handle, RuntimeValue value) {
    int64_t *p = (int64_t *)(uintptr_t)handle;
    if (!p) return 0;
    int64_t old = *p;
    *p += (int64_t)value;
    return (RuntimeValue)old;
}

RuntimeValue rt_atomic_int_fetch_sub(RuntimeValue handle, RuntimeValue value) {
    int64_t *p = (int64_t *)(uintptr_t)handle;
    if (!p) return 0;
    int64_t old = *p;
    *p -= (int64_t)value;
    return (RuntimeValue)old;
}

RuntimeValue rt_atomic_int_fetch_and(RuntimeValue handle, RuntimeValue value) {
    int64_t *p = (int64_t *)(uintptr_t)handle;
    if (!p) return 0;
    int64_t old = *p;
    *p &= (int64_t)value;
    return (RuntimeValue)old;
}

RuntimeValue rt_atomic_int_fetch_or(RuntimeValue handle, RuntimeValue value) {
    int64_t *p = (int64_t *)(uintptr_t)handle;
    if (!p) return 0;
    int64_t old = *p;
    *p |= (int64_t)value;
    return (RuntimeValue)old;
}

RuntimeValue rt_atomic_int_fetch_xor(RuntimeValue handle, RuntimeValue value) {
    int64_t *p = (int64_t *)(uintptr_t)handle;
    if (!p) return 0;
    int64_t old = *p;
    *p ^= (int64_t)value;
    return (RuntimeValue)old;
}

RuntimeValue rt_atomic_int_free(RuntimeValue handle) {
    (void)handle; /* bump allocator */
    return NIL_VALUE;
}

/* Atomic bool */
RuntimeValue rt_atomic_bool_new(RuntimeValue initial) {
    return rt_atomic_int_new(initial ? 1 : 0);
}

RuntimeValue rt_atomic_bool_load(RuntimeValue handle) {
    return rt_atomic_int_load(handle) ? 1 : 0;
}

RuntimeValue rt_atomic_bool_store(RuntimeValue handle, RuntimeValue value) {
    return rt_atomic_int_store(handle, value ? 1 : 0);
}

RuntimeValue rt_atomic_bool_swap(RuntimeValue handle, RuntimeValue value) {
    return rt_atomic_int_swap(handle, value ? 1 : 0) ? 1 : 0;
}

RuntimeValue rt_atomic_bool_free(RuntimeValue handle) {
    (void)handle;
    return NIL_VALUE;
}


/* ---- rt_shared_* / rt_unique_* / rt_weak_* (smart pointers) ---- */
/* On baremetal single-core: shared = ref-counted wrapper, unique = just a box */

typedef struct {
    HeapHeader   hdr;
    uint32_t     ref_count;
    uint32_t     _pad;
    RuntimeValue value;
} SharedBox;

#define HEAP_SHARED 9
#define HEAP_UNIQUE 10
#define HEAP_WEAK   11

RuntimeValue rt_shared_new(RuntimeValue value) {
    SharedBox *b = (SharedBox *)malloc(sizeof(SharedBox));
    if (!b) return NIL_VALUE;
    b->hdr.type = HEAP_SHARED;
    b->hdr.size = (uint32_t)sizeof(SharedBox);
    b->ref_count = 1;
    b->_pad = 0;
    b->value = value;
    return ENCODE_PTR(b);
}

RuntimeValue rt_shared_get(RuntimeValue shared) {
    if (!IS_HEAP(shared)) return NIL_VALUE;
    SharedBox *b = (SharedBox *)DECODE_PTR(shared);
    if (!b || b->hdr.type != HEAP_SHARED) return NIL_VALUE;
    return b->value;
}

RuntimeValue rt_shared_clone(RuntimeValue shared) {
    if (!IS_HEAP(shared)) return shared;
    SharedBox *b = (SharedBox *)DECODE_PTR(shared);
    if (!b || b->hdr.type != HEAP_SHARED) return shared;
    b->ref_count++;
    return shared;
}

RuntimeValue rt_shared_ref_count(RuntimeValue shared) {
    if (!IS_HEAP(shared)) return ENCODE_INT(0);
    SharedBox *b = (SharedBox *)DECODE_PTR(shared);
    if (!b || b->hdr.type != HEAP_SHARED) return ENCODE_INT(0);
    return ENCODE_INT((int64_t)b->ref_count);
}

RuntimeValue rt_shared_release(RuntimeValue shared) {
    if (!IS_HEAP(shared)) return NIL_VALUE;
    SharedBox *b = (SharedBox *)DECODE_PTR(shared);
    if (!b || b->hdr.type != HEAP_SHARED) return NIL_VALUE;
    if (b->ref_count > 0) b->ref_count--;
    return NIL_VALUE;
}

RuntimeValue rt_shared_needs_trace(RuntimeValue shared) {
    (void)shared;
    return 0;
}

RuntimeValue rt_shared_downgrade(RuntimeValue shared) {
    /* Create a weak reference — on baremetal, just store the pointer */
    if (!IS_HEAP(shared)) return NIL_VALUE;
    SharedBox *b = (SharedBox *)DECODE_PTR(shared);
    if (!b) return NIL_VALUE;
    /* Store the SharedBox pointer as a "weak" ref */
    SharedBox *w = (SharedBox *)malloc(sizeof(SharedBox));
    if (!w) return NIL_VALUE;
    w->hdr.type = HEAP_WEAK;
    w->hdr.size = (uint32_t)sizeof(SharedBox);
    w->ref_count = 0;
    w->value = shared; /* store original shared ref */
    return ENCODE_PTR(w);
}

RuntimeValue rt_unique_new(RuntimeValue value) {
    SharedBox *b = (SharedBox *)malloc(sizeof(SharedBox));
    if (!b) return NIL_VALUE;
    b->hdr.type = HEAP_UNIQUE;
    b->hdr.size = (uint32_t)sizeof(SharedBox);
    b->ref_count = 1;
    b->value = value;
    return ENCODE_PTR(b);
}

RuntimeValue rt_unique_get(RuntimeValue unique) {
    if (!IS_HEAP(unique)) return NIL_VALUE;
    SharedBox *b = (SharedBox *)DECODE_PTR(unique);
    if (!b || b->hdr.type != HEAP_UNIQUE) return NIL_VALUE;
    return b->value;
}

RuntimeValue rt_unique_set(RuntimeValue unique, RuntimeValue value) {
    if (!IS_HEAP(unique)) return NIL_VALUE;
    SharedBox *b = (SharedBox *)DECODE_PTR(unique);
    if (!b || b->hdr.type != HEAP_UNIQUE) return NIL_VALUE;
    b->value = value;
    return NIL_VALUE;
}

RuntimeValue rt_unique_free(RuntimeValue unique) {
    (void)unique; /* bump allocator */
    return NIL_VALUE;
}

RuntimeValue rt_unique_needs_trace(RuntimeValue unique) {
    (void)unique;
    return 0;
}

RuntimeValue rt_weak_new(RuntimeValue shared_value, RuntimeValue shared_addr) {
    (void)shared_addr;
    return rt_shared_downgrade(shared_value);
}

RuntimeValue rt_weak_upgrade(RuntimeValue weak) {
    if (!IS_HEAP(weak)) return NIL_VALUE;
    SharedBox *w = (SharedBox *)DECODE_PTR(weak);
    if (!w || w->hdr.type != HEAP_WEAK) return NIL_VALUE;
    return w->value;
}

RuntimeValue rt_weak_is_valid(RuntimeValue weak) {
    if (!IS_HEAP(weak)) return 0;
    SharedBox *w = (SharedBox *)DECODE_PTR(weak);
    if (!w || w->hdr.type != HEAP_WEAK) return 0;
    return !IS_NIL(w->value) ? 1 : 0;
}

RuntimeValue rt_weak_free(RuntimeValue weak) {
    (void)weak;
    return NIL_VALUE;
}


/* ---- rt_rwlock_* (single-core: no real locking needed) ---- */

RuntimeValue rt_rwlock_new(RuntimeValue initial) {
    /* Just box the value */
    return rt_shared_new(initial);
}

RuntimeValue rt_rwlock_read(RuntimeValue lock) {
    return rt_shared_get(lock);
}

RuntimeValue rt_rwlock_write(RuntimeValue lock) {
    return rt_shared_get(lock);
}

RuntimeValue rt_rwlock_set(RuntimeValue lock, RuntimeValue value) {
    if (!IS_HEAP(lock)) return NIL_VALUE;
    SharedBox *b = (SharedBox *)DECODE_PTR(lock);
    if (!b) return NIL_VALUE;
    b->value = value;
    return NIL_VALUE;
}

RuntimeValue rt_rwlock_try_read(RuntimeValue lock) {
    return rt_shared_get(lock);
}

RuntimeValue rt_rwlock_try_write(RuntimeValue lock) {
    return rt_shared_get(lock);
}


/* ---- rt_future_* (promise/future — single-core stub) ---- */

typedef struct {
    HeapHeader   hdr;
    int64_t      state; /* 0=pending, 1=resolved, 2=rejected */
    RuntimeValue result;
} RuntimeFuture;

#define HEAP_FUTURE 12

RuntimeValue rt_future_new(void) {
    RuntimeFuture *f = (RuntimeFuture *)malloc(sizeof(RuntimeFuture));
    if (!f) return NIL_VALUE;
    f->hdr.type = HEAP_FUTURE;
    f->hdr.size = (uint32_t)sizeof(RuntimeFuture);
    f->state = 0;
    f->result = NIL_VALUE;
    return ENCODE_PTR(f);
}

RuntimeValue rt_future_resolve(RuntimeValue future, RuntimeValue value) {
    if (!IS_HEAP(future)) return NIL_VALUE;
    RuntimeFuture *f = (RuntimeFuture *)DECODE_PTR(future);
    if (!f || f->hdr.type != HEAP_FUTURE) return NIL_VALUE;
    f->state = 1;
    f->result = value;
    return NIL_VALUE;
}

RuntimeValue rt_future_reject(RuntimeValue future, RuntimeValue error) {
    if (!IS_HEAP(future)) return NIL_VALUE;
    RuntimeFuture *f = (RuntimeFuture *)DECODE_PTR(future);
    if (!f || f->hdr.type != HEAP_FUTURE) return NIL_VALUE;
    f->state = 2;
    f->result = error;
    return NIL_VALUE;
}

RuntimeValue rt_future_is_ready(RuntimeValue future) {
    if (!IS_HEAP(future)) return 0;
    RuntimeFuture *f = (RuntimeFuture *)DECODE_PTR(future);
    if (!f || f->hdr.type != HEAP_FUTURE) return 0;
    return f->state != 0 ? 1 : 0;
}

RuntimeValue rt_future_get_result(RuntimeValue future) {
    if (!IS_HEAP(future)) return NIL_VALUE;
    RuntimeFuture *f = (RuntimeFuture *)DECODE_PTR(future);
    if (!f || f->hdr.type != HEAP_FUTURE) return NIL_VALUE;
    return f->result;
}

RuntimeValue rt_future_await(RuntimeValue future) {
    /* Synchronous on baremetal */
    return rt_future_get_result(future);
}

RuntimeValue rt_future_all(RuntimeValue futures) { (void)futures; return NIL_VALUE; }
RuntimeValue rt_future_race(RuntimeValue futures) { (void)futures; return NIL_VALUE; }


/* ---- rt_random_* (xorshift64 PRNG — no OS entropy on baremetal) ---- */

static uint64_t _prng_state = 0x12345678DEADBEEFULL;

RuntimeValue rt_random_seed(RuntimeValue seed) {
    _prng_state = (uint64_t)seed;
    if (_prng_state == 0) _prng_state = 1;
    return NIL_VALUE;
}

RuntimeValue rt_random_getstate(void) {
    return (RuntimeValue)(int64_t)_prng_state;
}

RuntimeValue rt_random_setstate(RuntimeValue state) {
    _prng_state = (uint64_t)(int64_t)state;
    if (_prng_state == 0) _prng_state = 1;
    return NIL_VALUE;
}

static uint64_t _prng_next(void) {
    uint64_t x = _prng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    _prng_state = x;
    return x;
}

RuntimeValue rt_random_random(void) {
    /* Return double in [0, 1) as raw bits */
    uint64_t r = _prng_next();
    double d = (double)(r >> 11) / (double)(1ULL << 53);
    RuntimeValue rv;
    __builtin_memcpy(&rv, &d, 8);
    return rv;
}

RuntimeValue rt_random_randint(RuntimeValue min, RuntimeValue max) {
    int64_t lo = (int64_t)min;
    int64_t hi = (int64_t)max;
    if (lo >= hi) return ENCODE_INT(lo);
    uint64_t range = (uint64_t)(hi - lo);
    uint64_t r = _prng_next() % range;
    return ENCODE_INT(lo + (int64_t)r);
}

RuntimeValue rt_random_hex(RuntimeValue len_rv) {
    static const char hex[] = "0123456789abcdef";
    int64_t len = (int64_t)len_rv;
    if (len <= 0 || len > 1024) len = 16;
    RuntimeString *s = (RuntimeString *)malloc(sizeof(RuntimeString) + (size_t)len + 1);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + (size_t)len + 1);
    s->len = (uint32_t)len;
    for (int64_t i = 0; i < len; i++) {
        s->data[i] = hex[_prng_next() & 0xF];
    }
    s->data[len] = '\0';
    return ENCODE_PTR(s);
}


/* ---- rt_rdrand (x86 hardware random) ---- */

RuntimeValue rt_rdrand(void) {
#if defined(__x86_64__)
    uint64_t val;
    unsigned char ok;
    __asm__ volatile("rdrand %0; setc %1" : "=r"(val), "=qm"(ok));
    if (ok) return ENCODE_INT((int64_t)val);
#endif
    /* Fallback to PRNG */
    return ENCODE_INT((int64_t)_prng_next());
}


/* ---- rt_memory_barrier (full memory fence) ---- */

RuntimeValue rt_memory_barrier(void) {
#if defined(__x86_64__) || defined(__i386__)
    __asm__ volatile("mfence" ::: "memory");
#else
    __asm__ volatile("" ::: "memory");
#endif
    return NIL_VALUE;
}


/* ---- rt_cpu_count (always 1 on baremetal unless SMP init) ---- */

RuntimeValue rt_cpu_count(void) {
    return ENCODE_INT(1);
}


/* ---- rt_platform_name ---- */

RuntimeValue rt_platform_name(void) {
    return rt_string_from_cstr("x86_64-baremetal-simpleos");
}


/* ---- rt_method_not_found / rt_contains / rt_unwrap_or_self ---- */

RuntimeValue rt_method_not_found(RuntimeValue name) {
    serial_puts("FATAL: method not found: ");
    RuntimeString *s = _decode_str(name);
    if (s) for (uint32_t i = 0; i < s->len; i++) serial_putchar(s->data[i]);
    serial_puts("\r\n");
    for (;;) {
#if defined(__x86_64__)
        __asm__ volatile("cli; hlt");
#endif
    }
    return NIL_VALUE;
}

RuntimeValue rt_contains(RuntimeValue haystack, RuntimeValue needle) {
    /* Array contains check */
    RuntimeArray *a = _decode_arr(haystack);
    if (a) {
        for (uint32_t i = 0; i < a->len; i++) {
            if (rt_native_eq(a->items[i], needle)) return 1;
        }
        return 0;
    }
    /* String contains check */
    RuntimeString *s = _decode_str(haystack);
    RuntimeString *n = _decode_str(needle);
    if (s && n) {
        if (n->len == 0) return 1;
        if (n->len > s->len) return 0;
        for (uint32_t i = 0; i <= s->len - n->len; i++) {
            uint32_t j;
            for (j = 0; j < n->len; j++) {
                if (s->data[i + j] != n->data[j]) break;
            }
            if (j == n->len) return 1;
        }
    }
    return 0;
}

RuntimeValue rt_unwrap_or_self(RuntimeValue val) {
    if (IS_NIL(val)) return NIL_VALUE;
    /* If it's an enum (Optional), unwrap payload */
    if (IS_HEAP(val)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
        if (h && h->type == HEAP_ENUM) {
            RuntimeEnum *e = (RuntimeEnum *)h;
            if (!IS_NIL(e->payload)) return e->payload;
        }
    }
    return val;
}

RuntimeValue rt_range(RuntimeValue start, RuntimeValue end) {
    int64_t s = DECODE_INT(start);
    int64_t e = DECODE_INT(end);
    RuntimeValue arr = rt_array_new(ENCODE_INT(e > s ? e - s : 0));
    for (int64_t i = s; i < e; i++) {
        rt_array_push(arr, ENCODE_INT(i));
    }
    return arr;
}

static int64_t _rv_to_index(RuntimeValue v) {
    /* Cranelift bare-metal slice lowering passes raw indices, not boxed ints. */
    return (int64_t)v;
}

RuntimeValue rt_slice(RuntimeValue collection, RuntimeValue start, RuntimeValue end, RuntimeValue step) {
    int64_t s = _rv_to_index(start);
    int64_t e = _rv_to_index(end);
    int64_t stride = _rv_to_index(step);
    if (stride == 0) return NIL_VALUE;

    RuntimeArray *a = _decode_arr(collection);
    if (a) {
        int64_t len = (int64_t)a->len;
        if (s < 0) s = 0;
        if (e > len) e = len;
        if (stride != 1) {
            if (stride > 0 && s >= e) return rt_array_new(ENCODE_INT(0));
            if (stride < 0 && s <= e) return rt_array_new(ENCODE_INT(0));
        } else if (s >= e) {
            return rt_array_new(ENCODE_INT(0));
        }
        RuntimeValue result = rt_array_new(ENCODE_INT(e > s ? e - s : 0));
        for (int64_t i = s; (stride > 0) ? (i < e) : (i > e); i += stride) {
            rt_array_push(result, a->items[i]);
        }
        return result;
    }

    RuntimeString *str = _decode_str(collection);
    if (str) {
        int64_t len = (int64_t)str->len;
        if (s < 0) s = 0;
        if (e > len) e = len;
        if (stride != 1) {
            return rt_string_from_cstr("");
        }
        if (s >= e) {
            return rt_string_from_cstr("");
        }
        return rt_string_new((RuntimeValue)(uintptr_t)(str->data + s), (RuntimeValue)(e - s));
    }

    return NIL_VALUE;
}


/* ---- Miscellaneous genuinely useful ---- */

RuntimeValue rt_char_from_code(RuntimeValue code) {
    int64_t c = DECODE_INT(code);
    if (c < 0 || c > 127) c = '?';
    char buf[2] = { (char)c, '\0' };
    return rt_string_from_cstr(buf);
}

RuntimeValue rt_str_hash(RuntimeValue str) {
    RuntimeString *s = _decode_str(str);
    if (!s) return ENCODE_INT(0);
    /* FNV-1a hash */
    uint64_t h = 0xcbf29ce484222325ULL;
    for (uint32_t i = 0; i < s->len; i++) {
        h ^= (uint64_t)(unsigned char)s->data[i];
        h *= 0x100000001b3ULL;
    }
    return ENCODE_INT((int64_t)h);
}

RuntimeValue rt_text_byte_len(RuntimeValue str) {
    RuntimeString *s = _decode_str(str);
    return s ? ENCODE_INT((int64_t)s->len) : ENCODE_INT(0);
}

RuntimeValue rt_text_to_bytes(RuntimeValue str) {
    RuntimeString *s = _decode_str(str);
    if (!s) return rt_array_new(ENCODE_INT(0));
    RuntimeValue arr = rt_array_new(ENCODE_INT(s->len));
    for (uint32_t i = 0; i < s->len; i++) {
        rt_array_push(arr, ENCODE_INT((int64_t)(unsigned char)s->data[i]));
    }
    return arr;
}

RuntimeValue rt_bytes_from_raw(RuntimeValue ptr, RuntimeValue len) {
    uint8_t *p = (uint8_t *)(uintptr_t)ptr;
    int64_t n = (int64_t)len;
    if (!p || n <= 0) return rt_array_new(ENCODE_INT(0));
    RuntimeValue arr = rt_array_new(ENCODE_INT(n));
    for (int64_t i = 0; i < n; i++) {
        rt_array_push(arr, ENCODE_INT((int64_t)p[i]));
    }
    return arr;
}

RuntimeValue rt_bytes_to_text(RuntimeValue arr_rv) {
    RuntimeArray *a = _decode_arr(arr_rv);
    if (!a || a->len == 0) return rt_string_from_cstr("");
    RuntimeString *s = (RuntimeString *)malloc(sizeof(RuntimeString) + a->len + 1);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + a->len + 1);
    s->len = a->len;
    for (uint32_t i = 0; i < a->len; i++) {
        s->data[i] = (char)(int64_t)DECODE_INT(a->items[i]);
    }
    s->data[a->len] = '\0';
    return ENCODE_PTR(s);
}

RuntimeValue rt_uuid_v4(void) {
    static const char hex[] = "0123456789abcdef";
    /* 8-4-4-4-12 format */
    char buf[37];
    int pos = 0;
    for (int i = 0; i < 32; i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) buf[pos++] = '-';
        buf[pos++] = hex[_prng_next() & 0xF];
    }
    buf[pos] = '\0';
    /* Fix version (4) and variant (8/9/a/b) */
    buf[14] = '4';
    buf[19] = hex[8 + (_prng_next() & 0x3)];
    return rt_string_from_cstr(buf);
}

RuntimeValue rt_free(RuntimeValue ptr) {
    (void)ptr; /* bump allocator */
    return NIL_VALUE;
}

RuntimeValue rt_list_dir_recursive(RuntimeValue path) {
    (void)path;
    /* No filesystem on baremetal — return empty array */
    return rt_array_new(ENCODE_INT(0));
}

RuntimeValue rt_sleep_secs(RuntimeValue secs) {
    /* Busy-wait delay on baremetal — very approximate */
    int64_t s = DECODE_INT(secs);
    for (int64_t i = 0; i < s * 100000000LL; i++) {
        __asm__ volatile("" ::: "memory");
    }
    return NIL_VALUE;
}

RuntimeValue rt_wait(RuntimeValue ms) {
    int64_t millis = DECODE_INT(ms);
    for (int64_t i = 0; i < millis * 100000LL; i++) {
        __asm__ volatile("" ::: "memory");
    }
    return NIL_VALUE;
}


/* ---- rt_time_* (TSC-based approximation, no real RTC on baremetal) ---- */

#if defined(__x86_64__)
static inline uint64_t _rdtsc(void) {
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}
#else
static inline uint64_t _rdtsc(void) { return 0; }
#endif

static uint64_t _boot_tsc = 0;

RuntimeValue rt_time_now(void) {
    if (_boot_tsc == 0) _boot_tsc = _rdtsc();
    uint64_t elapsed = _rdtsc() - _boot_tsc;
    /* Approximate: assume ~2 GHz TSC */
    return ENCODE_INT((int64_t)(elapsed / 2000000));
}

RuntimeValue rt_time_ms(void) {
    return rt_time_now();
}

RuntimeValue rt_time_millis(void) {
    return rt_time_now();
}

RuntimeValue rt_time_now_micros(void) {
    if (_boot_tsc == 0) _boot_tsc = _rdtsc();
    uint64_t elapsed = _rdtsc() - _boot_tsc;
    return ENCODE_INT((int64_t)(elapsed / 2000));
}

RuntimeValue rt_time_now_unix_micros(void) {
    return rt_time_now_micros();
}

RuntimeValue rt_time_now_unix_millis(void) {
    return rt_time_now();
}

RuntimeValue rt_time_now_iso(void) {
    return rt_string_from_cstr("1970-01-01T00:00:00Z");
}

/* Calendar components — no RTC, return epoch-zero */
RuntimeValue rt_time_year(RuntimeValue ts) { (void)ts; return ENCODE_INT(1970); }
RuntimeValue rt_time_month(RuntimeValue ts) { (void)ts; return ENCODE_INT(1); }
RuntimeValue rt_time_day(RuntimeValue ts) { (void)ts; return ENCODE_INT(1); }
RuntimeValue rt_time_hour(RuntimeValue ts) { (void)ts; return ENCODE_INT(0); }
RuntimeValue rt_time_minute(RuntimeValue ts) { (void)ts; return ENCODE_INT(0); }
RuntimeValue rt_time_second(RuntimeValue ts) { (void)ts; return ENCODE_INT(0); }

RuntimeValue rt_timestamp_from_iso(RuntimeValue s) { (void)s; return ENCODE_INT(0); }
RuntimeValue rt_timestamp_parse(RuntimeValue s, RuntimeValue fmt) { (void)s; (void)fmt; return ENCODE_INT(0); }
RuntimeValue rt_timestamp_to_iso(RuntimeValue ts) { (void)ts; return rt_string_from_cstr("1970-01-01T00:00:00Z"); }
RuntimeValue rt_timestamp_to_string(RuntimeValue ts) { (void)ts; return rt_string_from_cstr("1970-01-01T00:00:00Z"); }
RuntimeValue rt_timestamp_diff_seconds(RuntimeValue a, RuntimeValue b) { return ENCODE_INT(DECODE_INT(a) - DECODE_INT(b)); }


/* ---- rt_hash_* (crypto hashes — stub on baremetal, FNV for basic hashing) ---- */

RuntimeValue rt_hash_text(RuntimeValue str) {
    return rt_str_hash(str);
}

/* Crypto hashes: not available on baremetal — return empty string */
RuntimeValue rt_hash_sha256(RuntimeValue data) { (void)data; return rt_string_from_cstr(""); }
RuntimeValue rt_hash_sha512(RuntimeValue data) { (void)data; return rt_string_from_cstr(""); }
RuntimeValue rt_hash_sha3_256(RuntimeValue data) { (void)data; return rt_string_from_cstr(""); }
RuntimeValue rt_hash_blake3(RuntimeValue data) { (void)data; return rt_string_from_cstr(""); }


/* ---- rt_debug_* (debugger hooks — all no-ops on baremetal) ---- */

RuntimeValue rt_debug_is_active(void) { return 0; }
RuntimeValue rt_debug_is_paused(void) { return 0; }
RuntimeValue rt_debug_set_active(RuntimeValue v) { (void)v; return NIL_VALUE; }
RuntimeValue rt_debug_pause(void) { return NIL_VALUE; }
RuntimeValue rt_debug_continue(void) { return NIL_VALUE; }
RuntimeValue rt_debug_wait_for_continue(void) { return NIL_VALUE; }
RuntimeValue rt_debug_push_frame(RuntimeValue name) { (void)name; return NIL_VALUE; }
RuntimeValue rt_debug_pop_frame(void) { return NIL_VALUE; }
RuntimeValue rt_debug_stack_depth(void) { return ENCODE_INT(0); }
RuntimeValue rt_debug_stack_trace(void) { return rt_array_new(ENCODE_INT(0)); }
RuntimeValue rt_debug_add_breakpoint(RuntimeValue file, RuntimeValue line) { (void)file; (void)line; return NIL_VALUE; }
RuntimeValue rt_debug_remove_breakpoint(RuntimeValue file, RuntimeValue line) { (void)file; (void)line; return NIL_VALUE; }
RuntimeValue rt_debug_remove_all_breakpoints(void) { return NIL_VALUE; }
RuntimeValue rt_debug_clear_breakpoints(void) { return NIL_VALUE; }
RuntimeValue rt_debug_has_breakpoint(RuntimeValue file, RuntimeValue line) { (void)file; (void)line; return 0; }
RuntimeValue rt_debug_should_break(RuntimeValue file, RuntimeValue line) { (void)file; (void)line; return 0; }
RuntimeValue rt_debug_set_current_location(RuntimeValue file, RuntimeValue line, RuntimeValue col) { (void)file; (void)line; (void)col; return NIL_VALUE; }
RuntimeValue rt_debug_current_file(void) { return rt_string_from_cstr(""); }
RuntimeValue rt_debug_current_line(void) { return ENCODE_INT(0); }
RuntimeValue rt_debug_current_column(void) { return ENCODE_INT(0); }
RuntimeValue rt_debug_get_step_mode(void) { return ENCODE_INT(0); }
RuntimeValue rt_debug_set_step_mode(RuntimeValue mode) { (void)mode; return NIL_VALUE; }
RuntimeValue rt_debug_get_step_start_depth(void) { return ENCODE_INT(0); }
RuntimeValue rt_debug_set_step_start_depth(RuntimeValue d) { (void)d; return NIL_VALUE; }
RuntimeValue rt_debug_locals(void) { return rt_array_new(ENCODE_INT(0)); }
RuntimeValue rt_debug_globals(void) { return rt_array_new(ENCODE_INT(0)); }
RuntimeValue rt_debug_set_local(RuntimeValue name, RuntimeValue val) { (void)name; (void)val; return NIL_VALUE; }
RuntimeValue rt_debug_set_global(RuntimeValue name, RuntimeValue val) { (void)name; (void)val; return NIL_VALUE; }
RuntimeValue rt_debug_clear_locals(void) { return NIL_VALUE; }
RuntimeValue rt_debug_clear_globals(void) { return NIL_VALUE; }


/* ---- rt_is_* predicates (misc) ---- */

RuntimeValue rt_is_contract_violation(RuntimeValue v) { (void)v; return 0; }
RuntimeValue rt_is_debug_mode_enabled(void) { return 0; }
RuntimeValue rt_is_macro_trace_enabled(void) { return 0; }


/* ---- rt_fault_* (execution limits — no-ops on baremetal) ---- */

RuntimeValue rt_fault_set_timeout(RuntimeValue ms) { (void)ms; return NIL_VALUE; }
RuntimeValue rt_fault_set_execution_limit(RuntimeValue limit) { (void)limit; return NIL_VALUE; }
RuntimeValue rt_fault_set_max_recursion_depth(RuntimeValue depth) { (void)depth; return NIL_VALUE; }
RuntimeValue rt_fault_set_stack_overflow_detection(RuntimeValue enabled) { (void)enabled; return NIL_VALUE; }


/* ---- rt_gc_* ---- */

RuntimeValue rt_gc_init(void) { return NIL_VALUE; }
RuntimeValue rt_gc_malloc(RuntimeValue size) { return (RuntimeValue)(uintptr_t)malloc((size_t)size); }


/* ---- rt_get_* / rt_set_* / misc CLI ---- */

RuntimeValue rt_get_args(void) { return rt_array_new(ENCODE_INT(0)); }
RuntimeValue rt_get_env(RuntimeValue name) { (void)name; return NIL_VALUE; }
RuntimeValue rt_get_host_target_code(void) { return ENCODE_INT(0); }
RuntimeValue rt_getpid(void) { return ENCODE_INT(0); }
RuntimeValue rt_hostname(void) { return rt_string_from_cstr("simpleos"); }
RuntimeValue rt_native_build(RuntimeValue args) { (void)args; return NIL_VALUE; }
RuntimeValue rt_dyn_torch_tensor_from_bits_1d(RuntimeValue a, RuntimeValue b) { (void)a; (void)b; return NIL_VALUE; }

RuntimeValue rt_dict_insert(RuntimeValue d, RuntimeValue k, RuntimeValue v) { (void)d; (void)k; (void)v; return NIL_VALUE; }
RuntimeValue rt_ensure_dir(RuntimeValue path) { (void)path; return NIL_VALUE; }
RuntimeValue rt_exec(RuntimeValue cmd) { (void)cmd; return NIL_VALUE; }
RuntimeValue rt_shell(RuntimeValue cmd, RuntimeValue args, RuntimeValue env) { (void)cmd; (void)args; (void)env; return NIL_VALUE; }
RuntimeValue rt_system(RuntimeValue cmd) { (void)cmd; return ENCODE_INT(-1); }
RuntimeValue rt_mmap(RuntimeValue a, RuntimeValue b, RuntimeValue c) { (void)a; (void)b; (void)c; return NIL_VALUE; }
RuntimeValue rt_munmap(RuntimeValue a, RuntimeValue b) { (void)a; (void)b; return NIL_VALUE; }
RuntimeValue rt_msync(RuntimeValue a, RuntimeValue b, RuntimeValue c) { (void)a; (void)b; (void)c; return NIL_VALUE; }
RuntimeValue rt_madvise(RuntimeValue a, RuntimeValue b, RuntimeValue c) { (void)a; (void)b; (void)c; return NIL_VALUE; }


/* ===================================================================
 * 3. HOST-ONLY NO-OP STUBS
 *
 * All of these return NIL_VALUE (or 0 for predicates).
 * They need a host OS (file I/O, network, GPU, ML, database, etc.)
 * and are safe as no-ops on baremetal.
 *
 * Organised by macro arity to keep them compact.
 * =================================================================== */

/* Stub macros — silent return, NOT fatal halt */
#define NOP0(n)  RuntimeValue n(void) { return NIL_VALUE; }
#define NOP1(n)  RuntimeValue n(RuntimeValue a) { (void)a; return NIL_VALUE; }
#define NOP2(n)  RuntimeValue n(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return NIL_VALUE; }
#define NOP3(n)  RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c) { (void)a;(void)b;(void)c; return NIL_VALUE; }
#define NOP4(n)  RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d) { (void)a;(void)b;(void)c;(void)d; return NIL_VALUE; }
#define NOP5(n)  RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d, RuntimeValue e) { (void)a;(void)b;(void)c;(void)d;(void)e; return NIL_VALUE; }
#define NOP6(n)  RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d, RuntimeValue e, RuntimeValue f) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f; return NIL_VALUE; }
#define NOP7(n)  RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d, RuntimeValue e, RuntimeValue f, RuntimeValue g) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g; return NIL_VALUE; }
#define NOP8(n)  RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d, RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h; return NIL_VALUE; }

/* Void-return stub macros */
#define VNOP0(n)  void n(void) {}
#define VNOP1(n)  void n(RuntimeValue a) { (void)a; }
#define VNOP2(n)  void n(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; }

/* --- std.sys.args FFI: present-but-empty on SimpleOS until Phase 2 wires
 * argv through syscall 13. Returning 0 / "" / [] keeps std.sys.args.args()
 * callable from baremetal code without unresolved-symbol link errors.
 * Signatures match the Simple-side extern declarations at
 *   src/compiler_rust/lib/std/src/sys/args.spl:6-8
 *   rt_args_count() -> i32       (raw i32, not RuntimeValue)
 *   rt_args_get(i32) -> text     (raw i32 index, heap-tagged text)
 *   rt_args_all()  -> List<text> (heap-tagged array). */
extern RuntimeValue rt_array_new(RuntimeValue cap_val);
extern RuntimeValue rt_string_from_cstr(const char *cstr);
int32_t       rt_args_count(void)             { return 0; }
RuntimeValue  rt_args_get(int32_t index)      { (void)index; return rt_string_from_cstr(""); }
RuntimeValue  rt_args_all(void)               { return rt_array_new((RuntimeValue)0); }

/* --- Host CLI (process management, env, paths) --- */
NOP1(rt_cli_args)
NOP1(rt_cli_current_dir)
NOP0(rt_cli_dispatch_rust)
NOP1(rt_cli_env)
NOP1(rt_cli_exit)
NOP1(rt_cli_get_args)
NOP1(rt_cli_get_env)
NOP0(rt_cli_home_dir)
NOP0(rt_cli_hostname)
NOP0(rt_cli_is_interactive)
NOP1(rt_cli_is_tty)
NOP0(rt_cli_path_separator)
NOP0(rt_cli_print_help)
NOP0(rt_cli_print_version)
NOP1(rt_cli_read_line)
NOP1(rt_cli_read_password)
NOP2(rt_cli_run_command)
NOP1(rt_cli_run_file)
NOP3(rt_cli_run_file_with_args)
NOP2(rt_cli_run_shell)
NOP2(rt_cli_set_env)
NOP0(rt_cli_version)
NOP1(rt_cli_watch_file)
NOP2(rt_cli_write_file)

/* CLI handle-based I/O (52 functions from rt_cli_handle_* and rt_cli_run_*) */
NOP1(rt_cli_handle_close)
NOP1(rt_cli_handle_exit_code)
NOP1(rt_cli_handle_is_alive)
NOP1(rt_cli_handle_kill)
NOP1(rt_cli_handle_pid)
NOP2(rt_cli_handle_read)
NOP2(rt_cli_handle_read_err)
NOP2(rt_cli_handle_read_line)
NOP1(rt_cli_handle_read_stdout)
NOP1(rt_cli_handle_stderr)
NOP1(rt_cli_handle_stdin)
NOP1(rt_cli_handle_stdout)
NOP1(rt_cli_handle_wait)
NOP2(rt_cli_handle_write)
NOP3(rt_cli_handle_write_stdin)
NOP1(rt_cli_run_async)
NOP4(rt_cli_run_async_with_env)
NOP3(rt_cli_run_async_with_io)
NOP2(rt_cli_run_background)
NOP3(rt_cli_run_capture)
NOP4(rt_cli_run_capture_with_env)
NOP5(rt_cli_run_capture_with_limits)
NOP2(rt_cli_run_detached)
NOP2(rt_cli_run_exec)
NOP4(rt_cli_run_exec_with_env)
NOP2(rt_cli_run_in_dir)
NOP2(rt_cli_run_pipe)
NOP5(rt_cli_run_pipe_chain)
NOP3(rt_cli_run_pipe_with_input)
NOP3(rt_cli_run_shell_capture)
NOP4(rt_cli_run_shell_with_env)
NOP5(rt_cli_run_shell_with_limits)
NOP3(rt_cli_run_with_input)
NOP4(rt_cli_run_with_timeout)

/* --- Host env/set --- */
NOP1(rt_current_dir)
NOP1(rt_env_current_dir)
NOP1(rt_env_get)
NOP0(rt_env_home_dir)
NOP0(rt_env_os)
NOP0(rt_env_path_separator)
NOP2(rt_env_set)
NOP1(rt_command_run)
NOP2(rt_set_current_dir)
NOP1(rt_set_debug_mode)
NOP2(rt_set_env)
NOP1(rt_set_macro_trace)

/* --- Host file I/O --- */
NOP1(rt_file_copy)
NOP2(rt_file_copy_to)
NOP1(rt_file_create)
NOP1(rt_file_exists)
NOP1(rt_file_is_dir)
NOP1(rt_file_is_file)
NOP2(rt_file_lock)
NOP2(rt_file_metadata)
NOP2(rt_file_move)
NOP1(rt_file_rename)
NOP2(rt_file_symlink)
NOP1(rt_file_unlock)
NOP2(rt_mkdir)
NOP1(rt_dir_create)
NOP2(rt_dir_list)
NOP1(rt_read_stdin)
NOP1(rt_read_line)
NOP2(rt_read_file)
NOP2(rt_write_stdout)
NOP2(rt_write_stderr)

/* --- Host I/O (io_*, epoll, terminal, stdin/out) --- */
NOP1(rt_io_file_close)
NOP2(rt_io_file_flush)
NOP3(rt_io_file_metadata)
NOP2(rt_io_file_open)
NOP3(rt_io_file_read)
NOP3(rt_io_file_read_all)
NOP2(rt_io_file_read_line)
NOP3(rt_io_file_seek)
NOP3(rt_io_file_write)
NOP4(rt_io_file_write_at)
NOP2(rt_io_file_truncate)
NOP3(rt_io_tcp_accept)
NOP1(rt_io_tcp_close)
NOP3(rt_io_tcp_connect)
NOP4(rt_io_tcp_connect_timeout)
NOP2(rt_io_tcp_listen)
NOP4(rt_io_tcp_read)
NOP3(rt_io_tcp_set_keepalive)
NOP2(rt_io_tcp_set_nodelay)
NOP3(rt_io_tcp_set_timeout)
NOP1(rt_io_tcp_shutdown)
NOP2(rt_io_tcp_shutdown_how)
NOP4(rt_io_tcp_write)
NOP2(rt_io_tcp_write_all)
NOP4(rt_io_tcp_write_all_timeout)
NOP4(rt_io_tcp_write_timeout)
NOP2(rt_io_udp_bind)
NOP1(rt_io_udp_close)
NOP4(rt_io_udp_recv)
NOP4(rt_io_udp_recv_from)
NOP4(rt_io_udp_send)
NOP4(rt_io_udp_send_to)
NOP2(rt_io_udp_set_broadcast)
NOP3(rt_io_udp_set_timeout)
NOP1(rt_epoll_create)
NOP3(rt_epoll_add)
NOP1(rt_epoll_wait)
NOP1(rt_stdin_read)
NOP2(rt_stdin_read_byte)
NOP1(rt_stdin_read_char)
NOP2(rt_stdin_read_line)
/* rt_stdout_write / rt_stderr_write — emit Simple-string bytes to the
 * COM1 serial port. On SimpleOS the UART is the shared stdout/stderr
 * sink (no tty/pty layer yet); both names route to the same physical
 * path. This replaces the old NOP1 stubs so std.io.Stdout / std.io.Stderr
 * and host/sys_simple.rt_stdout_write callers actually produce output. */
static RuntimeValue rt_serial_write_value(RuntimeValue data) {
    if (IS_HEAP(data)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(data);
        if (h && h->type == HEAP_STRING) {
            RuntimeString *s = (RuntimeString *)h;
            if (s->len < 0x100000) {
                for (uint32_t i = 0; i < s->len; i++) serial_putchar(s->data[i]);
                return ENCODE_INT((int64_t)s->len);
            }
        }
    }
    return ENCODE_INT(0);
}
RuntimeValue rt_stdout_write(RuntimeValue data) { return rt_serial_write_value(data); }
NOP1(rt_stdout_flush)
RuntimeValue rt_stderr_write(RuntimeValue data) { return rt_serial_write_value(data); }
NOP1(rt_stderr_flush)
NOP1(rt_terminal_clear)
NOP3(rt_terminal_set_cursor)
NOP1(rt_terminal_size)
NOP3(rt_term_set_color)
NOP1(rt_term_reset)

/* --- Host path operations --- */
NOP1(rt_path_basename)
NOP1(rt_path_canonicalize)
NOP1(rt_path_dirname)
NOP1(rt_path_exists)
NOP1(rt_path_extension)
NOP1(rt_path_is_absolute)
NOP1(rt_path_is_dir)
NOP1(rt_path_is_file)
NOP1(rt_path_is_relative)
NOP2(rt_path_join)
NOP1(rt_path_normalize)
NOP2(rt_path_relative_to)
NOP1(rt_path_stem)

/* --- Host process --- */
NOP2(rt_process_kill)
NOP1(rt_process_wait)
NOP2(rt_fork_exec)
NOP2(rt_fork_exec_capture)
NOP3(rt_fork_exec_with_env)
NOP3(rt_fork_exec_with_timeout)
NOP1(rt_fork_wait)
NOP2(rt_ps_list)
NOP1(rt_ps_info)

/* --- Host concurrency (actors, channels, threads, barriers, mutexes) --- */
NOP2(rt_actor_spawn)
NOP2(rt_actor_send)
NOP0(rt_actor_recv)
NOP1(rt_actor_reply)
NOP1(rt_actor_join)
NOP1(rt_actor_id)
NOP1(rt_actor_is_alive)
NOP2(rt_channel_free)
NOP2(rt_channel_id)
NOP1(rt_channel_is_closed)
NOP2(rt_channel_recv_timeout)
NOP1(rt_barrier_new)
NOP1(rt_barrier_wait)
NOP1(rt_barrier_thread_count)
NOP1(rt_barrier_free)
NOP1(rt_condition_new)
NOP1(rt_condition_wait)
NOP1(rt_condition_notify)
NOP1(rt_condition_notify_all)
NOP1(rt_context_new)
NOP2(rt_context_get)
NOP3(rt_context_set)
NOP1(rt_mutex_new)
NOP1(rt_mutex_lock)
NOP1(rt_mutex_unlock)
NOP1(rt_mutex_try_lock)
NOP1(rt_semaphore_new)
NOP1(rt_semaphore_acquire)
NOP1(rt_semaphore_release)
NOP1(rt_semaphore_try_acquire)
NOP3(rt_par_for_each)
NOP3(rt_par_map)
NOP4(rt_par_reduce)
NOP2(rt_par_execute)
NOP1(rt_thread_new)
NOP1(rt_thread_join)
NOP1(rt_thread_detach)
NOP1(rt_thread_is_alive)
NOP1(rt_thread_id)
NOP1(rt_thread_name)

/* --- Host async I/O --- */
NOP2(rt_async_tcp_accept)
NOP3(rt_async_tcp_connect)
NOP3(rt_async_tcp_read)
NOP3(rt_async_tcp_write)
NOP1(rt_async_tcp_close)
NOP2(rt_async_tcp_listen)
NOP3(rt_async_file_read)
NOP3(rt_async_file_write)
NOP2(rt_async_udp_recv)
NOP3(rt_async_udp_send)
NOP3(rt_async_tls_connect)
NOP3(rt_async_tls_read)
NOP3(rt_async_tls_write)
NOP3(rt_async_http_request)
NOP2(rt_async_dns_resolve)
NOP1(rt_async_pipe_close)

/* --- Host async executor --- */
NOP0(rt_executor_new)
NOP1(rt_executor_run)
NOP2(rt_executor_spawn)
NOP1(rt_executor_block_on)
NOP1(rt_executor_shutdown)
NOP1(rt_executor_is_running)
NOP0(rt_executor_current)
NOP1(rt_executor_poll)
NOP1(rt_executor_wake)

/* --- Host testing (BDD, coverage, contracts, hooks) --- */
NOP2(rt_bdd_describe_start)
NOP0(rt_bdd_describe_end)
NOP2(rt_bdd_it_start)
NOP1(rt_bdd_it_end)
NOP1(rt_bdd_before_each)
NOP1(rt_bdd_after_each)
NOP2(rt_bdd_expect_eq)
NOP1(rt_bdd_expect_truthy)
NOP2(rt_bdd_expect_fail)
NOP0(rt_bdd_has_failure)
NOP0(rt_bdd_format_results)
NOP0(rt_bdd_clear_state)
NOP2(rt_bdd_describe_start_rv)
NOP1(rt_bdd_it_start_rv)
NOP2(rt_bdd_expect_eq_rv)
NOP1(rt_bdd_expect_truthy_rv)
NOP1(rt_coverage_clear)
NOP0(rt_coverage_dump)
NOP2(rt_coverage_file_end)
NOP2(rt_coverage_file_start)
NOP1(rt_coverage_fn_end)
NOP2(rt_coverage_fn_start)
NOP1(rt_coverage_hit)
NOP0(rt_coverage_is_enabled)
NOP3(rt_coverage_line)
NOP0(rt_coverage_report)
NOP0(rt_coverage_summary)
NOP2(rt_contract_violation)
NOP3(rt_contract_violation_msg)
NOP4(rt_contract_violation_detail)
NOP3(rt_contract_violation_with_context)
NOP1(rt_contract_violation_type)
NOP2(rt_hook_register)
NOP1(rt_hook_unregister)
NOP3(rt_hook_invoke)
NOP2(rt_hook_invoke_before)
NOP2(rt_hook_invoke_after)
NOP2(rt_hook_set)
NOP1(rt_hook_get)
NOP1(rt_hook_has)
NOP1(rt_hook_remove)
NOP0(rt_hook_clear)
NOP1(rt_hook_list)
NOP2(rt_hook_count)
NOP1(rt_hook_enabled)
NOP1(rt_test_assert)
NOP2(rt_test_assert_eq)
NOP2(rt_test_assert_ne)
NOP2(rt_test_assert_throws)
NOP2(rt_test_expect)
NOP1(rt_test_fail)

/* --- Host network (HTTP, TCP, TLS, FTP, SSH, SFTP, WebSocket) --- */
NOP3(rt_http_get)
NOP4(rt_http_post)
NOP4(rt_http_put)
NOP3(rt_http_delete)
NOP4(rt_http_patch)
NOP2(rt_http_head)
NOP4(rt_http_request)
NOP5(rt_http_request_with_body)
NOP3(rt_http_request_with_headers)
NOP2(rt_http_request_async)
NOP1(rt_http_response_body)
NOP1(rt_http_response_headers)
NOP1(rt_http_response_status)
NOP5(rt_http_client_new)
NOP2(rt_http_client_get)
NOP3(rt_http_client_post)
NOP2(rt_http_client_close)
NOP1(rt_http_client_set_timeout)
NOP4(rt_http_server_new)
NOP2(rt_http_server_route)
NOP1(rt_http_server_start)
NOP1(rt_http_server_stop)
NOP2(rt_http_server_on_request)
NOP1(rt_http_server_port)
NOP1(rt_tcp_connect)
NOP1(rt_udp_bind)
NOP1(rt_socket_close)
NOP2(rt_socket_read)
NOP2(rt_socket_write)
NOP3(rt_tls_client_new)
NOP4(rt_tls_client_connect)
NOP3(rt_tls_client_read)
NOP3(rt_tls_client_write)
NOP1(rt_tls_client_close)
NOP2(rt_tls_client_set_cert)
NOP2(rt_tls_client_set_key)
NOP2(rt_tls_client_set_ca)
NOP1(rt_tls_client_verify)
NOP2(rt_tls_client_set_timeout)
NOP1(rt_tls_client_peer_cert)
NOP3(rt_tls_server_new)
NOP4(rt_tls_server_bind)
NOP3(rt_tls_server_accept)
NOP1(rt_tls_server_close)
NOP2(rt_tls_server_set_cert)
NOP2(rt_tls_server_set_key)
NOP2(rt_tls_server_set_ca)
NOP1(rt_tls_server_verify)
NOP2(rt_tls_server_set_timeout)
NOP1(rt_tls_server_client_count)
NOP2(rt_tls_get_cert)
NOP2(rt_tls_get_cipher)
NOP2(rt_tls_get_protocol)
NOP2(rt_tls_get_peer)
NOP2(rt_tls_get_alpn)
NOP2(rt_tls_get_sni)
NOP1(rt_tls_get_error)
NOP4(rt_ftp_connect)
NOP1(rt_ftp_close)
NOP2(rt_ftp_get)
NOP3(rt_ftp_put)
NOP2(rt_ftp_delete)
NOP2(rt_ftp_mkdir)
NOP2(rt_ftp_rmdir)
NOP2(rt_ftp_list)
NOP2(rt_ftp_rename)
NOP2(rt_ftp_size)
NOP2(rt_ftp_exists)
NOP2(rt_ftp_cd)
NOP0(rt_ftp_pwd)
NOP3(rt_ftp_chmod)
NOP2(rt_ftp_type)
NOP2(rt_ftp_passive)
NOP3(rt_ftp_resume)
NOP3(rt_ftp_append)
NOP3(rt_ftp_download_range)
NOP3(rt_ftp_upload_stream)
NOP5(rt_ftp_connect_tls)
NOP1(rt_ftp_feat)
NOP4(rt_ftp_custom_command)
NOP2(rt_ftp_stat)
NOP1(rt_ftp_keepalive)
NOP4(rt_ssh_connect)
NOP1(rt_ssh_close)
NOP2(rt_ssh_exec)
NOP3(rt_ssh_exec_capture)
NOP2(rt_ssh_upload)
NOP2(rt_ssh_download)
NOP4(rt_ssh_connect_key)
NOP3(rt_ssh_port_forward)
NOP1(rt_ssh_is_connected)
NOP2(rt_ssh_sftp)
NOP3(rt_ssh_exec_stream)
NOP2(rt_ssh_shell)
NOP2(rt_ssh_tunnel)
NOP3(rt_ssh_exec_with_env)
NOP3(rt_ssh_exec_with_timeout)
NOP4(rt_ssh_exec_with_pty)
NOP3(rt_ssh_scp_upload)
NOP3(rt_ssh_scp_download)
NOP2(rt_ssh_agent_auth)
NOP2(rt_ssh_keepalive)
NOP2(rt_sftp_open)
NOP1(rt_sftp_close)
NOP2(rt_sftp_read)
NOP2(rt_sftp_write)
NOP2(rt_sftp_mkdir)
NOP2(rt_sftp_rmdir)
NOP2(rt_sftp_list)
NOP2(rt_sftp_stat)
NOP2(rt_sftp_rename)
NOP1(rt_sftp_remove)
NOP3(rt_ws_connect)
NOP2(rt_ws_send)
NOP1(rt_ws_recv)
NOP1(rt_ws_close)

/* --- Host database (SQLite) --- */
NOP1(rt_sqlite_open)
NOP1(rt_sqlite_close)
NOP2(rt_sqlite_exec)
NOP3(rt_sqlite_query)
NOP3(rt_sqlite_prepare)
NOP2(rt_sqlite_step)
NOP1(rt_sqlite_finalize)
NOP3(rt_sqlite_bind_int)
NOP3(rt_sqlite_bind_text)
NOP3(rt_sqlite_bind_float)
NOP2(rt_sqlite_bind_null)
NOP2(rt_sqlite_column_int)
NOP2(rt_sqlite_column_text)
NOP2(rt_sqlite_column_float)
NOP2(rt_sqlite_column_type)
NOP1(rt_sqlite_column_count)
NOP2(rt_sqlite_column_name)
NOP1(rt_sqlite_changes)
NOP1(rt_sqlite_last_insert_rowid)
NOP2(rt_sqlite_transaction)
NOP1(rt_sqlite_commit)
NOP1(rt_sqlite_rollback)
NOP2(rt_sqlite_pragma)
NOP3(rt_sqlite_create_function)
NOP2(rt_sqlite_table_exists)
NOP1(rt_sqlite_error_msg)
NOP1(rt_sqlite_backup)
NOP1(rt_sqlite_restore)

/* --- Host regex --- */
NOP1(rt_regex_new)
NOP2(rt_regex_match)
NOP2(rt_regex_is_match)
NOP2(rt_regex_find_all)
NOP3(rt_regex_replace)
NOP3(rt_regex_replace_all)
NOP2(rt_regex_split)
NOP1(rt_regex_groups)
NOP1(rt_regex_free)
NOP2(rt_regex_find)
NOP2(rt_regex_captures)
NOP3(rt_regex_replace_fn)
NOP2(rt_regex_count)
NOP1(rt_regex_pattern)
NOP1(rt_regex_flags)

/* --- Host debug tools (perf, ptrace, DWARF) --- */
NOP0(rt_perf_now)
NOP0(rt_perf_start)
NOP0(rt_perf_stop)
NOP0(rt_perf_elapsed)
NOP1(rt_perf_mark)
NOP1(rt_perf_measure)
NOP0(rt_perf_reset)
NOP1(rt_perf_report)
NOP2(rt_perf_compare)
NOP1(rt_perf_threshold)
NOP2(rt_ptrace_attach)
NOP1(rt_ptrace_detach)
NOP2(rt_ptrace_peek)
NOP3(rt_ptrace_poke)
NOP1(rt_ptrace_cont)
NOP1(rt_ptrace_step)
NOP1(rt_ptrace_getregs)
NOP2(rt_ptrace_setregs)
NOP2(rt_dwarf_open)
NOP1(rt_dwarf_close)
NOP2(rt_dwarf_lookup_symbol)
NOP2(rt_dwarf_addr_to_line)
NOP2(rt_dwarf_line_to_addr)
NOP1(rt_dwarf_list_functions)

/* --- Host GUI/graphics (lyon, font, image) --- */
NOP1(rt_lyon_path_new)
NOP2(rt_lyon_path_move_to)
NOP3(rt_lyon_path_line_to)
NOP5(rt_lyon_path_quad_to)
NOP7(rt_lyon_path_cubic_to)
NOP1(rt_lyon_path_close)
NOP1(rt_lyon_path_free)
NOP3(rt_lyon_fill_path)
NOP3(rt_lyon_fill_rect)
NOP3(rt_lyon_fill_circle)
NOP4(rt_lyon_fill_rounded_rect)
NOP4(rt_lyon_fill_polygon)
NOP3(rt_lyon_fill_ellipse)
NOP3(rt_lyon_fill_with_color)
NOP3(rt_lyon_stroke_path)
NOP4(rt_lyon_stroke_rect)
NOP4(rt_lyon_stroke_circle)
NOP5(rt_lyon_stroke_rounded_rect)
NOP4(rt_lyon_stroke_line)
NOP4(rt_lyon_stroke_polyline)
NOP4(rt_lyon_stroke_with_color)
NOP1(rt_lyon_vertex_count)
NOP2(rt_lyon_vertex_x)
NOP2(rt_lyon_vertex_y)
NOP2(rt_lyon_vertex_color)
NOP1(rt_lyon_vertex_free)
NOP4(rt_lyon_transform_translate)
NOP4(rt_lyon_transform_scale)
NOP3(rt_lyon_transform_rotate)
NOP2(rt_lyon_transform_matrix)
NOP1(rt_lyon_transform_identity)
NOP2(rt_lyon_transform_apply)
NOP1(rt_font_load)
NOP2(rt_font_load_from_memory)
NOP3(rt_font_render)
NOP3(rt_font_measure)
NOP1(rt_font_height)
NOP1(rt_font_ascent)
NOP1(rt_font_descent)
NOP2(rt_font_glyph)
NOP1(rt_font_free)
NOP1(rt_image_load)
NOP2(rt_image_save)
NOP3(rt_image_resize)
NOP4(rt_image_crop)
NOP1(rt_image_width)
NOP1(rt_image_height)

/* --- Host multimedia (SDL2, gamepad, audio) --- */
NOP0(rt_sdl2_init)
NOP0(rt_sdl2_quit)
NOP4(rt_sdl2_create_window)
NOP0(rt_sdl2_poll_event)
NOP1(rt_sdl2_get_event_type)
NOP1(rt_sdl2_get_key)
NOP1(rt_sdl2_get_mouse_x)
NOP1(rt_sdl2_get_mouse_y)
NOP1(rt_sdl2_get_mouse_button)
NOP1(rt_sdl2_event_is_quit)
NOP1(rt_sdl2_event_is_key_down)
NOP1(rt_sdl2_event_is_key_up)
NOP1(rt_sdl2_event_is_mouse_down)
NOP1(rt_sdl2_event_is_mouse_up)
NOP1(rt_sdl2_event_is_mouse_move)
NOP1(rt_sdl2_event_key_code)
NOP1(rt_sdl2_event_mouse_button)
NOP1(rt_sdl2_event_mouse_x)
NOP1(rt_sdl2_event_mouse_y)
NOP1(rt_sdl2_create_renderer)
NOP1(rt_sdl2_clear)
NOP1(rt_sdl2_present)
NOP5(rt_sdl2_draw_rect)
NOP3(rt_sdl2_set_draw_color)
NOP4(rt_sdl2_set_draw_color_rgba)
NOP2(rt_sdl2_load_texture)
NOP5(rt_sdl2_draw_texture)
NOP5(rt_sdl2_draw_line)
NOP5(rt_sdl2_draw_circle)
NOP6(rt_sdl2_fill_rect)
NOP3(rt_sdl2_get_ticks)
NOP1(rt_sdl2_delay)
NOP3(rt_sdl2_get_window_size)
NOP5(rt_sdl2_set_clip_rect)
NOP1(rt_sdl2_reset_clip_rect)
NOP2(rt_sdl2_set_window_title)
NOP3(rt_sdl2_set_window_size)
NOP1(rt_sdl2_destroy_window)
NOP1(rt_sdl2_destroy_renderer)
NOP0(rt_gamepad_init)
NOP0(rt_gamepad_shutdown)
NOP0(rt_gamepad_poll)
NOP1(rt_gamepad_is_connected)
NOP2(rt_gamepad_axis)
NOP2(rt_gamepad_button)
NOP0(rt_gamepad_count)
NOP1(rt_gamepad_name)
NOP1(rt_gamepad_rumble)
NOP2(rt_gamepad_event_type)
NOP2(rt_gamepad_event_id)
NOP2(rt_gamepad_event_axis)
NOP2(rt_gamepad_event_value)
NOP2(rt_gamepad_event_button)
NOP2(rt_gamepad_event_pressed)
NOP2(rt_gamepad_event_device)
NOP2(rt_gamepad_event_timestamp)
NOP2(rt_gamepad_event_axis_value)
NOP2(rt_gamepad_event_button_value)
NOP0(rt_audio_init)
NOP0(rt_audio_shutdown)
NOP1(rt_audio_load)
NOP1(rt_audio_play)
NOP1(rt_audio_stop)
NOP1(rt_audio_pause)
NOP1(rt_audio_resume)
NOP2(rt_audio_set_volume)
NOP2(rt_audio_set_pitch)
NOP2(rt_audio_set_pan)
NOP2(rt_audio_set_loop)
NOP2(rt_audio_set_position)
NOP1(rt_audio_is_playing)
NOP1(rt_audio_duration)
NOP1(rt_audio_position)
NOP0(rt_audio_master_volume)
NOP1(rt_audio_set_master_volume)
NOP1(rt_audio_free)

/* --- Host GPU (CUDA, Vulkan, Metal, OpenCL/oneAPI, ROCm, SIMD/vec) --- */
NOP0(rt_cuda_init)
NOP0(rt_cuda_available)
NOP0(rt_cuda_device_count)
NOP1(rt_cuda_device_get)
NOP1(rt_cuda_device_name)
NOP1(rt_cuda_device_compute_capability)
NOP1(rt_cuda_ctx_create)
NOP1(rt_cuda_ctx_destroy)
NOP0(rt_cuda_ctx_synchronize)
NOP1(rt_cuda_mem_alloc)
NOP1(rt_cuda_mem_free)
NOP3(rt_cuda_memcpy_htod)
NOP3(rt_cuda_memcpy_dtoh)
NOP3(rt_cuda_memcpy_dtod)
NOP3(rt_cuda_memset)
NOP1(rt_cuda_module_load)
NOP1(rt_cuda_module_load_data)
NOP1(rt_cuda_module_unload)
NOP2(rt_cuda_module_get_function)
NOP8(rt_cuda_launch_kernel)
NOP0(rt_cuda_sync)
NOP1(rt_cuda_get_error_string)
NOP1(rt_cuda_stream_create)
NOP1(rt_cuda_stream_destroy)
NOP1(rt_cuda_stream_sync)
NOP3(rt_cuda_memcpy_async)
NOP2(rt_cuda_event_create)
NOP2(rt_cuda_event_record)
NOP2(rt_cuda_event_sync)
NOP2(rt_cuda_event_elapsed)
NOP2(rt_cuda_event_destroy)
NOP2(rt_cuda_set_device)
NOP0(rt_cuda_get_device)
NOP1(rt_cuda_device_mem_info)
NOP2(rt_cuda_device_prop)
NOP1(rt_cuda_malloc_managed)
NOP1(rt_cuda_malloc_host)
NOP1(rt_cuda_free_host)

/* Vulkan — all 63 stubs */
NOP0(rt_vulkan_create_instance)
NOP1(rt_vulkan_destroy_instance)
NOP2(rt_vulkan_create_device)
NOP1(rt_vulkan_destroy_device)
NOP2(rt_vulkan_create_buffer)
NOP1(rt_vulkan_destroy_buffer)
NOP2(rt_vulkan_create_image)
NOP1(rt_vulkan_destroy_image)
NOP2(rt_vulkan_create_pipeline)
NOP1(rt_vulkan_destroy_pipeline)
NOP2(rt_vulkan_create_shader)
NOP1(rt_vulkan_destroy_shader)
NOP2(rt_vulkan_create_command_pool)
NOP1(rt_vulkan_destroy_command_pool)
NOP2(rt_vulkan_create_descriptor_set)
NOP1(rt_vulkan_destroy_descriptor_set)
NOP2(rt_vulkan_create_render_pass)
NOP1(rt_vulkan_destroy_render_pass)
NOP2(rt_vulkan_create_framebuffer)
NOP1(rt_vulkan_destroy_framebuffer)
NOP2(rt_vulkan_create_swapchain)
NOP1(rt_vulkan_destroy_swapchain)
NOP2(rt_vulkan_create_sampler)
NOP1(rt_vulkan_destroy_sampler)
NOP2(rt_vulkan_create_fence)
NOP1(rt_vulkan_destroy_fence)
NOP2(rt_vulkan_create_semaphore)
NOP1(rt_vulkan_destroy_semaphore)
NOP2(rt_vulkan_create_surface)
NOP2(rt_vulkan_allocate_memory)
NOP1(rt_vulkan_free_memory)
NOP3(rt_vulkan_bind_buffer_memory)
NOP3(rt_vulkan_bind_image_memory)
NOP3(rt_vulkan_map_memory)
NOP1(rt_vulkan_unmap_memory)
NOP2(rt_vulkan_begin_command_buffer)
NOP1(rt_vulkan_end_command_buffer)
NOP2(rt_vulkan_submit)
NOP1(rt_vulkan_queue_wait_idle)
NOP1(rt_vulkan_device_wait_idle)
NOP2(rt_vulkan_begin_render_pass)
NOP1(rt_vulkan_end_render_pass)
NOP2(rt_vulkan_cmd_bind_pipeline)
NOP3(rt_vulkan_cmd_bind_vertex_buffer)
NOP2(rt_vulkan_cmd_bind_index_buffer)
NOP3(rt_vulkan_cmd_draw)
NOP4(rt_vulkan_cmd_draw_indexed)
NOP3(rt_vulkan_cmd_dispatch)
NOP3(rt_vulkan_cmd_copy_buffer)
NOP4(rt_vulkan_cmd_copy_image)
NOP1(rt_vulkan_acquire_next_image)
NOP1(rt_vulkan_present)
NOP3(rt_vulkan_update_descriptor_set)
NOP2(rt_vulkan_cmd_push_constants)
NOP3(rt_vulkan_cmd_set_viewport)
NOP3(rt_vulkan_cmd_set_scissor)
NOP4(rt_vulkan_cmd_blit_image)
NOP2(rt_vulkan_cmd_clear_color)
NOP2(rt_vulkan_cmd_pipeline_barrier)
NOP2(rt_vulkan_wait_for_fence)
NOP1(rt_vulkan_reset_fence)
NOP0(rt_vulkan_get_physical_device)
NOP1(rt_vulkan_get_queue)

/* VK command-level stubs */
NOP2(rt_vk_cmd_begin_render_pass)
NOP1(rt_vk_cmd_end_render_pass)
NOP2(rt_vk_cmd_bind_pipeline)
NOP3(rt_vk_cmd_bind_vertex_buffer)
NOP2(rt_vk_cmd_bind_index_buffer)
NOP3(rt_vk_cmd_draw)
NOP3(rt_vk_cmd_dispatch)
NOP4(rt_vk_cmd_draw_indexed)
NOP3(rt_vk_cmd_copy_buffer)
NOP2(rt_vk_create_buffer)
NOP1(rt_vk_destroy_buffer)
NOP2(rt_vk_create_pipeline)
NOP1(rt_vk_destroy_pipeline)
NOP2(rt_vk_create_descriptor_set)
NOP1(rt_vk_destroy_descriptor_set)
NOP3(rt_vk_update_descriptor_set)
NOP2(rt_vk_create_shader_module)
NOP1(rt_vk_destroy_shader_module)
NOP2(rt_vk_allocate_memory)
NOP1(rt_vk_free_memory)
NOP3(rt_vk_bind_buffer_memory)
NOP3(rt_vk_map_memory)
NOP1(rt_vk_unmap_memory)
NOP2(rt_vk_create_command_pool)
NOP1(rt_vk_destroy_command_pool)
NOP2(rt_vk_allocate_command_buffer)
NOP1(rt_vk_begin_command_buffer)
NOP1(rt_vk_end_command_buffer)
NOP2(rt_vk_submit)
NOP1(rt_vk_queue_wait)
NOP2(rt_vk_create_fence)
NOP1(rt_vk_destroy_fence)
NOP1(rt_vk_wait_fence)
NOP1(rt_vk_reset_fence)
NOP2(rt_vk_create_semaphore)
NOP1(rt_vk_destroy_semaphore)
NOP2(rt_vk_image_create)
NOP1(rt_vk_image_destroy)
NOP2(rt_vk_image_view_create)
NOP1(rt_vk_image_view_destroy)
NOP2(rt_vk_image_layout_transition)
NOP0(rt_vk_instance_create)
NOP1(rt_vk_instance_destroy)
NOP0(rt_vk_device_create)
NOP1(rt_vk_device_destroy)

/* Metal stubs */
NOP0(rt_metal_create_device)
NOP1(rt_metal_destroy_device)
NOP2(rt_metal_create_buffer)
NOP1(rt_metal_destroy_buffer)
NOP2(rt_metal_create_pipeline)
NOP1(rt_metal_destroy_pipeline)
NOP2(rt_metal_create_command_queue)
NOP1(rt_metal_destroy_command_queue)
NOP1(rt_metal_create_command_buffer)
NOP2(rt_metal_create_library)
NOP1(rt_metal_destroy_library)
NOP2(rt_metal_create_function)
NOP1(rt_metal_destroy_function)
NOP2(rt_metal_create_compute_encoder)
NOP1(rt_metal_destroy_compute_encoder)
NOP2(rt_metal_create_render_encoder)
NOP1(rt_metal_destroy_render_encoder)
NOP3(rt_metal_dispatch_threads)
NOP1(rt_metal_commit)
NOP1(rt_metal_wait_completed)
NOP3(rt_metal_buffer_contents)
NOP3(rt_metal_set_buffer)
NOP3(rt_metal_set_bytes)
NOP2(rt_metal_create_texture)
NOP1(rt_metal_destroy_texture)
NOP3(rt_metal_create_sampler)
NOP1(rt_metal_destroy_sampler)
NOP3(rt_metal_set_texture)
NOP0(rt_metal_available)
NOP1(rt_metal_device_name)

NOP1(rt_opengl_init)

/* GPU general */
NOP0(rt_gpu_init)
NOP0(rt_gpu_available)
NOP1(rt_gpu_device_count)
NOP1(rt_gpu_device_name)
NOP2(rt_gpu_malloc)
NOP1(rt_gpu_free)
NOP3(rt_gpu_memcpy_h2d)
NOP3(rt_gpu_memcpy_d2h)
NOP3(rt_gpu_memcpy_d2d)
NOP3(rt_gpu_launch_kernel)
NOP0(rt_gpu_sync)
NOP1(rt_gpu_set_device)
NOP0(rt_gpu_get_device)
NOP8(rt_gpu_atomic_add)
NOP8(rt_gpu_atomic_sub)
NOP8(rt_gpu_atomic_max)
NOP8(rt_gpu_atomic_min)
NOP8(rt_gpu_atomic_and)
NOP8(rt_gpu_atomic_or)
NOP8(rt_gpu_atomic_xor)
NOP8(rt_gpu_atomic_cas)
NOP8(rt_gpu_atomic_exchange)
NOP8(rt_gpu_atomic_load)
NOP8(rt_gpu_atomic_store)
NOP8(rt_gpu_atomic_inc)
NOP8(rt_gpu_atomic_dec)
NOP8(rt_gpu_atomic_fetch_add)
NOP8(rt_gpu_atomic_fetch_sub)
NOP8(rt_gpu_atomic_fetch_and)
NOP8(rt_gpu_atomic_fetch_or)
NOP8(rt_gpu_atomic_fetch_xor)
NOP8(rt_gpu_atomic_fetch_max)
NOP8(rt_gpu_atomic_fetch_min)
NOP8(rt_gpu_atomic_compare_exchange)
NOP8(rt_gpu_atomic_exchange_strong)
NOP8(rt_gpu_atomic_exchange_weak)
NOP8(rt_gpu_atomic_fence)
NOP8(rt_gpu_atomic_thread_fence)
NOP8(rt_gpu_atomic_signal_fence)

/* oneAPI/ROCm */
NOP0(rt_oneapi_init)
NOP0(rt_oneapi_available)
NOP1(rt_oneapi_device_count)
NOP1(rt_oneapi_device_name)
NOP2(rt_oneapi_malloc)
NOP1(rt_oneapi_free)
NOP3(rt_oneapi_memcpy_h2d)
NOP3(rt_oneapi_memcpy_d2h)
NOP3(rt_oneapi_submit)
NOP1(rt_oneapi_wait)
NOP2(rt_oneapi_create_queue)
NOP1(rt_oneapi_destroy_queue)
NOP2(rt_oneapi_create_buffer)
NOP1(rt_oneapi_destroy_buffer)
NOP3(rt_oneapi_create_kernel)
NOP1(rt_oneapi_destroy_kernel)
NOP4(rt_oneapi_set_arg)
NOP5(rt_oneapi_launch)
NOP1(rt_oneapi_get_device)
NOP2(rt_oneapi_set_device)
NOP1(rt_oneapi_device_mem_info)
NOP2(rt_oneapi_device_prop)
NOP0(rt_rocm_init)
NOP0(rt_rocm_available)
NOP1(rt_rocm_device_count)
NOP1(rt_rocm_device_name)
NOP2(rt_rocm_malloc)
NOP1(rt_rocm_free)
NOP3(rt_rocm_memcpy_h2d)
NOP3(rt_rocm_memcpy_d2h)
NOP3(rt_rocm_memcpy_d2d)
NOP3(rt_rocm_launch_kernel)
NOP0(rt_rocm_sync)
NOP1(rt_rocm_set_device)
NOP0(rt_rocm_get_device)
NOP1(rt_rocm_device_mem_info)
NOP2(rt_rocm_device_prop)
NOP1(rt_rocm_stream_create)
NOP1(rt_rocm_stream_destroy)
NOP1(rt_rocm_stream_sync)
NOP2(rt_rocm_module_load)
NOP1(rt_rocm_module_unload)
NOP2(rt_rocm_module_get_function)

/* SIMD/vec stubs */
NOP2(rt_simd_add_f32x4)
NOP2(rt_simd_add_f32x8)
NOP2(rt_simd_add_f64x2)
NOP2(rt_simd_add_f64x4)
NOP2(rt_simd_add_i32x4)
NOP2(rt_simd_sub_f32x4)
NOP2(rt_simd_sub_f32x8)
NOP2(rt_simd_sub_f64x2)
NOP2(rt_simd_sub_f64x4)
NOP2(rt_simd_sub_i32x4)
NOP2(rt_simd_mul_f32x4)
NOP2(rt_simd_mul_f32x8)
NOP2(rt_simd_mul_f64x2)
NOP2(rt_simd_mul_f64x4)
NOP2(rt_simd_mul_i32x4)
NOP4(rt_simd_fma_f32x4)
NOP4(rt_simd_fma_f64x2)
NOP2(rt_simd_dot_f32x4)
NOP2(rt_simd_dot_f64x2)
NOP2(rt_simd_min_f32x4)
NOP2(rt_simd_min_f64x2)
NOP2(rt_simd_max_f32x4)
NOP2(rt_simd_max_f64x2)
NOP1(rt_simd_sqrt_f32x4)
NOP1(rt_simd_sqrt_f64x2)
NOP1(rt_simd_abs_f32x4)
NOP1(rt_simd_abs_f64x2)
NOP2(rt_simd_cmp_f32x4)

NOP2(rt_vec_abs)
NOP2(rt_vec_all)
NOP2(rt_vec_any)
NOP3(rt_vec_blend)
NOP2(rt_vec_ceil)
NOP3(rt_vec_clamp)
NOP2(rt_vec_extract)
NOP2(rt_vec_floor)
NOP4(rt_vec_fma)
NOP3(rt_vec_gather)
NOP2(rt_vec_load)
NOP3(rt_vec_masked_load)
NOP3(rt_vec_masked_store)
NOP2(rt_vec_max)
NOP2(rt_vec_max_vec)
NOP2(rt_vec_min)
NOP2(rt_vec_min_vec)
NOP1(rt_vec_product)
NOP1(rt_vec_recip)
NOP2(rt_vec_round)
NOP3(rt_vec_scatter)
NOP3(rt_vec_select)
NOP3(rt_vec_shuffle)
NOP1(rt_vec_sqrt)
NOP2(rt_vec_store)
NOP1(rt_vec_sum)
NOP3(rt_vec_with)

/* --- Host ML (PyTorch/Torch) --- all 76 stubs */
NOP2(rt_torch_add)
NOP2(rt_torch_sub)
NOP2(rt_torch_mul)
NOP2(rt_torch_div)
NOP2(rt_torch_matmul)
NOP1(rt_torch_relu)
NOP1(rt_torch_sigmoid)
NOP1(rt_torch_tanh)
NOP1(rt_torch_softmax)
NOP1(rt_torch_exp)
NOP1(rt_torch_log)
NOP1(rt_torch_sqrt)
NOP1(rt_torch_abs)
NOP2(rt_torch_pow)
NOP1(rt_torch_sum)
NOP1(rt_torch_mean)
NOP1(rt_torch_max)
NOP1(rt_torch_min)
NOP1(rt_torch_argmax)
NOP1(rt_torch_argmin)
NOP2(rt_torch_cat)
NOP2(rt_torch_stack)
NOP2(rt_torch_reshape)
NOP2(rt_torch_view)
NOP1(rt_torch_flatten)
NOP1(rt_torch_squeeze)
NOP2(rt_torch_unsqueeze)
NOP1(rt_torch_transpose)
NOP2(rt_torch_permute)
NOP1(rt_torch_contiguous)
NOP1(rt_torch_clone)
NOP1(rt_torch_detach)
NOP2(rt_torch_to_device)
NOP1(rt_torch_shape)
NOP1(rt_torch_dtype)
NOP1(rt_torch_device)
NOP1(rt_torch_numel)
NOP1(rt_torch_dim)
NOP2(rt_torch_zeros)
NOP2(rt_torch_ones)
NOP2(rt_torch_rand)
NOP2(rt_torch_randn)
NOP2(rt_torch_full)
NOP2(rt_torch_arange)
NOP3(rt_torch_linspace)
NOP1(rt_torch_eye)
NOP2(rt_torch_from_list)
NOP1(rt_torch_to_list)
NOP2(rt_torch_item)
NOP2(rt_torch_index)
NOP3(rt_torch_index_put)
NOP3(rt_torch_slice)
NOP1(rt_torch_backward)
NOP1(rt_torch_grad)
NOP1(rt_torch_no_grad_begin)
NOP0(rt_torch_no_grad_end)
NOP1(rt_torch_requires_grad)
NOP2(rt_torch_autograd_backward)
NOP3(rt_torch_autograd_grad)
NOP1(rt_torch_autograd_no_grad)
NOP1(rt_torch_autograd_enable_grad)
NOP1(rt_torch_autograd_is_grad_enabled)
NOP2(rt_torch_autograd_functional_jacobian)
NOP2(rt_torch_autograd_functional_hessian)
NOP1(rt_torch_autograd_grad_fn)
NOP3(rt_torch_nn_linear)
NOP3(rt_torch_nn_conv2d)
NOP2(rt_torch_nn_batchnorm)
NOP1(rt_torch_nn_dropout)
NOP2(rt_torch_nn_embedding)
NOP2(rt_torch_nn_layernorm)
NOP3(rt_torch_nn_rnn)
NOP2(rt_torch_tensor_data)
NOP2(rt_torch_tensor_stride)
NOP2(rt_torch_tensor_storage)
NOP1(rt_torch_tensor_is_contiguous)
NOP2(rt_torch_tensor_to)
NOP3(rt_torch_torchtensor_add)
NOP3(rt_torch_torchtensor_sub)
NOP3(rt_torch_torchtensor_mul)
NOP3(rt_torch_torchtensor_div)
NOP3(rt_torch_torchtensor_matmul)
NOP2(rt_torch_torchtensor_relu)
NOP2(rt_torch_torchtensor_sigmoid)
NOP2(rt_torch_torchtensor_tanh)
NOP2(rt_torch_torchtensor_softmax)
NOP2(rt_torch_torchtensor_sum)
NOP2(rt_torch_torchtensor_mean)
NOP2(rt_torch_torchtensor_max)
NOP2(rt_torch_torchtensor_min)
NOP2(rt_torch_torchtensor_argmax)
NOP2(rt_torch_torchtensor_argmin)
NOP3(rt_torch_torchtensor_reshape)
NOP3(rt_torch_torchtensor_view)
NOP2(rt_torch_torchtensor_flatten)
NOP2(rt_torch_torchtensor_squeeze)
NOP3(rt_torch_torchtensor_unsqueeze)
NOP2(rt_torch_torchtensor_transpose)
NOP3(rt_torch_torchtensor_permute)
NOP2(rt_torch_torchtensor_contiguous)
NOP2(rt_torch_torchtensor_clone)
NOP2(rt_torch_torchtensor_detach)
NOP3(rt_torch_torchtensor_to_device)
NOP2(rt_torch_torchtensor_shape)
NOP2(rt_torch_torchtensor_dtype)
NOP2(rt_torch_torchtensor_device)
NOP2(rt_torch_torchtensor_numel)
NOP2(rt_torch_torchtensor_dim)
NOP2(rt_torch_torchtensor_backward)
NOP2(rt_torch_torchtensor_grad)
NOP2(rt_torch_torchtensor_requires_grad)
NOP3(rt_torch_torchtensor_to_list)
NOP3(rt_torch_torchtensor_item)
NOP3(rt_torch_torchtensor_index)
NOP4(rt_torch_torchtensor_index_put)
NOP4(rt_torch_torchtensor_slice)
NOP3(rt_torch_torchtensor_pow)
NOP2(rt_torch_torchtensor_exp)
NOP2(rt_torch_torchtensor_log)
NOP2(rt_torch_torchtensor_sqrt)
NOP2(rt_torch_torchtensor_abs)
NOP3(rt_torch_torchtensor_cat)
NOP3(rt_torch_torchtensor_stack)
NOP3(rt_torch_torchtensor_data)
NOP3(rt_torch_torchtensor_stride)
NOP3(rt_torch_torchtensor_storage)
NOP2(rt_torch_torchtensor_is_contiguous)
NOP3(rt_torch_torchtensor_to)

/* --- Host physics (rapier2d) --- all 50 stubs */
NOP0(rt_rapier2d_world_new)
NOP1(rt_rapier2d_world_step)
NOP2(rt_rapier2d_world_set_gravity)
NOP1(rt_rapier2d_world_gravity)
NOP1(rt_rapier2d_world_body_count)
NOP1(rt_rapier2d_world_collider_count)
NOP1(rt_rapier2d_world_joint_count)
NOP1(rt_rapier2d_world_free)
NOP2(rt_rapier2d_world_remove_body)
NOP2(rt_rapier2d_world_remove_collider)
NOP4(rt_rapier2d_body_new_dynamic)
NOP4(rt_rapier2d_body_new_static)
NOP4(rt_rapier2d_body_new_kinematic)
NOP1(rt_rapier2d_body_position)
NOP1(rt_rapier2d_body_rotation)
NOP2(rt_rapier2d_body_set_position)
NOP2(rt_rapier2d_body_set_rotation)
NOP1(rt_rapier2d_body_velocity)
NOP2(rt_rapier2d_body_set_velocity)
NOP2(rt_rapier2d_body_apply_force)
NOP2(rt_rapier2d_body_apply_impulse)
NOP1(rt_rapier2d_body_mass)
NOP2(rt_rapier2d_body_set_mass)
NOP2(rt_rapier2d_body_set_damping)
NOP1(rt_rapier2d_body_is_sleeping)
NOP1(rt_rapier2d_body_wake_up)
NOP2(rt_rapier2d_body_set_gravity_scale)
NOP1(rt_rapier2d_body_collider)
NOP1(rt_rapier2d_body_type)
NOP4(rt_rapier2d_collider_ball)
NOP5(rt_rapier2d_collider_cuboid)
NOP5(rt_rapier2d_collider_capsule)
NOP4(rt_rapier2d_collider_segment)
NOP2(rt_rapier2d_collider_set_restitution)
NOP2(rt_rapier2d_collider_set_friction)
NOP2(rt_rapier2d_collider_set_density)
NOP1(rt_rapier2d_collider_shape)
NOP2(rt_rapier2d_collider_set_sensor)
NOP1(rt_rapier2d_collider_is_sensor)
NOP5(rt_rapier2d_joint_revolute)
NOP5(rt_rapier2d_joint_prismatic)
NOP5(rt_rapier2d_joint_fixed)
NOP5(rt_rapier2d_joint_ball)
NOP2(rt_rapier2d_joint_set_limits)
NOP2(rt_rapier2d_joint_set_motor)
NOP1(rt_rapier2d_joint_free)

/* --- Host compiler tools (AST, Cranelift, etc.) --- all 105 stubs */
NOP2(rt_ast_arg_free)
NOP2(rt_ast_arg_name)
NOP2(rt_ast_arg_value)
NOP2(rt_ast_expr_array_get)
NOP1(rt_ast_expr_array_len)
NOP1(rt_ast_expr_binary_left)
NOP1(rt_ast_expr_binary_op)
NOP1(rt_ast_expr_binary_right)
NOP1(rt_ast_expr_bool_value)
NOP2(rt_ast_expr_call_arg)
NOP1(rt_ast_expr_call_arg_count)
NOP1(rt_ast_expr_call_callee)
NOP1(rt_ast_expr_field_name)
NOP1(rt_ast_expr_field_receiver)
NOP1(rt_ast_expr_float_value)
NOP1(rt_ast_expr_free)
NOP1(rt_ast_expr_ident_name)
NOP1(rt_ast_expr_int_value)
NOP2(rt_ast_expr_method_arg)
NOP1(rt_ast_expr_method_arg_count)
NOP1(rt_ast_expr_method_name)
NOP1(rt_ast_expr_method_receiver)
NOP1(rt_ast_expr_string_value)
NOP1(rt_ast_expr_tag)
NOP1(rt_ast_expr_unary_op)
NOP1(rt_ast_expr_unary_operand)
NOP1(rt_ast_node_free)
NOP0(rt_ast_registry_clear)
NOP0(rt_ast_registry_count)

/* Cranelift — all 67 stubs */
NOP0(rt_cranelift_init)
NOP1(rt_cranelift_compile)
NOP2(rt_cranelift_module_new)
NOP1(rt_cranelift_module_free)
NOP3(rt_cranelift_func_new)
NOP1(rt_cranelift_func_free)
NOP2(rt_cranelift_emit)
NOP1(rt_cranelift_get_code)
NOP2(rt_cranelift_link)
NOP3(rt_cranelift_declare_function)
NOP3(rt_cranelift_define_function)
NOP2(rt_cranelift_finalize)
NOP2(rt_cranelift_get_symbol)
NOP1(rt_cranelift_reset)
NOP2(rt_cranelift_set_opt_level)
NOP3(rt_cranelift_add_import)
NOP2(rt_cranelift_create_context)
NOP1(rt_cranelift_destroy_context)
NOP2(rt_cranelift_verify)
NOP3(rt_cranelift_add_data)
NOP2(rt_cranelift_get_data)
NOP1(rt_cranelift_dump_ir)
NOP2(rt_cranelift_set_target)
NOP1(rt_cranelift_get_target)
NOP2(rt_cranelift_create_signature)
NOP1(rt_cranelift_destroy_signature)
NOP3(rt_cranelift_add_param)
NOP3(rt_cranelift_add_return)
NOP3(rt_cranelift_create_block)
NOP3(rt_cranelift_switch_to_block)
NOP4(rt_cranelift_ins_iconst)
NOP4(rt_cranelift_ins_fconst)
NOP4(rt_cranelift_ins_bconst)
NOP4(rt_cranelift_ins_iadd)
NOP4(rt_cranelift_ins_isub)
NOP4(rt_cranelift_ins_imul)
NOP4(rt_cranelift_ins_sdiv)
NOP4(rt_cranelift_ins_udiv)
NOP4(rt_cranelift_ins_srem)
NOP4(rt_cranelift_ins_urem)
NOP4(rt_cranelift_ins_band)
NOP4(rt_cranelift_ins_bor)
NOP4(rt_cranelift_ins_bxor)
NOP4(rt_cranelift_ins_ishl)
NOP4(rt_cranelift_ins_sshr)
NOP4(rt_cranelift_ins_ushr)
NOP4(rt_cranelift_ins_icmp)
NOP4(rt_cranelift_ins_fcmp)
NOP4(rt_cranelift_ins_fadd)
NOP4(rt_cranelift_ins_fsub)
NOP4(rt_cranelift_ins_fmul)
NOP4(rt_cranelift_ins_fdiv)
NOP4(rt_cranelift_ins_load)
NOP5(rt_cranelift_ins_store)
NOP4(rt_cranelift_ins_call)
NOP3(rt_cranelift_ins_call_indirect)
NOP3(rt_cranelift_ins_return)
NOP3(rt_cranelift_ins_jump)
NOP4(rt_cranelift_ins_brz)
NOP4(rt_cranelift_ins_brnz)
NOP4(rt_cranelift_ins_select)
NOP3(rt_cranelift_ins_trap)
NOP3(rt_cranelift_ins_nop)
NOP4(rt_cranelift_ins_stack_slot)
NOP4(rt_cranelift_ins_global_value)
NOP4(rt_cranelift_ins_heap_addr)
NOP3(rt_cranelift_block_param)
NOP3(rt_cranelift_seal_block)

/* --- Host tooling (cargo, i18n, package, SDN, etc.) --- */
NOP1(rt_cargo_build)
NOP1(rt_cargo_test)
NOP1(rt_cargo_run)
NOP2(rt_cargo_add)
NOP1(rt_cargo_clean)
NOP2(rt_cargo_install)
NOP1(rt_cargo_check)
NOP1(rt_i18n_get)
NOP2(rt_i18n_set)
NOP2(rt_i18n_format)
NOP1(rt_i18n_locale)
NOP1(rt_i18n_set_locale)
NOP2(rt_package_install)
NOP1(rt_package_uninstall)
NOP0(rt_package_list)
NOP1(rt_package_info)
NOP2(rt_package_search)
NOP2(rt_package_update)
NOP2(rt_package_add)
NOP1(rt_package_remove)
NOP2(rt_package_version)
NOP2(rt_package_resolve)
NOP2(rt_sdn_parse)
NOP2(rt_sdn_stringify)
NOP2(rt_sdn_get)
NOP3(rt_sdn_set)
NOP1(rt_sdn_keys)
NOP1(rt_sdn_values)
NOP2(rt_sdn_has)
NOP1(rt_sdn_type)

/* --- Host compress (gzip, tar, zip, deflate) --- */
NOP1(rt_gzip_compress)
NOP1(rt_gzip_decompress)
NOP2(rt_gzip_compress_level)
NOP1(rt_gzip_is_gzipped)
NOP2(rt_tar_create)
NOP2(rt_tar_extract)
NOP1(rt_tar_list)
NOP2(rt_tar_add_file)
NOP3(rt_tar_add_data)
NOP1(rt_tar_open)
NOP1(rt_tar_close)
NOP1(rt_tar_next_entry)
NOP2(rt_targz_create)
NOP1(rt_targz_extract)
NOP2(rt_zip_create)
NOP2(rt_zip_extract)
NOP1(rt_zip_list)
NOP3(rt_zip_add_file)
NOP3(rt_zip_add_data)
NOP1(rt_zip_open)
NOP1(rt_zip_close)
NOP1(rt_zip_entry_count)
NOP2(rt_deflate_compress)
NOP1(rt_deflate_decompress)

/* --- Host FFI --- */
NOP1(rt_ffi_load_library)
NOP1(rt_ffi_close_library)
NOP2(rt_ffi_get_symbol)
NOP3(rt_ffi_call)
NOP4(rt_ffi_call_with_args)
NOP2(rt_ffi_alloc)
NOP1(rt_ffi_free)
NOP3(rt_ffi_read)
NOP3(rt_ffi_write)
NOP2(rt_ffi_object_new)
NOP2(rt_ffi_object_get)
NOP3(rt_ffi_object_set)
NOP2(rt_ffi_object_call)
NOP1(rt_ffi_object_free)

/* --- Host crypto --- */
NOP2(rt_hmac_sha256)
NOP3(rt_encrypt_aes)
NOP3(rt_decrypt_aes)
NOP2(rt_password_hash)
NOP3(rt_password_verify)
NOP2(rt_password_hash_argon2)
NOP2(rt_password_hash_bcrypt)

/* --- AOP --- */
NOP4(rt_aop_invoke_around)
NOP1(rt_aop_proceed)

/* --- Settlement / store / compiler --- */
NOP1(rt_settlement_load)
NOP2(rt_store_get)
NOP3(rt_store_set)
NOP1(rt_interp_eval)
NOP2(rt_interp_exec)
NOP1(rt_compile_file)
NOP2(rt_load_module)
NOP1(rt_generate_code)
NOP2(rt_generate_ir)
NOP1(rt_derive_eq)
NOP1(rt_decision_tree_build)
NOP1(rt_neighbor_find)
NOP2(rt_capture_stdout)
NOP1(rt_capture_end)

/* --- Driver (GPU dispatching) --- */
NOP2(rt_driver_submit_compute)
NOP2(rt_driver_submit_render)
NOP2(rt_driver_submit_transfer)
NOP3(rt_driver_submit_copy)
NOP2(rt_driver_submit_dispatch)
NOP3(rt_driver_submit_draw)
NOP2(rt_driver_submit_blit)
NOP2(rt_driver_submit_clear)
NOP2(rt_driver_submit_barrier)
NOP1(rt_driver_submit_present)
NOP1(rt_driver_submit_fence)
NOP2(rt_driver_submit_wait)
NOP3(rt_driver_submit_signal)
NOP4(rt_driver_submit_pipeline)
NOP3(rt_driver_submit_bind)
NOP4(rt_driver_submit_set)
NOP2(rt_driver_submit_push)
NOP4(rt_driver_submit_viewport)
NOP4(rt_driver_submit_scissor)
NOP2(rt_driver_submit_blend)
NOP2(rt_driver_submit_depth)
NOP2(rt_driver_submit_stencil)


/* ===================================================================
 * 4. WRONG-ARCH STUBS — ARM32, ARM64, RISC-V instructions
 *
 * These are architecture-specific intrinsics for non-x86 targets.
 * Return 0/NIL safely since they should never be called on x86.
 * =================================================================== */

/* ARM32 — 28 functions */
NOP0(rt_arm32_cpsid_i)
NOP0(rt_arm32_cpsid_if)
NOP0(rt_arm32_cpsie_i)
NOP0(rt_arm32_cpsie_if)
NOP0(rt_arm32_dmb)
NOP0(rt_arm32_dsb)
NOP0(rt_arm32_isb)
NOP1(rt_arm32_mcr_cntp_ctl)
NOP1(rt_arm32_mcr_cntp_tval)
NOP1(rt_arm32_mcr_dacr)
NOP1(rt_arm32_mcr_sctlr)
NOP0(rt_arm32_mcr_tlbiall)
NOP1(rt_arm32_mcr_tlbimva)
NOP1(rt_arm32_mcr_ttbcr)
NOP1(rt_arm32_mcr_ttbr0)
NOP1(rt_arm32_mcr_ttbr1)
NOP1(rt_arm32_mcr_vbar)
NOP0(rt_arm32_mrc_cntfrq)
NOP0(rt_arm32_mrc_cntp_ctl)
NOP0(rt_arm32_mrc_midr)
NOP0(rt_arm32_mrc_mpidr)
NOP0(rt_arm32_mrc_sctlr)
NOP0(rt_arm32_mrc_ttbr0)
NOP0(rt_arm32_mrc_ttbr1)
NOP0(rt_arm32_mrrc_cntpct_hi)
NOP0(rt_arm32_mrrc_cntpct_lo)
NOP0(rt_arm32_wfe)
NOP0(rt_arm32_wfi)

/* ARM64 — 25 functions */
NOP1(rt_arm64_daif_clr)
NOP1(rt_arm64_daif_set)
NOP0(rt_arm64_dmb)
NOP0(rt_arm64_dsb)
NOP0(rt_arm64_isb)
NOP0(rt_arm64_mrs_cntfrq_el0)
NOP0(rt_arm64_mrs_cntp_ctl_el0)
NOP0(rt_arm64_mrs_cntpct_el0)
NOP0(rt_arm64_mrs_currentel)
NOP0(rt_arm64_mrs_mpidr_el1)
NOP0(rt_arm64_mrs_sctlr_el1)
NOP0(rt_arm64_mrs_ttbr0_el1)
NOP0(rt_arm64_mrs_ttbr1_el1)
NOP1(rt_arm64_msr_cntp_ctl_el0)
NOP1(rt_arm64_msr_cntp_tval_el0)
NOP1(rt_arm64_msr_mair_el1)
NOP1(rt_arm64_msr_sctlr_el1)
NOP1(rt_arm64_msr_tcr_el1)
NOP1(rt_arm64_msr_ttbr0_el1)
NOP1(rt_arm64_msr_ttbr1_el1)
NOP1(rt_arm64_msr_vbar_el1)
NOP0(rt_arm64_tlbi_alle1)
NOP1(rt_arm64_tlbi_vae1)
NOP0(rt_arm64_wfe)
NOP0(rt_arm64_wfi)

/* RISC-V 32 — 17 functions */
NOP0(rt_rv32_csrr_mcause)
NOP0(rt_rv32_csrr_mepc)
NOP0(rt_rv32_csrr_mhartid)
NOP0(rt_rv32_csrr_mie)
NOP0(rt_rv32_csrr_mstatus)
NOP0(rt_rv32_csrr_mtvec)
NOP0(rt_rv32_csrr_satp)
NOP0(rt_rv32_csrr_time)
NOP1(rt_rv32_csrw_mcause)
NOP1(rt_rv32_csrw_mepc)
NOP1(rt_rv32_csrw_mie)
NOP1(rt_rv32_csrw_mstatus)
NOP1(rt_rv32_csrw_mtvec)
NOP1(rt_rv32_fence)
NOP0(rt_rv32_mret)
NOP0(rt_rv32_wfi)
NOP0(rt_rv32_sfence_vma)

/* ===================================================================
 * 5. AUTO-GENERATED REMAINING STUBS
 *
 * These 816 functions are host-only variants not yet covered above.
 * All return NIL_VALUE safely.  NOP2 is used as the stub arity --
 * on x86_64 System V ABI, extra register arguments are harmlessly
 * ignored, so this is safe for any actual arity.
 * =================================================================== */

NOP2(rt_async_tcp_connect_timeout)
NOP2(rt_async_tcp_flush)
NOP2(rt_async_tcp_read_all)
NOP2(rt_async_tcp_read_exact)
NOP2(rt_async_tcp_read_line)
NOP2(rt_async_tcp_read_text)
NOP2(rt_async_tcp_write_all)
NOP2(rt_async_tcp_write_text)
NOP2(rt_async_udp_recv_from)
NOP2(rt_async_udp_send_to)
NOP2(rt_audio_get_master_volume)
NOP2(rt_audio_load_sound)
NOP2(rt_audio_play_looped)
NOP2(rt_audio_set_listener_direction)
NOP2(rt_audio_set_listener_position)
NOP2(rt_audio_set_listener_world_up)
NOP2(rt_audio_set_sound_max_distance)
NOP2(rt_audio_set_sound_min_distance)
NOP2(rt_audio_set_sound_position)
NOP2(rt_audio_set_spatialization_enabled)
NOP2(rt_audio_unload_sound)
NOP2(rt_capture_stderr_start)
NOP2(rt_capture_stdout_start)
NOP2(rt_cargo_fmt)
NOP2(rt_cargo_lint)
NOP2(rt_cargo_test_doc)
NOP2(rt_cli_handle_add)
NOP2(rt_cli_handle_cache)
NOP2(rt_cli_handle_compile)
NOP2(rt_cli_handle_diagram)
NOP2(rt_cli_handle_env)
NOP2(rt_cli_handle_init)
NOP2(rt_cli_handle_install)
NOP2(rt_cli_handle_linkers)
NOP2(rt_cli_handle_list)
NOP2(rt_cli_handle_lock)
NOP2(rt_cli_handle_remove)
NOP2(rt_cli_handle_run)
NOP2(rt_cli_handle_targets)
NOP2(rt_cli_handle_tree)
NOP2(rt_cli_handle_update)
NOP2(rt_cli_handle_web)
NOP2(rt_cli_read_file)
NOP2(rt_cli_run_brief)
NOP2(rt_cli_run_check)
NOP2(rt_cli_run_code)
NOP2(rt_cli_run_constr)
NOP2(rt_cli_run_context)
NOP2(rt_cli_run_diff)
NOP2(rt_cli_run_feature_gen)
NOP2(rt_cli_run_ffi_gen)
NOP2(rt_cli_run_fix)
NOP2(rt_cli_run_fmt)
NOP2(rt_cli_run_gen_lean)
NOP2(rt_cli_run_i18n)
NOP2(rt_cli_run_info)
NOP2(rt_cli_run_lex)
NOP2(rt_cli_run_lint)
NOP2(rt_cli_run_mcp)
NOP2(rt_cli_run_migrate)
NOP2(rt_cli_run_query)
NOP2(rt_cli_run_repl)
NOP2(rt_cli_run_replay)
NOP2(rt_cli_run_spec_coverage)
NOP2(rt_cli_run_spec_gen)
NOP2(rt_cli_run_spipe_docgen)
NOP2(rt_cli_run_task_gen)
NOP2(rt_cli_run_tests)
NOP2(rt_cli_run_todo_gen)
NOP2(rt_cli_run_todo_scan)
NOP2(rt_cli_run_verify)
NOP2(rt_command_output)
NOP2(rt_compile_to_native)
NOP2(rt_condition_probe)
NOP2(rt_context_generate)
NOP2(rt_context_stats)
NOP2(rt_contract_violation_free)
NOP2(rt_contract_violation_func_name)
NOP2(rt_contract_violation_kind)
NOP2(rt_contract_violation_message)
NOP2(rt_contract_violation_new)
NOP2(rt_coverage_condition_probe)
NOP2(rt_coverage_decision_probe)
NOP2(rt_coverage_dump_sdn)
NOP2(rt_coverage_enable)
NOP2(rt_coverage_enable_timed)
NOP2(rt_coverage_enabled)
NOP2(rt_coverage_free_sdn)
NOP2(rt_coverage_path_finalize)
NOP2(rt_coverage_path_finalizer)
NOP2(rt_coverage_path_probe)
NOP2(rt_cranelift_aot_define_function)
NOP2(rt_cranelift_append_block_param)
NOP2(rt_cranelift_append_func_params)
NOP2(rt_cranelift_band)
NOP2(rt_cranelift_bconst)
NOP2(rt_cranelift_begin_function)
NOP2(rt_cranelift_bitcast)
NOP2(rt_cranelift_bnot)
NOP2(rt_cranelift_bor)
NOP2(rt_cranelift_brif)
NOP2(rt_cranelift_bxor)
NOP2(rt_cranelift_call)
NOP2(rt_cranelift_call_function_ptr)
NOP2(rt_cranelift_call_indirect)
NOP2(rt_cranelift_emit_object)
NOP2(rt_cranelift_end_function)
NOP2(rt_cranelift_fadd)
NOP2(rt_cranelift_fcmp)
NOP2(rt_cranelift_fconst)
NOP2(rt_cranelift_fcvt_from_sint)
NOP2(rt_cranelift_fcvt_from_uint)
NOP2(rt_cranelift_fcvt_to_sint)
NOP2(rt_cranelift_fcvt_to_uint)
NOP2(rt_cranelift_fdiv)
NOP2(rt_cranelift_finalize_module)
NOP2(rt_cranelift_fmul)
NOP2(rt_cranelift_free_module)
NOP2(rt_cranelift_fsub)
NOP2(rt_cranelift_get_function_ptr)
NOP2(rt_cranelift_iadd)
NOP2(rt_cranelift_icmp)
NOP2(rt_cranelift_iconst)
NOP2(rt_cranelift_import_function)
NOP2(rt_cranelift_imul)
NOP2(rt_cranelift_ireduce)
NOP2(rt_cranelift_ishl)
NOP2(rt_cranelift_isub)
NOP2(rt_cranelift_jump)
NOP2(rt_cranelift_load)
NOP2(rt_cranelift_new_aot_module)
NOP2(rt_cranelift_new_module)
NOP2(rt_cranelift_new_signature)
NOP2(rt_cranelift_null)
NOP2(rt_cranelift_return)
NOP2(rt_cranelift_return_void)
NOP2(rt_cranelift_sdiv)
NOP2(rt_cranelift_seal_all_blocks)
NOP2(rt_cranelift_sextend)
NOP2(rt_cranelift_sig_add_param)
NOP2(rt_cranelift_sig_set_return)
NOP2(rt_cranelift_srem)
NOP2(rt_cranelift_sshr)
NOP2(rt_cranelift_stack_addr)
NOP2(rt_cranelift_stack_slot)
NOP2(rt_cranelift_store)
NOP2(rt_cranelift_trap)
NOP2(rt_cranelift_udiv)
NOP2(rt_cranelift_uextend)
NOP2(rt_cranelift_urem)
NOP2(rt_cranelift_ushr)
NOP2(rt_cuda_compile_ptx)
NOP2(rt_cuda_compute_capability)
NOP2(rt_cuda_device_memory)
NOP2(rt_cuda_free)
NOP2(rt_cuda_get_function)
NOP2(rt_cuda_get_last_error)
NOP2(rt_cuda_is_available)
NOP2(rt_cuda_malloc)
NOP2(rt_cuda_memcpy_d2d)
NOP2(rt_cuda_memcpy_d2h)
NOP2(rt_cuda_memcpy_h2d)
NOP2(rt_cuda_peek_last_error)
NOP2(rt_cuda_stream_synchronize)
NOP2(rt_cuda_synchronize)
NOP2(rt_cuda_unload_module)
NOP2(rt_decision_probe)
NOP2(rt_decrypt_aes256)
NOP2(rt_derive_key_pbkdf2)
NOP2(rt_dir_glob)
NOP2(rt_dir_walk)
NOP2(rt_driver_backend_name)
NOP2(rt_driver_cancel)
NOP2(rt_driver_create)
NOP2(rt_driver_destroy)
NOP2(rt_driver_flush)
NOP2(rt_driver_poll)
NOP2(rt_driver_poll_flags)
NOP2(rt_driver_poll_id)
NOP2(rt_driver_poll_result)
NOP2(rt_driver_submit_accept)
NOP2(rt_driver_submit_close)
NOP2(rt_driver_submit_connect)
NOP2(rt_driver_submit_fsync)
NOP2(rt_driver_submit_open)
NOP2(rt_driver_submit_read)
NOP2(rt_driver_submit_recv)
NOP2(rt_driver_submit_send)
NOP2(rt_driver_submit_sendfile)
NOP2(rt_driver_submit_timeout)
NOP2(rt_driver_submit_write)
NOP2(rt_driver_supports_sendfile)
NOP2(rt_driver_supports_zero_copy)
NOP2(rt_dwarf_free)
NOP2(rt_dwarf_function_at)
NOP2(rt_dwarf_load)
NOP2(rt_dwarf_locals_at)
NOP2(rt_encrypt_aes256)
NOP2(rt_env_cwd)
NOP2(rt_env_exists)
NOP2(rt_env_get_home)
NOP2(rt_env_home)
NOP2(rt_env_remove)
NOP2(rt_env_temp)
NOP2(rt_env_vars)
NOP2(rt_epoll_ctl)
NOP2(rt_execute_native)
NOP2(rt_executor_get_mode)
NOP2(rt_executor_is_manual)
NOP2(rt_executor_pending_count)
NOP2(rt_executor_poll_all)
NOP2(rt_executor_set_mode)
NOP2(rt_executor_set_workers)
NOP2(rt_executor_start)
NOP2(rt_ffi_call_method)
NOP2(rt_ffi_clone)
NOP2(rt_ffi_drop)
NOP2(rt_ffi_has_method)
NOP2(rt_ffi_new)
NOP2(rt_ffi_object_call_method)
NOP2(rt_ffi_object_has_method)
NOP2(rt_ffi_object_type_id)
NOP2(rt_ffi_object_type_name)
NOP2(rt_ffi_type_id)
NOP2(rt_ffi_type_name)
NOP2(rt_ffi_type_register)
NOP2(rt_file_atomic_write)
NOP2(rt_file_hash_sha256)
NOP2(rt_file_mmap_read_bytes)
NOP2(rt_file_mmap_read_text)
NOP2(rt_file_modified)
NOP2(rt_file_modified_time)
NOP2(rt_file_read_text_at)
NOP2(rt_file_write_text_at)
NOP2(rt_font_bitmap_free)
NOP2(rt_font_bitmap_get_pixel)
NOP2(rt_font_bitmap_height)
NOP2(rt_font_bitmap_width)
NOP2(rt_font_glyph_advance)
NOP2(rt_font_glyph_bitmap)
NOP2(rt_font_line_height)
NOP2(rt_fork_child_exit)
NOP2(rt_fork_child_setup)
NOP2(rt_fork_parent_stderr)
NOP2(rt_fork_parent_stdout)
NOP2(rt_fork_parent_wait)
NOP2(rt_ftp_cdup)
NOP2(rt_ftp_connect_secure)
NOP2(rt_ftp_cwd)
NOP2(rt_ftp_disconnect)
NOP2(rt_ftp_get_welcome_msg)
NOP2(rt_ftp_is_connected)
NOP2(rt_ftp_login)
NOP2(rt_ftp_mdtm)
NOP2(rt_ftp_noop)
NOP2(rt_ftp_quit)
NOP2(rt_ftp_set_mode_active)
NOP2(rt_ftp_set_mode_passive)
NOP2(rt_ftp_set_transfer_type_ascii)
NOP2(rt_ftp_set_transfer_type_binary)
NOP2(rt_gamepad_axis_data)
NOP2(rt_gamepad_button_data)
NOP2(rt_gamepad_button_is_pressed)
NOP2(rt_gamepad_event_free)
NOP2(rt_gamepad_event_get_axis)
NOP2(rt_gamepad_event_get_button)
NOP2(rt_gamepad_event_get_gamepad_id)
NOP2(rt_gamepad_event_get_type)
NOP2(rt_gamepad_event_get_value)
NOP2(rt_gamepad_get_last_error)
NOP2(rt_gamepad_get_name)
NOP2(rt_gamepad_get_power_info)
NOP2(rt_gamepad_poll_event)
NOP2(rt_gamepad_set_rumble)
NOP2(rt_gamepad_stop_rumble)
NOP2(rt_gamepad_update)
NOP2(rt_generate_key)
NOP2(rt_generate_key_hex)
NOP2(rt_gpu_atomic_add_i64)
NOP2(rt_gpu_atomic_add_u32)
NOP2(rt_gpu_atomic_and_i64)
NOP2(rt_gpu_atomic_and_u32)
NOP2(rt_gpu_atomic_cmpxchg)
NOP2(rt_gpu_atomic_cmpxchg_i64)
NOP2(rt_gpu_atomic_cmpxchg_u32)
NOP2(rt_gpu_atomic_max_i64)
NOP2(rt_gpu_atomic_max_u32)
NOP2(rt_gpu_atomic_min_i64)
NOP2(rt_gpu_atomic_min_u32)
NOP2(rt_gpu_atomic_or_i64)
NOP2(rt_gpu_atomic_or_u32)
NOP2(rt_gpu_atomic_sub_i64)
NOP2(rt_gpu_atomic_sub_u32)
NOP2(rt_gpu_atomic_xchg_i64)
NOP2(rt_gpu_atomic_xchg_u32)
NOP2(rt_gpu_atomic_xor_i64)
NOP2(rt_gpu_atomic_xor_u32)
NOP2(rt_gpu_barrier)
NOP2(rt_gpu_global_id)
NOP2(rt_gpu_global_size)
NOP2(rt_gpu_group_id)
NOP2(rt_gpu_launch)
NOP2(rt_gpu_launch_1d)
NOP2(rt_gpu_local_id)
NOP2(rt_gpu_local_size)
NOP2(rt_gpu_mem_fence)
NOP2(rt_gpu_num_groups)
NOP2(rt_gpu_shared_alloc)
NOP2(rt_gpu_shared_reset)
NOP2(rt_gzip_compress_file)
NOP2(rt_gzip_decompress_file)
NOP2(rt_handle_free)
NOP2(rt_handle_get)
NOP2(rt_handle_is_valid)
NOP2(rt_handle_new)
NOP2(rt_handle_set)
NOP2(rt_hmac_sha512)
NOP2(rt_hook_add_breakpoint)
NOP2(rt_hook_continue)
NOP2(rt_hook_disable_debugging)
NOP2(rt_hook_evaluate_condition)
NOP2(rt_hook_evaluate_expression)
NOP2(rt_hook_get_call_depth)
NOP2(rt_hook_get_stack_frames)
NOP2(rt_hook_get_variables)
NOP2(rt_hook_pause)
NOP2(rt_hook_remove_breakpoint)
NOP2(rt_hook_set_breakpoint_enabled)
NOP2(rt_hook_step)
NOP2(rt_hook_terminate)
NOP2(rt_http_client_create)
NOP2(rt_http_client_destroy)
NOP2(rt_http_client_request)
NOP2(rt_http_client_set_header)
NOP2(rt_http_download)
NOP2(rt_http_parse_json)
NOP2(rt_http_request_body)
NOP2(rt_http_request_header)
NOP2(rt_http_request_method)
NOP2(rt_http_request_path)
NOP2(rt_http_request_query)
NOP2(rt_http_response_create)
NOP2(rt_http_response_json)
NOP2(rt_http_response_set_header)
NOP2(rt_http_server_create)
NOP2(rt_http_server_destroy)
NOP2(rt_http_server_static)
NOP2(rt_http_stringify_json)
NOP2(rt_http_upload)
NOP2(rt_http_url_decode)
NOP2(rt_http_url_encode)
NOP2(rt_i18n_context_free)
NOP2(rt_i18n_context_insert)
NOP2(rt_i18n_context_new)
NOP2(rt_i18n_get_message)
NOP2(rt_i18n_severity_name)
NOP2(rt_image_channels)
NOP2(rt_image_free)
NOP2(rt_image_get_pixel)
NOP2(rt_interp_call)
NOP2(rt_io_file_delete)
NOP2(rt_io_file_exists)
NOP2(rt_io_file_set_permissions)
NOP2(rt_io_file_write_all)
NOP2(rt_io_tcp_accept_timeout)
NOP2(rt_io_tcp_bind)
NOP2(rt_io_tcp_flush)
NOP2(rt_io_tcp_local_addr)
NOP2(rt_io_tcp_peer_addr)
NOP2(rt_io_tcp_read_line)
NOP2(rt_io_tcp_set_read_timeout)
NOP2(rt_io_tcp_set_write_timeout)
NOP2(rt_io_tcp_write_text)
NOP2(rt_io_udp_connect)
NOP2(rt_io_udp_local_addr)
NOP2(rt_io_udp_set_read_timeout)
NOP2(rt_load_barrier)
NOP2(rt_lyon_fill_tessellate)
NOP2(rt_lyon_fill_tessellate_with_rule)
NOP2(rt_lyon_fill_tessellation_free)
NOP2(rt_lyon_fill_tessellation_get_indices)
NOP2(rt_lyon_fill_tessellation_get_vertices)
NOP2(rt_lyon_fill_tessellation_index_count)
NOP2(rt_lyon_fill_tessellation_vertex_count)
NOP2(rt_lyon_get_last_error)
NOP2(rt_lyon_index_buffer_free)
NOP2(rt_lyon_index_buffer_get)
NOP2(rt_lyon_index_buffer_size)
NOP2(rt_lyon_index_buffer_to_array)
NOP2(rt_lyon_path_builder_arc_to)
NOP2(rt_lyon_path_builder_begin)
NOP2(rt_lyon_path_builder_build)
NOP2(rt_lyon_path_builder_close)
NOP2(rt_lyon_path_builder_cubic_bezier_to)
NOP2(rt_lyon_path_builder_free)
NOP2(rt_lyon_path_builder_line_to)
NOP2(rt_lyon_path_builder_new)
NOP2(rt_lyon_path_builder_quadratic_bezier_to)
NOP2(rt_lyon_path_circle)
NOP2(rt_lyon_path_contains_point)
NOP2(rt_lyon_path_ellipse)
NOP2(rt_lyon_path_get_bounds)
NOP2(rt_lyon_path_polygon)
NOP2(rt_lyon_path_rectangle)
NOP2(rt_lyon_path_rounded_rectangle)
NOP2(rt_lyon_path_star)
NOP2(rt_lyon_path_transform)
NOP2(rt_lyon_stroke_tessellate)
NOP2(rt_lyon_stroke_tessellate_with_options)
NOP2(rt_lyon_stroke_tessellation_free)
NOP2(rt_lyon_stroke_tessellation_get_indices)
NOP2(rt_lyon_stroke_tessellation_get_vertices)
NOP2(rt_lyon_stroke_tessellation_index_count)
NOP2(rt_lyon_stroke_tessellation_vertex_count)
NOP2(rt_lyon_transform_free)
NOP2(rt_lyon_transform_multiply)
NOP2(rt_lyon_vertex_buffer_free)
NOP2(rt_lyon_vertex_buffer_get_normal)
NOP2(rt_lyon_vertex_buffer_get_position)
NOP2(rt_lyon_vertex_buffer_size)
NOP2(rt_lyon_vertex_buffer_to_array)
NOP2(rt_metal_alloc_buffer)
NOP2(rt_metal_begin_render_pass)
NOP2(rt_metal_buffer_download)
NOP2(rt_metal_buffer_upload)
NOP2(rt_metal_commit_command_buffer)
NOP2(rt_metal_compile_shader)
NOP2(rt_metal_create_compute_pipeline)
NOP2(rt_metal_create_render_pipeline)
NOP2(rt_metal_create_swapchain)
NOP2(rt_metal_destroy_render_pipeline)
NOP2(rt_metal_destroy_shader)
NOP2(rt_metal_device_count)
NOP2(rt_metal_device_memory)
NOP2(rt_metal_draw_indexed)
NOP2(rt_metal_end_render_pass)
NOP2(rt_metal_free_buffer)
NOP2(rt_metal_free_texture)
NOP2(rt_metal_get_last_error)
NOP2(rt_metal_init)
NOP2(rt_metal_is_available)
NOP2(rt_metal_present)
NOP2(rt_mkdir_p)
NOP2(rt_neighbor_load)
NOP2(rt_oneapi_compile_spirv)
NOP2(rt_oneapi_device_memory)
NOP2(rt_oneapi_device_type)
NOP2(rt_oneapi_get_function)
NOP2(rt_oneapi_get_last_error)
NOP2(rt_oneapi_is_available)
NOP2(rt_oneapi_malloc_device)
NOP2(rt_oneapi_malloc_shared)
NOP2(rt_oneapi_memset)
NOP2(rt_oneapi_queue_wait)
NOP2(rt_oneapi_submit_kernel)
NOP2(rt_oneapi_synchronize)
NOP2(rt_oneapi_unload_module)
NOP2(rt_opengl_available)
NOP2(rt_package_chmod)
NOP2(rt_package_copy_file)
NOP2(rt_package_create_symlink)
NOP2(rt_package_create_tarball)
NOP2(rt_package_exists)
NOP2(rt_package_extract_tarball)
NOP2(rt_package_file_size)
NOP2(rt_package_free_string)
NOP2(rt_package_is_dir)
NOP2(rt_package_mkdir_all)
NOP2(rt_package_remove_dir_all)
NOP2(rt_package_sha256)
NOP2(rt_par_filter)
NOP2(rt_password_verify_bcrypt)
NOP2(rt_path_absolute)
NOP2(rt_path_ext)
NOP2(rt_path_filename)
NOP2(rt_path_parent)
NOP2(rt_path_probe)
NOP2(rt_path_relative)
NOP2(rt_path_separator)
NOP2(rt_perf_clear)
NOP2(rt_perf_clock_ns)
NOP2(rt_perf_cycles_to_ns)
NOP2(rt_perf_dump_sdn)
NOP2(rt_perf_enable)
NOP2(rt_perf_enabled)
NOP2(rt_perf_free_sdn)
NOP2(rt_perf_rdtsc)
NOP2(rt_perf_region_enter)
NOP2(rt_perf_region_exit)
NOP2(rt_process_output)
NOP2(rt_process_run_capture)
NOP2(rt_ps_torch_tensor_from_data)
NOP2(rt_ps_torch_tensor_zeros)
NOP2(rt_ptrace_continue)
NOP2(rt_ptrace_get_registers)
NOP2(rt_ptrace_read_memory)
NOP2(rt_ptrace_single_step)
NOP2(rt_ptrace_wait_stop)
NOP2(rt_ptrace_write_memory)
NOP2(rt_rapier2d_body_apply_torque)
NOP2(rt_rapier2d_body_apply_torque_impulse)
NOP2(rt_rapier2d_body_free)
NOP2(rt_rapier2d_body_get_mass)
NOP2(rt_rapier2d_body_get_position)
NOP2(rt_rapier2d_body_get_velocity)
NOP2(rt_rapier2d_body_set_angular_damping)
NOP2(rt_rapier2d_body_set_linear_damping)
NOP2(rt_rapier2d_body_sleep)
NOP2(rt_rapier2d_collider_free)
NOP2(rt_rapier2d_collider_new_box)
NOP2(rt_rapier2d_collider_new_capsule)
NOP2(rt_rapier2d_collider_new_circle)
NOP2(rt_rapier2d_collider_new_polygon)
NOP2(rt_rapier2d_collider_set_offset)
NOP2(rt_rapier2d_contacts_count)
NOP2(rt_rapier2d_contacts_free)
NOP2(rt_rapier2d_contacts_get)
NOP2(rt_rapier2d_get_last_error)
NOP2(rt_rapier2d_joint_distance)
NOP2(rt_rapier2d_world_cast_ray)
NOP2(rt_rapier2d_world_get_contacts)
NOP2(rt_rapier2d_world_intersection_test)
NOP2(rt_read_cr0)
NOP2(rt_read_msr)
NOP2(rt_read_stdin_line)
NOP2(rt_regex_captures_len)
NOP2(rt_regex_destroy)
NOP2(rt_regex_find_quick)
NOP2(rt_regex_is_match_quick)
NOP2(rt_regex_replace_all_quick)
NOP2(rt_regex_replace_quick)
NOP2(rt_regex_split_quick)
NOP2(rt_rocm_compile_hsaco)
NOP2(rt_rocm_create_stream)
NOP2(rt_rocm_destroy_stream)
NOP2(rt_rocm_device_memory)
NOP2(rt_rocm_get_function)
NOP2(rt_rocm_get_last_error)
NOP2(rt_rocm_is_available)
NOP2(rt_rocm_memset)
NOP2(rt_rocm_stream_synchronize)
NOP2(rt_rocm_synchronize)
NOP2(rt_rocm_unload_module)
NOP2(rt_rv32_csrr_scause)
NOP2(rt_rv32_csrr_sepc)
NOP2(rt_rv32_csrr_sie)
NOP2(rt_rv32_csrr_sip)
NOP2(rt_rv32_csrr_sstatus)
NOP2(rt_rv32_csrr_stval)
NOP2(rt_rv32_csrr_stvec)
NOP2(rt_rv32_csrw_satp)
NOP2(rt_rv32_csrw_sepc)
NOP2(rt_rv32_csrw_sie)
NOP2(rt_rv32_csrw_sstatus)
NOP2(rt_rv32_csrw_stvec)
NOP2(rt_rv32_sbi_call)
NOP2(rt_rv32_sfence_vma_all)
NOP2(rt_sdl2_clear_quit)
NOP2(rt_sdl2_event_key_mod)
NOP2(rt_sdl2_event_key_sym)
NOP2(rt_sdl2_event_wheel_x)
NOP2(rt_sdl2_event_wheel_y)
NOP2(rt_sdl2_event_window_data1)
NOP2(rt_sdl2_event_window_data2)
NOP2(rt_sdl2_event_window_event_id)
NOP2(rt_sdl2_get_ticks_ms)
NOP2(rt_sdl2_get_ticks_ns)
NOP2(rt_sdl2_get_window_height)
NOP2(rt_sdl2_get_window_position_x)
NOP2(rt_sdl2_get_window_position_y)
NOP2(rt_sdl2_get_window_width)
NOP2(rt_sdl2_hide_window)
NOP2(rt_sdl2_is_key_pressed)
NOP2(rt_sdl2_is_mouse_button_pressed)
NOP2(rt_sdl2_present_rgba)
NOP2(rt_sdl2_set_cursor_grab)
NOP2(rt_sdl2_set_cursor_visible)
NOP2(rt_sdl2_set_window_fullscreen)
NOP2(rt_sdl2_set_window_position)
NOP2(rt_sdl2_set_window_resizable)
NOP2(rt_sdl2_show_window)
NOP2(rt_sdl2_warp_mouse)
NOP2(rt_sdl2_window_should_close)
NOP2(rt_sdn_check)
NOP2(rt_sdn_fmt)
NOP2(rt_sdn_from_json)
NOP2(rt_sdn_to_json)
NOP2(rt_sdn_version)
NOP2(rt_settlement_main)
NOP2(rt_sftp_download)
NOP2(rt_sftp_init)
NOP2(rt_sftp_readdir)
NOP2(rt_sftp_shutdown)
NOP2(rt_sftp_unlink)
NOP2(rt_sftp_upload)
NOP2(rt_shell_exec)
NOP2(rt_shell_output)
NOP2(rt_simd_add_i32x8)
NOP2(rt_simd_div_f32x4)
NOP2(rt_simd_div_f32x8)
NOP2(rt_simd_div_f64x4)
NOP2(rt_simd_fma_f32x8)
NOP2(rt_simd_fma_f64x4)
NOP2(rt_simd_hadd_f32x4)
NOP2(rt_simd_has_avx)
NOP2(rt_simd_has_avx2)
NOP2(rt_simd_has_neon)
NOP2(rt_simd_has_sse)
NOP2(rt_simd_hmax_f32x4)
NOP2(rt_simd_hmin_f32x4)
NOP2(rt_simd_mul_i32x8)
NOP2(rt_simd_sub_i32x8)
NOP2(rt_socket_set_nonblocking)
NOP2(rt_socket_set_reuseaddr)
NOP2(rt_socket_set_reuseport)
NOP2(rt_sqlite_begin)
NOP2(rt_sqlite_error_message)
NOP2(rt_sqlite_execute)
NOP2(rt_sqlite_execute_batch)
NOP2(rt_sqlite_open_memory)
NOP2(rt_sqlite_query_done)
NOP2(rt_sqlite_query_next)
NOP2(rt_sqlite_reset)
NOP2(rt_ssh_add_known_host)
NOP2(rt_ssh_auth_agent)
NOP2(rt_ssh_auth_password)
NOP2(rt_ssh_auth_pubkey)
NOP2(rt_ssh_channel_close)
NOP2(rt_ssh_channel_read)
NOP2(rt_ssh_channel_write)
NOP2(rt_ssh_check_host_key)
NOP2(rt_ssh_disconnect)
NOP2(rt_ssh_forward_close)
NOP2(rt_ssh_forward_local)
NOP2(rt_ssh_forward_remote)
NOP2(rt_ssh_get_banner)
NOP2(rt_ssh_get_host_key)
NOP2(rt_ssh_get_methods)
NOP2(rt_ssh_is_authenticated)
NOP2(rt_ssh_set_timeout)
NOP2(rt_stdin_read_all)
NOP2(rt_store_barrier)
NOP2(rt_tar_extract_file)
NOP2(rt_tcp_connect_timeout)
NOP2(rt_term_enable_ansi)
NOP2(rt_term_get_size)
NOP2(rt_terminal_disable_raw_mode)
NOP2(rt_terminal_enable_raw_mode)
NOP2(rt_terminal_get_size)
NOP2(rt_test_db_cleanup_stale_runs)
NOP2(rt_test_db_enable_validation)
NOP2(rt_test_db_validate)
NOP2(rt_test_it)
NOP2(rt_test_run_is_stale)
NOP2(rt_test_skip)
NOP2(rt_thread_available_parallelism)
NOP2(rt_thread_free)
NOP2(rt_thread_is_done)
NOP2(rt_thread_spawn_isolated)
NOP2(rt_thread_spawn_isolated_with_args)
NOP2(rt_tls_client_config_add_root_cert)
NOP2(rt_tls_client_config_enable_sni)
NOP2(rt_tls_client_config_free)
NOP2(rt_tls_client_config_new)
NOP2(rt_tls_client_config_set_alpn)
NOP2(rt_tls_client_config_set_verify_mode)
NOP2(rt_tls_client_connect_with_sni)
NOP2(rt_tls_free_cert)
NOP2(rt_tls_generate_self_signed_cert)
NOP2(rt_tls_get_cert_expiry)
NOP2(rt_tls_get_cert_issuer)
NOP2(rt_tls_get_cert_subject)
NOP2(rt_tls_get_cipher_suite)
NOP2(rt_tls_get_negotiated_alpn)
NOP2(rt_tls_get_peer_cert)
NOP2(rt_tls_get_protocol_version)
NOP2(rt_tls_hash_cert)
NOP2(rt_tls_is_handshake_complete)
NOP2(rt_tls_load_cert)
NOP2(rt_tls_load_key)
NOP2(rt_tls_server_close_connection)
NOP2(rt_tls_server_config_free)
NOP2(rt_tls_server_config_new)
NOP2(rt_tls_server_config_require_client_cert)
NOP2(rt_tls_server_config_set_alpn)
NOP2(rt_tls_server_create)
NOP2(rt_tls_server_read)
NOP2(rt_tls_server_shutdown)
NOP2(rt_tls_server_write)
NOP2(rt_tls_verify_cert)
NOP2(rt_torch_autograd_detach)
NOP2(rt_torch_autograd_no_grad_begin)
NOP2(rt_torch_autograd_no_grad_end)
NOP2(rt_torch_autograd_requires_grad)
NOP2(rt_torch_autograd_set_requires_grad)
NOP2(rt_torch_autograd_zero_grad)
NOP2(rt_torch_available)
NOP2(rt_torch_copy_data_to_cpu)
NOP2(rt_torch_cuda_available)
NOP2(rt_torch_free)
NOP2(rt_torch_nn_batch_norm)
NOP2(rt_torch_nn_cross_entropy)
NOP2(rt_torch_nn_max_pool2d)
NOP2(rt_torch_nn_mse_loss)
NOP2(rt_torch_stream_create)
NOP2(rt_torch_tensor_empty)
NOP2(rt_torch_tensor_eye)
NOP2(rt_torch_tensor_full)
NOP2(rt_torch_tensor_ones)
NOP2(rt_torch_tensor_rand)
NOP2(rt_torch_tensor_randn)
NOP2(rt_torch_to_cpu)
NOP2(rt_torch_to_cuda)
NOP2(rt_torch_torchstream_free)
NOP2(rt_torch_torchstream_query)
NOP2(rt_torch_torchstream_sync)
NOP2(rt_torch_torchtensor_add_scalar)
NOP2(rt_torch_torchtensor_cpu)
NOP2(rt_torch_torchtensor_cuda)
NOP2(rt_torch_torchtensor_free)
NOP2(rt_torch_torchtensor_gather)
NOP2(rt_torch_torchtensor_gelu)
NOP2(rt_torch_torchtensor_is_cuda)
NOP2(rt_torch_torchtensor_leaky_relu)
NOP2(rt_torch_torchtensor_log_softmax)
NOP2(rt_torch_torchtensor_mean_dim)
NOP2(rt_torch_torchtensor_mul_scalar)
NOP2(rt_torch_torchtensor_ndim)
NOP2(rt_torch_torchtensor_neg)
NOP2(rt_torch_torchtensor_squeeze_dim)
NOP2(rt_torch_torchtensor_sum_dim)
NOP2(rt_torch_torchtensor_to_stream)
NOP2(rt_torch_version)
NOP2(rt_udp_send)
NOP2(rt_vk_available)
NOP2(rt_vk_buffer_alloc)
NOP2(rt_vk_buffer_download)
NOP2(rt_vk_buffer_free)
NOP2(rt_vk_buffer_upload)
NOP2(rt_vk_cmd_set_scissor)
NOP2(rt_vk_cmd_set_viewport)
NOP2(rt_vk_command_buffer_begin)
NOP2(rt_vk_command_buffer_end)
NOP2(rt_vk_command_buffer_free)
NOP2(rt_vk_command_buffer_submit)
NOP2(rt_vk_device_free)
NOP2(rt_vk_device_sync)
NOP2(rt_vk_framebuffer_create)
NOP2(rt_vk_framebuffer_create_for_swapchain)
NOP2(rt_vk_framebuffer_free)
NOP2(rt_vk_framebuffer_get_dimensions)
NOP2(rt_vk_graphics_pipeline_create)
NOP2(rt_vk_graphics_pipeline_free)
NOP2(rt_vk_image_create_2d)
NOP2(rt_vk_image_download)
NOP2(rt_vk_image_free)
NOP2(rt_vk_image_get_view)
NOP2(rt_vk_image_upload)
NOP2(rt_vk_kernel_compile)
NOP2(rt_vk_kernel_free)
NOP2(rt_vk_kernel_launch)
NOP2(rt_vk_kernel_launch_1d)
NOP2(rt_vk_render_pass_create_simple)
NOP2(rt_vk_render_pass_create_with_depth)
NOP2(rt_vk_render_pass_free)
NOP2(rt_vk_render_pass_get_color_format)
NOP2(rt_vk_sampler_create)
NOP2(rt_vk_sampler_free)
NOP2(rt_vk_shader_module_create)
NOP2(rt_vk_shader_module_free)
NOP2(rt_vulkan_alloc_buffer)
NOP2(rt_vulkan_api_version)
NOP2(rt_vulkan_available)
NOP2(rt_vulkan_begin_compute)
NOP2(rt_vulkan_begin_render_pass_gfx)
NOP2(rt_vulkan_bind_buffer)
NOP2(rt_vulkan_bind_descriptors)
NOP2(rt_vulkan_bind_graphics_pipeline)
NOP2(rt_vulkan_bind_index_buffer)
NOP2(rt_vulkan_bind_pipeline)
NOP2(rt_vulkan_bind_texture)
NOP2(rt_vulkan_bind_vertex_buffer)
NOP2(rt_vulkan_compile_glsl)
NOP2(rt_vulkan_compile_spirv)
NOP2(rt_vulkan_copy_buffer)
NOP2(rt_vulkan_copy_from_buffer)
NOP2(rt_vulkan_copy_to_buffer)
NOP2(rt_vulkan_create_compute_pipeline)
NOP2(rt_vulkan_create_graphics_pipeline)
NOP2(rt_vulkan_destroy_graphics_pipeline)
NOP2(rt_vulkan_device_count)
NOP2(rt_vulkan_device_memory)
NOP2(rt_vulkan_device_name)
NOP2(rt_vulkan_device_type)
NOP2(rt_vulkan_dispatch)
NOP2(rt_vulkan_draw)
NOP2(rt_vulkan_draw_indexed)
NOP2(rt_vulkan_end_compute)
NOP2(rt_vulkan_end_render_pass_gfx)
NOP2(rt_vulkan_free_buffer)
NOP2(rt_vulkan_get_device)
NOP2(rt_vulkan_get_last_error)
NOP2(rt_vulkan_init)
NOP2(rt_vulkan_is_available)
NOP2(rt_vulkan_push_constants)
NOP2(rt_vulkan_select_device)
NOP2(rt_vulkan_set_scissor)
NOP2(rt_vulkan_set_viewport)
NOP2(rt_vulkan_shutdown)
NOP2(rt_vulkan_submit_and_wait)
NOP2(rt_vulkan_wait_fence)
NOP2(rt_vulkan_wait_idle)
NOP2(rt_write_cr0)
NOP2(rt_write_msr)
NOP2(rt_ws_receive)
NOP2(rt_zip_extract_file)

/* --- Desktop E2E freestanding closure spillovers ---
 *
 * The x86_64 desktop smoke links a large UI/browser closure. Some optional
 * hosted backends survive section GC as symbol references even though the
 * baremetal path uses BGA/Engine2D CPU drawing. Keep those host-only hooks as
 * unavailable no-ops here instead of requiring CUDA/OpenGL/WebGPU/TLS stacks
 * in the freestanding kernel image.
 */
RuntimeValue FontRenderer_dot_browser_serif_default(void) { return NIL_VALUE; }
RuntimeValue KLogEntry_dot_from_bytes(void) { return NIL_VALUE; }
RuntimeValue QualcommBackend_dot_is_adreno_gpu(void) { return FALSE_VALUE; }
RuntimeValue tools__pkg__pkg_repository__TlsClient(void) { return NIL_VALUE; }
RuntimeValue generate_css(void) { return rt_string_from_cstr(""); }
RuntimeValue noalloc_log_debug(void) { return NIL_VALUE; }
RuntimeValue panic(void) { return NIL_VALUE; }

RuntimeValue arch__x86_64__desktop_e2e_entry__SYS_IPC_CREATE_PORT(void) { return ENCODE_INT(22); }

RuntimeValue cuda_available(void) { return FALSE_VALUE; }
RuntimeValue cuda_device_count(void) { return ENCODE_INT(0); }
RuntimeValue cuda_init(void) { return ENCODE_INT(-1); }
RuntimeValue cuda_launch_kernel(void) { return ENCODE_INT(-1); }
RuntimeValue cuda_mem_alloc(void) { return ENCODE_INT(0); }
RuntimeValue cuda_mem_free(void) { return ENCODE_INT(0); }
RuntimeValue cuda_memset(void) { return ENCODE_INT(0); }
RuntimeValue cuda_module_load_data(void) { return ENCODE_INT(0); }
RuntimeValue cuda_module_unload(void) { return ENCODE_INT(0); }
RuntimeValue cuda_sync(void) { return ENCODE_INT(0); }

RuntimeValue emu_draw_blur_rect(void) { return NIL_VALUE; }
RuntimeValue emu_draw_image_scaled(void) { return NIL_VALUE; }
RuntimeValue emu_draw_image_transform(void) { return NIL_VALUE; }
RuntimeValue emu_draw_rect_blend_mode(void) { return NIL_VALUE; }
RuntimeValue emu_draw_shadow_rect(void) { return NIL_VALUE; }
RuntimeValue emu_draw_triangle_outline(void) { return NIL_VALUE; }

RuntimeValue rt_aes_decrypt_block_with_expanded(void) { return NIL_VALUE; }
RuntimeValue rt_aes_encrypt_block_with_expanded(void) { return NIL_VALUE; }
RuntimeValue rt_arm64_set_exec_image(void) { return NIL_VALUE; }
RuntimeValue rt_arm_array_clone_bytes(void) { return NIL_VALUE; }
RuntimeValue rt_arm_array_empty_exact(void) { return rt_array_new(ENCODE_INT(0)); }
RuntimeValue rt_arm_array_slice_bytes(void) { return rt_array_new(ENCODE_INT(0)); }
RuntimeValue rt_arm_fat32_probe_bpb_from_virtio(void) { return NIL_VALUE; }
RuntimeValue rt_arm_smf_elf_stub_size(void) { return ENCODE_INT(0); }
RuntimeValue rt_byte_array_new(RuntimeValue capacity) { return rt_array_new(capacity); }
RuntimeValue rt_bytes_alloc(RuntimeValue size) { return rt_array_new(size); }
RuntimeValue rt_bytes_u32_le_at(void) { return ENCODE_INT(0); }
RuntimeValue rt_bytes_u64_le_at(void) { return ENCODE_INT(0); }
RuntimeValue rt_bytes_u8_set(void) { return TRUE_VALUE; }
RuntimeValue rt_cpu_present_pixels(void) { return NIL_VALUE; }
RuntimeValue rt_driver_poll_data(void) { return NIL_VALUE; }
RuntimeValue rt_driver_poll_data_len(void) { return ENCODE_INT(0); }
RuntimeValue rt_engine2d_download_pixels(void) { return NIL_VALUE; }
RuntimeValue rt_engine2d_pack_args_4(void) { return NIL_VALUE; }
RuntimeValue rt_engine2d_pack_args_8(void) { return NIL_VALUE; }
RuntimeValue rt_engine2d_rocm_download_pixels(void) { return NIL_VALUE; }
RuntimeValue rt_engine2d_rocm_upload_host_buf(void) { return NIL_VALUE; }
RuntimeValue rt_engine2d_rocm_upload_pixels(void) { return NIL_VALUE; }
RuntimeValue rt_engine2d_upload_host_buf(void) { return NIL_VALUE; }
RuntimeValue rt_engine2d_upload_pixels(void) { return NIL_VALUE; }
RuntimeValue rt_env_get_i64(void) { return ENCODE_INT(0); }
RuntimeValue rt_fb_blit32(void) { return NIL_VALUE; }
RuntimeValue rt_fb_fill32(void) { return NIL_VALUE; }
RuntimeValue rt_file_fsync(void) { return FALSE_VALUE; }
RuntimeValue rt_file_fsync_cached(void) { return FALSE_VALUE; }
RuntimeValue rt_file_mmap_len(void) { return ENCODE_INT(0); }
RuntimeValue rt_file_mmap_read_bytes_rv(void) { return NIL_VALUE; }
RuntimeValue rt_file_mmap_read_text_rv(void) { return rt_string_from_cstr(""); }
RuntimeValue rt_file_write_text_at_cached(void) { return FALSE_VALUE; }
RuntimeValue rt_file_write_text_at_cached_repeat(void) { return FALSE_VALUE; }
RuntimeValue rt_intel_engine2d_download_pixels(void) { return NIL_VALUE; }
RuntimeValue rt_intel_engine2d_set_args_blit(void) { return NIL_VALUE; }
RuntimeValue rt_intel_engine2d_set_args_circle(void) { return NIL_VALUE; }
RuntimeValue rt_intel_engine2d_set_args_clear(void) { return NIL_VALUE; }
RuntimeValue rt_intel_engine2d_set_args_gradient(void) { return NIL_VALUE; }
RuntimeValue rt_intel_engine2d_set_args_line(void) { return NIL_VALUE; }
RuntimeValue rt_intel_engine2d_set_args_rect(void) { return NIL_VALUE; }
RuntimeValue rt_intel_engine2d_set_args_rounded_rect(void) { return NIL_VALUE; }
RuntimeValue rt_intel_engine2d_set_args_triangle(void) { return NIL_VALUE; }
RuntimeValue rt_intel_engine2d_upload_host_buf(void) { return NIL_VALUE; }
RuntimeValue rt_intel_engine2d_upload_pixels(void) { return NIL_VALUE; }
RuntimeValue rt_io_tcp_drain_line(void) { return rt_string_from_cstr(""); }
RuntimeValue rt_io_tcp_read_exact(void) { return rt_string_from_cstr(""); }
RuntimeValue rt_io_tcp_read_exact_len(void) { return ENCODE_INT(0); }
RuntimeValue rt_io_tcp_write_text_read_exact_len(void) { return ENCODE_INT(0); }
RuntimeValue rt_metal_dispatch_compute(void) { return ENCODE_INT(-1); }
RuntimeValue rt_metal_end_compute_encoder(void) { return ENCODE_INT(-1); }
RuntimeValue rt_oneapi_compile_opencl(void) { return ENCODE_INT(0); }
RuntimeValue rt_opengl_bind_fbo(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_clear(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_clear_scissor(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_create_fbo(void) { return ENCODE_INT(0); }
RuntimeValue rt_opengl_destroy(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_destroy_fbo(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_draw_circle(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_draw_gradient_rect(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_draw_image(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_draw_line(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_draw_rect(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_draw_rounded_rect(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_draw_triangle(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_flush(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_get_last_error(void) { return ENCODE_INT(0); }
RuntimeValue rt_opengl_is_available(void) { return FALSE_VALUE; }
RuntimeValue rt_opengl_read_pixels(void) { return NIL_VALUE; }
RuntimeValue rt_opengl_set_scissor(void) { return NIL_VALUE; }
RuntimeValue rt_ptr_load(void) { return NIL_VALUE; }
RuntimeValue rt_ptr_store(void) { return NIL_VALUE; }
RuntimeValue rt_rank_query(void) { return ENCODE_INT(0); }
RuntimeValue rt_rank_select_build(void) { return NIL_VALUE; }
RuntimeValue rt_rank_select_free(void) { return NIL_VALUE; }
RuntimeValue rt_select_query(void) { return ENCODE_INT(0); }
RuntimeValue rt_simd_aes_round_last_u8x16(void) { return NIL_VALUE; }
RuntimeValue rt_simd_aes_round_u8x16(void) { return NIL_VALUE; }
RuntimeValue rt_simd_detect_profile(void) { return ENCODE_INT(0); }
RuntimeValue rt_simd_has_rvv(void) { return FALSE_VALUE; }
RuntimeValue rt_simd_profile_name(void) { return rt_string_from_cstr("scalar"); }
RuntimeValue rt_simd_str_equal(RuntimeValue a, RuntimeValue b) { return rt_native_eq(a, b); }
RuntimeValue rt_simd_str_last_index_of(void) { return ENCODE_INT(-1); }
RuntimeValue rt_simd_str_search(void) { return ENCODE_INT(-1); }
RuntimeValue rt_sqrt(RuntimeValue value) { return value; }
RuntimeValue rt_swi_build(void) { return NIL_VALUE; }
RuntimeValue rt_swi_byte_to_char(RuntimeValue value) { return value; }
RuntimeValue rt_swi_char_to_byte(RuntimeValue value) { return value; }
RuntimeValue rt_swi_free(void) { return NIL_VALUE; }
RuntimeValue rt_text_count_codepoints(RuntimeValue value) { return rt_string_len(value); }
RuntimeValue rt_text_to_lower_ascii(RuntimeValue value) { return value; }
RuntimeValue rt_text_to_upper_ascii(RuntimeValue value) { return value; }
RuntimeValue rt_time_now_unix(void) { return ENCODE_INT(0); }
RuntimeValue rt_typed_bytes_u64_le_unchecked(void) { return ENCODE_INT(0); }
RuntimeValue rt_typed_bytes_u8_push(RuntimeValue array, RuntimeValue value) {
    return rt_array_push(array, ENCODE_INT(((uint64_t)value) & 0xFF)) ? TRUE_VALUE : FALSE_VALUE;
}
RuntimeValue rt_typed_bytes_u8_unchecked(void) { return ENCODE_INT(0); }
RuntimeValue rt_typed_words_u32_at(void) { return ENCODE_INT(0); }
RuntimeValue rt_typed_words_u32_push(RuntimeValue array, RuntimeValue value) {
    return rt_array_push(array, ENCODE_INT(DECODE_INT(value) & 0xFFFFFFFFULL)) ? TRUE_VALUE : FALSE_VALUE;
}
RuntimeValue rt_typed_words_u32_set(void) { return TRUE_VALUE; }
RuntimeValue rt_utf8_count_codepoints(RuntimeValue value) { return rt_string_len(value); }
RuntimeValue rt_utf8_find_invalid(void) { return ENCODE_INT(-1); }
RuntimeValue rt_utf8_validate(void) { return TRUE_VALUE; }
RuntimeValue rt_webgpu_compute_draw(void) { return NIL_VALUE; }
RuntimeValue rt_webgpu_create_surface(void) { return ENCODE_INT(0); }
RuntimeValue rt_webgpu_destroy_surface(void) { return NIL_VALUE; }
RuntimeValue rt_webgpu_init(void) { return ENCODE_INT(-1); }
RuntimeValue rt_webgpu_is_available(void) { return FALSE_VALUE; }
RuntimeValue rt_webgpu_present(void) { return NIL_VALUE; }
RuntimeValue rt_webgpu_shutdown(void) { return NIL_VALUE; }
RuntimeValue rt_webgpu_upload_pixels(void) { return NIL_VALUE; }
RuntimeValue text_dot_from_char_code(RuntimeValue code) {
    char buf[2];
    buf[0] = (char)(DECODE_INT(code) & 0x7F);
    buf[1] = '\0';
    return rt_string_from_cstr(buf);
}

/* End of rt_extras.c */
