/*
 * SimpleOS x86_64 Baremetal Primitives
 *
 * Implements reference counting (Arc/Rc), byte conversion, float bit-casting,
 * string/array utilities, memory/pointer operations, synchronization stubs,
 * and miscellaneous helpers for the Simple language runtime on freestanding
 * x86_64 (no libc).
 *
 * All functions here provide strong definitions that override the weak
 * auto-generated stubs in auto_stubs.c.
 *
 * Depends on:
 *   - malloc()             (bump allocator from baremetal_stubs.c)
 *   - strlen()             (from baremetal_stubs.c)
 *   - rt_string_from_cstr  (from baremetal_stubs.c)
 *   - rt_array_new         (from baremetal_stubs.c)
 *   - rt_array_push        (from baremetal_stubs.c)
 */

#include <stdint.h>
#include <stddef.h>
#include "../../common/baremetal_runtime.h"

/* ===================================================================
 * External dependencies
 * =================================================================== */

extern void *malloc(unsigned long sz);
extern unsigned long strlen(const char *s);
extern RuntimeValue rt_string_from_cstr(const char *cstr);
extern RuntimeValue rt_array_new(RuntimeValue cap_val);
extern int8_t rt_array_push(RuntimeValue arr, RuntimeValue val);

/* ===================================================================
 * 1. Arc/Rc box — reference-counted heap objects
 *
 * On single-threaded baremetal, Arc and Rc share the same layout and
 * identical semantics. No atomics needed.
 * =================================================================== */

typedef struct {
    uint32_t strong;       /* strong reference count */
    uint32_t weak;         /* weak reference count */
    uint32_t value_size;   /* size of the stored value in bytes */
    uint32_t _pad;         /* alignment padding */
    /* value data follows immediately after this struct */
} ArcBox;

/* --- Arc functions --- */

RuntimeValue arc_box_init(RuntimeValue size)
{
    int64_t sz = DECODE_INT(size);
    if (sz < 0) sz = 0;
    if (sz > 0x100000) sz = 0x100000; /* safety limit: 1 MB */
    ArcBox *box = (ArcBox *)malloc(sizeof(ArcBox) + (unsigned long)sz);
    if (!box) return NIL_VALUE;
    box->strong = 1;
    box->weak = 0;
    box->value_size = (uint32_t)sz;
    box->_pad = 0;
    /* Zero-init value area */
    uint8_t *val = (uint8_t *)(box + 1);
    for (int64_t i = 0; i < sz; i++) val[i] = 0;
    return ENCODE_PTR(box);
}

RuntimeValue arc_box_get_value(RuntimeValue box)
{
    if (!IS_HEAP(box)) return NIL_VALUE;
    ArcBox *b = (ArcBox *)DECODE_PTR(box);
    if (!b) return NIL_VALUE;
    /* Value area starts immediately after the ArcBox header */
    void *val = (void *)(b + 1);
    return ENCODE_PTR(val);
}

RuntimeValue arc_box_inc_strong(RuntimeValue box)
{
    if (!IS_HEAP(box)) return NIL_VALUE;
    ArcBox *b = (ArcBox *)DECODE_PTR(box);
    if (!b) return NIL_VALUE;
    b->strong++;
    return box;
}

RuntimeValue arc_box_dec_strong(RuntimeValue box)
{
    if (!IS_HEAP(box)) return ENCODE_INT(0);
    ArcBox *b = (ArcBox *)DECODE_PTR(box);
    if (!b) return ENCODE_INT(0);
    if (b->strong > 0) b->strong--;
    return ENCODE_INT(b->strong);
}

RuntimeValue arc_box_inc_weak(RuntimeValue box)
{
    if (!IS_HEAP(box)) return NIL_VALUE;
    ArcBox *b = (ArcBox *)DECODE_PTR(box);
    if (!b) return NIL_VALUE;
    b->weak++;
    return NIL_VALUE;
}

RuntimeValue arc_box_dec_weak(RuntimeValue box)
{
    if (!IS_HEAP(box)) return NIL_VALUE;
    ArcBox *b = (ArcBox *)DECODE_PTR(box);
    if (!b) return NIL_VALUE;
    if (b->weak > 0) b->weak--;
    return NIL_VALUE;
}

RuntimeValue arc_box_drop_value(RuntimeValue box)
{
    /* No-op on bump allocator — memory is never freed */
    (void)box;
    return NIL_VALUE;
}

RuntimeValue arc_box_strong_count(RuntimeValue box)
{
    if (!IS_HEAP(box)) return ENCODE_INT(0);
    ArcBox *b = (ArcBox *)DECODE_PTR(box);
    if (!b) return ENCODE_INT(0);
    return ENCODE_INT(b->strong);
}

RuntimeValue arc_box_weak_count(RuntimeValue box)
{
    if (!IS_HEAP(box)) return ENCODE_INT(0);
    ArcBox *b = (ArcBox *)DECODE_PTR(box);
    if (!b) return ENCODE_INT(0);
    return ENCODE_INT(b->weak);
}

RuntimeValue arc_box_size(RuntimeValue box)
{
    if (!IS_HEAP(box)) return ENCODE_INT(0);
    ArcBox *b = (ArcBox *)DECODE_PTR(box);
    if (!b) return ENCODE_INT(0);
    return ENCODE_INT(b->value_size);
}

/* --- Rc functions (identical to Arc on single-threaded baremetal) --- */

RuntimeValue rc_box_init(RuntimeValue size)        { return arc_box_init(size); }
RuntimeValue rc_box_get_value(RuntimeValue box)    { return arc_box_get_value(box); }
RuntimeValue rc_box_inc_strong(RuntimeValue box)   { return arc_box_inc_strong(box); }
RuntimeValue rc_box_dec_strong(RuntimeValue box)   { return arc_box_dec_strong(box); }
RuntimeValue rc_box_inc_weak(RuntimeValue box)     { return arc_box_inc_weak(box); }
RuntimeValue rc_box_dec_weak(RuntimeValue box)     { return arc_box_dec_weak(box); }
RuntimeValue rc_box_drop_value(RuntimeValue box)   { return arc_box_drop_value(box); }
RuntimeValue rc_box_strong_count(RuntimeValue box) { return arc_box_strong_count(box); }
RuntimeValue rc_box_weak_count(RuntimeValue box)   { return arc_box_weak_count(box); }
RuntimeValue rc_box_size(RuntimeValue box)         { return arc_box_size(box); }

/* ===================================================================
 * 2. Byte/float conversion
 * =================================================================== */

/* --- Byte packing --- */

RuntimeValue bytes_to_u16_be(RuntimeValue b0, RuntimeValue b1)
{
    uint64_t v0 = (uint64_t)DECODE_INT(b0) & 0xFF;
    uint64_t v1 = (uint64_t)DECODE_INT(b1) & 0xFF;
    return ENCODE_INT((v0 << 8) | v1);
}

RuntimeValue bytes_to_u16_le(RuntimeValue b0, RuntimeValue b1)
{
    uint64_t v0 = (uint64_t)DECODE_INT(b0) & 0xFF;
    uint64_t v1 = (uint64_t)DECODE_INT(b1) & 0xFF;
    return ENCODE_INT((v1 << 8) | v0);
}

RuntimeValue bytes_to_u32_be(RuntimeValue b0, RuntimeValue b1,
                             RuntimeValue b2, RuntimeValue b3)
{
    uint64_t v0 = (uint64_t)DECODE_INT(b0) & 0xFF;
    uint64_t v1 = (uint64_t)DECODE_INT(b1) & 0xFF;
    uint64_t v2 = (uint64_t)DECODE_INT(b2) & 0xFF;
    uint64_t v3 = (uint64_t)DECODE_INT(b3) & 0xFF;
    return ENCODE_INT((v0 << 24) | (v1 << 16) | (v2 << 8) | v3);
}

RuntimeValue bytes_to_u64_be(RuntimeValue b0, RuntimeValue b1,
                             RuntimeValue b2, RuntimeValue b3,
                             RuntimeValue b4, RuntimeValue b5,
                             RuntimeValue b6, RuntimeValue b7)
{
    uint64_t v0 = (uint64_t)DECODE_INT(b0) & 0xFF;
    uint64_t v1 = (uint64_t)DECODE_INT(b1) & 0xFF;
    uint64_t v2 = (uint64_t)DECODE_INT(b2) & 0xFF;
    uint64_t v3 = (uint64_t)DECODE_INT(b3) & 0xFF;
    uint64_t v4 = (uint64_t)DECODE_INT(b4) & 0xFF;
    uint64_t v5 = (uint64_t)DECODE_INT(b5) & 0xFF;
    uint64_t v6 = (uint64_t)DECODE_INT(b6) & 0xFF;
    uint64_t v7 = (uint64_t)DECODE_INT(b7) & 0xFF;
    uint64_t result = (v0 << 56) | (v1 << 48) | (v2 << 40) | (v3 << 32) |
                      (v4 << 24) | (v5 << 16) | (v6 << 8)  | v7;
    return ENCODE_INT((int64_t)result);
}

RuntimeValue bytes_to_u64_le(RuntimeValue b0, RuntimeValue b1,
                             RuntimeValue b2, RuntimeValue b3,
                             RuntimeValue b4, RuntimeValue b5,
                             RuntimeValue b6, RuntimeValue b7)
{
    uint64_t v0 = (uint64_t)DECODE_INT(b0) & 0xFF;
    uint64_t v1 = (uint64_t)DECODE_INT(b1) & 0xFF;
    uint64_t v2 = (uint64_t)DECODE_INT(b2) & 0xFF;
    uint64_t v3 = (uint64_t)DECODE_INT(b3) & 0xFF;
    uint64_t v4 = (uint64_t)DECODE_INT(b4) & 0xFF;
    uint64_t v5 = (uint64_t)DECODE_INT(b5) & 0xFF;
    uint64_t v6 = (uint64_t)DECODE_INT(b6) & 0xFF;
    uint64_t v7 = (uint64_t)DECODE_INT(b7) & 0xFF;
    uint64_t result = (v7 << 56) | (v6 << 48) | (v5 << 40) | (v4 << 32) |
                      (v3 << 24) | (v2 << 16) | (v1 << 8)  | v0;
    return ENCODE_INT((int64_t)result);
}

/* --- Byte unpacking (returns RuntimeArray) --- */

/* Helper: create a small array of ENCODE_INT byte values */
static RuntimeValue make_byte_array(const uint8_t *bytes, int count)
{
    RuntimeValue arr = rt_array_new(ENCODE_INT(count));
    for (int i = 0; i < count; i++) {
        rt_array_push(arr, ENCODE_INT(bytes[i]));
    }
    return arr;
}

RuntimeValue u16_to_bytes_be(RuntimeValue v)
{
    uint64_t val = (uint64_t)DECODE_INT(v);
    uint8_t bytes[2];
    bytes[0] = (uint8_t)((val >> 8) & 0xFF);
    bytes[1] = (uint8_t)(val & 0xFF);
    return make_byte_array(bytes, 2);
}

RuntimeValue u16_to_bytes_le(RuntimeValue v)
{
    uint64_t val = (uint64_t)DECODE_INT(v);
    uint8_t bytes[2];
    bytes[0] = (uint8_t)(val & 0xFF);
    bytes[1] = (uint8_t)((val >> 8) & 0xFF);
    return make_byte_array(bytes, 2);
}

RuntimeValue u32_to_bytes_be(RuntimeValue v)
{
    uint64_t val = (uint64_t)DECODE_INT(v);
    uint8_t bytes[4];
    bytes[0] = (uint8_t)((val >> 24) & 0xFF);
    bytes[1] = (uint8_t)((val >> 16) & 0xFF);
    bytes[2] = (uint8_t)((val >> 8) & 0xFF);
    bytes[3] = (uint8_t)(val & 0xFF);
    return make_byte_array(bytes, 4);
}

RuntimeValue u64_to_bytes_be(RuntimeValue v)
{
    uint64_t val = (uint64_t)DECODE_INT(v);
    uint8_t bytes[8];
    bytes[0] = (uint8_t)((val >> 56) & 0xFF);
    bytes[1] = (uint8_t)((val >> 48) & 0xFF);
    bytes[2] = (uint8_t)((val >> 40) & 0xFF);
    bytes[3] = (uint8_t)((val >> 32) & 0xFF);
    bytes[4] = (uint8_t)((val >> 24) & 0xFF);
    bytes[5] = (uint8_t)((val >> 16) & 0xFF);
    bytes[6] = (uint8_t)((val >> 8) & 0xFF);
    bytes[7] = (uint8_t)(val & 0xFF);
    return make_byte_array(bytes, 8);
}

RuntimeValue u64_to_bytes_le(RuntimeValue v)
{
    uint64_t val = (uint64_t)DECODE_INT(v);
    uint8_t bytes[8];
    bytes[0] = (uint8_t)(val & 0xFF);
    bytes[1] = (uint8_t)((val >> 8) & 0xFF);
    bytes[2] = (uint8_t)((val >> 16) & 0xFF);
    bytes[3] = (uint8_t)((val >> 24) & 0xFF);
    bytes[4] = (uint8_t)((val >> 32) & 0xFF);
    bytes[5] = (uint8_t)((val >> 40) & 0xFF);
    bytes[6] = (uint8_t)((val >> 48) & 0xFF);
    bytes[7] = (uint8_t)((val >> 56) & 0xFF);
    return make_byte_array(bytes, 8);
}

/* --- Float bit-casting ---
 *
 * The Simple runtime uses tagged values. Floats are stored with TAG_FLOAT (0x2)
 * in the low 3 bits. The upper 61 bits hold IEEE 754 bits shifted left by 3.
 * For f32: the 32-bit IEEE 754 pattern is zero-extended to 61 bits.
 * For f64: the 64-bit IEEE 754 pattern is shifted right by 3 (lossy for
 *          the lowest 3 mantissa bits, acceptable for baremetal).
 */

RuntimeValue f32_from_bits(RuntimeValue bits)
{
    /* bits is ENCODE_INT(ieee_f32_bits). Extract the 32-bit pattern. */
    uint32_t fbits = (uint32_t)(DECODE_INT(bits) & 0xFFFFFFFF);
    /* Encode as tagged float: upper bits hold the IEEE pattern, low 3 = TAG_FLOAT */
    return (RuntimeValue)(((uint64_t)fbits << 3) | TAG_FLOAT);
}

RuntimeValue f32_to_bits(RuntimeValue val)
{
    /* val is a tagged float. Extract the 32-bit IEEE pattern. */
    uint32_t fbits = (uint32_t)((uint64_t)val >> 3);
    return ENCODE_INT(fbits);
}

RuntimeValue f64_from_bits(RuntimeValue bits)
{
    /* bits is ENCODE_INT(ieee_f64_bits). Reinterpret as tagged float. */
    uint64_t fbits = (uint64_t)DECODE_INT(bits);
    return (RuntimeValue)((fbits << 3) | TAG_FLOAT);
}

RuntimeValue f64_to_bits(RuntimeValue val)
{
    /* val is a tagged float. Extract the IEEE 754 bits. */
    uint64_t fbits = (uint64_t)val >> 3;
    return ENCODE_INT((int64_t)fbits);
}

RuntimeValue float_to_bits(RuntimeValue val)   { return f64_to_bits(val); }
RuntimeValue bits_to_float(RuntimeValue bits)  { return f64_from_bits(bits); }
RuntimeValue spl_bits_to_f64(RuntimeValue bits){ return f64_from_bits(bits); }
RuntimeValue spl_f64_to_bits(RuntimeValue val) { return f64_to_bits(val); }

RuntimeValue is_nan_bits(RuntimeValue bits)
{
    /* Check if IEEE 754 double bits represent NaN.
     * NaN: exponent all 1s (bits 52-62) and mantissa non-zero. */
    uint64_t fbits = (uint64_t)DECODE_INT(bits);
    uint64_t exp = (fbits >> 52) & 0x7FF;
    uint64_t mantissa = fbits & 0x000FFFFFFFFFFFFFULL;
    if (exp == 0x7FF && mantissa != 0)
        return TRUE_VALUE;
    return FALSE_VALUE;
}

/* ===================================================================
 * 3. String/array primitives
 * =================================================================== */

/* Helper: convert int64 to decimal string (RuntimeValue string) */
static RuntimeValue prim_int_to_str(int64_t n)
{
    if (n == 0) return rt_string_from_cstr("0");
    /* Handle INT64_MIN */
    if (n == (-9223372036854775807LL - 1))
        return rt_string_from_cstr("-9223372036854775808");

    char buf[21];
    int pos = 0, neg = 0;
    uint64_t uv;
    if (n < 0) { neg = 1; uv = (uint64_t)(-n); } else { uv = (uint64_t)n; }
    while (uv > 0) { buf[pos++] = '0' + (char)(uv % 10); uv /= 10; }

    uint32_t len = (uint32_t)(pos + neg);
    RuntimeString *s = (RuntimeString *)malloc(sizeof(RuntimeString) + len + 1);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + len + 1);
    s->len = len;
    int out = 0;
    if (neg) s->data[out++] = '-';
    while (pos > 0) s->data[out++] = buf[--pos];
    s->data[out] = '\0';
    return ENCODE_PTR(s);
}

RuntimeValue int_to_string(RuntimeValue v)
{
    return prim_int_to_str(DECODE_INT(v));
}

RuntimeValue core_int_to_str(RuntimeValue v)
{
    return prim_int_to_str(DECODE_INT(v));
}

RuntimeValue parse_int(RuntimeValue s)
{
    if (!IS_HEAP(s)) return NIL_VALUE;
    RuntimeString *str = (RuntimeString *)DECODE_PTR(s);
    if (!str || str->hdr.type != HEAP_STRING || str->len == 0)
        return NIL_VALUE;

    int64_t result = 0;
    int neg = 0;
    uint32_t i = 0;

    if (str->data[0] == '-') { neg = 1; i = 1; }
    else if (str->data[0] == '+') { i = 1; }

    if (i >= str->len) return NIL_VALUE; /* just a sign, no digits */

    for (; i < str->len; i++) {
        char c = str->data[i];
        if (c < '0' || c > '9') return NIL_VALUE; /* non-digit */
        result = result * 10 + (c - '0');
    }
    if (neg) result = -result;
    return ENCODE_INT(result);
}

RuntimeValue string_byte_at(RuntimeValue s, RuntimeValue i)
{
    if (!IS_HEAP(s)) return ENCODE_INT(0);
    RuntimeString *str = (RuntimeString *)DECODE_PTR(s);
    if (!str || str->hdr.type != HEAP_STRING) return ENCODE_INT(0);
    int64_t idx = (int64_t)i;
    if (idx < 0 || (uint32_t)idx >= str->len) return ENCODE_INT(0);
    return ENCODE_INT((unsigned char)str->data[idx]);
}

RuntimeValue string_char_at(RuntimeValue s, RuntimeValue i)
{
    /* For ASCII, char_at == byte_at as a single-char string */
    if (!IS_HEAP(s)) return NIL_VALUE;
    RuntimeString *str = (RuntimeString *)DECODE_PTR(s);
    if (!str || str->hdr.type != HEAP_STRING) return NIL_VALUE;
    int64_t idx = (int64_t)i;
    if (idx < 0 || (uint32_t)idx >= str->len) return NIL_VALUE;

    char buf[2];
    buf[0] = str->data[idx];
    buf[1] = '\0';
    return rt_string_from_cstr(buf);
}

RuntimeValue string_char_code(RuntimeValue s, RuntimeValue i)
{
    if (!IS_HEAP(s)) return ENCODE_INT(0);
    RuntimeString *str = (RuntimeString *)DECODE_PTR(s);
    if (!str || str->hdr.type != HEAP_STRING) return ENCODE_INT(0);
    int64_t idx = (int64_t)i;
    if (idx < 0 || (uint32_t)idx >= str->len) return ENCODE_INT(0);
    return ENCODE_INT((unsigned char)str->data[idx]);
}

int64_t rt_text_find(RuntimeValue haystack, RuntimeValue needle, int64_t start)
{
    if (!IS_HEAP(haystack) || !IS_HEAP(needle)) return -1;
    RuntimeString *hs = (RuntimeString *)DECODE_PTR(haystack);
    RuntimeString *nd = (RuntimeString *)DECODE_PTR(needle);
    if (!hs || hs->hdr.type != HEAP_STRING || !nd || nd->hdr.type != HEAP_STRING) return -1;
    int64_t start_idx = start;
    if (start_idx < 0) return -1;
    if (nd->len == 0) return start_idx;
    if ((uint32_t)start_idx >= hs->len || nd->len > hs->len) return -1;
    uint32_t start_u = (uint32_t)start_idx;
    uint32_t limit = hs->len - nd->len;
    for (uint32_t i = start_u; i <= limit; i++) {
        uint32_t j = 0;
        for (; j < nd->len; j++) {
            if (hs->data[i + j] != nd->data[j]) break;
        }
        if (j == nd->len) return (int64_t)i;
    }
    return -1;
}

RuntimeValue string_from_byte(RuntimeValue b)
{
    char buf[2];
    buf[0] = (char)(DECODE_INT(b) & 0xFF);
    buf[1] = '\0';
    return rt_string_from_cstr(buf);
}

RuntimeValue string_from_char_code(RuntimeValue code)
{
    char buf[2];
    buf[0] = (char)(DECODE_INT(code) & 0xFF);
    buf[1] = '\0';
    return rt_string_from_cstr(buf);
}

RuntimeValue from_char_code(RuntimeValue code)
{
    return string_from_char_code(code);
}

static int64_t _rv_to_index_compat(RuntimeValue v)
{
    /* Bare-metal string helpers receive a mix of raw and boxed indices depending
     * on which lowering/runtime path reached them. Prefer raw values for the
     * browser/runtime lane, but still accept classic boxed ints. */
    if (IS_INT(v)) return DECODE_INT(v);
    return (int64_t)v;
}

RuntimeValue substring(RuntimeValue s, RuntimeValue start, RuntimeValue end)
{
    if (!IS_HEAP(s)) return NIL_VALUE;
    RuntimeString *str = (RuntimeString *)DECODE_PTR(s);
    if (!str || str->hdr.type != HEAP_STRING) return NIL_VALUE;

    int64_t a = _rv_to_index_compat(start);
    int64_t b = _rv_to_index_compat(end);
    if (a < 0) a = 0;
    if (b > (int64_t)str->len) b = (int64_t)str->len;
    if (a >= b) return rt_string_from_cstr("");

    uint32_t len = (uint32_t)(b - a);
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + len + 1);
    if (!r) return NIL_VALUE;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + len + 1);
    r->len = len;
    for (uint32_t j = 0; j < len; j++) r->data[j] = str->data[a + j];
    r->data[len] = '\0';
    return ENCODE_PTR(r);
}

RuntimeValue char_to_lower(RuntimeValue c)
{
    int64_t ch = DECODE_INT(c);
    if (ch >= 'A' && ch <= 'Z') ch = ch + ('a' - 'A');
    return ENCODE_INT(ch);
}

RuntimeValue trim_start(RuntimeValue s)
{
    if (!IS_HEAP(s)) return NIL_VALUE;
    RuntimeString *str = (RuntimeString *)DECODE_PTR(s);
    if (!str || str->hdr.type != HEAP_STRING) return NIL_VALUE;

    uint32_t i = 0;
    while (i < str->len && (str->data[i] == ' ' || str->data[i] == '\t' ||
           str->data[i] == '\n' || str->data[i] == '\r'))
        i++;

    if (i == 0) return s; /* no leading whitespace */

    uint32_t len = str->len - i;
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + len + 1);
    if (!r) return NIL_VALUE;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + len + 1);
    r->len = len;
    for (uint32_t j = 0; j < len; j++) r->data[j] = str->data[i + j];
    r->data[len] = '\0';
    return ENCODE_PTR(r);
}

RuntimeValue trim_end(RuntimeValue s)
{
    if (!IS_HEAP(s)) return NIL_VALUE;
    RuntimeString *str = (RuntimeString *)DECODE_PTR(s);
    if (!str || str->hdr.type != HEAP_STRING) return NIL_VALUE;

    int64_t end = (int64_t)str->len;
    while (end > 0 && (str->data[end - 1] == ' ' || str->data[end - 1] == '\t' ||
           str->data[end - 1] == '\n' || str->data[end - 1] == '\r'))
        end--;

    if (end == (int64_t)str->len) return s; /* no trailing whitespace */

    uint32_t len = (uint32_t)end;
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + len + 1);
    if (!r) return NIL_VALUE;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + len + 1);
    r->len = len;
    for (uint32_t j = 0; j < len; j++) r->data[j] = str->data[j];
    r->data[len] = '\0';
    return ENCODE_PTR(r);
}

/* --- Array primitives ---
 * These are wrappers around the rt_array_* functions, providing the
 * non-prefixed names that the Simple compiler emits. */

RuntimeValue array_new(RuntimeValue cap)
{
    return rt_array_new(cap);
}

RuntimeValue array_get(RuntimeValue arr, RuntimeValue i)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    int64_t idx = (int64_t)i;
    if (idx < 0 || (uint32_t)idx >= a->len) return NIL_VALUE;
    return a->items[idx];
}

RuntimeValue array_set(RuntimeValue arr, RuntimeValue i, RuntimeValue v)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    int64_t idx = (int64_t)i;
    if (idx < 0 || (uint32_t)idx >= a->len) return NIL_VALUE;
    a->items[idx] = v;
    return v;
}

RuntimeValue array_append(RuntimeValue arr, RuntimeValue v)
{
    rt_array_push(arr, v);
    return arr;
}

RuntimeValue array_len(RuntimeValue arr)
{
    if (!IS_HEAP(arr)) return ENCODE_INT(0);
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return ENCODE_INT(0);
    return ENCODE_INT(a->len);
}

RuntimeValue array_length(RuntimeValue arr)
{
    return array_len(arr);
}

/* ===================================================================
 * 4. Memory/pointer operations
 * =================================================================== */

RuntimeValue allocate_buffer(RuntimeValue size)
{
    int64_t sz = DECODE_INT(size);
    if (sz <= 0) sz = 64;
    if (sz > 0x1000000) sz = 0x1000000; /* 16 MB safety limit */
    void *p = malloc((unsigned long)sz);
    if (!p) return NIL_VALUE;
    /* Zero-init */
    uint8_t *bp = (uint8_t *)p;
    for (int64_t i = 0; i < sz; i++) bp[i] = 0;
    return ENCODE_PTR(p);
}

RuntimeValue spl_alloc_buffer(RuntimeValue size)
{
    return allocate_buffer(size);
}

RuntimeValue spl_free_buffer(RuntimeValue ptr)
{
    /* No-op on bump allocator */
    (void)ptr;
    return NIL_VALUE;
}

RuntimeValue spl_read_u8(RuntimeValue ptr, RuntimeValue off)
{
    if (!IS_HEAP(ptr)) return ENCODE_INT(0);
    uint8_t *base = (uint8_t *)DECODE_PTR(ptr);
    if (!base) return ENCODE_INT(0);
    int64_t offset = DECODE_INT(off);
    return ENCODE_INT(base[offset]);
}

RuntimeValue spl_read_i32(RuntimeValue ptr, RuntimeValue off)
{
    if (!IS_HEAP(ptr)) return ENCODE_INT(0);
    uint8_t *base = (uint8_t *)DECODE_PTR(ptr);
    if (!base) return ENCODE_INT(0);
    int64_t offset = DECODE_INT(off);
    /* Unaligned read — byte by byte for portability */
    uint8_t *p = base + offset;
    int32_t val = (int32_t)((uint32_t)p[0] |
                            ((uint32_t)p[1] << 8) |
                            ((uint32_t)p[2] << 16) |
                            ((uint32_t)p[3] << 24));
    return ENCODE_INT(val);
}

RuntimeValue spl_read_ptr(RuntimeValue ptr, RuntimeValue off)
{
    if (!IS_HEAP(ptr)) return NIL_VALUE;
    uint8_t *base = (uint8_t *)DECODE_PTR(ptr);
    if (!base) return NIL_VALUE;
    int64_t offset = DECODE_INT(off);
    /* Read a pointer-sized value (8 bytes on x86_64) */
    uint8_t *p = base + offset;
    uint64_t val = 0;
    for (int i = 0; i < 8; i++)
        val |= ((uint64_t)p[i]) << (i * 8);
    void *result = (void *)(uintptr_t)val;
    if (!result) return NIL_VALUE;
    return ENCODE_PTR(result);
}

RuntimeValue spl_read_bytes(RuntimeValue ptr, RuntimeValue off, RuntimeValue len)
{
    if (!IS_HEAP(ptr)) return NIL_VALUE;
    uint8_t *base = (uint8_t *)DECODE_PTR(ptr);
    if (!base) return NIL_VALUE;
    int64_t offset = DECODE_INT(off);
    int64_t count = DECODE_INT(len);
    if (count <= 0) return rt_array_new(ENCODE_INT(1));
    if (count > 0x100000) count = 0x100000; /* safety limit */

    RuntimeValue arr = rt_array_new(ENCODE_INT(count));
    uint8_t *src = base + offset;
    for (int64_t i = 0; i < count; i++) {
        rt_array_push(arr, ENCODE_INT(src[i]));
    }
    return arr;
}

RuntimeValue spl_write_u8(RuntimeValue ptr, RuntimeValue off, RuntimeValue val)
{
    if (!IS_HEAP(ptr)) return NIL_VALUE;
    uint8_t *base = (uint8_t *)DECODE_PTR(ptr);
    if (!base) return NIL_VALUE;
    int64_t offset = DECODE_INT(off);
    base[offset] = (uint8_t)(DECODE_INT(val) & 0xFF);
    return NIL_VALUE;
}

RuntimeValue spl_write_bytes(RuntimeValue ptr, RuntimeValue off,
                             RuntimeValue data, RuntimeValue len)
{
    if (!IS_HEAP(ptr) || !IS_HEAP(data)) return NIL_VALUE;
    uint8_t *base = (uint8_t *)DECODE_PTR(ptr);
    if (!base) return NIL_VALUE;
    int64_t offset = DECODE_INT(off);
    int64_t count = DECODE_INT(len);

    /* data can be a RuntimeArray or RuntimeString */
    HeapHeader *hdr = (HeapHeader *)DECODE_PTR(data);
    if (!hdr) return NIL_VALUE;

    uint8_t *dst = base + offset;

    if (hdr->type == HEAP_ARRAY) {
        RuntimeArray *arr = (RuntimeArray *)hdr;
        if (count > (int64_t)arr->len) count = (int64_t)arr->len;
        for (int64_t i = 0; i < count; i++) {
            dst[i] = (uint8_t)(DECODE_INT(arr->items[i]) & 0xFF);
        }
    } else if (hdr->type == HEAP_STRING) {
        RuntimeString *str = (RuntimeString *)hdr;
        if (count > (int64_t)str->len) count = (int64_t)str->len;
        for (int64_t i = 0; i < count; i++) {
            dst[i] = (uint8_t)str->data[i];
        }
    }
    return NIL_VALUE;
}

RuntimeValue spl_str_ptr(RuntimeValue s)
{
    if (!IS_HEAP(s)) return NIL_VALUE;
    RuntimeString *str = (RuntimeString *)DECODE_PTR(s);
    if (!str || str->hdr.type != HEAP_STRING) return NIL_VALUE;
    return ENCODE_PTR(str->data);
}

RuntimeValue spl_str_to_cstr(RuntimeValue s)
{
    if (!IS_HEAP(s)) return NIL_VALUE;
    RuntimeString *str = (RuntimeString *)DECODE_PTR(s);
    if (!str || str->hdr.type != HEAP_STRING) return NIL_VALUE;
    /* String data is already null-terminated by rt_string_new/rt_string_from_cstr */
    return ENCODE_PTR(str->data);
}

RuntimeValue spl_cstr_to_str(RuntimeValue ptr)
{
    if (!IS_HEAP(ptr)) return NIL_VALUE;
    const char *cstr = (const char *)DECODE_PTR(ptr);
    if (!cstr) return NIL_VALUE;
    return rt_string_from_cstr(cstr);
}

RuntimeValue ptr_to_i64(RuntimeValue ptr)
{
    if (!IS_HEAP(ptr)) return ENCODE_INT(0);
    void *p = DECODE_PTR(ptr);
    return ENCODE_INT((int64_t)(uintptr_t)p);
}

RuntimeValue i64_to_ptr(RuntimeValue i)
{
    int64_t val = DECODE_INT(i);
    void *p = (void *)(uintptr_t)val;
    if (!p) return NIL_VALUE;
    return ENCODE_PTR(p);
}

RuntimeValue ptr_add(RuntimeValue ptr, RuntimeValue off)
{
    if (!IS_HEAP(ptr)) return NIL_VALUE;
    uint8_t *p = (uint8_t *)DECODE_PTR(ptr);
    if (!p) return NIL_VALUE;
    int64_t offset = DECODE_INT(off);
    return ENCODE_PTR(p + offset);
}

RuntimeValue ptr_sub(RuntimeValue ptr, RuntimeValue off)
{
    if (!IS_HEAP(ptr)) return NIL_VALUE;
    uint8_t *p = (uint8_t *)DECODE_PTR(ptr);
    if (!p) return NIL_VALUE;
    int64_t offset = DECODE_INT(off);
    return ENCODE_PTR(p - offset);
}

RuntimeValue spl_i64_is_zero(RuntimeValue v)
{
    int64_t val = DECODE_INT(v);
    return val == 0 ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue unsafe_addr_of(RuntimeValue v)
{
    /* Return the raw RuntimeValue bits as an integer.
     * This exposes the tagged pointer/int/float encoding. */
    return ENCODE_INT((int64_t)(uint64_t)v);
}

/* ===================================================================
 * 5. Synchronization stubs (no-ops for single-threaded baremetal)
 * =================================================================== */

RuntimeValue spl_mutex_create(void)
{
    /* Return a dummy non-nil handle so callers don't think it failed */
    return ENCODE_INT(1);
}

RuntimeValue spl_mutex_lock(RuntimeValue m)
{
    (void)m;
    return NIL_VALUE;
}

RuntimeValue spl_mutex_try_lock(RuntimeValue m)
{
    (void)m;
    return TRUE_VALUE; /* always succeeds */
}

RuntimeValue spl_mutex_unlock(RuntimeValue m)
{
    (void)m;
    return NIL_VALUE;
}

RuntimeValue spl_mutex_destroy(RuntimeValue m)
{
    (void)m;
    return NIL_VALUE;
}

RuntimeValue spl_condvar_create(void)
{
    return ENCODE_INT(1); /* dummy handle */
}

RuntimeValue spl_condvar_wait(RuntimeValue c, RuntimeValue m)
{
    (void)c; (void)m;
    return NIL_VALUE;
}

RuntimeValue spl_condvar_wait_timeout(RuntimeValue c, RuntimeValue m, RuntimeValue ms)
{
    (void)c; (void)m; (void)ms;
    return FALSE_VALUE; /* timed out — no actual wait */
}

RuntimeValue spl_condvar_signal(RuntimeValue c)
{
    (void)c;
    return NIL_VALUE;
}

RuntimeValue spl_condvar_broadcast(RuntimeValue c)
{
    (void)c;
    return NIL_VALUE;
}

RuntimeValue spl_condvar_destroy(RuntimeValue c)
{
    (void)c;
    return NIL_VALUE;
}

RuntimeValue spl_thread_create(RuntimeValue fn, RuntimeValue arg)
{
    (void)fn; (void)arg;
    return NIL_VALUE; /* no threads on baremetal */
}

RuntimeValue spl_thread_join(RuntimeValue t)
{
    (void)t;
    return NIL_VALUE;
}

RuntimeValue spl_thread_detach(RuntimeValue t)
{
    (void)t;
    return NIL_VALUE;
}

RuntimeValue spl_thread_yield(void)
{
    return NIL_VALUE;
}

RuntimeValue spl_thread_sleep(RuntimeValue ms)
{
    (void)ms;
    return NIL_VALUE;
}

RuntimeValue spl_thread_current_id(void)
{
    return ENCODE_INT(0);
}

RuntimeValue spl_thread_cpu_count(void)
{
    return ENCODE_INT(1);
}

RuntimeValue spl_thread_pool_spawn_worker(RuntimeValue fn, RuntimeValue arg)
{
    (void)fn; (void)arg;
    return NIL_VALUE;
}

/* ===================================================================
 * 6. Miscellaneous
 * =================================================================== */

RuntimeValue clock_ms(void)
{
    /* No timer driver yet — return 0 */
    return ENCODE_INT(0);
}

RuntimeValue current_time_micros(void)
{
    return ENCODE_INT(0);
}

RuntimeValue type_id_of(RuntimeValue v)
{
    /* Return a type identifier based on the tag:
     *   0 = int, 1 = heap, 2 = float, 3 = special (nil) */
    uint64_t tag = (uint64_t)v & TAG_MASK;
    return ENCODE_INT((int64_t)tag);
}

RuntimeValue sort_values(RuntimeValue arr)
{
    /* Simple insertion sort on a RuntimeArray of tagged integers */
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY || a->len <= 1) return arr;

    for (uint32_t i = 1; i < a->len; i++) {
        RuntimeValue key = a->items[i];
        int64_t kv = (int64_t)key; /* compare raw tagged values */
        int64_t j = (int64_t)i - 1;
        while (j >= 0 && (int64_t)a->items[j] > kv) {
            a->items[j + 1] = a->items[j];
            j--;
        }
        a->items[j + 1] = key;
    }
    return arr;
}

RuntimeValue glob_matches(RuntimeValue pattern, RuntimeValue path)
{
    /* Simple wildcard matching: '*' matches any sequence, '?' matches one char.
     * Both args are RuntimeValue strings. */
    if (!IS_HEAP(pattern) || !IS_HEAP(path)) return FALSE_VALUE;
    RuntimeString *pat = (RuntimeString *)DECODE_PTR(pattern);
    RuntimeString *str = (RuntimeString *)DECODE_PTR(path);
    if (!pat || !str || pat->hdr.type != HEAP_STRING || str->hdr.type != HEAP_STRING)
        return FALSE_VALUE;

    /* Iterative glob match with backtracking */
    uint32_t pi = 0, si = 0;
    uint32_t star_p = (uint32_t)-1, star_s = 0;

    while (si < str->len) {
        if (pi < pat->len && (pat->data[pi] == '?' || pat->data[pi] == str->data[si])) {
            pi++; si++;
        } else if (pi < pat->len && pat->data[pi] == '*') {
            star_p = pi++;
            star_s = si;
        } else if (star_p != (uint32_t)-1) {
            pi = star_p + 1;
            si = ++star_s;
        } else {
            return FALSE_VALUE;
        }
    }
    /* Consume trailing '*' in pattern */
    while (pi < pat->len && pat->data[pi] == '*') pi++;
    return pi == pat->len ? TRUE_VALUE : FALSE_VALUE;
}

/* Basic xorshift64 PRNG state (seeded on first use) */
static uint64_t prng_state = 0;

static uint64_t xorshift64(void)
{
    if (prng_state == 0) prng_state = 0xDEADBEEFCAFEBABEULL; /* seed */
    uint64_t x = prng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    prng_state = x;
    return x;
}

RuntimeValue random_randint(RuntimeValue lo, RuntimeValue hi)
{
    int64_t a = DECODE_INT(lo);
    int64_t b = DECODE_INT(hi);
    if (a > b) { int64_t t = a; a = b; b = t; }
    if (a == b) return ENCODE_INT(a);
    uint64_t range = (uint64_t)(b - a + 1);
    uint64_t r = xorshift64() % range;
    return ENCODE_INT(a + (int64_t)r);
}

RuntimeValue random_uniform(void)
{
    /* Return a float 0.0 to 1.0 (approximately).
     * Use the PRNG to generate a double in [0,1) and tag it as float. */
    uint64_t r = xorshift64();
    /* Scale to [0, 1): divide by 2^64 using integer math.
     * We store the IEEE 754 double bits in the tagged float format. */
    /* Simple approximation: r >> 11 gives 53 significant bits,
     * then multiply conceptually by 2^-53. Construct IEEE 754 directly. */
    uint64_t mantissa = r >> 11; /* 53 bits */
    /* 0.0 to ~1.0: exponent = 1023-1 = 1022 (for 0.5 range), shift to cover full [0,1) */
    /* Simpler: construct 1.0 + fraction, then subtract 1.0 */
    uint64_t dbits = 0x3FF0000000000000ULL | (mantissa >> 1); /* 1.0 + fraction */
    /* This gives values in [1.0, 2.0). Subtract 1.0 by adjusting exponent. */
    /* Actually, let's just use the raw fraction approach */
    /* exponent for [0.5, 1.0): 0x3FE (1022) */
    dbits = (0x3FEULL << 52) | (mantissa & 0x000FFFFFFFFFFFFFULL);
    /* Tag as float */
    return (RuntimeValue)((dbits << 3) | TAG_FLOAT);
}

RuntimeValue platform_name(void)
{
    return rt_string_from_cstr("x86_64");
}

RuntimeValue platform_lib_extension(void)
{
    return rt_string_from_cstr(".a");
}

RuntimeValue temp_dir(void)
{
    return rt_string_from_cstr("/tmp");
}
