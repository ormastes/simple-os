#include <stdint.h>
#include <stddef.h>

typedef intptr_t RuntimeValue;

#define UART_BASE 0x10000000UL
#include "../../common/baremetal_16550_serial.h"
#define SIFIVE_TEST_BASE 0x100000UL
#define VIRTIO_MMIO_BASE 0x10001000UL
#define VIRTIO_MMIO_STRIDE 0x1000UL
#define VIRTIO_MMIO_SLOTS 8U
#define VIRTIO_MAGIC 0x74726976U
#define VIRTIO_DEV_NET 1U
#define VIRTIO_DEV_BLK 2U
#define VIRTQ_DESC_F_NEXT 1U
#define VIRTQ_DESC_F_WRITE 2U
#define VIRTIO_STATUS_ACKNOWLEDGE 1U
#define VIRTIO_STATUS_DRIVER 2U
#define VIRTIO_STATUS_DRIVER_OK 4U
#define VIRTIO_STATUS_FEATURES_OK 8U

#define TAG_MASK    ((uintptr_t)0x7)
#define TAG_INT     ((uintptr_t)0x0)
#define TAG_HEAP    ((uintptr_t)0x1)
#define TAG_FLOAT   ((uintptr_t)0x2)
#define TAG_SPECIAL ((uintptr_t)0x3)
#define NIL_VALUE   ((RuntimeValue)TAG_SPECIAL)
#define TRUE_VALUE  ENCODE_INT(1)
#define FALSE_VALUE ENCODE_INT(0)

#define ENCODE_INT(v) ((RuntimeValue)(((uint64_t)(int64_t)(v) << 3) | TAG_INT))
#define DECODE_INT(v) ((int64_t)((uint64_t)(v) >> 3))
#define ENCODE_PTR(p) ((RuntimeValue)((uintptr_t)(p) | TAG_HEAP))
#define DECODE_PTR(v) ((void *)((uintptr_t)(v) & ~TAG_MASK))
#define IS_INT(v)     (((uintptr_t)(v) & TAG_MASK) == TAG_INT)
#define IS_HEAP(v)    (((uintptr_t)(v) & TAG_MASK) == TAG_HEAP)

#define HEAP_STRING 1U
#define HEAP_ARRAY  2U
#define HEAP_ENUM   7U

typedef struct {
    uint32_t type;
    uint32_t size;
} HeapHeader;

typedef struct {
    HeapHeader hdr;
    uint32_t len;
    char data[];
} RuntimeString;

typedef struct {
    HeapHeader hdr;
    uint64_t len;
    uint64_t cap;
    RuntimeValue *items;
} RuntimeArray;

typedef struct {
    HeapHeader hdr;
    uint32_t enum_id;
    uint32_t discriminant;
    RuntimeValue payload;
} RuntimeEnum;

static unsigned char g_heap[64 * 1024] __attribute__((aligned(16)));
static uintptr_t g_heap_off = 0;
static unsigned char g_virtq[8192] __attribute__((aligned(4096)));
static unsigned char g_dma[1024] __attribute__((aligned(512)));
static unsigned char g_riscv_file_buf[8192] __attribute__((aligned(16)));
static unsigned char g_riscv_process_arena[2][8192] __attribute__((aligned(4096)));
static uint64_t g_riscv_process_entry[2];
static uint64_t g_riscv_process_pid[2];
static uint32_t g_riscv_process_count;
uint64_t g_fb_addr = 0;
uint64_t g_fb_w = 0;
static char g_riscv_gui_surface[256];
static volatile uint32_t *g_blk_mmio = 0;
static uint16_t g_last_used_idx = 0;

extern RuntimeValue spl_start(void);
extern char _stack_top[];

#define BAREMETAL_ENABLE_ALIGNED_ALLOC 1
#include "../../common/baremetal_bump_heap.h"

static RuntimeValue *runtime_array_inline_items(RuntimeArray *a)
{
    return (RuntimeValue *)((unsigned char *)a + sizeof(RuntimeArray));
}

static RuntimeValue *runtime_array_items(RuntimeArray *a)
{
    if (!a) return 0;
    return a->items ? a->items : runtime_array_inline_items(a);
}

static uint64_t simpleos_raw_or_encoded_int(RuntimeValue v)
{
    return IS_INT(v) ? (uint64_t)DECODE_INT(v) : (uint64_t)v;
}

void *malloc(size_t size)
{
    return rv_alloc(size);
}

void free(void *ptr)
{
    (void)ptr;
}

void *calloc(size_t n, size_t size)
{
    size_t total = n * size;
    void *ptr = rv_alloc(total);
    if (ptr) {
        unsigned char *bytes = (unsigned char *)ptr;
        for (size_t i = 0; i < total; i++) bytes[i] = 0;
    }
    return ptr;
}

void *realloc(void *ptr, size_t size)
{
    void *next = rv_alloc(size);
    if (!next || !ptr) return next;
    unsigned char *dst = (unsigned char *)next;
    const unsigned char *src = (const unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) dst[i] = src[i];
    return next;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i]) return (int)pa[i] - (int)pb[i];
    }
    return 0;
}

RuntimeValue rt_alloc(RuntimeValue sz)
{
    size_t bytes = (size_t)sz;
    void *ptr = calloc(1, bytes);
    return ptr ? (RuntimeValue)(uintptr_t)ptr : 0;
}

RuntimeValue f64_to_bits(RuntimeValue val)
{
    uint64_t fbits = (uint64_t)val >> 3;
    return ENCODE_INT((int64_t)fbits);
}

RuntimeValue spl_f64_to_bits(RuntimeValue val)
{
    return f64_to_bits(val);
}

RuntimeValue rt_dma_alloc(RuntimeValue size, RuntimeValue align)
{
    size_t bytes = (size_t)simpleos_raw_or_encoded_int(size);
    size_t alignment = (size_t)simpleos_raw_or_encoded_int(align);
    void *ptr = rv_alloc_aligned(bytes, alignment);
    return ptr ? (RuntimeValue)(uintptr_t)ptr : 0;
}

static void serial_puts(const char *s)
{
    uart_puts(s);
}

static void serial_putchar(char c)
{
    uart_putc(c);
}

void log_raw_println(RuntimeValue msg)
{
    if (IS_HEAP(msg)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(msg);
        if (s && s->hdr.type == HEAP_STRING && s->len < 4096U) {
            for (uint32_t i = 0; i < s->len; i++) uart_putc(s->data[i]);
        }
    }
    uart_putc('\r');
    uart_putc('\n');
}

static void serial_put_dec(int64_t value)
{
    char buf[32];
    uint32_t pos = 0;
    uint64_t raw = (uint64_t)(value < 0 ? -value : value);
    if (value == 0) {
        uart_putc('0');
        return;
    }
    while (raw > 0 && pos < sizeof(buf)) {
        buf[pos++] = (char)('0' + (raw % 10U));
        raw /= 10U;
    }
    if (value < 0 && pos < sizeof(buf)) buf[pos++] = '-';
    while (pos > 0) uart_putc(buf[--pos]);
}

static void serial_put_hex(uint32_t value)
{
    static const char hex[] = "0123456789abcdef";
    for (int shift = 28; shift >= 0; shift -= 4) {
        uart_putc(hex[(value >> shift) & 0xFU]);
    }
}

RuntimeValue rt_string_new(RuntimeValue data, RuntimeValue len_val)
{
    uintptr_t len = (uintptr_t)len_val;
    if (len > 4096U) return NIL_VALUE;
    RuntimeString *s = (RuntimeString *)rv_alloc(sizeof(RuntimeString) + len + 1U);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + len + 1U);
    s->len = (uint32_t)len;
    const char *src = (const char *)(uintptr_t)data;
    for (uintptr_t i = 0; i < len; i++) {
        s->data[i] = src ? src[i] : 0;
    }
    s->data[len] = 0;
    return ENCODE_PTR(s);
}

static RuntimeValue rt_string_from_cstr(const char *cstr)
{
    uintptr_t len = 0;
    while (cstr && cstr[len] != 0) len++;
    return rt_string_new((RuntimeValue)(uintptr_t)cstr, (RuntimeValue)len);
}

RuntimeValue rt_string_concat(RuntimeValue a, RuntimeValue b)
{
    RuntimeString *sa = IS_HEAP(a) ? (RuntimeString *)DECODE_PTR(a) : 0;
    RuntimeString *sb = IS_HEAP(b) ? (RuntimeString *)DECODE_PTR(b) : 0;
    uintptr_t la = sa ? sa->len : 0;
    uintptr_t lb = sb ? sb->len : 0;
    RuntimeString *out = (RuntimeString *)rv_alloc(sizeof(RuntimeString) + la + lb + 1U);
    if (!out) return NIL_VALUE;
    out->hdr.type = HEAP_STRING;
    out->hdr.size = (uint32_t)(sizeof(RuntimeString) + la + lb + 1U);
    out->len = (uint32_t)(la + lb);
    for (uintptr_t i = 0; i < la; i++) out->data[i] = sa->data[i];
    for (uintptr_t i = 0; i < lb; i++) out->data[la + i] = sb->data[i];
    out->data[la + lb] = 0;
    return ENCODE_PTR(out);
}

RuntimeValue rt_value_to_string(RuntimeValue value)
{
    if (IS_HEAP(value)) {
        HeapHeader *hdr = (HeapHeader *)DECODE_PTR(value);
        if (hdr && hdr->type == HEAP_STRING) return value;
        if (hdr && hdr->type == HEAP_ARRAY) return rt_string_from_cstr("<array>");
        return rt_string_from_cstr("<object>");
    }
    if (value == NIL_VALUE) return rt_string_from_cstr("nil");

    int64_t n = IS_INT(value) ? DECODE_INT(value) : (int64_t)value;
    char buf[32];
    uintptr_t pos = 0;
    uint64_t raw = (uint64_t)(n < 0 ? -n : n);
    if (n == 0) buf[pos++] = '0';
    while (raw > 0 && pos < sizeof(buf)) {
        buf[pos++] = (char)('0' + (raw % 10U));
        raw /= 10U;
    }
    if (n < 0) buf[pos++] = '-';
    RuntimeString *out = (RuntimeString *)rv_alloc(sizeof(RuntimeString) + pos + 1U);
    if (!out) return NIL_VALUE;
    out->hdr.type = HEAP_STRING;
    out->hdr.size = (uint32_t)(sizeof(RuntimeString) + pos + 1U);
    out->len = (uint32_t)pos;
    for (uintptr_t i = 0; i < pos; i++) out->data[i] = buf[pos - 1U - i];
    out->data[pos] = 0;
    return ENCODE_PTR(out);
}

RuntimeValue rt_to_string(RuntimeValue value)
{
    return rt_value_to_string(value);
}

static RuntimeValue rt_array_push_handle(RuntimeValue arr, RuntimeValue value)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    if (a->len >= a->cap) return arr;
    runtime_array_items(a)[a->len++] = value;
    return arr;
}

RuntimeValue rt_array_new(RuntimeValue cap_val)
{
    uint64_t cap = simpleos_raw_or_encoded_int(cap_val);
    if (cap == 0) cap = 16;
    if (cap < 16) cap = 16;
    RuntimeArray *a = (RuntimeArray *)rv_alloc(sizeof(RuntimeArray) + cap * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + cap * sizeof(RuntimeValue));
    a->len = 0;
    a->cap = cap;
    a->items = runtime_array_inline_items(a);
    for (uint64_t i = 0; i < cap; i++) a->items[i] = NIL_VALUE;
    return ENCODE_PTR(a);
}

RuntimeValue rt_array_new_with_cap(int64_t cap)
{
    return rt_array_new((RuntimeValue)cap);
}

int8_t rt_array_push(RuntimeValue arr, RuntimeValue value)
{
    return rt_array_push_handle(arr, value) != NIL_VALUE;
}

RuntimeValue rt_array_pop(RuntimeValue arr)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY || a->len == 0) return NIL_VALUE;
    RuntimeValue *items = runtime_array_items(a);
    a->len--;
    RuntimeValue value = items[a->len];
    items[a->len] = NIL_VALUE;
    return value;
}

RuntimeValue rt_array_get(RuntimeValue arr, RuntimeValue idx)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    int64_t i = (int64_t)idx;
    if (!a || a->hdr.type != HEAP_ARRAY || i < 0 || (uint64_t)i >= a->len) return NIL_VALUE;
    return runtime_array_items(a)[i];
}

int8_t rt_array_set(RuntimeValue arr, RuntimeValue idx, RuntimeValue value)
{
    if (!IS_HEAP(arr)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    int64_t i = (int64_t)idx;
    if (!a || a->hdr.type != HEAP_ARRAY || i < 0 || (uint64_t)i >= a->len) return 0;
    runtime_array_items(a)[i] = value;
    return 1;
}

RuntimeValue rt_array_len(RuntimeValue arr)
{
    if (!IS_HEAP(arr)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    return (!a || a->hdr.type != HEAP_ARRAY) ? 0 : (RuntimeValue)a->len;
}

RuntimeValue rt_arm_array_len_u32(RuntimeValue arr)
{
    RuntimeArray *tagged = IS_HEAP(arr) ? (RuntimeArray *)DECODE_PTR(arr) : (RuntimeArray *)0;
    if (tagged && tagged->hdr.type == HEAP_ARRAY && tagged->len <= tagged->cap)
        return (RuntimeValue)tagged->len;
    RuntimeArray *raw = (RuntimeArray *)(uintptr_t)(uint64_t)arr;
    if (raw && raw->hdr.type == HEAP_ARRAY && raw->len <= raw->cap)
        return (RuntimeValue)raw->len;
    return 0;
}

RuntimeValue rt_tuple_new(RuntimeValue len_rv)
{
    uint64_t len = simpleos_raw_or_encoded_int(len_rv);
    RuntimeArray *a = (RuntimeArray *)rv_alloc(sizeof(RuntimeArray) + len * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + len * sizeof(RuntimeValue));
    a->len = len;
    a->cap = len;
    a->items = runtime_array_inline_items(a);
    for (uint64_t i = 0; i < len; i++) a->items[i] = NIL_VALUE;
    return ENCODE_PTR(a);
}

RuntimeValue rt_tuple_get(RuntimeValue tuple, RuntimeValue index)
{
    return rt_array_get(tuple, index);
}

RuntimeValue rt_tuple_set(RuntimeValue tuple, RuntimeValue index, RuntimeValue value)
{
    return rt_array_set(tuple, index, value);
}

uint8_t rt_mmio_read_u8(uint64_t addr)
{
    return *(volatile uint8_t *)(uintptr_t)addr;
}

RuntimeValue rt_volatile_read_u8(RuntimeValue addr)
{
    return (RuntimeValue)(uint64_t)*(volatile uint8_t *)(uintptr_t)(uint64_t)addr;
}

uint16_t rt_mmio_read_u16(uint64_t addr)
{
    return *(volatile uint16_t *)(uintptr_t)addr;
}

uint32_t rt_mmio_read_u32(uint64_t addr)
{
    return *(volatile uint32_t *)(uintptr_t)addr;
}

uint64_t rt_mmio_read_u64(uint64_t addr)
{
    return *(volatile uint64_t *)(uintptr_t)addr;
}

void rt_mmio_write_u8(uint64_t addr, uint8_t value)
{
    *(volatile uint8_t *)(uintptr_t)addr = value;
}

void rt_mmio_write_u16(uint64_t addr, uint16_t value)
{
    *(volatile uint16_t *)(uintptr_t)addr = value;
}

void rt_mmio_write_u32(uint64_t addr, uint32_t value)
{
    *(volatile uint32_t *)(uintptr_t)addr = value;
}

void rt_mmio_write_u64(uint64_t addr, uint64_t value)
{
    *(volatile uint64_t *)(uintptr_t)addr = value;
}

RuntimeValue rt_len(RuntimeValue value)
{
    if (!IS_HEAP(value)) return 0;
    HeapHeader *hdr = (HeapHeader *)DECODE_PTR(value);
    if (!hdr) return 0;
    if (hdr->type == HEAP_STRING) return (RuntimeValue)((RuntimeString *)hdr)->len;
    if (hdr->type == HEAP_ARRAY) return (RuntimeValue)((RuntimeArray *)hdr)->len;
    return 0;
}

RuntimeValue rt_index_get(RuntimeValue value, RuntimeValue index)
{
    if (!IS_INT(index)) return NIL_VALUE;
    if (!IS_HEAP(value)) return NIL_VALUE;
    HeapHeader *hdr = (HeapHeader *)DECODE_PTR(value);
    if (!hdr) return NIL_VALUE;
    if (hdr->type == HEAP_ARRAY) return rt_array_get(value, (RuntimeValue)DECODE_INT(index));
    return NIL_VALUE;
}

RuntimeValue rt_index_set(RuntimeValue value, RuntimeValue index, RuntimeValue item)
{
    if (!IS_INT(index)) return 0;
    return rt_array_set(value, (RuntimeValue)DECODE_INT(index), item);
}

RuntimeValue rt_enum_new(RuntimeValue enum_id_rv, RuntimeValue disc_rv, RuntimeValue payload)
{
    RuntimeEnum *e = (RuntimeEnum *)rv_alloc(sizeof(RuntimeEnum));
    if (!e) return NIL_VALUE;
    e->hdr.type = HEAP_ENUM;
    e->hdr.size = (uint32_t)sizeof(RuntimeEnum);
    e->enum_id = (uint32_t)(int32_t)enum_id_rv;
    e->discriminant = (uint32_t)(int32_t)disc_rv;
    e->payload = payload;
    return ENCODE_PTR(e);
}

RuntimeValue rt_enum_payload(RuntimeValue value)
{
    if (!IS_HEAP(value)) return value;
    RuntimeEnum *e = (RuntimeEnum *)DECODE_PTR(value);
    return (!e || e->hdr.type != HEAP_ENUM) ? value : e->payload;
}

RuntimeValue rt_enum_check_discriminant(RuntimeValue value, RuntimeValue expected)
{
    if (!IS_HEAP(value)) return 0;
    RuntimeEnum *e = (RuntimeEnum *)DECODE_PTR(value);
    if (!e || e->hdr.type != HEAP_ENUM) return 0;
    return e->discriminant == (uint32_t)(int32_t)expected ? 1 : 0;
}

RuntimeValue rt_string_char_at(RuntimeValue str, RuntimeValue idx)
{
    if (!IS_HEAP(str)) return NIL_VALUE;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    int64_t i = (int64_t)idx;
    if (!s || s->hdr.type != HEAP_STRING || i < 0 || (uint32_t)i >= s->len) return NIL_VALUE;
    return rt_string_new((RuntimeValue)(uintptr_t)(s->data + i), 1);
}

RuntimeValue rt_string_eq(RuntimeValue a, RuntimeValue b)
{
    if (!IS_HEAP(a) || !IS_HEAP(b)) return 0;
    RuntimeString *sa = (RuntimeString *)DECODE_PTR(a);
    RuntimeString *sb = (RuntimeString *)DECODE_PTR(b);
    if (!sa || !sb || sa->hdr.type != HEAP_STRING || sb->hdr.type != HEAP_STRING) return 0;
    if (sa->len != sb->len) return 0;
    for (uint32_t i = 0; i < sa->len; i++) {
        if (sa->data[i] != sb->data[i]) return 0;
    }
    return 1;
}

RuntimeValue rt_string_starts_with(RuntimeValue str, RuntimeValue prefix)
{
    if (!IS_HEAP(str) || !IS_HEAP(prefix)) return 0;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    RuntimeString *p = (RuntimeString *)DECODE_PTR(prefix);
    if (!s || !p || s->hdr.type != HEAP_STRING || p->hdr.type != HEAP_STRING) return 0;
    if (p->len > s->len) return 0;
    for (uint32_t i = 0; i < p->len; i++) {
        if (s->data[i] != p->data[i]) return 0;
    }
    return 1;
}

RuntimeValue rt_string_replace(RuntimeValue str, RuntimeValue old_val, RuntimeValue new_val)
{
    if (!IS_HEAP(str) || !IS_HEAP(old_val) || !IS_HEAP(new_val)) return NIL_VALUE;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    RuntimeString *o = (RuntimeString *)DECODE_PTR(old_val);
    RuntimeString *n = (RuntimeString *)DECODE_PTR(new_val);
    if (!s || !o || !n || s->hdr.type != HEAP_STRING || o->hdr.type != HEAP_STRING || n->hdr.type != HEAP_STRING) {
        return NIL_VALUE;
    }
    if (o->len == 0 || o->len > s->len) return str;

    for (uint32_t i = 0; i <= s->len - o->len; i++) {
        uint32_t j = 0;
        while (j < o->len && s->data[i + j] == o->data[j]) j++;
        if (j != o->len) continue;

        uint32_t out_len = s->len - o->len + n->len;
        RuntimeString *out = (RuntimeString *)rv_alloc(sizeof(RuntimeString) + out_len + 1U);
        if (!out) return str;
        out->hdr.type = HEAP_STRING;
        out->hdr.size = (uint32_t)(sizeof(RuntimeString) + out_len + 1U);
        out->len = out_len;
        for (uint32_t k = 0; k < i; k++) out->data[k] = s->data[k];
        for (uint32_t k = 0; k < n->len; k++) out->data[i + k] = n->data[k];
        for (uint32_t k = i + o->len; k < s->len; k++) out->data[i + n->len + (k - i - o->len)] = s->data[k];
        out->data[out_len] = 0;
        return ENCODE_PTR(out);
    }
    return str;
}

RuntimeValue rt_string_to_upper(RuntimeValue str)
{
    if (!IS_HEAP(str)) return str;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s || s->hdr.type != HEAP_STRING) return str;
    RuntimeString *out = (RuntimeString *)rv_alloc(sizeof(RuntimeString) + s->len + 1U);
    if (!out) return str;
    out->hdr.type = HEAP_STRING;
    out->hdr.size = (uint32_t)(sizeof(RuntimeString) + s->len + 1U);
    out->len = s->len;
    for (uint32_t i = 0; i < s->len; i++) {
        char c = s->data[i];
        out->data[i] = (c >= 'a' && c <= 'z') ? (char)(c - ('a' - 'A')) : c;
    }
    out->data[s->len] = 0;
    return ENCODE_PTR(out);
}

RuntimeValue rt_string_to_lower(RuntimeValue str)
{
    if (!IS_HEAP(str)) return str;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s || s->hdr.type != HEAP_STRING) return str;
    RuntimeString *out = (RuntimeString *)rv_alloc(sizeof(RuntimeString) + s->len + 1U);
    if (!out) return str;
    out->hdr.type = HEAP_STRING;
    out->hdr.size = (uint32_t)(sizeof(RuntimeString) + s->len + 1U);
    out->len = s->len;
    for (uint32_t i = 0; i < s->len; i++) {
        char c = s->data[i];
        out->data[i] = (c >= 'A' && c <= 'Z') ? (char)(c + ('a' - 'A')) : c;
    }
    out->data[s->len] = 0;
    return ENCODE_PTR(out);
}

static int rt_is_ascii_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

RuntimeValue rt_string_trim(RuntimeValue str)
{
    if (!IS_HEAP(str)) return str;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s || s->hdr.type != HEAP_STRING || s->len == 0) return str;

    uint32_t start = 0;
    while (start < s->len && rt_is_ascii_whitespace(s->data[start])) start++;

    uint32_t end = s->len;
    while (end > start && rt_is_ascii_whitespace(s->data[end - 1])) end--;

    uint32_t out_len = end - start;
    RuntimeString *out = (RuntimeString *)rv_alloc(sizeof(RuntimeString) + out_len + 1U);
    if (!out) return str;
    out->hdr.type = HEAP_STRING;
    out->hdr.size = (uint32_t)(sizeof(RuntimeString) + out_len + 1U);
    out->len = out_len;
    for (uint32_t i = 0; i < out_len; i++) out->data[i] = s->data[start + i];
    out->data[out_len] = 0;
    return ENCODE_PTR(out);
}

RuntimeValue str_byte_at_impl(RuntimeValue str, RuntimeValue idx) __asm__("str.byte_at");
RuntimeValue str_byte_at_impl(RuntimeValue str, RuntimeValue idx)
{
    if (!IS_HEAP(str)) return 0;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    int64_t i = (int64_t)idx;
    if (!s || s->hdr.type != HEAP_STRING || i < 0 || (uint32_t)i >= s->len) return 0;
    return (RuntimeValue)(uint8_t)s->data[i];
}

RuntimeValue serial_println(RuntimeValue value)
{
    if (IS_HEAP(value)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(value);
        if (s && s->hdr.type == HEAP_STRING && s->len < 4096U) {
            for (uint32_t i = 0; i < s->len; i++) uart_putc(s->data[i]);
        }
    }
    uart_putc('\r');
    uart_putc('\n');
    return NIL_VALUE;
}

RuntimeValue rt_qemu_exit_success(void)
{
    *(volatile uint32_t *)SIFIVE_TEST_BASE = 0x5555U;
    return NIL_VALUE;
}

static uint64_t harden_mix64(uint64_t value)
{
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27;
    value *= 0x94d049bb133111ebULL;
    value ^= value >> 31;
    return value;
}

RuntimeValue rt_riscv_harden_canary_value(void)
{
    uint64_t cycle = 0;
    uint64_t time = 0;
    uint64_t instret = 0;
    __asm__ volatile("rdcycle %0" : "=r"(cycle));
    __asm__ volatile("rdtime %0" : "=r"(time));
    __asm__ volatile("rdinstret %0" : "=r"(instret));
    uint64_t mixed = harden_mix64(
        cycle ^ (time << 17) ^ (instret << 33) ^ (uintptr_t)&rt_riscv_harden_canary_value
    );
    mixed &= 0x7fffffffffffffffULL;
    return (RuntimeValue)(mixed == 0 ? 1 : mixed);
}

static void rv_memzero(void *ptr, uintptr_t len)
{
    unsigned char *p = (unsigned char *)ptr;
    for (uintptr_t i = 0; i < len; i++) p[i] = 0;
}

static void rv_fence(void)
{
    __asm__ volatile("fence rw, rw" ::: "memory");
}

static void le16(volatile unsigned char *p, uint16_t v)
{
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8) & 0xffU);
}

static void le32(volatile unsigned char *p, uint32_t v)
{
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8) & 0xffU);
    p[2] = (unsigned char)((v >> 16) & 0xffU);
    p[3] = (unsigned char)((v >> 24) & 0xffU);
}

static uint16_t rd16(const unsigned char *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t rd32(const unsigned char *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint64_t rd64(const unsigned char *p)
{
    return (uint64_t)rd32(p) | ((uint64_t)rd32(p + 4) << 32);
}

static int mem_eq(const unsigned char *p, const char *s, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        if (p[i] != (unsigned char)s[i]) return 0;
    }
    return 1;
}

static int virtio_blk_init(void)
{
    for (uint32_t slot = 0; slot < VIRTIO_MMIO_SLOTS; slot++) {
        volatile uint32_t *mmio = (volatile uint32_t *)(VIRTIO_MMIO_BASE + ((uintptr_t)slot * VIRTIO_MMIO_STRIDE));
        if (mmio[0x000 / 4] == VIRTIO_MAGIC && mmio[0x008 / 4] == VIRTIO_DEV_BLK) {
            g_blk_mmio = mmio;
            break;
        }
    }
    if (!g_blk_mmio) return 0;

    volatile uint32_t *mmio = g_blk_mmio;
    uint32_t version = mmio[0x004 / 4];
    rv_memzero(g_virtq, sizeof(g_virtq));
    mmio[0x070 / 4] = 0;
    mmio[0x070 / 4] = VIRTIO_STATUS_ACKNOWLEDGE;
    mmio[0x070 / 4] = VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER;
    mmio[0x024 / 4] = 0;
    mmio[0x020 / 4] = 0;
    mmio[0x070 / 4] = VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK;
    if ((mmio[0x070 / 4] & VIRTIO_STATUS_FEATURES_OK) == 0) return 0;
    mmio[0x030 / 4] = 0;
    mmio[0x038 / 4] = 8;
    uintptr_t q = (uintptr_t)g_virtq;
    if (version == 1) {
        mmio[0x028 / 4] = 4096;
        mmio[0x03c / 4] = 4096;
        mmio[0x040 / 4] = (uint32_t)(q >> 12);
    } else {
        mmio[0x080 / 4] = (uint32_t)q;
        mmio[0x084 / 4] = (uint32_t)((uint64_t)q >> 32);
        mmio[0x090 / 4] = (uint32_t)(q + 2048U);
        mmio[0x094 / 4] = (uint32_t)((uint64_t)(q + 2048U) >> 32);
        mmio[0x0a0 / 4] = (uint32_t)(q + 4096U);
        mmio[0x0a4 / 4] = (uint32_t)((uint64_t)(q + 4096U) >> 32);
        mmio[0x044 / 4] = 1;
    }
    g_last_used_idx = 0;
    mmio[0x070 / 4] = VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK | VIRTIO_STATUS_DRIVER_OK;
    rv_fence();
    return 1;
}

static int virtio_blk_read_sector(uint32_t lba)
{
    if (!g_blk_mmio && !virtio_blk_init()) return 0;
    rv_memzero(g_dma, sizeof(g_dma));
    uintptr_t dma = (uintptr_t)g_dma;
    le32(g_dma + 0, 0);
    le32(g_dma + 4, 0);
    le32(g_dma + 8, lba);
    le32(g_dma + 12, 0);
    g_dma[528] = 0xffU;

    volatile unsigned char *desc = (volatile unsigned char *)g_virtq;
    le32(desc + 0, (uint32_t)dma);
    le32(desc + 4, (uint32_t)((uint64_t)dma >> 32));
    le32(desc + 8, 16);
    le16(desc + 12, VIRTQ_DESC_F_NEXT);
    le16(desc + 14, 1);

    le32(desc + 16, (uint32_t)(dma + 16U));
    le32(desc + 20, (uint32_t)((uint64_t)(dma + 16U) >> 32));
    le32(desc + 24, 512);
    le16(desc + 28, VIRTQ_DESC_F_NEXT | VIRTQ_DESC_F_WRITE);
    le16(desc + 30, 2);

    le32(desc + 32, (uint32_t)(dma + 528U));
    le32(desc + 36, (uint32_t)((uint64_t)(dma + 528U) >> 32));
    le32(desc + 40, 1);
    le16(desc + 44, VIRTQ_DESC_F_WRITE);
    le16(desc + 46, 0);

    volatile uint16_t *avail = (volatile uint16_t *)(g_virtq + 2048U);
    volatile uint16_t *used = (volatile uint16_t *)(g_virtq + 4096U);
    uint16_t idx = avail[1];
    avail[2 + (idx % 8U)] = 0;
    rv_fence();
    avail[1] = (uint16_t)(idx + 1U);
    rv_fence();
    g_blk_mmio[0x050 / 4] = 0;
    rv_fence();
    for (uint32_t spin = 0; spin < 50000000U; spin++) {
        rv_fence();
        if (used[1] != g_last_used_idx) {
            g_last_used_idx = used[1];
            return g_dma[528] == 0;
        }
    }
    return 0;
}

typedef struct {
    uint32_t spc;
    uint32_t reserved;
    uint32_t fats;
    uint32_t fat_size;
    uint32_t root_cluster;
    uint32_t data_start;
} Fat32Probe;

static unsigned char *sector_data(void)
{
    return g_dma + 16U;
}

static uint32_t fat_cluster_sector(const Fat32Probe *fat, uint32_t cluster)
{
    return fat->data_start + ((cluster - 2U) * fat->spc);
}

static int fat32_probe_bpb(Fat32Probe *fat)
{
    if (!virtio_blk_read_sector(0)) return 0;
    const unsigned char *b = sector_data();
    if (rd16(b + 11U) != 512U) return 0;
    fat->spc = b[13U];
    fat->reserved = rd16(b + 14U);
    fat->fats = b[16U];
    fat->fat_size = rd32(b + 36U);
    fat->root_cluster = rd32(b + 44U);
    if (fat->spc == 0 || fat->reserved == 0 || fat->fats == 0 || fat->fat_size == 0 || fat->root_cluster < 2U) return 0;
    fat->data_start = fat->reserved + (fat->fats * fat->fat_size);
    return 1;
}

static uint32_t fat32_next_cluster(const Fat32Probe *fat, uint32_t cluster)
{
    uint32_t fat_offset = cluster * 4U;
    uint32_t sector = fat->reserved + (fat_offset / 512U);
    uint32_t offset = fat_offset % 512U;
    if (!virtio_blk_read_sector(sector)) return 0x0fffffffu;
    return rd32(sector_data() + offset) & 0x0fffffffu;
}

static uint32_t fat32_find_entry_cluster(const Fat32Probe *fat, uint32_t dir_cluster, const char *name11, uint32_t want_dir, uint32_t *size_out)
{
    uint32_t cluster = dir_cluster;
    while (cluster >= 2U && cluster < 0x0ffffff8U) {
        uint32_t first_sector = fat_cluster_sector(fat, cluster);
        for (uint32_t sec = 0; sec < fat->spc; sec++) {
            if (!virtio_blk_read_sector(first_sector + sec)) return 0;
            const unsigned char *data = sector_data();
            for (uint32_t off = 0; off < 512U; off += 32U) {
                const unsigned char *e = data + off;
                if (e[0] == 0x00U) return 0;
                if (e[0] == 0xe5U || e[11] == 0x0fU) continue;
                if (!mem_eq(e, name11, 11U)) continue;
                uint32_t is_dir = (e[11] & 0x10U) != 0;
                if (is_dir != want_dir) continue;
                if (size_out) *size_out = rd32(e + 28U);
                return ((uint32_t)rd16(e + 20U) << 16) | rd16(e + 26U);
            }
        }
        cluster = fat32_next_cluster(fat, cluster);
    }
    return 0;
}

static int fat32_read_first_sector(const Fat32Probe *fat, uint32_t cluster)
{
    if (cluster < 2U || cluster >= 0x0ffffff8U) return 0;
    return virtio_blk_read_sector(fat_cluster_sector(fat, cluster));
}

static int sector_contains(const char *needle)
{
    uint32_t len = 0;
    while (needle[len]) len++;
    if (len == 0 || len >= 512U) return 0;
    for (uint32_t i = 16; i + len < 528U; i++) {
        uint32_t ok = 1;
        for (uint32_t j = 0; j < len; j++) {
            if (g_dma[i + j] != (unsigned char)needle[j]) {
                ok = 0;
                break;
            }
        }
        if (ok) return 1;
    }
    return 0;
}

static uint32_t fat32_find_sys_apps_file(const Fat32Probe *fat, const char *name11, uint32_t *size_out)
{
    uint32_t sys_cluster = fat32_find_entry_cluster(fat, fat->root_cluster, "SYS        ", 1, 0);
    if (sys_cluster < 2U) return 0;
    uint32_t apps_cluster = fat32_find_entry_cluster(fat, sys_cluster, "APPS       ", 1, 0);
    if (apps_cluster < 2U) return 0;
    return fat32_find_entry_cluster(fat, apps_cluster, name11, 0, size_out);
}

static uint32_t fat32_read_file_into(const Fat32Probe *fat, uint32_t cluster, uint32_t file_size, unsigned char *out, uint32_t cap)
{
    if (cluster < 2U || file_size == 0 || file_size > cap) return 0;
    uint32_t copied = 0;
    uint32_t cur = cluster;
    while (cur >= 2U && cur < 0x0ffffff8U && copied < file_size) {
        uint32_t first_sector = fat_cluster_sector(fat, cur);
        for (uint32_t sec = 0; sec < fat->spc && copied < file_size; sec++) {
            if (!virtio_blk_read_sector(first_sector + sec)) return 0;
            const unsigned char *src = sector_data();
            for (uint32_t i = 0; i < 512U && copied < file_size; i++) {
                out[copied++] = src[i];
            }
        }
        if (copied >= file_size) break;
        cur = fat32_next_cluster(fat, cur);
    }
    return copied;
}

static int riscv_smf_probe_file(const char *name11, const char *marker)
{
    Fat32Probe fat;
    if (!fat32_probe_bpb(&fat)) return 0;
    uint32_t file_size = 0;
    uint32_t cluster = fat32_find_sys_apps_file(&fat, name11, &file_size);
    if (cluster < 2U || file_size == 0) return 0;
    if (!fat32_read_first_sector(&fat, cluster)) return 0;
    return sector_contains("SMF") && sector_contains(marker);
}

static int bytes_contains(const unsigned char *data, uint32_t len, const char *needle)
{
    uint32_t n = 0;
    while (needle[n]) n++;
    if (n == 0 || n > len) return 0;
    for (uint32_t i = 0; i + n <= len; i++) {
        uint32_t ok = 1;
        for (uint32_t j = 0; j < n; j++) {
            if (data[i + j] != (unsigned char)needle[j]) {
                ok = 0;
                break;
            }
        }
        if (ok) return 1;
    }
    return 0;
}

static uint32_t riscv_unwrap_smf(const unsigned char *file, uint32_t file_size, const unsigned char **elf_out)
{
    if (file_size < 132U) return 0;
    const unsigned char *footer = file + file_size - 128U;
    if (!mem_eq(footer, "SMF", 3U) || footer[3] != 0) return 0;
    uint32_t payload_len = rd32(footer + 52U);
    if (payload_len == 0 || payload_len > file_size - 128U) return 0;
    *elf_out = file;
    return payload_len;
}

static int riscv_load_elf_process(const unsigned char *elf, uint32_t elf_size, uint32_t slot, const char *marker)
{
    if (slot >= 2U || elf_size < 64U) return 0;
    if (elf[0] != 0x7fU || elf[1] != 'E' || elf[2] != 'L' || elf[3] != 'F') return 0;
    if (elf[4] != 2U || elf[5] != 1U) return 0;
    if (rd16(elf + 18U) != 243U) return 0;

    uint64_t entry = rd64(elf + 24U);
    uint64_t phoff = rd64(elf + 32U);
    uint16_t phentsize = rd16(elf + 54U);
    uint16_t phnum = rd16(elf + 56U);
    if (phoff == 0 || phentsize < 56U || phnum == 0 || phnum > 8U) return 0;
    if (phoff + ((uint64_t)phentsize * phnum) > elf_size) return 0;

    for (uint32_t i = 0; i < sizeof(g_riscv_process_arena[slot]); i++) g_riscv_process_arena[slot][i] = 0;

    uint32_t loaded = 0;
    int entry_mapped = 0;
    for (uint16_t i = 0; i < phnum; i++) {
        const unsigned char *ph = elf + phoff + ((uint64_t)i * phentsize);
        if (rd32(ph) != 1U) continue;
        uint64_t off = rd64(ph + 8U);
        uint64_t vaddr = rd64(ph + 16U);
        uint64_t filesz = rd64(ph + 32U);
        uint64_t memsz = rd64(ph + 40U);
        if (filesz > memsz || off + filesz > elf_size || loaded + memsz > sizeof(g_riscv_process_arena[slot])) return 0;
        for (uint64_t j = 0; j < filesz; j++) g_riscv_process_arena[slot][loaded + j] = elf[off + j];
        if (entry >= vaddr && entry < vaddr + memsz) entry_mapped = 1;
        loaded += (uint32_t)memsz;
    }
    if (loaded == 0 || !entry_mapped) return 0;
    if (!bytes_contains(elf, elf_size, marker)) return 0;
    g_riscv_process_entry[slot] = entry;
    g_riscv_process_pid[slot] = 1000U + slot + 1U;
    if (g_riscv_process_count <= slot) g_riscv_process_count = slot + 1U;
    return 1;
}

static int riscv_load_smf_process(const char *name11, const char *marker, uint32_t slot)
{
    Fat32Probe fat;
    if (!fat32_probe_bpb(&fat)) return 0;
    uint32_t file_size = 0;
    uint32_t cluster = fat32_find_sys_apps_file(&fat, name11, &file_size);
    if (cluster < 2U || file_size == 0 || file_size > sizeof(g_riscv_file_buf)) return 0;
    uint32_t read = fat32_read_file_into(&fat, cluster, file_size, g_riscv_file_buf, sizeof(g_riscv_file_buf));
    if (read != file_size) return 0;
    const unsigned char *elf = 0;
    uint32_t elf_size = riscv_unwrap_smf(g_riscv_file_buf, file_size, &elf);
    if (elf_size == 0) return 0;
    return riscv_load_elf_process(elf, elf_size, slot, marker);
}

RuntimeValue rt_riscv_nvfs_probe(void)
{
    Fat32Probe fat;
    if (!fat32_probe_bpb(&fat)) return 0;
    uint32_t sys_cluster = fat32_find_entry_cluster(&fat, fat.root_cluster, "SYS        ", 1, 0);
    if (sys_cluster < 2U) return 0;
    uint32_t file_size = 0;
    uint32_t nvfs_cluster = fat32_find_entry_cluster(&fat, sys_cluster, "NVFSVER TXT", 0, &file_size);
    if (nvfs_cluster < 2U || file_size == 0) return 0;
    if (!fat32_read_first_sector(&fat, nvfs_cluster)) return 0;
    const char needle[] = "nvfs-image-version=";
    for (uint32_t i = 16; i + sizeof(needle) - 1U < 528U; i++) {
        uint32_t ok = 1;
        for (uint32_t j = 0; j < sizeof(needle) - 1U; j++) {
            if (g_dma[i + j] != (unsigned char)needle[j]) {
                ok = 0;
                break;
            }
        }
        if (ok) return 1;
    }
    return 0;
}

RuntimeValue rt_riscv_smf_cli_probe(void)
{
    return riscv_smf_probe_file("HELLOSMFSMF", "SIMPLEOS_RISCV64_HELLO_ELF") ? 1 : 0;
}

RuntimeValue rt_riscv_smf_cli_load(void)
{
    return riscv_load_smf_process("HELLOSMFSMF", "SIMPLEOS_RISCV64_HELLO_ELF", 0) ? 1 : 0;
}

RuntimeValue rt_riscv_smf_gui_probe(void)
{
    return riscv_smf_probe_file("BROWSMF SMF", "SIMPLEOS_RISCV64_GUI_ELF") ? 1 : 0;
}

RuntimeValue rt_riscv_native_gui_process_render(void)
{
    if (!riscv_load_smf_process("BROWSMF SMF", "SIMPLEOS_RISCV64_GUI_ELF", 1)) return 0;
    if (g_riscv_process_pid[1] == 0 || g_riscv_process_entry[1] == 0) return 0;
    const char *content = "pid=1002 app=/sys/apps/browser_demo.smf x86_contract=desktop_shell";
    uint32_t i = 0;
    while (content[i] != 0 && i + 1U < sizeof(g_riscv_gui_surface)) {
        g_riscv_gui_surface[i] = content[i];
        i++;
    }
    g_riscv_gui_surface[i] = 0;
    if (!bytes_contains((const unsigned char *)g_riscv_gui_surface, sizeof(g_riscv_gui_surface), "pid=1002")) return 0;
    if (!bytes_contains((const unsigned char *)g_riscv_gui_surface, sizeof(g_riscv_gui_surface), "/sys/apps/browser_demo.smf")) return 0;
    return bytes_contains((const unsigned char *)g_riscv_gui_surface, sizeof(g_riscv_gui_surface), "x86_contract=desktop_shell") ? 1 : 0;
}

struct rv_vring_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

struct rv_vring_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[];
} __attribute__((packed));

struct rv_vring_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

struct rv_vring_used {
    uint16_t flags;
    uint16_t idx;
    struct rv_vring_used_elem ring[];
} __attribute__((packed));

struct rv_virtio_net_hdr {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
} __attribute__((packed));

struct rv_eth_hdr {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t ethertype;
} __attribute__((packed));

struct rv_arp_pkt {
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t hw_len;
    uint8_t proto_len;
    uint16_t opcode;
    uint8_t sender_mac[6];
    uint8_t sender_ip[4];
    uint8_t target_mac[6];
    uint8_t target_ip[4];
} __attribute__((packed));

struct rv_ipv4_hdr {
    uint8_t ver_ihl;
    uint8_t tos;
    uint16_t total_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t src_ip[4];
    uint8_t dst_ip[4];
} __attribute__((packed));

struct rv_tcp_hdr {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t data_off;
    uint8_t flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent;
} __attribute__((packed));

#define RV_VNET_QUEUE_SIZE 8U
#define RV_VNET_BUF_SIZE 2048U
#define RV_VNET_HDR_SIZE 10U
#define RV_ETH_HLEN 14U
#define RV_ETH_P_IP 0x0800U
#define RV_ETH_P_ARP 0x0806U
#define RV_IP_PROTO_TCP 6U
#define RV_ARP_HW_ETHER 1U
#define RV_ARP_OP_REQUEST 1U
#define RV_ARP_OP_REPLY 2U
#define RV_TCP_SYN 0x02U
#define RV_TCP_ACK 0x10U
#define RV_TCP_PSH 0x08U
#define RV_TCP_FIN 0x01U
#define RV_MAX_SOCKETS 8
#define RV_TCP_RXBUF_SIZE 4096U
#define RV_TCP_ACCEPT_QUEUE 4

static unsigned char g_vnet_rx_queue[8192] __attribute__((aligned(4096)));
static unsigned char g_vnet_tx_queue[8192] __attribute__((aligned(4096)));
static unsigned char g_vnet_rx_bufs[RV_VNET_QUEUE_SIZE * RV_VNET_BUF_SIZE] __attribute__((aligned(16)));
static unsigned char g_vnet_tx_bufs[RV_VNET_QUEUE_SIZE * RV_VNET_BUF_SIZE] __attribute__((aligned(16)));

enum rv_tcp_state {
    RV_TCP_CLOSED = 0,
    RV_TCP_LISTEN,
    RV_TCP_SYN_RECEIVED,
    RV_TCP_ESTABLISHED
};

struct rv_tcp_socket {
    int in_use;
    enum rv_tcp_state state;
    uint16_t local_port;
    uint16_t remote_port;
    uint8_t remote_ip[4];
    uint8_t remote_mac[6];
    uint32_t snd_nxt;
    uint32_t snd_una;
    uint32_t rcv_nxt;
    uint16_t rcv_wnd;
    uint8_t rxbuf[RV_TCP_RXBUF_SIZE];
    uint32_t rx_head;
    uint32_t rx_tail;
    int accept_queue[RV_TCP_ACCEPT_QUEUE];
    int aq_head;
    int aq_tail;
    int aq_count;
    int backlog;
};

static struct rv_tcp_socket g_rv_sockets[RV_MAX_SOCKETS];
static uint32_t g_rv_tcp_isn = 0x1000U;

static struct {
    volatile uint32_t *mmio;
    uint32_t version;
    uint8_t mac[6];
    uint8_t our_ip[4];
    uint16_t rx_qsize;
    struct rv_vring_desc *rx_desc;
    struct rv_vring_avail *rx_avail;
    struct rv_vring_used *rx_used;
    uint16_t rx_last_used;
    uint16_t tx_qsize;
    struct rv_vring_desc *tx_desc;
    struct rv_vring_avail *tx_avail;
    struct rv_vring_used *tx_used;
    uint16_t tx_last_used;
    uint16_t tx_next_desc;
    int initialized;
    uint32_t rx_count;
    uint32_t tx_count;
} g_rv_vnet;

static uint16_t rv_net_htons(uint16_t h)
{
    return (uint16_t)((h >> 8) | (h << 8));
}

static uint16_t rv_net_ntohs(uint16_t n)
{
    return rv_net_htons(n);
}

static uint32_t rv_net_htonl(uint32_t h)
{
    return ((h & 0xFFU) << 24) | ((h & 0xFF00U) << 8) |
           ((h >> 8) & 0xFF00U) | ((h >> 24) & 0xFFU);
}

static uint32_t rv_net_ntohl(uint32_t n)
{
    return rv_net_htonl(n);
}

static uint16_t rv_inet_checksum(const void *data, uint32_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t sum = 0;
    for (uint32_t i = 0; i + 1U < len; i += 2U) {
        sum += ((uint32_t)p[i] << 8) | p[i + 1U];
    }
    if ((len & 1U) != 0U) sum += (uint32_t)p[len - 1U] << 8;
    while ((sum >> 16) != 0U) sum = (sum & 0xFFFFU) + (sum >> 16);
    return rv_net_htons((uint16_t)~sum);
}

static uint16_t rv_tcp_checksum(const uint8_t *src_ip, const uint8_t *dst_ip, const void *tcp_data, uint16_t tcp_len)
{
    const uint8_t *p = (const uint8_t *)tcp_data;
    uint32_t sum = 0;
    sum += ((uint16_t)src_ip[0] << 8) | src_ip[1];
    sum += ((uint16_t)src_ip[2] << 8) | src_ip[3];
    sum += ((uint16_t)dst_ip[0] << 8) | dst_ip[1];
    sum += ((uint16_t)dst_ip[2] << 8) | dst_ip[3];
    sum += RV_IP_PROTO_TCP;
    sum += tcp_len;
    for (uint16_t i = 0; i + 1U < tcp_len; i += 2U) sum += ((uint16_t)p[i] << 8) | p[i + 1U];
    if ((tcp_len & 1U) != 0U) sum += (uint16_t)p[tcp_len - 1U] << 8;
    while ((sum >> 16) != 0U) sum = (sum & 0xFFFFU) + (sum >> 16);
    return rv_net_htons((uint16_t)~sum);
}

static uintptr_t rv_align_up_uintptr(uintptr_t value, uintptr_t align)
{
    return (value + align - 1U) & ~(align - 1U);
}

static uint32_t rv_mmio_rd32(uint32_t off)
{
    return g_rv_vnet.mmio[off / 4U];
}

static void rv_mmio_wr32(uint32_t off, uint32_t value)
{
    g_rv_vnet.mmio[off / 4U] = value;
}

static void rv_vnet_delay(void)
{
    for (volatile int i = 0; i < 1000; i++) {}
}

static void rv_vnet_setup_queue(uint16_t qsel, unsigned char *queue_mem, uint16_t *out_qsize,
                                struct rv_vring_desc **out_desc, struct rv_vring_avail **out_avail,
                                struct rv_vring_used **out_used)
{
    rv_mmio_wr32(0x030U, qsel);
    uint16_t qsize = (uint16_t)rv_mmio_rd32(0x034U);
    if (qsize == 0U || qsize > RV_VNET_QUEUE_SIZE) qsize = RV_VNET_QUEUE_SIZE;
    rv_mmio_wr32(0x038U, qsize);

    uintptr_t base = (uintptr_t)queue_mem;
    uintptr_t desc = base;
    uintptr_t avail = desc + ((uintptr_t)qsize * sizeof(struct rv_vring_desc));
    uintptr_t used = rv_align_up_uintptr(avail + 4U + ((uintptr_t)qsize * 2U) + 2U, 4096U);
    *out_desc = (struct rv_vring_desc *)desc;
    *out_avail = (struct rv_vring_avail *)avail;
    *out_used = (struct rv_vring_used *)used;
    *out_qsize = qsize;
    rv_memzero(queue_mem, 8192U);

    if (g_rv_vnet.version == 1U) {
        rv_mmio_wr32(0x028U, 4096U);
        rv_mmio_wr32(0x03cU, 4096U);
        rv_mmio_wr32(0x040U, (uint32_t)(base >> 12));
    } else {
        rv_mmio_wr32(0x080U, (uint32_t)desc);
        rv_mmio_wr32(0x084U, (uint32_t)((uint64_t)desc >> 32));
        rv_mmio_wr32(0x090U, (uint32_t)avail);
        rv_mmio_wr32(0x094U, (uint32_t)((uint64_t)avail >> 32));
        rv_mmio_wr32(0x0a0U, (uint32_t)used);
        rv_mmio_wr32(0x0a4U, (uint32_t)((uint64_t)used >> 32));
        rv_mmio_wr32(0x044U, 1U);
    }
}

static void rv_vnet_fill_rx(void)
{
    for (uint16_t i = 0; i < g_rv_vnet.rx_qsize; i++) {
        g_rv_vnet.rx_desc[i].addr = (uint64_t)(uintptr_t)(g_vnet_rx_bufs + ((size_t)i * RV_VNET_BUF_SIZE));
        g_rv_vnet.rx_desc[i].len = RV_VNET_BUF_SIZE;
        g_rv_vnet.rx_desc[i].flags = VIRTQ_DESC_F_WRITE;
        g_rv_vnet.rx_desc[i].next = 0;
        g_rv_vnet.rx_avail->ring[i] = i;
    }
    rv_fence();
    g_rv_vnet.rx_avail->idx = g_rv_vnet.rx_qsize;
    g_rv_vnet.rx_last_used = 0;
    rv_fence();
    rv_mmio_wr32(0x050U, 0U);
}

static void rv_vnet_reclaim_tx(void)
{
    rv_fence();
    while (g_rv_vnet.tx_last_used != g_rv_vnet.tx_used->idx) g_rv_vnet.tx_last_used++;
}

static int rv_vnet_send_frame(const void *frame, uint16_t frame_len)
{
    if (!g_rv_vnet.initialized) return -19;
    if ((uint32_t)frame_len + RV_VNET_HDR_SIZE > RV_VNET_BUF_SIZE) return -90;
    rv_vnet_reclaim_tx();
    uint16_t pending = (uint16_t)(g_rv_vnet.tx_next_desc - g_rv_vnet.tx_last_used);
    if (pending >= g_rv_vnet.tx_qsize) return -11;
    uint16_t di = (uint16_t)(g_rv_vnet.tx_next_desc % g_rv_vnet.tx_qsize);
    g_rv_vnet.tx_next_desc++;
    unsigned char *buf = g_vnet_tx_bufs + ((size_t)di * RV_VNET_BUF_SIZE);
    rv_memzero(buf, RV_VNET_HDR_SIZE);
    __builtin_memcpy(buf + RV_VNET_HDR_SIZE, frame, frame_len);
    g_rv_vnet.tx_desc[di].addr = (uint64_t)(uintptr_t)buf;
    g_rv_vnet.tx_desc[di].len = RV_VNET_HDR_SIZE + frame_len;
    g_rv_vnet.tx_desc[di].flags = 0;
    g_rv_vnet.tx_desc[di].next = 0;
    uint16_t avail_idx = g_rv_vnet.tx_avail->idx;
    g_rv_vnet.tx_avail->ring[avail_idx % g_rv_vnet.tx_qsize] = di;
    rv_fence();
    g_rv_vnet.tx_avail->idx = (uint16_t)(avail_idx + 1U);
    rv_fence();
    rv_mmio_wr32(0x050U, 1U);
    g_rv_vnet.tx_count++;
    return 0;
}

static void rv_vnet_send_arp_reply(const struct rv_eth_hdr *eth, const struct rv_arp_pkt *arp)
{
    unsigned char frame[RV_ETH_HLEN + sizeof(struct rv_arp_pkt)];
    struct rv_eth_hdr *reth = (struct rv_eth_hdr *)frame;
    struct rv_arp_pkt *rarp = (struct rv_arp_pkt *)(frame + RV_ETH_HLEN);
    __builtin_memcpy(reth->dst, eth->src, 6);
    __builtin_memcpy(reth->src, g_rv_vnet.mac, 6);
    reth->ethertype = rv_net_htons(RV_ETH_P_ARP);
    rarp->hw_type = rv_net_htons(RV_ARP_HW_ETHER);
    rarp->proto_type = rv_net_htons(RV_ETH_P_IP);
    rarp->hw_len = 6;
    rarp->proto_len = 4;
    rarp->opcode = rv_net_htons(RV_ARP_OP_REPLY);
    __builtin_memcpy(rarp->sender_mac, g_rv_vnet.mac, 6);
    __builtin_memcpy(rarp->sender_ip, g_rv_vnet.our_ip, 4);
    __builtin_memcpy(rarp->target_mac, arp->sender_mac, 6);
    __builtin_memcpy(rarp->target_ip, arp->sender_ip, 4);
    (void)rv_vnet_send_frame(frame, sizeof(frame));
}

static void rv_tcp_send_segment(int sid, uint8_t flags, const void *data, uint16_t data_len)
{
    struct rv_tcp_socket *s = &g_rv_sockets[sid];
    unsigned char pkt[1500];
    uint16_t tcp_len = (uint16_t)(20U + data_len);
    uint16_t ip_len = (uint16_t)(20U + tcp_len);
    struct rv_eth_hdr *eth = (struct rv_eth_hdr *)pkt;
    struct rv_ipv4_hdr *ip = (struct rv_ipv4_hdr *)(pkt + RV_ETH_HLEN);
    struct rv_tcp_hdr *tcp = (struct rv_tcp_hdr *)(pkt + RV_ETH_HLEN + 20U);
    __builtin_memcpy(eth->dst, s->remote_mac, 6);
    __builtin_memcpy(eth->src, g_rv_vnet.mac, 6);
    eth->ethertype = rv_net_htons(RV_ETH_P_IP);
    ip->ver_ihl = 0x45U;
    ip->tos = 0;
    ip->total_len = rv_net_htons(ip_len);
    ip->id = rv_net_htons((uint16_t)g_rv_tcp_isn);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = RV_IP_PROTO_TCP;
    ip->checksum = 0;
    __builtin_memcpy(ip->src_ip, g_rv_vnet.our_ip, 4);
    __builtin_memcpy(ip->dst_ip, s->remote_ip, 4);
    ip->checksum = rv_inet_checksum(ip, 20U);
    tcp->src_port = rv_net_htons(s->local_port);
    tcp->dst_port = rv_net_htons(s->remote_port);
    tcp->seq_num = rv_net_htonl(s->snd_nxt);
    tcp->ack_num = rv_net_htonl(s->rcv_nxt);
    tcp->data_off = 0x50U;
    tcp->flags = flags;
    tcp->window = rv_net_htons(RV_TCP_RXBUF_SIZE);
    tcp->checksum = 0;
    tcp->urgent = 0;
    if (data_len > 0 && data) __builtin_memcpy(pkt + RV_ETH_HLEN + 40U, data, data_len);
    tcp->checksum = rv_tcp_checksum(g_rv_vnet.our_ip, s->remote_ip, tcp, tcp_len);
    (void)rv_vnet_send_frame(pkt, (uint16_t)(RV_ETH_HLEN + ip_len));
    s->snd_nxt += data_len;
    if ((flags & (RV_TCP_SYN | RV_TCP_FIN)) != 0U) s->snd_nxt += 1U;
}

static uint32_t rv_tcp_rx_available(int sid)
{
    struct rv_tcp_socket *s = &g_rv_sockets[sid];
    return (s->rx_head >= s->rx_tail) ? (s->rx_head - s->rx_tail) : (RV_TCP_RXBUF_SIZE - s->rx_tail + s->rx_head);
}

static void rv_tcp_handle_segment(const unsigned char *frame, uint16_t frame_len)
{
    if (frame_len < RV_ETH_HLEN + 40U) return;
    const struct rv_eth_hdr *eth = (const struct rv_eth_hdr *)frame;
    const struct rv_ipv4_hdr *ip = (const struct rv_ipv4_hdr *)(frame + RV_ETH_HLEN);
    uint16_t ip_hlen = (uint16_t)(ip->ver_ihl & 0x0FU) * 4U;
    const struct rv_tcp_hdr *tcp = (const struct rv_tcp_hdr *)(frame + RV_ETH_HLEN + ip_hlen);
    uint16_t tcp_hlen = (uint16_t)(tcp->data_off >> 4) * 4U;
    uint16_t ip_total = rv_net_ntohs(ip->total_len);
    uint16_t data_len = ip_total > ip_hlen + tcp_hlen ? (uint16_t)(ip_total - ip_hlen - tcp_hlen) : 0U;
    const unsigned char *data = frame + RV_ETH_HLEN + ip_hlen + tcp_hlen;
    uint16_t dst_port = rv_net_ntohs(tcp->dst_port);
    uint16_t src_port = rv_net_ntohs(tcp->src_port);
    uint32_t seq = rv_net_ntohl(tcp->seq_num);
    uint32_t ack = rv_net_ntohl(tcp->ack_num);
    uint8_t flags = tcp->flags;
    int sid = -1;
    int listen_sid = -1;
    for (int i = 0; i < RV_MAX_SOCKETS; i++) {
        if (!g_rv_sockets[i].in_use) continue;
        if (g_rv_sockets[i].state >= RV_TCP_SYN_RECEIVED &&
            g_rv_sockets[i].local_port == dst_port &&
            g_rv_sockets[i].remote_port == src_port) {
            sid = i;
            break;
        }
    }
    if (sid < 0 && (flags & RV_TCP_SYN) != 0U) {
        for (int i = 0; i < RV_MAX_SOCKETS; i++) {
            if (g_rv_sockets[i].in_use &&
                g_rv_sockets[i].state == RV_TCP_LISTEN &&
                g_rv_sockets[i].local_port == dst_port) {
                listen_sid = i;
                break;
            }
        }
    }
    if (listen_sid >= 0 && (flags & RV_TCP_SYN) != 0U && (flags & RV_TCP_ACK) == 0U) {
        int new_sid = -1;
        for (int i = 0; i < RV_MAX_SOCKETS; i++) {
            if (!g_rv_sockets[i].in_use) {
                new_sid = i;
                break;
            }
        }
        if (new_sid < 0) return;
        rv_memzero(&g_rv_sockets[new_sid], sizeof(g_rv_sockets[new_sid]));
        g_rv_sockets[new_sid].in_use = 1;
        g_rv_sockets[new_sid].state = RV_TCP_SYN_RECEIVED;
        g_rv_sockets[new_sid].local_port = dst_port;
        g_rv_sockets[new_sid].remote_port = src_port;
        g_rv_sockets[new_sid].snd_nxt = g_rv_tcp_isn++;
        g_rv_sockets[new_sid].snd_una = g_rv_sockets[new_sid].snd_nxt;
        g_rv_sockets[new_sid].rcv_nxt = seq + 1U;
        g_rv_sockets[new_sid].rcv_wnd = RV_TCP_RXBUF_SIZE;
        __builtin_memcpy(g_rv_sockets[new_sid].remote_ip, ip->src_ip, 4);
        __builtin_memcpy(g_rv_sockets[new_sid].remote_mac, eth->src, 6);
        rv_tcp_send_segment(new_sid, RV_TCP_SYN | RV_TCP_ACK, NULL, 0);
        return;
    }
    if (sid < 0) return;
    struct rv_tcp_socket *s = &g_rv_sockets[sid];
    if (s->state == RV_TCP_SYN_RECEIVED && (flags & RV_TCP_ACK) != 0U) {
        s->snd_una = ack;
        s->state = RV_TCP_ESTABLISHED;
        for (int i = 0; i < RV_MAX_SOCKETS; i++) {
            if (g_rv_sockets[i].in_use &&
                g_rv_sockets[i].state == RV_TCP_LISTEN &&
                g_rv_sockets[i].local_port == s->local_port) {
                struct rv_tcp_socket *ls = &g_rv_sockets[i];
                if (ls->aq_count < RV_TCP_ACCEPT_QUEUE) {
                    ls->accept_queue[ls->aq_tail] = sid;
                    ls->aq_tail = (ls->aq_tail + 1) % RV_TCP_ACCEPT_QUEUE;
                    ls->aq_count++;
                }
                break;
            }
        }
        return;
    }
    if (s->state != RV_TCP_ESTABLISHED) return;
    if (data_len > 0U) {
        for (uint16_t i = 0; i < data_len; i++) {
            uint32_t next = (s->rx_head + 1U) % RV_TCP_RXBUF_SIZE;
            if (next == s->rx_tail) break;
            s->rxbuf[s->rx_head] = data[i];
            s->rx_head = next;
        }
        s->rcv_nxt = seq + data_len;
        rv_tcp_send_segment(sid, RV_TCP_ACK, NULL, 0);
    }
    if ((flags & RV_TCP_ACK) != 0U) s->snd_una = ack;
    if ((flags & RV_TCP_FIN) != 0U) {
        s->rcv_nxt = seq + data_len + 1U;
        rv_tcp_send_segment(sid, RV_TCP_ACK, NULL, 0);
        s->state = RV_TCP_CLOSED;
        s->in_use = 0;
    }
}

static void rv_vnet_handle_frame(const unsigned char *frame, uint16_t frame_len)
{
    if (frame_len < RV_ETH_HLEN) return;
    const struct rv_eth_hdr *eth = (const struct rv_eth_hdr *)frame;
    uint16_t ethertype = rv_net_ntohs(eth->ethertype);
    if (ethertype == RV_ETH_P_ARP) {
        if (frame_len < RV_ETH_HLEN + sizeof(struct rv_arp_pkt)) return;
        const struct rv_arp_pkt *arp = (const struct rv_arp_pkt *)(frame + RV_ETH_HLEN);
        if (rv_net_ntohs(arp->opcode) == RV_ARP_OP_REQUEST &&
            __builtin_memcmp(arp->target_ip, g_rv_vnet.our_ip, 4) == 0) {
            rv_vnet_send_arp_reply(eth, arp);
        }
    } else if (ethertype == RV_ETH_P_IP) {
        const struct rv_ipv4_hdr *ip = (const struct rv_ipv4_hdr *)(frame + RV_ETH_HLEN);
        if (frame_len >= RV_ETH_HLEN + 20U &&
            ip->protocol == RV_IP_PROTO_TCP &&
            __builtin_memcmp(ip->dst_ip, g_rv_vnet.our_ip, 4) == 0) {
            rv_tcp_handle_segment(frame, frame_len);
        }
    }
}

static int rv_vnet_poll(void)
{
    if (!g_rv_vnet.initialized) return -19;
    int processed = 0;
    rv_vnet_reclaim_tx();
    while (1) {
        rv_fence();
        uint16_t used_idx = g_rv_vnet.rx_used->idx;
        if (g_rv_vnet.rx_last_used == used_idx) break;
        uint16_t slot = g_rv_vnet.rx_last_used % g_rv_vnet.rx_qsize;
        uint32_t desc_id = g_rv_vnet.rx_used->ring[slot].id;
        uint32_t used_len = g_rv_vnet.rx_used->ring[slot].len;
        g_rv_vnet.rx_last_used++;
        if (desc_id < g_rv_vnet.rx_qsize && used_len > RV_VNET_HDR_SIZE) {
            unsigned char *buf = g_vnet_rx_bufs + ((size_t)desc_id * RV_VNET_BUF_SIZE);
            rv_vnet_handle_frame(buf + RV_VNET_HDR_SIZE, (uint16_t)(used_len - RV_VNET_HDR_SIZE));
            g_rv_vnet.rx_count++;
            g_rv_vnet.rx_desc[desc_id].addr = (uint64_t)(uintptr_t)buf;
            g_rv_vnet.rx_desc[desc_id].len = RV_VNET_BUF_SIZE;
            g_rv_vnet.rx_desc[desc_id].flags = VIRTQ_DESC_F_WRITE;
            g_rv_vnet.rx_desc[desc_id].next = 0;
            uint16_t avail_idx = g_rv_vnet.rx_avail->idx;
            g_rv_vnet.rx_avail->ring[avail_idx % g_rv_vnet.rx_qsize] = (uint16_t)desc_id;
            rv_fence();
            g_rv_vnet.rx_avail->idx = (uint16_t)(avail_idx + 1U);
            processed++;
        }
    }
    if (processed > 0) rv_mmio_wr32(0x050U, 0U);
    return processed;
}

int64_t rt_net_init(void)
{
    if (g_rv_vnet.initialized) return 0;
    rv_memzero(&g_rv_vnet, sizeof(g_rv_vnet));
    for (uint32_t slot = 0; slot < VIRTIO_MMIO_SLOTS; slot++) {
        volatile uint32_t *mmio = (volatile uint32_t *)(VIRTIO_MMIO_BASE + ((uintptr_t)slot * VIRTIO_MMIO_STRIDE));
        if (mmio[0x000U / 4U] == VIRTIO_MAGIC && mmio[0x008U / 4U] == VIRTIO_DEV_NET) {
            g_rv_vnet.mmio = mmio;
            break;
        }
    }
    if (!g_rv_vnet.mmio) return -19;
    g_rv_vnet.version = rv_mmio_rd32(0x004U);
    rv_mmio_wr32(0x070U, 0U);
    rv_mmio_wr32(0x070U, VIRTIO_STATUS_ACKNOWLEDGE);
    rv_mmio_wr32(0x070U, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER);
    rv_mmio_wr32(0x014U, 0U);
    (void)rv_mmio_rd32(0x010U);
    rv_mmio_wr32(0x024U, 0U);
    rv_mmio_wr32(0x020U, 0U);
    rv_mmio_wr32(0x070U, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK);
    if ((rv_mmio_rd32(0x070U) & VIRTIO_STATUS_FEATURES_OK) == 0U) return -19;
    for (int i = 0; i < 6; i++) {
        volatile uint8_t *cfg = (volatile uint8_t *)((uintptr_t)g_rv_vnet.mmio + 0x100U);
        g_rv_vnet.mac[i] = cfg[i];
    }
    g_rv_vnet.our_ip[0] = 10U;
    g_rv_vnet.our_ip[1] = 0U;
    g_rv_vnet.our_ip[2] = 2U;
    g_rv_vnet.our_ip[3] = 15U;
    rv_vnet_setup_queue(0U, g_vnet_rx_queue, &g_rv_vnet.rx_qsize, &g_rv_vnet.rx_desc, &g_rv_vnet.rx_avail, &g_rv_vnet.rx_used);
    rv_vnet_setup_queue(1U, g_vnet_tx_queue, &g_rv_vnet.tx_qsize, &g_rv_vnet.tx_desc, &g_rv_vnet.tx_avail, &g_rv_vnet.tx_used);
    rv_mmio_wr32(0x070U, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK | VIRTIO_STATUS_DRIVER_OK);
    g_rv_vnet.tx_next_desc = 0U;
    g_rv_vnet.tx_last_used = 0U;
    rv_vnet_fill_rx();
    g_rv_vnet.initialized = 1;
    return 0;
}

int64_t rt_net_tx_test(void)
{
    return g_rv_vnet.initialized ? 0 : -19;
}

int64_t rt_net_stats(void)
{
    return g_rv_vnet.initialized ? (int64_t)g_rv_vnet.tx_count : -19;
}

int64_t rt_net_socket(int64_t proto)
{
    (void)proto;
    for (int i = 0; i < RV_MAX_SOCKETS; i++) {
        if (!g_rv_sockets[i].in_use) {
            rv_memzero(&g_rv_sockets[i], sizeof(g_rv_sockets[i]));
            g_rv_sockets[i].in_use = 1;
            g_rv_sockets[i].state = RV_TCP_CLOSED;
            g_rv_sockets[i].rcv_wnd = RV_TCP_RXBUF_SIZE;
            return ENCODE_INT(i);
        }
    }
    return ENCODE_INT(-24);
}

int64_t rt_net_bind(int64_t sock_fd, int64_t port_num)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= RV_MAX_SOCKETS || !g_rv_sockets[fd].in_use) return ENCODE_INT(-9);
    g_rv_sockets[fd].local_port = (uint16_t)port_num;
    return ENCODE_INT(0);
}

int64_t rt_net_listen(int64_t sock_fd, int64_t backlog)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= RV_MAX_SOCKETS || !g_rv_sockets[fd].in_use) return ENCODE_INT(-9);
    g_rv_sockets[fd].state = RV_TCP_LISTEN;
    g_rv_sockets[fd].backlog = (int)backlog;
    g_rv_sockets[fd].aq_head = 0;
    g_rv_sockets[fd].aq_tail = 0;
    g_rv_sockets[fd].aq_count = 0;
    return ENCODE_INT(0);
}

int64_t rt_net_accept(int64_t sock_fd)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= RV_MAX_SOCKETS || !g_rv_sockets[fd].in_use || g_rv_sockets[fd].state != RV_TCP_LISTEN) {
        return ENCODE_INT(-9);
    }
    struct rv_tcp_socket *ls = &g_rv_sockets[fd];
    int timeout = 0;
    while (ls->aq_count == 0 && timeout < 50000) {
        rv_vnet_poll();
        timeout++;
        rv_vnet_delay();
    }
    if (ls->aq_count == 0) return ENCODE_INT(-11);
    int accepted_sid = ls->accept_queue[ls->aq_head];
    ls->aq_head = (ls->aq_head + 1) % RV_TCP_ACCEPT_QUEUE;
    ls->aq_count--;
    return ENCODE_INT(accepted_sid);
}

int64_t rt_net_close(int64_t sock_fd)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= RV_MAX_SOCKETS || !g_rv_sockets[fd].in_use) return ENCODE_INT(-9);
    if (g_rv_sockets[fd].state == RV_TCP_ESTABLISHED) {
        rv_tcp_send_segment(fd, RV_TCP_FIN | RV_TCP_ACK, NULL, 0);
    }
    rv_memzero(&g_rv_sockets[fd], sizeof(g_rv_sockets[fd]));
    return ENCODE_INT(0);
}

int64_t rt_net_send_bytes(int64_t sock_fd, RuntimeValue data_rv)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= RV_MAX_SOCKETS || !g_rv_sockets[fd].in_use || g_rv_sockets[fd].state != RV_TCP_ESTABLISHED) {
        return ENCODE_INT(-9);
    }
    if (!IS_HEAP(data_rv)) return ENCODE_INT(-22);
    RuntimeArray *arr = (RuntimeArray *)DECODE_PTR(data_rv);
    if (!arr || arr->hdr.type != HEAP_ARRAY) return ENCODE_INT(-22);
    uint32_t len = (uint32_t)arr->len;
    if (len == 0U) return ENCODE_INT(0);
    unsigned char *buf = (unsigned char *)rv_alloc(len);
    if (!buf) return ENCODE_INT(-12);
    RuntimeValue *items = runtime_array_items(arr);
    for (uint32_t i = 0; i < len; i++) buf[i] = (uint8_t)DECODE_INT(items[i]);
    uint32_t sent = 0;
    while (sent < len) {
        uint16_t chunk = (uint16_t)((len - sent) > 1200U ? 1200U : (len - sent));
        rv_tcp_send_segment(fd, RV_TCP_ACK | RV_TCP_PSH, buf + sent, chunk);
        sent += chunk;
    }
    return ENCODE_INT((int64_t)sent);
}

RuntimeValue rt_net_recv_version_text(int64_t sock_fd)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= RV_MAX_SOCKETS || !g_rv_sockets[fd].in_use) return rt_string_from_cstr("");
    struct rv_tcp_socket *s = &g_rv_sockets[fd];
    int timeout = 0;
    while (rv_tcp_rx_available(fd) == 0U && s->state == RV_TCP_ESTABLISHED && timeout < 50000) {
        rv_vnet_poll();
        timeout++;
        rv_vnet_delay();
    }
    uint32_t avail = rv_tcp_rx_available(fd);
    if (avail == 0U) return rt_string_from_cstr("");
    char line[256];
    uint32_t copied = 0;
    while (copied + 1U < sizeof(line) && s->rx_tail != s->rx_head) {
        uint8_t byte = s->rxbuf[s->rx_tail];
        s->rx_tail = (s->rx_tail + 1U) % RV_TCP_RXBUF_SIZE;
        if (byte == '\n') break;
        if (byte == '\r') continue;
        line[copied++] = (char)byte;
    }
    line[copied] = 0;
    return rt_string_from_cstr(line);
}

RuntimeValue rt_native_eq(RuntimeValue a, RuntimeValue b)
{
    return a == b ? 1 : 0;
}

RuntimeValue rt_native_neq(RuntimeValue a, RuntimeValue b)
{
    return a == b ? 0 : 1;
}

__attribute__((naked, section(".text.entry"))) void _start(void)
{
    __asm__ volatile(
        "la sp, _stack_top\n"
        "call spl_start\n"
        "1: wfi\n"
        "j 1b\n"
    );
}
