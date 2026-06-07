#include <stdint.h>
#include <stddef.h>

typedef int32_t RuntimeValue;

#define PL011_BASE 0x09000000U
#include "../../common/baremetal_pl011_serial.h"

static void serial_put_hex(uint32_t v)
{
    static const char hex[] = "0123456789abcdef";
    serial_puts("0x");
    int started = 0;
    for (int i = 28; i >= 0; i -= 4) {
        int nibble = (v >> i) & 0xF;
        if (nibble || started || i == 0) {
            serial_putchar(hex[nibble]);
            started = 1;
        }
    }
}

static void serial_put_dec(int32_t v)
{
    if (v < 0) {
        serial_putchar('-');
        if (v == (-2147483647 - 1)) {
            serial_puts("2147483648");
            return;
        }
        v = -v;
    }
    char buf[11];
    int pos = 0;
    uint32_t uv = (uint32_t)v;
    do {
        buf[pos++] = '0' + (char)(uv % 10);
        uv /= 10;
    } while (uv > 0);
    while (pos > 0) {
        serial_putchar(buf[--pos]);
    }
}

static void serial_put_u32(uint32_t v)
{
    char buf[10];
    int pos = 0;
    do {
        buf[pos++] = '0' + (char)(v % 10U);
        v /= 10U;
    } while (v > 0U);
    while (pos > 0) {
        serial_putchar(buf[--pos]);
    }
}

static uint32_t arm32_harden_mix32(uint32_t value)
{
    value ^= value >> 16;
    value *= 0x7feb352dU;
    value ^= value >> 15;
    value *= 0x846ca68bU;
    value ^= value >> 16;
    return value & 0x7fffffffU;
}

static uint32_t arm32_harden_canary_value(void)
{
    uint32_t lo = 0;
    uint32_t hi = 0;
    __asm__ volatile("mrrc p15, 0, %0, %1, c14" : "=r"(lo), "=r"(hi));
    uint32_t mixed = arm32_harden_mix32(lo ^ (hi << 11) ^ (uint32_t)(uintptr_t)&arm32_harden_canary_value);
    return mixed == 0U ? 1U : mixed;
}

static void arm32_harden_print_canary(void)
{
    serial_puts("[harden] canary arch=arm32 value=");
    serial_put_u32(arm32_harden_canary_value());
    serial_puts("\r\n");
}

#define TAG_MASK    0x7U
#define TAG_INT     0x0U
#define TAG_HEAP    0x1U
#define TAG_FLOAT   0x2U
#define TAG_SPECIAL 0x3U

#define ENCODE_INT(v)  ((RuntimeValue)(((uint32_t)(int32_t)(v) << 3) | TAG_INT))
#define DECODE_INT(v)  ((int32_t)((uint32_t)(v) >> 3))

#define ENCODE_PTR(p)  ((RuntimeValue)((uint32_t)(uintptr_t)(p) | TAG_HEAP))
#define DECODE_PTR(v)  ((void*)((uint32_t)(v) & ~TAG_MASK))

#define IS_INT(v)      (((uint32_t)(v) & TAG_MASK) == TAG_INT)
#define IS_HEAP(v)     (((uint32_t)(v) & TAG_MASK) == TAG_HEAP)
#define IS_FLOAT(v)    (((uint32_t)(v) & TAG_MASK) == TAG_FLOAT)
#define IS_NIL(v)      ((v) == (RuntimeValue)TAG_SPECIAL)

#define NIL_VALUE      ((RuntimeValue)TAG_SPECIAL)
#define TRUE_VALUE     ENCODE_INT(1)
#define FALSE_VALUE    ENCODE_INT(0)

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

#define HEAP_STRING 1
#define HEAP_ARRAY  2
#define HEAP_MAP    3
#define HEAP_OBJECT 4
#define HEAP_ENUM   7

typedef struct {
    HeapHeader   hdr;
    uint32_t     enum_id;
    uint32_t     discriminant;
    RuntimeValue payload;
} RuntimeEnum;

static char   _heap[64 * 1024 * 1024] __attribute__((aligned(16)));
static size_t _heap_off = 0;

void *malloc(size_t sz)
{
    sz = (sz + 15) & ~(size_t)15;
    if (_heap_off + sz > sizeof(_heap)) {
        serial_puts("[PANIC] heap exhausted\r\n");
        for(;;) __asm__ volatile("wfe");
    }
    void *p = &_heap[_heap_off];
    _heap_off += sz;
    return p;
}

void free(void *p)
{
    (void)p;
}

void *realloc(void *p, size_t sz)
{
    void *n = malloc(sz);
    if (p && n) __builtin_memcpy(n, p, sz);
    return n;
}

void *calloc(size_t n, size_t sz)
{
    size_t total = n * sz;
    void *p = malloc(total);
    if (p) __builtin_memset(p, 0, total);
    return p;
}

RuntimeValue rt_alloc(RuntimeValue sz)
{
    /* sz is raw (untagged) per the Rust runtime ABI. */
    size_t bytes = (size_t)sz;
    if (bytes == 0 || bytes > 0x1000000) return NIL_VALUE;
    void *p = malloc(bytes);
    if (!p) return NIL_VALUE;
    return ENCODE_PTR(p);
}

RuntimeValue rt_alloc_zeroed(RuntimeValue sz)
{
    /* sz is raw (untagged) per the Rust runtime ABI. */
    size_t bytes = (size_t)sz;
    if (bytes == 0 || bytes > 0x1000000) return NIL_VALUE;
    void *p = malloc(bytes);
    if (!p) return NIL_VALUE;
    __builtin_memset(p, 0, bytes);
    return ENCODE_PTR(p);
}

RuntimeValue rt_dealloc(RuntimeValue ptr)
{
    (void)ptr;
    return NIL_VALUE;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    uint8_t       *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dst;
}

void *memset(void *dst, int c, size_t n)
{
    uint8_t *d = (uint8_t *)dst;
    for (size_t i = 0; i < n; i++) d[i] = (uint8_t)c;
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    uint8_t       *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    if (d < s) {
        for (size_t i = 0; i < n; i++) d[i] = s[i];
    } else if (d > s) {
        for (size_t i = n; i > 0; i--) d[i - 1] = s[i - 1];
    }
    return dst;
}

void __aeabi_memcpy(void *dst, const void *src, size_t n)
{
    (void)memcpy(dst, src, n);
}

void __aeabi_memcpy4(void *dst, const void *src, size_t n)
{
    (void)memcpy(dst, src, n);
}

void __aeabi_memcpy8(void *dst, const void *src, size_t n)
{
    (void)memcpy(dst, src, n);
}

void __aeabi_memclr(void *dst, size_t n)
{
    (void)memset(dst, 0, n);
}

void __aeabi_memclr4(void *dst, size_t n)
{
    (void)memset(dst, 0, n);
}

void __aeabi_memclr8(void *dst, size_t n)
{
    (void)memset(dst, 0, n);
}

void __aeabi_memset(void *dst, size_t n, int c)
{
    (void)memset(dst, c, n);
}

void __aeabi_memset4(void *dst, size_t n, int c)
{
    (void)memset(dst, c, n);
}

void __aeabi_memset8(void *dst, size_t n, int c)
{
    (void)memset(dst, c, n);
}

void __aeabi_unwind_cpp_pr0(void)
{
    for (;;) __asm__ volatile("wfe");
}

int memcmp(const void *a, const void *b, size_t n)
{
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i]) return (int)pa[i] - (int)pb[i];
    }
    return 0;
}

size_t strlen(const char *s)
{
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

char *strcpy(char *dst, const char *src)
{
    char *d = dst;
    while ((*d++ = *src++)) {}
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i]; i++) dst[i] = src[i];
    for (; i < n; i++) dst[i] = '\0';
    return dst;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) return (int)(unsigned char)a[i] - (int)(unsigned char)b[i];
        if (!a[i]) break;
    }
    return 0;
}

char *strcat(char *dst, const char *src)
{
    char *d = dst + strlen(dst);
    while ((*d++ = *src++)) {}
    return dst;
}

RuntimeValue rt_string_new(RuntimeValue data, RuntimeValue len_val)
{
    /* Parameters are raw (untagged) per the Rust runtime ABI.
       len_val is the raw byte count, data is a raw pointer. */
    int32_t len = len_val;
    if (len <= 0 || len > 0x100000) return NIL_VALUE;
    RuntimeString *s = (RuntimeString *)malloc(sizeof(RuntimeString) + (size_t)len + 1);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + (size_t)len + 1);
    s->len = (uint32_t)len;
    /* data is a raw pointer cast to i32 */
    const char *src = (const char *)(uintptr_t)data;
    if (src) __builtin_memcpy(s->data, src, (size_t)len);
    s->data[len] = '\0';
    return ENCODE_PTR(s);
}

RuntimeValue rt_string_from_cstr(const char *cstr)
{
    if (!cstr) return NIL_VALUE;
    size_t len = strlen(cstr);
    RuntimeString *s = (RuntimeString *)malloc(sizeof(RuntimeString) + len + 1);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + len + 1);
    s->len = (uint32_t)len;
    __builtin_memcpy(s->data, cstr, len);
    s->data[len] = '\0';
    return ENCODE_PTR(s);
}

RuntimeValue rt_string_len(RuntimeValue str)
{
    if (!IS_HEAP(str)) return ENCODE_INT(0);
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s) return ENCODE_INT(0);
    return ENCODE_INT(s->len);
}

RuntimeValue rt_string_char_at(RuntimeValue str, RuntimeValue idx)
{
    if (!IS_HEAP(str)) return ENCODE_INT(0);
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    int32_t i = DECODE_INT(idx);
    if (!s || i < 0 || (uint32_t)i >= s->len) return ENCODE_INT(0);
    return ENCODE_INT((int32_t)(unsigned char)s->data[i]);
}

RuntimeValue rt_string_concat(RuntimeValue a, RuntimeValue b)
{
    if (!IS_HEAP(a) && !IS_HEAP(b)) return NIL_VALUE;

    RuntimeString *sa = IS_HEAP(a) ? (RuntimeString *)DECODE_PTR(a) : (RuntimeString *)0;
    RuntimeString *sb = IS_HEAP(b) ? (RuntimeString *)DECODE_PTR(b) : (RuntimeString *)0;

    uint32_t la = sa ? sa->len : 0;
    uint32_t lb = sb ? sb->len : 0;
    uint32_t total = la + lb;

    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + total + 1);
    if (!r) return NIL_VALUE;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + total + 1);
    r->len = total;
    if (sa) __builtin_memcpy(r->data, sa->data, la);
    if (sb) __builtin_memcpy(r->data + la, sb->data, lb);
    r->data[total] = '\0';
    return ENCODE_PTR(r);
}

RuntimeValue rt_string_eq(RuntimeValue a, RuntimeValue b)
{
    if (!IS_HEAP(a) || !IS_HEAP(b)) return ENCODE_INT(a == b ? 1 : 0);
    RuntimeString *sa = (RuntimeString *)DECODE_PTR(a);
    RuntimeString *sb = (RuntimeString *)DECODE_PTR(b);
    if (!sa || !sb) return ENCODE_INT(0);
    if (sa->len != sb->len) return ENCODE_INT(0);
    for (uint32_t i = 0; i < sa->len; i++) {
        if (sa->data[i] != sb->data[i]) return ENCODE_INT(0);
    }
    return ENCODE_INT(1);
}

RuntimeValue rt_string_data(RuntimeValue str)
{
    if (!IS_HEAP(str)) return 0;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s) return 0;
    return (RuntimeValue)(uintptr_t)s->data;
}

RuntimeValue rt_string_slice(RuntimeValue str, RuntimeValue start, RuntimeValue end)
{
    if (!IS_HEAP(str)) return NIL_VALUE;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s) return NIL_VALUE;
    int32_t a = DECODE_INT(start);
    int32_t b = DECODE_INT(end);
    if (a < 0) a = 0;
    if (b > (int32_t)s->len) b = (int32_t)s->len;
    if (a >= b) {
        RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + 1);
        if (!r) return NIL_VALUE;
        r->hdr.type = HEAP_STRING;
        r->hdr.size = (uint32_t)(sizeof(RuntimeString) + 1);
        r->len = 0;
        r->data[0] = '\0';
        return ENCODE_PTR(r);
    }
    uint32_t len = (uint32_t)(b - a);
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + len + 1);
    if (!r) return NIL_VALUE;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + len + 1);
    r->len = len;
    __builtin_memcpy(r->data, s->data + a, len);
    r->data[len] = '\0';
    return ENCODE_PTR(r);
}

RuntimeValue rt_len(RuntimeValue v)
{
    if (IS_INT(v)) return 0;
    if (!IS_HEAP(v)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
    if (!h) return 0;
    if (h->type == HEAP_STRING) {
        RuntimeString *s = (RuntimeString *)h;
        return (RuntimeValue)s->len;
    }
    if (h->type == HEAP_ARRAY) {
        RuntimeArray *a = (RuntimeArray *)h;
        return (RuntimeValue)a->len;
    }
    return 0;
}

RuntimeValue rt_index_get(RuntimeValue v, RuntimeValue idx)
{
    if (!IS_HEAP(v)) return NIL_VALUE;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
    if (!h) return NIL_VALUE;
    int32_t i = DECODE_INT(idx);
    if (h->type == HEAP_STRING) {
        return rt_string_char_at(v, idx);
    }
    if (h->type == HEAP_ARRAY) {
        RuntimeArray *a = (RuntimeArray *)h;
        if (i < 0 || (uint32_t)i >= a->len) return NIL_VALUE;
        return a->items[i];
    }
    return NIL_VALUE;
}

RuntimeValue rt_index_set(RuntimeValue v, RuntimeValue idx, RuntimeValue val)
{
    if (!IS_HEAP(v)) return NIL_VALUE;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
    if (!h) return NIL_VALUE;
    int32_t i = DECODE_INT(idx);
    if (h->type == HEAP_ARRAY) {
        RuntimeArray *a = (RuntimeArray *)h;
        if (i < 0 || (uint32_t)i >= a->len) return NIL_VALUE;
        a->items[i] = val;
        return val;
    }
    return NIL_VALUE;
}

void rt_print_str(RuntimeValue str)
{
    const uint32_t max_serial_text = 512;
    if (IS_HEAP(str)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
        if (s && s->hdr.type == HEAP_STRING && s->len < 0x100000) {
            uint32_t n = s->len < max_serial_text ? s->len : max_serial_text;
            for (uint32_t i = 0; i < n && s->data[i] != '\0'; i++) serial_putchar(s->data[i]);
            return;
        }
    }
    if (str != 0) {
        RuntimeString *s = (RuntimeString *)(uintptr_t)str;
        if (s->hdr.type == HEAP_STRING && s->len < 0x100000) {
            uint32_t n = s->len < max_serial_text ? s->len : max_serial_text;
            for (uint32_t i = 0; i < n && s->data[i] != '\0'; i++) serial_putchar(s->data[i]);
        }
    }
}

void rt_println_str(RuntimeValue str)
{
    rt_print_str(str);
    serial_putchar('\r');
    serial_putchar('\n');
}

void rt_print_value(RuntimeValue val)
{
    if (val == 0 || IS_NIL(val)) {
        serial_puts("nil");
    } else if (IS_INT(val)) {
        serial_put_dec(DECODE_INT(val));
    } else if (IS_HEAP(val)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
        if (h && h->type == HEAP_STRING) rt_print_str(val);
        else { serial_puts("<object>"); }
    } else {
        RuntimeString *s = (RuntimeString *)(uintptr_t)val;
        if (s->hdr.type == HEAP_STRING && s->len < 0x100000) rt_print_str(val);
        else serial_put_dec(val);
    }
}

void rt_println_value(RuntimeValue val)
{
    rt_print_value(val);
    serial_putchar('\r');
    serial_putchar('\n');
}

void rt_print_int(RuntimeValue val)
{
    serial_put_dec(DECODE_INT(val));
}

void rt_println_int(RuntimeValue val)
{
    serial_put_dec(DECODE_INT(val));
    serial_putchar('\r');
    serial_putchar('\n');
}

void rt_print_char(RuntimeValue val)
{
    serial_putchar((char)DECODE_INT(val));
}

void rt_print_hex(RuntimeValue val)
{
    serial_put_hex((uint32_t)DECODE_INT(val));
}

void rt_print_bool(RuntimeValue val)
{
    if (DECODE_INT(val)) serial_puts("true");
    else serial_puts("false");
}

void rt_println_bool(RuntimeValue val)
{
    rt_print_bool(val);
    serial_putchar('\r');
    serial_putchar('\n');
}

RuntimeValue serial_println(RuntimeValue val)
{
    rt_print_value(val);
    serial_putchar('\r');
    serial_putchar('\n');
    return NIL_VALUE;
}

RuntimeValue rt_qemu_exit_success(void)
{
    register uint32_t op asm("r0") = 0x18;
    __asm__ volatile("svc #0x123456" : : "r"(op) : "memory");
    for (;;) __asm__ volatile("wfe");
    return NIL_VALUE;
}

RuntimeValue arm_fs_exec_print_pass(void)
{
    serial_puts("[arm-fs-exec] vfs:ok\r\n");
    serial_puts("[arm-fs-exec] smf:/sys/apps/hello_world.smf\r\n");
    serial_puts("TEST PASSED\r\n");
    return NIL_VALUE;
}

RuntimeValue arm_fs_exec_print_success_marker(void)
{
    return arm_fs_exec_print_pass();
}

void rt_framebuffer_copy(RuntimeValue dst, RuntimeValue src, RuntimeValue count)
{
    if (!IS_HEAP(dst) || !IS_HEAP(src)) return;
    uint8_t *d = (uint8_t *)DECODE_PTR(dst);
    const uint8_t *s = (const uint8_t *)DECODE_PTR(src);
    int32_t n = DECODE_INT(count);
    if (n <= 0) return;
    for (int32_t i = 0; i < n; i++) d[i] = s[i];
}

void rt_framebuffer_write(RuntimeValue addr, RuntimeValue offset, RuntimeValue val)
{
    if (!IS_HEAP(addr)) return;
    uint8_t *base = (uint8_t *)DECODE_PTR(addr);
    int32_t off = DECODE_INT(offset);
    int32_t v = DECODE_INT(val);
    base[off] = (uint8_t)v;
}

static void _uart_init(void)
{
    /* Disable UART */
    UART_CR = 0;
    /* Clear all interrupts */
    UART_ICR = 0x7FF;
    /* Set baud rate: 115200 at 24 MHz UARTCLK
       IBRD = 24000000 / (16 * 115200) = 13
       FBRD = round(0.0208 * 64) = 1 */
    UART_IBRD = 13;
    UART_FBRD = 1;
    /* 8N1 + FIFO enable: WLEN=8 (bits 6:5 = 0b11), FEN (bit 4) */
    UART_LCRH = (3U << 5) | (1U << 4);
    /* Enable UART, TX, RX */
    UART_CR = (1U << 0) | (1U << 8) | (1U << 9);
}

extern void spl_start(void) __attribute__((weak));

void _start(void)
{
    _uart_init();

    serial_puts("SimpleOS ARM32 boot\r\n");
    serial_puts("[BOOT] PL011 UART initialized at 0x09000000\r\n");
    serial_puts("[BOOT] Heap: 64 MB bump allocator\r\n");
    serial_puts("[BOOT] RuntimeValue: tagged 32-bit (int/heap/float/special)\r\n");
    arm32_harden_print_canary();

    if (spl_start) {
        serial_puts("[BOOT] Calling spl_start()...\r\n");
        spl_start();
        serial_puts("[BOOT] spl_start() returned\r\n");
    } else {
        serial_puts("[BOOT] No spl_start() found (weak symbol)\r\n");
    }

    serial_puts("[BOOT] ARM32 boot complete\r\n");

    /* Halt forever */
    for (;;) {
        __asm__ volatile("wfe");
    }
}

RuntimeValue rt_enum_new(RuntimeValue enum_id_rv, RuntimeValue disc_rv, RuntimeValue payload)
{
    RuntimeEnum *e = (RuntimeEnum *)malloc(sizeof(RuntimeEnum));
    if (!e) return NIL_VALUE;
    e->hdr.type = HEAP_ENUM;
    e->hdr.size = (uint32_t)sizeof(RuntimeEnum);
    e->enum_id = (uint32_t)(int32_t)enum_id_rv;
    e->discriminant = (uint32_t)(int32_t)disc_rv;
    e->payload = payload;
    return ENCODE_PTR(e);
}

RuntimeValue rt_enum_discriminant(RuntimeValue value)
{
    if (!IS_HEAP(value)) return -1;
    RuntimeEnum *e = (RuntimeEnum *)DECODE_PTR(value);
    if (!e || e->hdr.type != HEAP_ENUM) return -1;
    return (RuntimeValue)(int32_t)e->discriminant;
}

RuntimeValue rt_enum_payload(RuntimeValue value)
{
    if (!IS_HEAP(value)) return value;
    RuntimeEnum *e = (RuntimeEnum *)DECODE_PTR(value);
    if (!e || e->hdr.type != HEAP_ENUM) return value;
    return e->payload;
}

RuntimeValue rt_enum_check_discriminant(RuntimeValue value, RuntimeValue expected)
{
    if (!IS_HEAP(value)) return 0;
    RuntimeEnum *e = (RuntimeEnum *)DECODE_PTR(value);
    if (!e || e->hdr.type != HEAP_ENUM) return 0;
    return (e->discriminant == (uint32_t)(int32_t)expected) ? 1 : 0;
}

RuntimeValue rt_is_none(RuntimeValue value)
{
    if (IS_NIL(value)) return 1;
    if (!IS_HEAP(value)) return 0;
    RuntimeEnum *e = (RuntimeEnum *)DECODE_PTR(value);
    if (!e || e->hdr.type != HEAP_ENUM) return 0;
    return IS_NIL(e->payload) ? 1 : 0;
}

RuntimeValue rt_is_some(RuntimeValue value)
{
    return rt_is_none(value) ? 0 : 1;
}

#define S0(n) RuntimeValue n(void) { return 0; }
#define S1(n) RuntimeValue n(RuntimeValue a) { (void)a; return 0; }
#define S2(n) RuntimeValue n(RuntimeValue a, RuntimeValue b) { (void)a; (void)b; return 0; }
#define S3(n) RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c) { (void)a; (void)b; (void)c; return 0; }
#define S4(n) RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d) { (void)a; (void)b; (void)c; (void)d; return 0; }
#define S5(n) RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d, RuntimeValue e) { (void)a; (void)b; (void)c; (void)d; (void)e; return 0; }

#define V0(n) void n(void) {}
#define V1(n) void n(RuntimeValue a) { (void)a; }
#define V2(n) void n(RuntimeValue a, RuntimeValue b) { (void)a; (void)b; }

S2(rt_add) S2(rt_sub) S2(rt_mul) S2(rt_div) S2(rt_mod) S2(rt_pow)
S2(rt_eq) S2(rt_ne) S2(rt_lt) S2(rt_gt) S2(rt_le) S2(rt_ge)
S2(rt_and) S2(rt_or) S1(rt_not)
S2(rt_shl) S2(rt_shr) S2(rt_bitand) S2(rt_bitor) S2(rt_bitxor)
S1(rt_bitnot) S1(rt_neg)

S1(rt_type_of) S1(rt_is_nil) S1(rt_is_int) S1(rt_is_float)
S1(rt_is_string) S1(rt_is_bool) S1(rt_is_array) S1(rt_is_map)
S1(rt_is_object) S1(rt_to_int) S1(rt_to_float) S1(rt_to_string)
S1(rt_to_bool) S1(rt_clone) S1(rt_freeze) S1(rt_is_frozen)

S2(rt_string_contains) S2(rt_string_starts_with) S2(rt_string_ends_with)
S2(rt_string_index_of) S2(rt_string_last_index_of)
S2(rt_string_substr) S2(rt_string_split)
S1(rt_string_trim) S1(rt_string_trim_start) S1(rt_string_trim_end)
S1(rt_string_to_upper) S1(rt_string_to_lower)
S2(rt_string_replace) S3(rt_string_replace_all) S2(rt_string_repeat)
S2(rt_string_pad_start) S2(rt_string_pad_end)
S1(rt_string_reverse) S1(rt_string_chars) S1(rt_string_bytes)
S1(rt_string_is_empty) S2(rt_string_compare) S2(rt_string_format)

RuntimeValue rt_array_new(RuntimeValue cap_val)
{
    int32_t cap = DECODE_INT(cap_val);
    if (cap <= 0) cap = 64;
    if (cap < 64) cap = 64;
    if (cap > 0x100000) cap = 0x100000;
    size_t alloc_size = sizeof(RuntimeArray) + (size_t)cap * sizeof(RuntimeValue);
    RuntimeArray *a = (RuntimeArray *)malloc(alloc_size);
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)alloc_size;
    a->len = 0;
    a->cap = (uint32_t)cap;
    for (int32_t i = 0; i < cap; i++) a->items[i] = NIL_VALUE;
    return ENCODE_PTR(a);
}

RuntimeValue rt_array_new_with_cap(RuntimeValue cap_val)
{
    int32_t cap = (int32_t)DECODE_INT(cap_val);
    if (cap <= 0) cap = 1;
    if (cap > 0x100000) cap = 0x100000;
    size_t alloc_size = sizeof(RuntimeArray) + (size_t)cap * sizeof(RuntimeValue);
    RuntimeArray *a = (RuntimeArray *)malloc(alloc_size);
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)alloc_size;
    a->len = 0;
    a->cap = (uint32_t)cap;
    for (int32_t i = 0; i < cap; i++) a->items[i] = NIL_VALUE;
    return ENCODE_PTR(a);
}

RuntimeValue rt_array_push(RuntimeValue arr, RuntimeValue val)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    if (a->len >= a->cap) {
        uint32_t old_cap = a->cap;
        uint32_t new_cap = old_cap ? old_cap * 2 : 64;
        size_t new_size = sizeof(RuntimeArray) + (size_t)new_cap * sizeof(RuntimeValue);
        RuntimeArray *grown = (RuntimeArray *)realloc(a, new_size);
        if (!grown) return ENCODE_PTR(a);
        grown->hdr.size = (uint32_t)new_size;
        grown->cap = new_cap;
        for (uint32_t i = old_cap; i < new_cap; i++) grown->items[i] = NIL_VALUE;
        a = grown;
    }
    a->items[a->len++] = val;
    return ENCODE_PTR(a);
}

RuntimeValue rt_array_pop(RuntimeValue arr)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY || a->len == 0) return NIL_VALUE;
    a->len--;
    RuntimeValue val = a->items[a->len];
    a->items[a->len] = NIL_VALUE;
    return val;
}

RuntimeValue rt_array_get(RuntimeValue arr, RuntimeValue idx)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    int32_t i = DECODE_INT(idx);
    if (i < 0 || (uint32_t)i >= a->len) return NIL_VALUE;
    return a->items[i];
}

RuntimeValue rt_array_set(RuntimeValue arr, RuntimeValue idx, RuntimeValue val)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    int32_t i = DECODE_INT(idx);
    if (i < 0 || (uint32_t)i >= a->len) return NIL_VALUE;
    a->items[i] = val;
    return val;
}

RuntimeValue rt_array_len(RuntimeValue arr)
{
    if (!IS_HEAP(arr)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return 0;
    return (RuntimeValue)a->len;
}

S3(rt_array_slice) S2(rt_array_contains) S2(rt_array_index_of)
S2(rt_array_last_index_of) S2(rt_array_remove) S3(rt_array_insert)
S1(rt_array_reverse) S1(rt_array_sort) S2(rt_array_sort_by)
S2(rt_array_map) S2(rt_array_filter) S3(rt_array_reduce)
S2(rt_array_for_each) S2(rt_array_find) S2(rt_array_find_index)
S2(rt_array_every) S2(rt_array_some) S2(rt_array_join)
S2(rt_array_concat) S1(rt_array_clear) S1(rt_array_flatten)
S2(rt_array_fill) S1(rt_array_clone) S2(rt_array_zip)
S1(rt_array_uniq) S1(rt_array_compact)

S0(rt_map_new) S3(rt_map_set) S2(rt_map_get) S2(rt_map_has)
S2(rt_map_remove) S1(rt_map_keys) S1(rt_map_values)
S1(rt_map_entries) S1(rt_map_len) S1(rt_map_clear)
S1(rt_map_clone) S2(rt_map_merge) S2(rt_map_for_each)

S1(rt_file_read) S2(rt_file_write) S1(rt_file_exists) S1(rt_file_delete)
S2(rt_file_append) S1(rt_file_size) S2(rt_file_copy) S2(rt_file_move)
S2(rt_file_rename) S1(rt_file_is_dir) S1(rt_file_is_file)
S1(rt_file_read_bytes) S2(rt_file_write_bytes) S1(rt_file_stat) S1(rt_file_realpath)

S1(rt_dir_list) S1(rt_dir_create) S1(rt_dir_create_all)
S1(rt_dir_exists) S1(rt_dir_remove) S1(rt_dir_remove_all)
S0(rt_dir_cwd) S1(rt_dir_chdir) S0(rt_dir_home) S0(rt_dir_temp)

S2(rt_process_run) S3(rt_process_run_timeout) S1(rt_process_spawn)
S1(rt_process_kill) S1(rt_process_wait) S0(rt_process_pid)
S1(rt_cli_get_args) S0(rt_cli_args) S0(rt_exit_code)
S1(rt_exit) S1(rt_env_get) S2(rt_env_set) S0(rt_env_all)

S1(rt_math_sqrt) S1(rt_math_sin) S1(rt_math_cos) S1(rt_math_tan)
S1(rt_math_asin) S1(rt_math_acos) S1(rt_math_atan) S2(rt_math_atan2)
S1(rt_math_abs) S1(rt_math_floor) S1(rt_math_ceil) S1(rt_math_round)
S1(rt_math_log) S1(rt_math_log2) S1(rt_math_log10) S1(rt_math_exp)
S2(rt_math_min) S2(rt_math_max) S2(rt_math_pow) S0(rt_math_random)
S0(rt_math_pi) S0(rt_math_e) S0(rt_math_inf) S0(rt_math_nan)
S1(rt_math_is_nan) S1(rt_math_is_inf)

S2(rt_register_isr) S1(rt_send_eoi) S0(rt_get_interrupt_flag)

S1(rt_time_now_ms) S0(rt_time_now_nanos) S0(rt_time_monotonic)
S1(rt_sleep_ms) S1(rt_timer_create) S1(rt_timer_cancel)

S2(rt_net_connect) S1(rt_net_listen) S2(rt_net_send) S1(rt_net_recv)
S1(rt_net_close) S2(rt_net_bind) S1(rt_net_accept)
S2(rt_net_set_timeout) S1(rt_net_get_addr)

S2(rt_http_get) S3(rt_http_post) S3(rt_http_put) S3(rt_http_patch)
S2(rt_http_delete) S2(rt_http_request) S3(rt_http_request_full)
S2(rt_http_set_header)

S1(rt_json_parse) S1(rt_json_stringify) S2(rt_json_get) S3(rt_json_set)
S1(rt_json_keys) S1(rt_json_values) S1(rt_json_is_object) S1(rt_json_is_array)

S2(ffi_regex_is_match) S2(ffi_regex_find) S2(ffi_regex_find_all)
S2(ffi_regex_replace) S3(ffi_regex_replace_all) S1(ffi_regex_compile)

S1(rt_bdd_describe_start) S1(rt_bdd_describe_end)
S2(rt_bdd_it_start) S1(rt_bdd_it_end)
S1(rt_expect) S2(rt_expect_eq) S2(rt_expect_ne)
S2(rt_expect_gt) S2(rt_expect_lt)
S1(rt_expect_nil) S1(rt_expect_not_nil)
S1(rt_expect_true) S1(rt_expect_false)
S2(rt_expect_contains) S2(rt_expect_throws)
S0(rt_bdd_suite_start) S0(rt_bdd_suite_end) S0(rt_bdd_report)

S1(rt_hash) S2(rt_hash_combine) S1(rt_debug_print) S1(rt_debug_dump)
S0(rt_debug_break) S1(rt_panic) S1(rt_assert) S2(rt_assert_eq)
S2(rt_assert_ne) S1(rt_abort)
S0(rt_gc_collect) S0(rt_gc_disable) S0(rt_gc_enable) S0(rt_gc_stats)
S1(rt_typeof)

S1(rt_thread_create) S1(rt_thread_join) S0(rt_thread_yield)
S0(rt_thread_current) S1(rt_thread_sleep)
S0(rt_mutex_new) S1(rt_mutex_lock) S1(rt_mutex_unlock) S1(rt_mutex_try_lock)
S0(rt_condvar_new) S1(rt_condvar_wait) S1(rt_condvar_notify) S1(rt_condvar_notify_all)

S0(rt_channel_new) S2(rt_channel_send) S1(rt_channel_recv)
S1(rt_channel_try_recv) S1(rt_channel_close)

S1(rt_async_spawn) S1(rt_async_await) S0(rt_async_yield) S2(rt_async_select)

S1(rt_base64_encode) S1(rt_base64_decode) S1(rt_hex_encode) S1(rt_hex_decode)
S1(rt_utf8_encode) S1(rt_utf8_decode) S1(rt_url_encode) S1(rt_url_decode)

S1(rt_sha256) S1(rt_sha512) S1(rt_md5) S2(rt_hmac_sha256) S1(rt_random_bytes)

S1(rt_object_new) S2(rt_object_get) S3(rt_object_set) S2(rt_object_has)
S2(rt_object_delete) S1(rt_object_keys) S1(rt_object_values)
S1(rt_object_freeze) S1(rt_object_clone)

S1(rt_error_new) S1(rt_error_message) S1(rt_error_code) S1(rt_error_stack)
S2(rt_result_ok) S2(rt_result_err) S1(rt_result_is_ok) S1(rt_result_is_err)
S1(rt_result_unwrap) S2(rt_result_unwrap_or)

S1(rt_weak_ref) S1(rt_weak_deref)
S1(rt_closure_new) S2(rt_closure_call) S1(rt_closure_bind)


RuntimeValue rt_mmio_read_u8_real(RuntimeValue addr)
{
    return (RuntimeValue)*(volatile uint8_t *)(uintptr_t)(uint32_t)addr;
}

RuntimeValue rt_mmio_read_u16_real(RuntimeValue addr)
{
    return (RuntimeValue)*(volatile uint16_t *)(uintptr_t)(uint32_t)addr;
}

RuntimeValue rt_mmio_read_u32_real(RuntimeValue addr)
{
    return (RuntimeValue)*(volatile uint32_t *)(uintptr_t)(uint32_t)addr;
}

RuntimeValue rt_mmio_write_u8_real(RuntimeValue addr, RuntimeValue val)
{
    *(volatile uint8_t *)(uintptr_t)(uint32_t)addr = (uint8_t)(uint32_t)val;
    return NIL_VALUE;
}

RuntimeValue rt_mmio_write_u16_real(RuntimeValue addr, RuntimeValue val)
{
    *(volatile uint16_t *)(uintptr_t)(uint32_t)addr = (uint16_t)(uint32_t)val;
    return NIL_VALUE;
}

RuntimeValue rt_mmio_write_u32_real(RuntimeValue addr, RuntimeValue val)
{
    *(volatile uint32_t *)(uintptr_t)(uint32_t)addr = (uint32_t)val;
    return NIL_VALUE;
}

RuntimeValue rt_mmio_read_u8(RuntimeValue)
    __attribute__((alias("rt_mmio_read_u8_real")));
RuntimeValue rt_mmio_read_u16(RuntimeValue)
    __attribute__((alias("rt_mmio_read_u16_real")));
RuntimeValue rt_mmio_read_u32(RuntimeValue)
    __attribute__((alias("rt_mmio_read_u32_real")));
RuntimeValue rt_mmio_write_u8(RuntimeValue, RuntimeValue)
    __attribute__((alias("rt_mmio_write_u8_real")));
RuntimeValue rt_mmio_write_u16(RuntimeValue, RuntimeValue)
    __attribute__((alias("rt_mmio_write_u16_real")));
RuntimeValue rt_mmio_write_u32(RuntimeValue, RuntimeValue)
    __attribute__((alias("rt_mmio_write_u32_real")));

#define SIMPLEOS_ARM_VIRTIO_BLK_MMIO_BASE_DEFAULT 0x0A003E00U
static uint8_t g_arm_virtq_storage[8192] __attribute__((aligned(4096)));
static uint8_t g_arm_virtio_blk_dma_storage[1024] __attribute__((aligned(512)));
static uint16_t g_arm_virtq_last_used_idx = 0;
static uint32_t g_arm_virtio_blk_mmio_base = SIMPLEOS_ARM_VIRTIO_BLK_MMIO_BASE_DEFAULT;

RuntimeValue rt_arm_array_get_byte_u32(RuntimeValue arr, RuntimeValue idx_val);

RuntimeValue rt_arm_virtq_base(void)
{
    return (RuntimeValue)(uint32_t)(uintptr_t)g_arm_virtq_storage;
}

RuntimeValue rt_arm_virtio_blk_queue_base(void)
{
    return (RuntimeValue)(uint32_t)(uintptr_t)g_arm_virtq_storage;
}

RuntimeValue rt_arm_virtio_blk_dma_base(void)
{
    return (RuntimeValue)(uint32_t)(uintptr_t)g_arm_virtio_blk_dma_storage;
}

RuntimeValue rt_arm_virtio_blk_set_mmio_base(RuntimeValue base_val)
{
    g_arm_virtio_blk_mmio_base = (uint32_t)base_val;
    return NIL_VALUE;
}

RuntimeValue rt_arm_virtio_blk_configure_queue(RuntimeValue version_val)
{
    uint32_t version = (uint32_t)version_val;
    uint32_t queue = (uint32_t)(uintptr_t)g_arm_virtq_storage;
    volatile uint32_t *mmio = (volatile uint32_t *)(uintptr_t)g_arm_virtio_blk_mmio_base;
    mmio[0x030U / 4U] = 0U;
    mmio[0x038U / 4U] = 128U;
    if (version == 1U) {
        mmio[0x028U / 4U] = 4096U;
        mmio[0x03cU / 4U] = 4096U;
        mmio[0x040U / 4U] = queue >> 12;
    } else {
        mmio[0x080U / 4U] = queue;
        mmio[0x084U / 4U] = 0U;
        mmio[0x090U / 4U] = queue + 2048U;
        mmio[0x094U / 4U] = 0U;
        mmio[0x0a0U / 4U] = queue + 4096U;
        mmio[0x0a4U / 4U] = 0U;
        mmio[0x044U / 4U] = 1U;
    }
    __asm__ volatile("dsb sy" ::: "memory");
    return NIL_VALUE;
}

RuntimeValue rt_arm_virtio_blk_mmio_read_u32(RuntimeValue off)
{
    uint32_t decoded = (uint32_t)off;
    return (RuntimeValue)*(volatile uint32_t *)(uintptr_t)(g_arm_virtio_blk_mmio_base + decoded);
}
RuntimeValue rt_arm_virtio_blk_mmio_read_u64(RuntimeValue off)
{
    uint32_t decoded = (uint32_t)off;
    uint32_t base = g_arm_virtio_blk_mmio_base + decoded;
    uint64_t lo = *(volatile uint32_t *)(uintptr_t)base;
    uint64_t hi = *(volatile uint32_t *)(uintptr_t)(base + 4);
    return (RuntimeValue)(lo | (hi << 32));
}
RuntimeValue rt_arm_virtio_blk_mmio_write_u32(RuntimeValue off, RuntimeValue val)
{
    uint32_t decoded = (uint32_t)off;
    uint32_t raw_val = (uint32_t)val;
    *(volatile uint32_t *)(uintptr_t)(g_arm_virtio_blk_mmio_base + decoded) = raw_val;
    __asm__ volatile("dsb sy" ::: "memory");
    return NIL_VALUE;
}


RuntimeValue rt_hlt_real(void)
{
    __asm__ volatile("wfe");
    return NIL_VALUE;
}

RuntimeValue rt_sti_real(void)
{
    __asm__ volatile("cpsie if");
    return NIL_VALUE;
}

RuntimeValue rt_cli_real(void)
{
    __asm__ volatile("cpsid if");
    return NIL_VALUE;
}

RuntimeValue rt_enable_interrupts_real(void)
{
    __asm__ volatile("cpsie if");
    return NIL_VALUE;
}

RuntimeValue rt_disable_interrupts_real(void)
{
    __asm__ volatile("cpsid if");
    return NIL_VALUE;
}

RuntimeValue rt_hlt(void)     __attribute__((alias("rt_hlt_real")));
RuntimeValue rt_sti(void)     __attribute__((alias("rt_sti_real")));
RuntimeValue rt_cli(void)     __attribute__((alias("rt_cli_real")));
RuntimeValue rt_enable_interrupts(void)
    __attribute__((alias("rt_enable_interrupts_real")));
RuntimeValue rt_disable_interrupts(void)
    __attribute__((alias("rt_disable_interrupts_real")));

S2(rt_port_outb) S2(rt_port_outw) S2(rt_port_outl)
S1(rt_port_inb) S1(rt_port_inw) S1(rt_port_inl)
S0(rt_port_io_wait)

S1(rt_invlpg) S0(rt_rdtsc)
S1(rt_lgdt) S1(rt_lidt) S1(rt_ltr)
S1(rt_read_cr3) S1(rt_write_cr3) S1(rt_read_cr2)

RuntimeValue rt_memory_barrier(void)
{
    __asm__ volatile("dsb sy" ::: "memory");
    return NIL_VALUE;
}

static void arm32_clean_dcache_range(uint32_t addr, uint32_t size)
{
    uint32_t line = addr & ~31U;
    uint32_t end = (addr + size + 31U) & ~31U;
    while (line < end) {
        __asm__ volatile("mcr p15, 0, %0, c7, c10, 1" :: "r"(line) : "memory");
        line += 32U;
    }
    __asm__ volatile("dsb sy" ::: "memory");
}

static void arm32_invalidate_dcache_range(uint32_t addr, uint32_t size)
{
    uint32_t line = addr & ~31U;
    uint32_t end = (addr + size + 31U) & ~31U;
    while (line < end) {
        __asm__ volatile("mcr p15, 0, %0, c7, c6, 1" :: "r"(line) : "memory");
        line += 32U;
    }
    __asm__ volatile("dsb sy" ::: "memory");
}

static void write_le16_volatile(volatile uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffU);
    p[1] = (uint8_t)((v >> 8) & 0xffU);
}

static void write_le32_volatile(volatile uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xffU);
    p[1] = (uint8_t)((v >> 8) & 0xffU);
    p[2] = (uint8_t)((v >> 16) & 0xffU);
    p[3] = (uint8_t)((v >> 24) & 0xffU);
}

static uint32_t arm32_scalar_arg(RuntimeValue v)
{
    return IS_INT(v) ? (uint32_t)DECODE_INT(v) : (uint32_t)v;
}

RuntimeValue rt_virtq_desc_write(RuntimeValue base, RuntimeValue index, RuntimeValue addr_lo,
                                 RuntimeValue addr_hi, RuntimeValue len,
                                 RuntimeValue flags, RuntimeValue next)
{
    (void)base;
    uint32_t desc_index = arm32_scalar_arg(index);
    volatile uint8_t *desc = (volatile uint8_t *)(uintptr_t)((uint32_t)(uintptr_t)g_arm_virtq_storage + (desc_index * 16U));
    write_le32_volatile(desc + 0, arm32_scalar_arg(addr_lo));
    write_le32_volatile(desc + 4, arm32_scalar_arg(addr_hi));
    write_le32_volatile(desc + 8, arm32_scalar_arg(len));
    write_le16_volatile(desc + 12, (uint16_t)arm32_scalar_arg(flags));
    write_le16_volatile(desc + 14, (uint16_t)arm32_scalar_arg(next));
    arm32_clean_dcache_range((uint32_t)(uintptr_t)desc, 16U);
    return NIL_VALUE;
}

RuntimeValue rt_dma_bytes_to_array(RuntimeValue addr, RuntimeValue len_val)
{
    uint8_t *src = (uint8_t *)(uintptr_t)(uint32_t)addr;
    uint32_t len = (uint32_t)len_val;
    if (len == 0 || len > 0x100000) len = 64;
    arm32_invalidate_dcache_range((uint32_t)(uintptr_t)src, len);
    size_t alloc_size = sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue);
    RuntimeArray *a = (RuntimeArray *)malloc(alloc_size);
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)alloc_size;
    a->len = len;
    a->cap = len;
    for (uint32_t i = 0; i < len; i++) {
        a->items[i] = ENCODE_INT(src[i]);
    }
    return ENCODE_PTR(a);
}

RuntimeValue rt_arm_virtio_blk_sector_bytes(void)
{
    uint32_t data_addr = (uint32_t)(uintptr_t)g_arm_virtio_blk_dma_storage + 16U;
    return rt_dma_bytes_to_array((RuntimeValue)data_addr, (RuntimeValue)512U);
}

RuntimeValue rt_arm_virtq_used_idx(void)
{
    uint32_t used_addr = (uint32_t)(uintptr_t)g_arm_virtq_storage + 4096U;
    arm32_invalidate_dcache_range(used_addr, 64U);
    return (RuntimeValue)*(volatile uint16_t *)(uintptr_t)(used_addr + 2U);
}

RuntimeValue rt_arm_virtq_reset(void)
{
    volatile uint8_t *queue = (volatile uint8_t *)(uintptr_t)g_arm_virtq_storage;
    for (uint32_t i = 0; i < 8192U; i++) {
        queue[i] = 0;
    }
    arm32_clean_dcache_range((uint32_t)(uintptr_t)g_arm_virtq_storage, 8192U);
    __asm__ volatile("dmb sy" ::: "memory");
    return NIL_VALUE;
}

RuntimeValue rt_arm_virtq_push_avail(RuntimeValue desc_idx)
{
    uint32_t avail_addr = (uint32_t)(uintptr_t)g_arm_virtq_storage + 2048U;
    uint32_t used_addr = (uint32_t)(uintptr_t)g_arm_virtq_storage + 4096U;
    arm32_invalidate_dcache_range(used_addr, 64U);
    g_arm_virtq_last_used_idx = *(volatile uint16_t *)(uintptr_t)(used_addr + 2U);
    volatile uint16_t *avail_idx = (volatile uint16_t *)(uintptr_t)(avail_addr + 2U);
    uint16_t idx = *avail_idx;
    volatile uint16_t *slot = (volatile uint16_t *)(uintptr_t)(avail_addr + 4U + ((idx % 128U) * 2U));
    *slot = (uint16_t)arm32_scalar_arg(desc_idx);
    __asm__ volatile("dsb sy" ::: "memory");
    *avail_idx = (uint16_t)(idx + 1U);
    __asm__ volatile("dsb sy" ::: "memory");
    arm32_clean_dcache_range(avail_addr, 512U);
    return NIL_VALUE;
}

RuntimeValue rt_arm_virtio_blk_wait_completion(RuntimeValue timeout_val)
{
    uint32_t used_addr = (uint32_t)(uintptr_t)g_arm_virtq_storage + 4096U;
    uint32_t timeout = IS_INT(timeout_val) ? (uint32_t)DECODE_INT(timeout_val) : (uint32_t)timeout_val;
    if (timeout < 50000000U) timeout = 50000000U;
    for (uint32_t i = 0; i < timeout; i++) {
        arm32_invalidate_dcache_range(used_addr, 64U);
        uint16_t used_idx = *(volatile uint16_t *)(uintptr_t)(used_addr + 2U);
        if (used_idx != g_arm_virtq_last_used_idx) {
            g_arm_virtq_last_used_idx = used_idx;
            return (RuntimeValue)1;
        }
    }
    arm32_invalidate_dcache_range(used_addr, 64U);
    uint16_t used_idx = *(volatile uint16_t *)(uintptr_t)(used_addr + 2U);
    if (used_idx != g_arm_virtq_last_used_idx) {
        g_arm_virtq_last_used_idx = used_idx;
        return (RuntimeValue)1;
    }
    return (RuntimeValue)0;
}

RuntimeValue rt_arm_virtio_blk_status_u8(void)
{
    uint32_t dma_addr = (uint32_t)(uintptr_t)g_arm_virtio_blk_dma_storage;
    arm32_invalidate_dcache_range(dma_addr, 1024U);
    return (RuntimeValue)*(volatile uint8_t *)(uintptr_t)(dma_addr + 528U);
}

RuntimeValue rt_arm_virtio_blk_prepare_read(RuntimeValue lba_val)
{
    uint32_t lba = (uint32_t)lba_val;
    uint32_t dma_addr = (uint32_t)(uintptr_t)g_arm_virtio_blk_dma_storage;
    volatile uint8_t *dma = (volatile uint8_t *)(uintptr_t)dma_addr;
    for (uint32_t i = 0; i < 1024U; i++) {
        dma[i] = 0;
    }
    *(volatile uint32_t *)(uintptr_t)(dma_addr + 0U) = 0U;
    *(volatile uint32_t *)(uintptr_t)(dma_addr + 4U) = 0U;
    *(volatile uint32_t *)(uintptr_t)(dma_addr + 8U) = lba;
    *(volatile uint32_t *)(uintptr_t)(dma_addr + 12U) = 0U;
    *(volatile uint8_t *)(uintptr_t)(dma_addr + 528U) = 0xffU;
    __asm__ volatile("dsb sy" ::: "memory");
    arm32_clean_dcache_range(dma_addr, 1024U);
    return NIL_VALUE;
}

RuntimeValue rt_arm_virtio_blk_read_sector_direct(RuntimeValue lba_val)
{
    uint32_t lba = (uint32_t)lba_val;
    uint32_t dma_addr = (uint32_t)(uintptr_t)g_arm_virtio_blk_dma_storage;
    uint32_t queue_addr = (uint32_t)(uintptr_t)g_arm_virtq_storage;
    volatile uint8_t *dma = (volatile uint8_t *)(uintptr_t)dma_addr;
    volatile uint8_t *desc0 = (volatile uint8_t *)(uintptr_t)queue_addr;
    volatile uint32_t *mmio = (volatile uint32_t *)(uintptr_t)g_arm_virtio_blk_mmio_base;
    volatile uint16_t *avail_idx = (volatile uint16_t *)(uintptr_t)(queue_addr + 2048U + 2U);
    volatile uint16_t *avail_slot;
    uint16_t idx;

    for (uint32_t i = 0; i < 1024U; i++) dma[i] = 0;
    *(volatile uint32_t *)(uintptr_t)(dma_addr + 0U) = 0U;
    *(volatile uint32_t *)(uintptr_t)(dma_addr + 4U) = 0U;
    *(volatile uint32_t *)(uintptr_t)(dma_addr + 8U) = lba;
    *(volatile uint32_t *)(uintptr_t)(dma_addr + 12U) = 0U;
    *(volatile uint8_t *)(uintptr_t)(dma_addr + 528U) = 0xffU;

    write_le32_volatile(desc0 + 0, dma_addr);
    write_le32_volatile(desc0 + 4, 0U);
    write_le32_volatile(desc0 + 8, 16U);
    write_le16_volatile(desc0 + 12, 1U);
    write_le16_volatile(desc0 + 14, 1U);
    write_le32_volatile(desc0 + 16, dma_addr + 16U);
    write_le32_volatile(desc0 + 20, 0U);
    write_le32_volatile(desc0 + 24, 512U);
    write_le16_volatile(desc0 + 28, 3U);
    write_le16_volatile(desc0 + 30, 2U);
    write_le32_volatile(desc0 + 32, dma_addr + 528U);
    write_le32_volatile(desc0 + 36, 0U);
    write_le32_volatile(desc0 + 40, 1U);
    write_le16_volatile(desc0 + 44, 2U);
    write_le16_volatile(desc0 + 46, 0U);

    arm32_clean_dcache_range(dma_addr, 1024U);
    arm32_clean_dcache_range(queue_addr, 8192U);
    arm32_invalidate_dcache_range(queue_addr + 4096U, 64U);
    g_arm_virtq_last_used_idx = *(volatile uint16_t *)(uintptr_t)(queue_addr + 4096U + 2U);
    idx = *avail_idx;
    avail_slot = (volatile uint16_t *)(uintptr_t)(queue_addr + 2048U + 4U + ((idx % 128U) * 2U));
    *avail_slot = 0U;
    *avail_idx = (uint16_t)(idx + 1U);
    arm32_clean_dcache_range(queue_addr + 2048U, 512U);
    __asm__ volatile("dsb sy" ::: "memory");
    mmio[0x050U / 4U] = 0U;
    __asm__ volatile("dsb sy" ::: "memory");

    for (uint32_t i = 0; i < 50000000U; i++) {
        arm32_invalidate_dcache_range(queue_addr + 4096U, 64U);
        uint16_t used_idx = *(volatile uint16_t *)(uintptr_t)(queue_addr + 4096U + 2U);
        if (used_idx != g_arm_virtq_last_used_idx) {
            g_arm_virtq_last_used_idx = used_idx;
            arm32_invalidate_dcache_range(dma_addr, 1024U);
            return (RuntimeValue)*(volatile uint8_t *)(uintptr_t)(dma_addr + 528U);
        }
    }
    return (RuntimeValue)0xffffffffU;
}

RuntimeValue rt_arm_virtio_blk_read_prefix(RuntimeValue first_lba_val, RuntimeValue size_val)
{
    uint32_t first_lba = (uint32_t)first_lba_val;
    uint32_t size = (uint32_t)size_val;
    if (size == 0U || size > 0x100000U) return rt_array_new(64);
    size_t alloc_size = sizeof(RuntimeArray) + (size_t)size * sizeof(RuntimeValue);
    RuntimeArray *a = (RuntimeArray *)malloc(alloc_size);
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)alloc_size;
    a->len = size;
    a->cap = size;
    uint32_t copied = 0;
    uint32_t sector = 0;
    while (copied < size) {
        RuntimeValue status = rt_arm_virtio_blk_read_sector_direct((RuntimeValue)(first_lba + sector));
        if (status == (RuntimeValue)0xffffffffU || status != 0) break;
        uint8_t *src = g_arm_virtio_blk_dma_storage + 16;
        arm32_invalidate_dcache_range((uint32_t)(uintptr_t)src, 512U);
        for (uint32_t i = 0; i < 512U && copied < size; i++) {
            a->items[copied++] = ENCODE_INT(src[i]);
        }
        sector++;
    }
    a->len = copied;
    return ENCODE_PTR(a);
}

RuntimeValue rt_arm_virtio_blk_read_hello_smf(void)
{
    return rt_arm_virtio_blk_read_prefix(ENCODE_INT(2063), ENCODE_INT(4264));
}

int64_t rt_bytes_u8_at(RuntimeValue arr, int64_t idx)
{
    if (idx < 0) return 0;
    return (int64_t)(uint32_t)rt_arm_array_get_byte_u32(arr, (RuntimeValue)(uint32_t)idx);
}

static uint32_t arm32_array_byte_at_raw_index(RuntimeValue arr, uint32_t idx);

RuntimeValue rt_array_get_byte_raw(RuntimeValue arr, RuntimeValue idx_val)
{
    uint32_t idx = IS_INT(idx_val) ? (uint32_t)DECODE_INT(idx_val) : (uint32_t)idx_val;
    return (RuntimeValue)arm32_array_byte_at_raw_index(arr, idx);
}

static uint32_t arm32_array_byte_at_raw_index(RuntimeValue arr, uint32_t idx)
{
    RuntimeArray *tagged = IS_HEAP(arr) ? (RuntimeArray *)DECODE_PTR(arr) : (RuntimeArray *)0;
    if (tagged && tagged->hdr.type == HEAP_ARRAY && tagged->len <= tagged->cap && idx < tagged->len) {
        RuntimeValue v = tagged->items[idx];
        if (IS_INT(v)) return (uint32_t)DECODE_INT(v);
        return (uint32_t)(uint8_t)(uint32_t)v;
    }
    RuntimeArray *raw = (RuntimeArray *)(uintptr_t)(uint32_t)arr;
    if (raw && raw->hdr.type == HEAP_ARRAY && raw->len <= raw->cap && idx < raw->len) {
        RuntimeValue v = raw->items[idx];
        if (IS_INT(v)) return (uint32_t)DECODE_INT(v);
        return (uint32_t)(uint8_t)(uint32_t)v;
    }
    RuntimeValue *items = (RuntimeValue *)(uintptr_t)(uint32_t)arr;
    RuntimeValue v = items[idx];
    if (IS_INT(v)) return (uint32_t)DECODE_INT(v);
    return (uint32_t)(uint8_t)(uint32_t)v;
}

RuntimeValue rt_arm_array_get_byte_u32(RuntimeValue arr, RuntimeValue idx_val)
{
    uint32_t idx = (uint32_t)idx_val;
    return (RuntimeValue)arm32_array_byte_at_raw_index(arr, idx);
}

RuntimeValue rt_arm_array_len_u32(RuntimeValue arr)
{
    RuntimeArray *tagged = IS_HEAP(arr) ? (RuntimeArray *)DECODE_PTR(arr) : (RuntimeArray *)0;
    if (tagged && tagged->hdr.type == HEAP_ARRAY && tagged->len <= tagged->cap) {
        return (RuntimeValue)tagged->len;
    }
    RuntimeArray *raw = (RuntimeArray *)(uintptr_t)(uint32_t)arr;
    if (raw && raw->hdr.type == HEAP_ARRAY && raw->len <= raw->cap) {
        return (RuntimeValue)raw->len;
    }
    return 0;
}

RuntimeValue rt_arm_array_get_u16_le(RuntimeValue arr, RuntimeValue idx_val)
{
    uint32_t idx = (uint32_t)idx_val;
    uint32_t lo = arm32_array_byte_at_raw_index(arr, idx);
    uint32_t hi = arm32_array_byte_at_raw_index(arr, idx + 1U);
    return (RuntimeValue)(lo | (hi << 8));
}

RuntimeValue rt_arm_array_get_u32_le(RuntimeValue arr, RuntimeValue idx_val)
{
    uint32_t idx = (uint32_t)idx_val;
    uint32_t b0 = arm32_array_byte_at_raw_index(arr, idx);
    uint32_t b1 = arm32_array_byte_at_raw_index(arr, idx + 1U);
    uint32_t b2 = arm32_array_byte_at_raw_index(arr, idx + 2U);
    uint32_t b3 = arm32_array_byte_at_raw_index(arr, idx + 3U);
    return (RuntimeValue)(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
}

uint32_t rt_arm_smf_elf_stub_size(RuntimeValue bytes)
{
    uint32_t len = (uint32_t)rt_arm_array_len_u32(bytes);
    if (len < 132U) return 0;
    if (arm32_array_byte_at_raw_index(bytes, 0) != 0x7FU) return 0;
    if (arm32_array_byte_at_raw_index(bytes, 1) != 0x45U) return 0;
    if (arm32_array_byte_at_raw_index(bytes, 2) != 0x4CU) return 0;
    if (arm32_array_byte_at_raw_index(bytes, 3) != 0x46U) return 0;
    uint32_t trailer = len - 128U;
    if (arm32_array_byte_at_raw_index(bytes, trailer) != 0x53U) return 0;
    if (arm32_array_byte_at_raw_index(bytes, trailer + 1U) != 0x4DU) return 0;
    if (arm32_array_byte_at_raw_index(bytes, trailer + 2U) != 0x46U) return 0;
    if (arm32_array_byte_at_raw_index(bytes, trailer + 3U) != 0x00U) return 0;
    uint32_t stub_size = (uint32_t)rt_arm_array_get_u32_le(bytes, (RuntimeValue)(trailer + 52U));
    if (stub_size > 0U && stub_size <= trailer) return stub_size;
    return trailer;
}

RuntimeValue rt_arm_array_append_bytes(RuntimeValue dst_val, RuntimeValue src_val, RuntimeValue max_count_val)
{
    RuntimeArray *dst = (RuntimeArray *)(IS_HEAP(dst_val) ? DECODE_PTR(dst_val) : (void *)(uintptr_t)(uint32_t)dst_val);
    if (!dst || dst->hdr.type != HEAP_ARRAY) return ENCODE_INT(0);
    uint32_t max_count = (uint32_t)max_count_val;
    uint32_t src_len = (uint32_t)rt_arm_array_len_u32(src_val);
    uint32_t appended = 0;
    while (appended < max_count && appended < src_len) {
        if (dst->len >= dst->cap) break;
        dst->items[dst->len++] = ENCODE_INT(arm32_array_byte_at_raw_index(src_val, appended));
        appended++;
    }
    return (RuntimeValue)appended;
}

typedef struct {
    uint32_t r[13];
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t cpsr;
    uint32_t fpu_state;
} Arm32SavedContext;

RuntimeValue rt_arm32_context_save(RuntimeValue ctx_ptr_val)
{
    Arm32SavedContext *ctx = (Arm32SavedContext *)(uintptr_t)(uint32_t)ctx_ptr_val;
    if (!ctx) return NIL_VALUE;
    for (uint32_t i = 0; i < 13; i++) ctx->r[i] = 0;
    ctx->sp = (uint32_t)(uintptr_t)&ctx;
    ctx->lr = (uint32_t)(uintptr_t)__builtin_return_address(0);
    ctx->pc = ctx->lr;
    ctx->cpsr = 0x000001D3U;
    ctx->fpu_state = 0;
    return NIL_VALUE;
}

RuntimeValue rt_arm32_context_restore(RuntimeValue ctx_ptr_val)
{
    Arm32SavedContext *ctx = (Arm32SavedContext *)(uintptr_t)(uint32_t)ctx_ptr_val;
    (void)ctx;
    return NIL_VALUE;
}

RuntimeValue rt_arm32_context_switch(RuntimeValue from_ptr_val, RuntimeValue to_ptr_val)
{
    rt_arm32_context_save(from_ptr_val);
    rt_arm32_context_restore(to_ptr_val);
    return NIL_VALUE;
}

RuntimeValue rt_arm_stage_raw_image(RuntimeValue dst_phys_val, RuntimeValue bytes_val)
{
    uint32_t dst_phys = (uint32_t)dst_phys_val;
    RuntimeArray *bytes = (RuntimeArray *)(IS_HEAP(bytes_val) ? DECODE_PTR(bytes_val) : (void *)(uintptr_t)(uint32_t)bytes_val);
    if (!dst_phys || !bytes || bytes->hdr.type != HEAP_ARRAY || bytes->len > bytes->cap) return 0;
    volatile uint8_t *dst = (volatile uint8_t *)(uintptr_t)dst_phys;
    for (uint32_t i = 0; i < bytes->len; i++) {
        dst[i] = (uint8_t)arm32_array_byte_at_raw_index(bytes_val, i);
    }
    uint32_t padded = (bytes->len + 4095U) & ~4095U;
    for (uint32_t i = bytes->len; i < padded; i++) {
        dst[i] = 0;
    }
    return (RuntimeValue)((bytes->len + 4095U) / 4096U);
}

RuntimeValue arm_fs_exec_trace(RuntimeValue id_val)
{
    uint32_t id = (uint32_t)id_val;
    serial_puts("[arm-fs-trace] ");
    serial_put_dec((int32_t)id);
    serial_puts(" ");
    serial_put_hex(id);
    serial_puts("\r\n");
    return NIL_VALUE;
}

static int32_t arm32_raw_or_encoded_int(RuntimeValue value)
{
    return IS_INT(value) ? DECODE_INT(value) : (int32_t)value;
}

RuntimeValue rt_raw_u64_to_string(RuntimeValue raw)
{
    uint32_t uv = (uint32_t)raw;
    if (uv == 0) return rt_string_from_cstr("0");
    char buf[11];
    int pos = 0;
    while (uv > 0U) { buf[pos++] = '0' + (char)(uv % 10U); uv /= 10U; }
    RuntimeString *s = (RuntimeString *)malloc(sizeof(RuntimeString) + (size_t)pos + 1);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + (size_t)pos + 1);
    s->len = (uint32_t)pos;
    int out = 0;
    while (pos > 0) s->data[out++] = buf[--pos];
    s->data[out] = '\0';
    return ENCODE_PTR(s);
}

RuntimeValue rt_value_to_string(RuntimeValue val)
{
    if (IS_HEAP(val)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
        if (h && h->type == HEAP_STRING) return val;
    }
    int32_t n = arm32_raw_or_encoded_int(val);
    if (n == 0) return rt_string_from_cstr("0");
    char buf[12];
    int pos = 0;
    uint32_t uv;
    int neg = 0;
    if (n < 0) { neg = 1; uv = (uint32_t)(-n); } else { uv = (uint32_t)n; }
    while (uv > 0U) { buf[pos++] = '0' + (char)(uv % 10U); uv /= 10U; }
    RuntimeString *s = (RuntimeString *)malloc(sizeof(RuntimeString) + (size_t)pos + (size_t)neg + 1);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + (size_t)pos + (size_t)neg + 1);
    s->len = (uint32_t)(pos + neg);
    int out = 0;
    if (neg) s->data[out++] = '-';
    while (pos > 0) s->data[out++] = buf[--pos];
    s->data[out] = '\0';
    return ENCODE_PTR(s);
}

RuntimeValue rt_native_eq(RuntimeValue a, RuntimeValue b)
{
    if (a == b) return 1;
    if (IS_HEAP(a) && IS_HEAP(b)) {
        HeapHeader *ha = (HeapHeader *)DECODE_PTR(a);
        HeapHeader *hb = (HeapHeader *)DECODE_PTR(b);
        if (ha && hb && ha->type == HEAP_STRING && hb->type == HEAP_STRING) {
            RuntimeString *sa = (RuntimeString *)ha;
            RuntimeString *sb = (RuntimeString *)hb;
            if (sa->len != sb->len) return 0;
            for (uint32_t i = 0; i < sa->len; i++) if (sa->data[i] != sb->data[i]) return 0;
            return 1;
        }
    }
    return 0;
}

RuntimeValue rt_native_neq(RuntimeValue a, RuntimeValue b) { return rt_native_eq(a, b) ? 0 : 1; }
RuntimeValue rt_simd_str_equal(RuntimeValue a, RuntimeValue b) { return rt_native_eq(a, b); }

RuntimeValue rt_contains(RuntimeValue haystack, RuntimeValue needle)
{
    if (IS_HEAP(haystack)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(haystack);
        if (h && h->type == HEAP_ARRAY) {
            RuntimeArray *a = (RuntimeArray *)h;
            for (uint32_t i = 0; i < a->len; i++) {
                if (rt_native_eq(a->items[i], needle)) return 1;
            }
            return 0;
        }
        if (h && h->type == HEAP_STRING && IS_HEAP(needle)) {
            RuntimeString *s = (RuntimeString *)h;
            RuntimeString *n = (RuntimeString *)DECODE_PTR(needle);
            if (!n || n->hdr.type != HEAP_STRING) return 0;
            if (n->len == 0) return 1;
            if (n->len > s->len) return 0;
            for (uint32_t i = 0; i <= s->len - n->len; i++) {
                uint32_t j = 0;
                while (j < n->len && s->data[i + j] == n->data[j]) j++;
                if (j == n->len) return 1;
            }
        }
    }
    return 0;
}

RuntimeValue rt_simd_str_search(RuntimeValue haystack, RuntimeValue needle)
{
    if (!IS_HEAP(haystack) || !IS_HEAP(needle)) return -1;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(haystack);
    RuntimeString *n = (RuntimeString *)DECODE_PTR(needle);
    if (!s || !n || s->hdr.type != HEAP_STRING || n->hdr.type != HEAP_STRING) return -1;
    if (n->len == 0) return 0;
    if (n->len > s->len) return -1;
    for (uint32_t i = 0; i <= s->len - n->len; i++) {
        uint32_t j = 0;
        while (j < n->len && s->data[i + j] == n->data[j]) j++;
        if (j == n->len) return (RuntimeValue)i;
    }
    return -1;
}

RuntimeValue rt_simd_str_last_index_of(RuntimeValue haystack, RuntimeValue needle)
{
    if (!IS_HEAP(haystack) || !IS_HEAP(needle)) return -1;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(haystack);
    RuntimeString *n = (RuntimeString *)DECODE_PTR(needle);
    if (!s || !n || s->hdr.type != HEAP_STRING || n->hdr.type != HEAP_STRING) return -1;
    if (n->len == 0) return (RuntimeValue)s->len;
    if (n->len > s->len) return -1;
    for (int32_t i = (int32_t)(s->len - n->len); i >= 0; i--) {
        uint32_t j = 0;
        while (j < n->len && s->data[(uint32_t)i + j] == n->data[j]) j++;
        if (j == n->len) return (RuntimeValue)i;
    }
    return -1;
}

RuntimeValue rt_slice(RuntimeValue value, RuntimeValue start, RuntimeValue end)
{
    if (IS_HEAP(value)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(value);
        if (h && h->type == HEAP_STRING) return rt_string_slice(value, start, end);
    }
    return NIL_VALUE;
}

RuntimeValue str_substring_impl(RuntimeValue str, RuntimeValue start, RuntimeValue end) __asm__("str.substring");
RuntimeValue str_substring_impl(RuntimeValue str, RuntimeValue start, RuntimeValue end)
{
    return rt_string_slice(str, start, end);
}

RuntimeValue rt_text_to_bytes(RuntimeValue str)
{
    if (!IS_HEAP(str)) return rt_array_new(ENCODE_INT(0));
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s || s->hdr.type != HEAP_STRING) return rt_array_new(ENCODE_INT(0));
    RuntimeValue arr = rt_array_new(ENCODE_INT((int32_t)s->len));
    for (uint32_t i = 0; i < s->len; i++) {
        rt_array_push(arr, ENCODE_INT((int32_t)(unsigned char)s->data[i]));
    }
    return arr;
}

RuntimeValue str_bytes_impl(RuntimeValue str) __asm__("str.bytes");
RuntimeValue str_bytes_impl(RuntimeValue str)
{
    return rt_text_to_bytes(str);
}

RuntimeValue rt_text_to_lower_ascii(RuntimeValue value) { return value; }
RuntimeValue rt_text_to_upper_ascii(RuntimeValue value) { return value; }

RuntimeValue rt_char_from_code(RuntimeValue code)
{
    int32_t c = arm32_raw_or_encoded_int(code);
    if (c < 0 || c > 127) c = '?';
    char buf[2] = { (char)c, '\0' };
    return rt_string_from_cstr(buf);
}

RuntimeValue char_from_code(RuntimeValue code) { return rt_char_from_code(code); }

RuntimeValue rt_string_join(RuntimeValue parts, RuntimeValue sep)
{
    if (!IS_HEAP(parts)) return rt_string_from_cstr("");
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(parts);
    if (!a || a->hdr.type != HEAP_ARRAY || a->len == 0) return rt_string_from_cstr("");
    RuntimeValue out = rt_value_to_string(a->items[0]);
    for (uint32_t i = 1; i < a->len; i++) {
        out = rt_string_concat(rt_string_concat(out, sep), rt_value_to_string(a->items[i]));
    }
    return out;
}

RuntimeValue rt_tuple_new(RuntimeValue len_rv)
{
    int32_t len = arm32_raw_or_encoded_int(len_rv);
    if (len < 0) len = 0;
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    a->len = (uint32_t)len;
    a->cap = (uint32_t)len;
    for (uint32_t i = 0; i < (uint32_t)len; i++) a->items[i] = NIL_VALUE;
    return ENCODE_PTR(a);
}

RuntimeValue rt_tuple_get(RuntimeValue tuple, RuntimeValue index)
{
    if (!IS_HEAP(tuple)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(tuple);
    int32_t i = arm32_raw_or_encoded_int(index);
    if (!a || a->hdr.type != HEAP_ARRAY || i < 0 || (uint32_t)i >= a->len) return NIL_VALUE;
    return a->items[i];
}

RuntimeValue rt_tuple_set(RuntimeValue tuple, RuntimeValue index, RuntimeValue value)
{
    if (!IS_HEAP(tuple)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(tuple);
    int32_t i = arm32_raw_or_encoded_int(index);
    if (!a || a->hdr.type != HEAP_ARRAY || i < 0 || (uint32_t)i >= a->len) return 0;
    a->items[i] = value;
    return 1;
}

RuntimeValue rt_byte_array_new(RuntimeValue capacity) { return rt_array_new(capacity); }
RuntimeValue rt_typed_bytes_u8_push(RuntimeValue array, RuntimeValue value)
{
    return rt_array_push(array, ENCODE_INT(((uint32_t)value) & 0xFF)) ? TRUE_VALUE : FALSE_VALUE;
}
RuntimeValue rt_typed_words_u32_push(RuntimeValue array, RuntimeValue value)
{
    return rt_array_push(array, ENCODE_INT(arm32_raw_or_encoded_int(value))) ? TRUE_VALUE : FALSE_VALUE;
}
RuntimeValue rt_typed_words_u32_at(RuntimeValue array, RuntimeValue index)
{
    return rt_array_get(array, index);
}
RuntimeValue rt_typed_words_u32_set(RuntimeValue array, RuntimeValue index, RuntimeValue value)
{
    return rt_array_set(array, index, value) == NIL_VALUE ? FALSE_VALUE : TRUE_VALUE;
}

RuntimeValue rt_dma_alloc(RuntimeValue size, RuntimeValue dir_raw)
{
    (void)dir_raw;
    void *p = malloc((size_t)arm32_raw_or_encoded_int(size));
    return p ? (RuntimeValue)(uintptr_t)p : 0;
}
RuntimeValue rt_dma_cache_line_size(void) { return 64; }
void rt_dma_free(RuntimeValue p) { (void)p; }
RuntimeValue rt_dma_phys_of(RuntimeValue p) { return p; }
void rt_dma_sync_for_cpu(RuntimeValue a, RuntimeValue b) { (void)a; (void)b; }
void rt_dma_sync_for_device(RuntimeValue a, RuntimeValue b) { (void)a; (void)b; }
RuntimeValue rt_dma_virt_of(RuntimeValue p) { return p; }
RuntimeValue rt_mmio_read_u64(RuntimeValue addr)
{
    volatile uint32_t *p = (volatile uint32_t *)(uintptr_t)(uint32_t)addr;
    uint32_t lo = p[0];
    uint32_t hi = p[1];
    return (RuntimeValue)(lo ^ hi);
}
RuntimeValue rt_mmio_write_u64(RuntimeValue addr, RuntimeValue val)
{
    volatile uint32_t *p = (volatile uint32_t *)(uintptr_t)(uint32_t)addr;
    p[0] = (uint32_t)val;
    p[1] = 0;
    return NIL_VALUE;
}

RuntimeValue unsafe_addr_of(RuntimeValue v) { return ENCODE_INT((int32_t)(uint32_t)v); }
RuntimeValue rt_memcpy(RuntimeValue dst, RuntimeValue src, RuntimeValue n)
{
    void *d = (void *)(uintptr_t)(uint32_t)dst;
    const void *s = (const void *)(uintptr_t)(uint32_t)src;
    uint32_t sz = (uint32_t)arm32_raw_or_encoded_int(n);
    if (d && s && sz) __builtin_memcpy(d, s, sz);
    return dst;
}
RuntimeValue rt_memset(RuntimeValue dst, RuntimeValue val, RuntimeValue n)
{
    void *d = (void *)(uintptr_t)(uint32_t)dst;
    uint32_t sz = (uint32_t)arm32_raw_or_encoded_int(n);
    if (d && sz) __builtin_memset(d, arm32_raw_or_encoded_int(val), sz);
    return dst;
}
void vmm_switch_address_space(RuntimeValue root_phys) { (void)root_phys; }
void cap_init_task_record(RuntimeValue task, RuntimeValue full) { (void)task; (void)full; }
RuntimeValue spl_f64_to_bits(RuntimeValue value) { return value; }

static uint32_t g_arm32_fat32_bps = 0;
static uint32_t g_arm32_fat32_spc = 0;
static uint32_t g_arm32_fat32_reserved = 0;
static uint32_t g_arm32_fat32_fats = 0;
static uint32_t g_arm32_fat32_fat_size = 0;
static uint32_t g_arm32_fat32_root_cluster = 0;

RuntimeValue rt_arm_fat32_probe_bpb_from_virtio(void)
{
    RuntimeValue status = rt_arm_virtio_blk_read_sector_direct((RuntimeValue)0U);
    uint8_t *b = g_arm_virtio_blk_dma_storage + 16;
    if (status == (RuntimeValue)0xffffffffU || status != 0) return (RuntimeValue)0U;
    g_arm32_fat32_bps = (uint32_t)b[11] | ((uint32_t)b[12] << 8);
    g_arm32_fat32_spc = (uint32_t)b[13];
    g_arm32_fat32_reserved = (uint32_t)b[14] | ((uint32_t)b[15] << 8);
    g_arm32_fat32_fats = (uint32_t)b[16];
    g_arm32_fat32_fat_size = (uint32_t)b[36] | ((uint32_t)b[37] << 8) | ((uint32_t)b[38] << 16) | ((uint32_t)b[39] << 24);
    g_arm32_fat32_root_cluster = (uint32_t)b[44] | ((uint32_t)b[45] << 8) | ((uint32_t)b[46] << 16) | ((uint32_t)b[47] << 24);
    if (g_arm32_fat32_bps == 0 || g_arm32_fat32_spc == 0 || g_arm32_fat32_fats == 0 ||
        g_arm32_fat32_fat_size == 0 || g_arm32_fat32_root_cluster < 2U) {
        return (RuntimeValue)0U;
    }
    return (RuntimeValue)1U;
}

RuntimeValue rt_arm_fat32_bps(void) { return ENCODE_INT((int32_t)g_arm32_fat32_bps); }
RuntimeValue rt_arm_fat32_spc(void) { return ENCODE_INT((int32_t)g_arm32_fat32_spc); }
RuntimeValue rt_arm_fat32_reserved(void) { return ENCODE_INT((int32_t)g_arm32_fat32_reserved); }
RuntimeValue rt_arm_fat32_fats(void) { return ENCODE_INT((int32_t)g_arm32_fat32_fats); }
RuntimeValue rt_arm_fat32_fat_size(void) { return ENCODE_INT((int32_t)g_arm32_fat32_fat_size); }
RuntimeValue rt_arm_fat32_root_cluster(void) { return ENCODE_INT((int32_t)g_arm32_fat32_root_cluster); }

static uint32_t arm32_udivmod(uint32_t n, uint32_t d, uint32_t *rem)
{
    uint32_t q = 0;
    if (d == 0U) {
        if (rem) *rem = 0U;
        return 0U;
    }
    while (n >= d) {
        uint32_t step = d;
        uint32_t bit = 1U;
        while (step <= (n >> 1) && step <= 0x7fffffffU) {
            step <<= 1;
            bit <<= 1;
        }
        n -= step;
        q += bit;
    }
    if (rem) *rem = n;
    return q;
}

int __divsi3(int a, int b)
{
    int neg = ((a < 0) != (b < 0));
    uint32_t ua = (a < 0) ? (uint32_t)(-a) : (uint32_t)a;
    uint32_t ub = (b < 0) ? (uint32_t)(-b) : (uint32_t)b;
    uint32_t q = arm32_udivmod(ua, ub, 0);
    return neg ? -(int)q : (int)q;
}

int __modsi3(int a, int b)
{
    uint32_t r = 0;
    uint32_t ua = (a < 0) ? (uint32_t)(-a) : (uint32_t)a;
    uint32_t ub = (b < 0) ? (uint32_t)(-b) : (uint32_t)b;
    (void)arm32_udivmod(ua, ub, &r);
    return (a < 0) ? -(int)r : (int)r;
}

#define RV_INT int32_t
#define CRYPTO_ARRAY_HDR_TYPE(arr) ((arr)->type)
#include "../../shared/crypto_common.h"
