/*
 * SimpleOS x86_32 (i686) Baremetal Runtime Stubs
 *
 * Provides a complete freestanding runtime for the Simple language compiler
 * targeting x86_32 bare-metal (QEMU isa-debug-exit, COM1 serial).
 *
 * Differences from x86_64:
 *   - RuntimeValue is int32_t (32-bit pointers)
 *   - Heap is 4MB (not 64MB)
 *   - No 64-bit register instructions
 *
 * Sections:
 *   1. Includes and types
 *   2. Serial I/O (COM1 0x3F8)
 *   3. RuntimeValue tagging
 *   4. Heap allocator (bump, 4 MB)
 *   5. Memory functions
 *   6. String operations
 *   7. Print functions
 *   8. Framebuffer copy
 *   9. _start (serial init, call spl_start, isa-debug-exit)
 *  10. No-op stubs (~200 runtime functions)
 *  11. Real x86_32 port-I/O and MMIO overrides
 */

/* ===================================================================
 * 1. Includes and types
 * =================================================================== */

#include <stdint.h>
#include <stddef.h>

typedef int32_t RuntimeValue;

/* ===================================================================
 * 2. Serial I/O — COM1 at 0x3F8 via x86 outb / inb
 * =================================================================== */

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t r;
    __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

static inline void outw(uint16_t port, uint16_t val)
{
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t r;
    __asm__ volatile("inw %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

static inline void outl(uint16_t port, uint32_t val)
{
    __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t r;
    __asm__ volatile("inl %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

static void serial_putchar(char c)
{
    /* Wait until transmit holding register is empty (bit 5 of LSR) */
    while (!(inb(0x3F8 + 5) & 0x20)) {}
    outb(0x3F8, (uint8_t)c);
}

static void serial_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') serial_putchar('\r');
        serial_putchar(*s++);
    }
}

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

static uint32_t x86_32_harden_mix32(uint32_t value)
{
    value ^= value >> 16;
    value *= 0x7feb352dU;
    value ^= value >> 15;
    value *= 0x846ca68bU;
    value ^= value >> 16;
    return value & 0x7fffffffU;
}

static uint32_t x86_32_harden_canary_value(void)
{
    uint32_t lo = 0;
    uint32_t hi = 0;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    uint32_t mixed = x86_32_harden_mix32(lo ^ (hi << 11) ^ (uint32_t)(uintptr_t)&x86_32_harden_canary_value);
    return mixed == 0U ? 1U : mixed;
}

static void x86_32_harden_print_canary(void)
{
    serial_puts("[harden] canary arch=x86_32 value=");
    serial_put_u32(x86_32_harden_canary_value());
    serial_puts("\r\n");
}

/* ===================================================================
 * 3. RuntimeValue tagging (32-bit)
 * =================================================================== */

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

/* ===================================================================
 * 4. Heap allocator — bump allocator, 4 MB
 * =================================================================== */

static char   _heap[4 * 1024 * 1024] __attribute__((aligned(16)));
static size_t _heap_off = 0;

void *malloc(size_t sz)
{
    sz = (sz + 15) & ~(size_t)15;
    if (_heap_off + sz > sizeof(_heap)) {
        serial_puts("[PANIC] heap exhausted\r\n");
        for(;;) outb(0xF4, 0);
    }
    void *p = &_heap[_heap_off];
    _heap_off += sz;
    return p;
}

void free(void *p)
{
    (void)p; /* bump allocator: no-op */
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

/* ===================================================================
 * 5. Memory functions — freestanding replacements
 * =================================================================== */

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

/* ===================================================================
 * 6. String operations
 * =================================================================== */

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
            for (uint32_t i = 0; i < sa->len; i++) {
                if (sa->data[i] != sb->data[i]) return 0;
            }
            return 1;
        }
    }
    return 0;
}

RuntimeValue rt_native_neq(RuntimeValue a, RuntimeValue b)
{
    return rt_native_eq(a, b) ? 0 : 1;
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
    if (IS_INT(v)) return ENCODE_INT(0);
    if (!IS_HEAP(v)) return ENCODE_INT(0);
    HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
    if (!h) return ENCODE_INT(0);
    if (h->type == HEAP_STRING) {
        RuntimeString *s = (RuntimeString *)h;
        return ENCODE_INT(s->len);
    }
    if (h->type == HEAP_ARRAY) {
        RuntimeArray *a = (RuntimeArray *)h;
        return ENCODE_INT(a->len);
    }
    return ENCODE_INT(0);
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

/* ===================================================================
 * 7. Print functions
 * =================================================================== */

void rt_print_str(RuntimeValue str)
{
    if (IS_HEAP(str)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
        if (s && s->hdr.type == HEAP_STRING && s->len < 0x100000) {
            for (uint32_t i = 0; i < s->len; i++) serial_putchar(s->data[i]);
            return;
        }
    }
    /* Fallback: try as raw pointer */
    if (str != 0) {
        RuntimeString *s = (RuntimeString *)(uintptr_t)str;
        if (s->hdr.type == HEAP_STRING && s->len < 0x100000) {
            for (uint32_t i = 0; i < s->len; i++) serial_putchar(s->data[i]);
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
        /* Try as raw pointer */
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

/* ===================================================================
 * 8. Framebuffer copy
 * =================================================================== */

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

/* ===================================================================
 * 9. _start — serial init, spl_start, isa-debug-exit
 * =================================================================== */

static void _serial_init(void)
{
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x01);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

extern void spl_start(void) __attribute__((weak));

static uint32_t x86_32_initrd_start = 0;
static uint32_t x86_32_initrd_end = 0;

static void x86_32_capture_multiboot_modules(uint32_t magic, uint32_t mbi_addr)
{
    if (magic != 0x2BADB002U || mbi_addr == 0U) return;
    uint32_t *mbi = (uint32_t *)(uintptr_t)mbi_addr;
    uint32_t flags = mbi[0];
    if ((flags & (1U << 3)) == 0U) return;
    uint32_t mods_count = mbi[5];
    uint32_t mods_addr = mbi[6];
    if (mods_count == 0U || mods_addr == 0U) return;
    uint32_t *mod = (uint32_t *)(uintptr_t)mods_addr;
    if (mod[1] > mod[0]) {
        x86_32_initrd_start = mod[0];
        x86_32_initrd_end = mod[1];
    }
}

static int x86_32_initrd_contains_ascii(const char *needle)
{
    if (x86_32_initrd_start == 0U || x86_32_initrd_end <= x86_32_initrd_start) return 0;
    const uint8_t *base = (const uint8_t *)(uintptr_t)x86_32_initrd_start;
    uint32_t len = x86_32_initrd_end - x86_32_initrd_start;
    uint32_t needle_len = 0;
    while (needle[needle_len] != '\0') needle_len++;
    if (needle_len == 0U || len < needle_len) return 0;
    for (uint32_t i = 0; i + needle_len <= len; i++) {
        uint32_t j = 0;
        while (j < needle_len && base[i + j] == (uint8_t)needle[j]) j++;
        if (j == needle_len) return 1;
    }
    return 0;
}

RuntimeValue rt_x86_32_initrd_present(void)
{
    return (x86_32_initrd_end > x86_32_initrd_start) ? 1 : 0;
}

RuntimeValue rt_x86_32_initrd_contains_hello_smf(void)
{
    return x86_32_initrd_contains_ascii("HELLOSMF") ? 1 : 0;
}

RuntimeValue rt_x86_32_initrd_contains_browser_smf(void)
{
    return x86_32_initrd_contains_ascii("BROWSMF") ? 1 : 0;
}

RuntimeValue rt_x86_32_initrd_contains_x86_32_marker(void)
{
    if (x86_32_initrd_contains_ascii("SIMPLEOS_X86_32_HELLO_ELF")) return 1;
    return x86_32_initrd_contains_ascii("elf-machine=x86_32") ? 1 : 0;
}

void _start(uint32_t multiboot_magic, uint32_t multiboot_info)
{
    x86_32_capture_multiboot_modules(multiboot_magic, multiboot_info);
    _serial_init();

    serial_puts("SimpleOS x86_32 boot\r\n");
    serial_puts("[BOOT] COM1 serial initialized at 115200 baud\r\n");
    serial_puts("[BOOT] Heap: 4 MB bump allocator\r\n");
    serial_puts("[BOOT] RuntimeValue: tagged 32-bit (int/heap/float/special)\r\n");
    x86_32_harden_print_canary();

    if (spl_start) {
        serial_puts("[BOOT] Calling spl_start()...\r\n");
        spl_start();
        serial_puts("[BOOT] spl_start() returned\r\n");
    } else {
        serial_puts("[BOOT] No spl_start() found (weak symbol)\r\n");
    }

    serial_puts("[BOOT] x86_32 boot complete\r\n");

    /* isa-debug-exit: writing to port 0xF4 causes QEMU to exit */
    outb(0xF4, 0x00);

    /* If that didn't work, halt forever */
    for (;;) {
        __asm__ volatile("hlt");
    }
}

/* ===================================================================
 * 9b. Enum / Optional / Result operations
 * =================================================================== */

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

/* ===================================================================
 * 10. No-op stubs — macro-generated runtime function stubs
 * =================================================================== */

#define S0(n) RuntimeValue n(void) { return 0; }
#define S1(n) RuntimeValue n(RuntimeValue a) { (void)a; return 0; }
#define S2(n) RuntimeValue n(RuntimeValue a, RuntimeValue b) { (void)a; (void)b; return 0; }
#define S3(n) RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c) { (void)a; (void)b; (void)c; return 0; }
#define S4(n) RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d) { (void)a; (void)b; (void)c; (void)d; return 0; }
#define S5(n) RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d, RuntimeValue e) { (void)a; (void)b; (void)c; (void)d; (void)e; return 0; }

#define V0(n) void n(void) {}
#define V1(n) void n(RuntimeValue a) { (void)a; }
#define V2(n) void n(RuntimeValue a, RuntimeValue b) { (void)a; (void)b; }

/* --- Arithmetic / comparison --- */
S2(rt_add) S2(rt_sub) S2(rt_mul) S2(rt_div) S2(rt_mod) S2(rt_pow)
S2(rt_eq) S2(rt_ne) S2(rt_lt) S2(rt_gt) S2(rt_le) S2(rt_ge)
S2(rt_and) S2(rt_or) S1(rt_not)
S2(rt_shl) S2(rt_shr) S2(rt_bitand) S2(rt_bitor) S2(rt_bitxor)
S1(rt_bitnot) S1(rt_neg)

/* --- Type introspection / conversion --- */
S1(rt_type_of) S1(rt_is_nil) S1(rt_is_int) S1(rt_is_float)
S1(rt_is_string) S1(rt_is_bool) S1(rt_is_array) S1(rt_is_map)
S1(rt_is_object) S1(rt_to_int) S1(rt_to_float) S1(rt_to_string)
S1(rt_to_bool) S1(rt_clone) S1(rt_freeze) S1(rt_is_frozen)

/* --- String extras --- */
S2(rt_string_contains) S2(rt_string_starts_with) S2(rt_string_ends_with)
S2(rt_string_index_of) S2(rt_string_last_index_of)
S2(rt_string_substr) S2(rt_string_split)
S1(rt_string_trim) S1(rt_string_trim_start) S1(rt_string_trim_end)
S1(rt_string_to_upper) S1(rt_string_to_lower)
S2(rt_string_replace) S3(rt_string_replace_all) S2(rt_string_repeat)
S2(rt_string_pad_start) S2(rt_string_pad_end)
S1(rt_string_reverse) S1(rt_string_chars) S1(rt_string_bytes)
S1(rt_string_is_empty) S2(rt_string_compare) S2(rt_string_format)

/* --- Array --- */
S1(rt_array_new) S2(rt_array_push) S1(rt_array_pop)
S2(rt_array_get) S3(rt_array_set) S1(rt_array_len)
S3(rt_array_slice) S2(rt_array_contains) S2(rt_array_index_of)
S2(rt_array_last_index_of) S2(rt_array_remove) S3(rt_array_insert)
S1(rt_array_reverse) S1(rt_array_sort) S2(rt_array_sort_by)
S2(rt_array_map) S2(rt_array_filter) S3(rt_array_reduce)
S2(rt_array_for_each) S2(rt_array_find) S2(rt_array_find_index)
S2(rt_array_every) S2(rt_array_some) S2(rt_array_join)
S2(rt_array_concat) S1(rt_array_clear) S1(rt_array_flatten)
S2(rt_array_fill) S1(rt_array_clone) S2(rt_array_zip)
S1(rt_array_uniq) S1(rt_array_compact)

/* --- Map / Dictionary --- */
S0(rt_map_new) S3(rt_map_set) S2(rt_map_get) S2(rt_map_has)
S2(rt_map_remove) S1(rt_map_keys) S1(rt_map_values)
S1(rt_map_entries) S1(rt_map_len) S1(rt_map_clear)
S1(rt_map_clone) S2(rt_map_merge) S2(rt_map_for_each)

/* --- File I/O --- */
S1(rt_file_read) S2(rt_file_write) S1(rt_file_exists) S1(rt_file_delete)
S2(rt_file_append) S1(rt_file_size) S2(rt_file_copy) S2(rt_file_move)
S2(rt_file_rename) S1(rt_file_is_dir) S1(rt_file_is_file)
S1(rt_file_read_bytes) S2(rt_file_write_bytes) S1(rt_file_stat) S1(rt_file_realpath)

/* --- Directory I/O --- */
S1(rt_dir_list) S1(rt_dir_create) S1(rt_dir_create_all)
S1(rt_dir_exists) S1(rt_dir_remove) S1(rt_dir_remove_all)
S0(rt_dir_cwd) S1(rt_dir_chdir) S0(rt_dir_home) S0(rt_dir_temp)

/* --- Process --- */
S2(rt_process_run) S3(rt_process_run_timeout) S1(rt_process_spawn)
S1(rt_process_kill) S1(rt_process_wait) S0(rt_process_pid)
S1(rt_cli_get_args) S0(rt_cli_args) S0(rt_exit_code)
S1(rt_exit) S1(rt_env_get) S2(rt_env_set) S0(rt_env_all)

/* --- Math --- */
S1(rt_math_sqrt) S1(rt_math_sin) S1(rt_math_cos) S1(rt_math_tan)
S1(rt_math_asin) S1(rt_math_acos) S1(rt_math_atan) S2(rt_math_atan2)
S1(rt_math_abs) S1(rt_math_floor) S1(rt_math_ceil) S1(rt_math_round)
S1(rt_math_log) S1(rt_math_log2) S1(rt_math_log10) S1(rt_math_exp)
S2(rt_math_min) S2(rt_math_max) S2(rt_math_pow) S0(rt_math_random)
S0(rt_math_pi) S0(rt_math_e) S0(rt_math_inf) S0(rt_math_nan)
S1(rt_math_is_nan) S1(rt_math_is_inf)

/* --- Interrupts --- */
S2(rt_register_isr) S1(rt_send_eoi) S0(rt_get_interrupt_flag)

/* --- Timer / Clock --- */
S1(rt_time_now_ms) S0(rt_time_now_nanos) S0(rt_time_monotonic)
S1(rt_sleep_ms) S1(rt_timer_create) S1(rt_timer_cancel)

/* --- Network --- */
S2(rt_net_connect) S1(rt_net_listen) S2(rt_net_send) S1(rt_net_recv)
S1(rt_net_close) S2(rt_net_bind) S1(rt_net_accept)
S2(rt_net_set_timeout) S1(rt_net_get_addr)

/* --- HTTP --- */
S2(rt_http_get) S3(rt_http_post) S3(rt_http_put) S3(rt_http_patch)
S2(rt_http_delete) S2(rt_http_request) S3(rt_http_request_full)
S2(rt_http_set_header)

/* --- JSON --- */
S1(rt_json_parse) S1(rt_json_stringify) S2(rt_json_get) S3(rt_json_set)
S1(rt_json_keys) S1(rt_json_values) S1(rt_json_is_object) S1(rt_json_is_array)

/* --- Regex --- */
S2(ffi_regex_is_match) S2(ffi_regex_find) S2(ffi_regex_find_all)
S2(ffi_regex_replace) S3(ffi_regex_replace_all) S1(ffi_regex_compile)

/* --- Test / BDD --- */
S1(rt_bdd_describe_start) S1(rt_bdd_describe_end)
S2(rt_bdd_it_start) S1(rt_bdd_it_end)
S1(rt_expect) S2(rt_expect_eq) S2(rt_expect_ne)
S2(rt_expect_gt) S2(rt_expect_lt)
S1(rt_expect_nil) S1(rt_expect_not_nil)
S1(rt_expect_true) S1(rt_expect_false)
S2(rt_expect_contains) S2(rt_expect_throws)
S0(rt_bdd_suite_start) S0(rt_bdd_suite_end) S0(rt_bdd_report)

/* --- Misc / Debug --- */
S1(rt_hash) S2(rt_hash_combine) S1(rt_debug_print) S1(rt_debug_dump)
S0(rt_debug_break) S1(rt_panic) S1(rt_assert) S2(rt_assert_eq)
S2(rt_assert_ne) S1(rt_abort)
S0(rt_gc_collect) S0(rt_gc_disable) S0(rt_gc_enable) S0(rt_gc_stats)
S1(rt_typeof)

/* --- Threading --- */
S1(rt_thread_create) S1(rt_thread_join) S0(rt_thread_yield)
S0(rt_thread_current) S1(rt_thread_sleep)
S0(rt_mutex_new) S1(rt_mutex_lock) S1(rt_mutex_unlock) S1(rt_mutex_try_lock)
S0(rt_condvar_new) S1(rt_condvar_wait) S1(rt_condvar_notify) S1(rt_condvar_notify_all)

/* --- Channels --- */
S0(rt_channel_new) S2(rt_channel_send) S1(rt_channel_recv)
S1(rt_channel_try_recv) S1(rt_channel_close)

/* --- Async --- */
S1(rt_async_spawn) S1(rt_async_await) S0(rt_async_yield) S2(rt_async_select)

/* --- Encoding --- */
S1(rt_base64_encode) S1(rt_base64_decode) S1(rt_hex_encode) S1(rt_hex_decode)
S1(rt_utf8_encode) S1(rt_utf8_decode) S1(rt_url_encode) S1(rt_url_decode)

/* --- Crypto --- */
S1(rt_sha256) S1(rt_sha512) S1(rt_md5) S2(rt_hmac_sha256) S1(rt_random_bytes)

/* --- Object / Struct --- */
S1(rt_object_new) S2(rt_object_get) S3(rt_object_set) S2(rt_object_has)
S2(rt_object_delete) S1(rt_object_keys) S1(rt_object_values)
S1(rt_object_freeze) S1(rt_object_clone)

/* --- Error handling --- */
S1(rt_error_new) S1(rt_error_message) S1(rt_error_code) S1(rt_error_stack)
S2(rt_result_ok) S2(rt_result_err) S1(rt_result_is_ok) S1(rt_result_is_err)
S1(rt_result_unwrap) S2(rt_result_unwrap_or)

/* --- Weak references & closures --- */
S1(rt_weak_ref) S1(rt_weak_deref)
S1(rt_closure_new) S2(rt_closure_call) S1(rt_closure_bind)

/* ===================================================================
 * 11. Real x86_32 port-I/O, MMIO, and CPU overrides
 * =================================================================== */

/* --- Port I/O: real x86 implementations --- */

RuntimeValue rt_port_outb_real(RuntimeValue port, RuntimeValue val)
{
    outb((uint16_t)port, (uint8_t)val);
    return NIL_VALUE;
}

RuntimeValue rt_port_outw_real(RuntimeValue port, RuntimeValue val)
{
    outw((uint16_t)port, (uint16_t)val);
    return NIL_VALUE;
}

RuntimeValue rt_port_outl_real(RuntimeValue port, RuntimeValue val)
{
    outl((uint16_t)port, (uint32_t)val);
    return NIL_VALUE;
}

RuntimeValue rt_port_inb_real(RuntimeValue port)
{
    return (RuntimeValue)inb((uint16_t)port);
}

RuntimeValue rt_port_inw_real(RuntimeValue port)
{
    return (RuntimeValue)inw((uint16_t)port);
}

RuntimeValue rt_port_inl_real(RuntimeValue port)
{
    return (RuntimeValue)inl((uint16_t)port);
}

RuntimeValue rt_port_io_wait_real(void)
{
    io_wait();
    return NIL_VALUE;
}

/* --- int 0x80 live trap proof support --- */

typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} X86_32IdtEntry;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} X86_32IdtPointer;

extern int32_t x86_32_dispatch_installed_syscall_abi(
    uint32_t id,
    uint32_t arg0,
    uint32_t arg1,
    uint32_t arg2,
    uint32_t arg3,
    uint32_t arg4,
    uint32_t arg5
) __attribute__((weak));
extern int32_t arch__x86_32__int80_probe_entry__x86_32_dispatch_installed_syscall_abi(
    uint32_t id,
    uint32_t arg0,
    uint32_t arg1,
    uint32_t arg2,
    uint32_t arg3,
    uint32_t arg4,
    uint32_t arg5
) __attribute__((weak));
extern int32_t arch__x86_32__initrd_fs_exec_probe_entry__x86_32_dispatch_installed_syscall_abi(
    uint32_t id,
    uint32_t arg0,
    uint32_t arg1,
    uint32_t arg2,
    uint32_t arg3,
    uint32_t arg4,
    uint32_t arg5
) __attribute__((weak));
extern int32_t os__kernel__arch__x86_32__early_syscall__x86_32_dispatch_installed_syscall_abi(
    uint32_t id,
    uint32_t arg0,
    uint32_t arg1,
    uint32_t arg2,
    uint32_t arg3,
    uint32_t arg4,
    uint32_t arg5
) __attribute__((weak));

static X86_32IdtEntry x86_32_probe_idt[256] __attribute__((aligned(8)));
static X86_32IdtPointer x86_32_probe_idtr;

int32_t simpleos_x86_32_dispatch_int80_probe(
    uint32_t id,
    uint32_t arg0,
    uint32_t arg1,
    uint32_t arg2,
    uint32_t arg3,
    uint32_t arg4,
    uint32_t arg5
)
{
    if (os__kernel__arch__x86_32__early_syscall__x86_32_dispatch_installed_syscall_abi) {
        return os__kernel__arch__x86_32__early_syscall__x86_32_dispatch_installed_syscall_abi(
            id, arg0, arg1, arg2, arg3, arg4, arg5
        );
    }
    if (x86_32_dispatch_installed_syscall_abi) {
        return x86_32_dispatch_installed_syscall_abi(id, arg0, arg1, arg2, arg3, arg4, arg5);
    }
    if (arch__x86_32__int80_probe_entry__x86_32_dispatch_installed_syscall_abi) {
        return arch__x86_32__int80_probe_entry__x86_32_dispatch_installed_syscall_abi(
            id, arg0, arg1, arg2, arg3, arg4, arg5
        );
    }
    if (arch__x86_32__initrd_fs_exec_probe_entry__x86_32_dispatch_installed_syscall_abi) {
        return arch__x86_32__initrd_fs_exec_probe_entry__x86_32_dispatch_installed_syscall_abi(
            id, arg0, arg1, arg2, arg3, arg4, arg5
        );
    }
    return -38;
}

__attribute__((naked)) void x86_32_int80_probe_handler(void)
{
    __asm__ volatile(
        "pusha\n\t"
        "pushl $0\n\t"
        "pushl 4(%esp)\n\t"
        "pushl 12(%esp)\n\t"
        "pushl 32(%esp)\n\t"
        "pushl 40(%esp)\n\t"
        "pushl 36(%esp)\n\t"
        "pushl 52(%esp)\n\t"
        "call simpleos_x86_32_dispatch_int80_probe\n\t"
        "addl $28, %esp\n\t"
        "movl %eax, 28(%esp)\n\t"
        "popa\n\t"
        "iret\n\t"
    );
}

RuntimeValue rt_x86_32_install_int80_probe(void)
{
    uint32_t handler = (uint32_t)(uintptr_t)x86_32_int80_probe_handler;
    x86_32_probe_idt[0x80].offset_low = (uint16_t)(handler & 0xFFFFU);
    x86_32_probe_idt[0x80].selector = 0x08U;
    x86_32_probe_idt[0x80].zero = 0;
    x86_32_probe_idt[0x80].type_attr = 0xEEU;
    x86_32_probe_idt[0x80].offset_high = (uint16_t)((handler >> 16) & 0xFFFFU);
    x86_32_probe_idtr.limit = (uint16_t)(sizeof(x86_32_probe_idt) - 1U);
    x86_32_probe_idtr.base = (uint32_t)(uintptr_t)x86_32_probe_idt;
    __asm__ volatile("lidt (%0)" : : "r"(&x86_32_probe_idtr) : "memory");
    return (RuntimeValue)1;
}

RuntimeValue rt_x86_32_trigger_int80(
    RuntimeValue id,
    RuntimeValue arg0,
    RuntimeValue arg1,
    RuntimeValue arg2,
    RuntimeValue arg3,
    RuntimeValue arg4,
    RuntimeValue arg5
)
{
    uint32_t eax = (uint32_t)id;
    uint32_t ebx = (uint32_t)arg0;
    uint32_t ecx = (uint32_t)arg1;
    uint32_t edx = (uint32_t)arg2;
    uint32_t esi = (uint32_t)arg3;
    uint32_t edi = (uint32_t)arg4;
    __asm__ volatile(
        "int $0x80\n\t"
        : "+a"(eax), "+b"(ebx), "+c"(ecx), "+d"(edx), "+S"(esi), "+D"(edi)
        :
        : "memory", "cc"
    );
    (void)arg5;
    return (RuntimeValue)eax;
}

RuntimeValue rt_port_outb(RuntimeValue port, RuntimeValue val)
    __attribute__((alias("rt_port_outb_real")));
RuntimeValue rt_port_outw(RuntimeValue port, RuntimeValue val)
    __attribute__((alias("rt_port_outw_real")));
RuntimeValue rt_port_outl(RuntimeValue port, RuntimeValue val)
    __attribute__((alias("rt_port_outl_real")));
RuntimeValue rt_port_inb(RuntimeValue port)
    __attribute__((alias("rt_port_inb_real")));
RuntimeValue rt_port_inw(RuntimeValue port)
    __attribute__((alias("rt_port_inw_real")));
RuntimeValue rt_port_inl(RuntimeValue port)
    __attribute__((alias("rt_port_inl_real")));
RuntimeValue rt_port_io_wait(void)
    __attribute__((alias("rt_port_io_wait_real")));

/* --- MMIO: real x86_32 implementations --- */

RuntimeValue rt_mmio_read_u8_real(RuntimeValue addr)
{
    return ENCODE_INT(*(volatile uint8_t *)(uintptr_t)DECODE_INT(addr));
}

RuntimeValue rt_mmio_read_u16_real(RuntimeValue addr)
{
    return ENCODE_INT(*(volatile uint16_t *)(uintptr_t)DECODE_INT(addr));
}

RuntimeValue rt_mmio_read_u32_real(RuntimeValue addr)
{
    return ENCODE_INT(*(volatile uint32_t *)(uintptr_t)DECODE_INT(addr));
}

RuntimeValue rt_mmio_write_u8_real(RuntimeValue addr, RuntimeValue val)
{
    *(volatile uint8_t *)(uintptr_t)DECODE_INT(addr) = (uint8_t)DECODE_INT(val);
    return NIL_VALUE;
}

RuntimeValue rt_mmio_write_u16_real(RuntimeValue addr, RuntimeValue val)
{
    *(volatile uint16_t *)(uintptr_t)DECODE_INT(addr) = (uint16_t)DECODE_INT(val);
    return NIL_VALUE;
}

RuntimeValue rt_mmio_write_u32_real(RuntimeValue addr, RuntimeValue val)
{
    *(volatile uint32_t *)(uintptr_t)DECODE_INT(addr) = (uint32_t)DECODE_INT(val);
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

/* --- CPU: real x86_32 implementations --- */

RuntimeValue rt_hlt_real(void)
{
    __asm__ volatile("hlt");
    return NIL_VALUE;
}

RuntimeValue rt_sti_real(void)
{
    __asm__ volatile("sti");
    return NIL_VALUE;
}

RuntimeValue rt_cli_real(void)
{
    __asm__ volatile("cli");
    return NIL_VALUE;
}

RuntimeValue rt_enable_interrupts_real(void)
{
    __asm__ volatile("sti");
    return NIL_VALUE;
}

RuntimeValue rt_disable_interrupts_real(void)
{
    __asm__ volatile("cli");
    return NIL_VALUE;
}

RuntimeValue rt_invlpg_real(RuntimeValue addr)
{
    __asm__ volatile("invlpg (%0)" : : "r"((uintptr_t)DECODE_INT(addr)) : "memory");
    return NIL_VALUE;
}

RuntimeValue rt_rdtsc_real(void)
{
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    /* In 32-bit mode we can only return the low 32 bits */
    return ENCODE_INT((int32_t)lo);
}

RuntimeValue rt_lgdt_real(RuntimeValue desc)
{
    __asm__ volatile("lgdt (%0)" : : "r"((uintptr_t)desc) : "memory");
    return NIL_VALUE;
}

RuntimeValue rt_lidt_real(RuntimeValue desc)
{
    __asm__ volatile("lidt (%0)" : : "r"((uintptr_t)desc) : "memory");
    return NIL_VALUE;
}

RuntimeValue rt_ltr_real(RuntimeValue sel)
{
    uint16_t selector = (uint16_t)sel;
    __asm__ volatile("ltr %0" : : "r"(selector));
    return NIL_VALUE;
}

RuntimeValue rt_read_cr3_real(RuntimeValue dummy)
{
    (void)dummy;
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return ENCODE_INT((int32_t)cr3);
}

RuntimeValue rt_write_cr3_real(RuntimeValue val)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"((uint32_t)DECODE_INT(val)) : "memory");
    return NIL_VALUE;
}

RuntimeValue rt_read_cr2_real(RuntimeValue dummy)
{
    (void)dummy;
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    return ENCODE_INT((int32_t)cr2);
}

RuntimeValue rt_hlt(void)     __attribute__((alias("rt_hlt_real")));
RuntimeValue rt_sti(void)     __attribute__((alias("rt_sti_real")));
RuntimeValue rt_cli(void)     __attribute__((alias("rt_cli_real")));
RuntimeValue rt_enable_interrupts(void)
    __attribute__((alias("rt_enable_interrupts_real")));
RuntimeValue rt_disable_interrupts(void)
    __attribute__((alias("rt_disable_interrupts_real")));
RuntimeValue rt_invlpg(RuntimeValue)
    __attribute__((alias("rt_invlpg_real")));
RuntimeValue rt_rdtsc(void)
    __attribute__((alias("rt_rdtsc_real")));
RuntimeValue rt_lgdt(RuntimeValue)
    __attribute__((alias("rt_lgdt_real")));
RuntimeValue rt_lidt(RuntimeValue)
    __attribute__((alias("rt_lidt_real")));
RuntimeValue rt_ltr(RuntimeValue)
    __attribute__((alias("rt_ltr_real")));
RuntimeValue rt_read_cr3(RuntimeValue)
    __attribute__((alias("rt_read_cr3_real")));
RuntimeValue rt_write_cr3(RuntimeValue)
    __attribute__((alias("rt_write_cr3_real")));
RuntimeValue rt_read_cr2(RuntimeValue)
    __attribute__((alias("rt_read_cr2_real")));

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t es;
    uint32_t fpu_state;
} X86_32SavedContext;

RuntimeValue rt_x86_32_context_switch(RuntimeValue from_ptr_val, RuntimeValue to_ptr_val)
{
    X86_32SavedContext *from = (X86_32SavedContext *)(uintptr_t)(uint32_t)from_ptr_val;
    X86_32SavedContext *to = (X86_32SavedContext *)(uintptr_t)(uint32_t)to_ptr_val;
    if (from) {
        from->eax = 0;
        from->ebx = 0;
        from->ecx = 0;
        from->edx = 0;
        from->esi = 0;
        from->edi = 0;
        from->ebp = (uint32_t)(uintptr_t)&from;
        from->esp = (uint32_t)(uintptr_t)&from;
        from->eip = (uint32_t)(uintptr_t)__builtin_return_address(0);
        from->eflags = 0x202U;
    }
    (void)to;
    return NIL_VALUE;
}

RuntimeValue rt_x86_32_fpu_save(RuntimeValue ctx_ptr_val)
{
    X86_32SavedContext *ctx = (X86_32SavedContext *)(uintptr_t)(uint32_t)ctx_ptr_val;
    if (!ctx || !ctx->fpu_state) return NIL_VALUE;
    __asm__ volatile("fnsave (%0)" : : "r"((uintptr_t)ctx->fpu_state) : "memory");
    return NIL_VALUE;
}

RuntimeValue rt_x86_32_fpu_restore(RuntimeValue ctx_ptr_val)
{
    X86_32SavedContext *ctx = (X86_32SavedContext *)(uintptr_t)(uint32_t)ctx_ptr_val;
    if (!ctx || !ctx->fpu_state) return NIL_VALUE;
    __asm__ volatile("frstor (%0)" : : "r"((uintptr_t)ctx->fpu_state) : "memory");
    return NIL_VALUE;
}

/* ===================================================================
 * Crypto — shared portable implementation
 * =================================================================== */
#define RV_INT int32_t
#define CRYPTO_ARRAY_HDR_TYPE(arr) ((arr)->type)
#include "../../shared/crypto_common.h"

/* End of x86_32 baremetal_stubs.c */
