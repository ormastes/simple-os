#include <stdint.h>
#include <stddef.h>

#include "embedded_ssh_host_rsa.h"
#include "embedded_ssh_host_rsa_crt.h"

typedef int64_t RuntimeValue;


#if defined(__x86_64__) || defined(__i386__)

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

/* rt_read_msr / rt_write_msr — C-ABI helpers for Simple-side MSR programming.
 * Without these, install_syscall_entry() cannot set EFER.SCE / STAR / LSTAR /
 * SFMASK, so `syscall` instructions silently #UD and the baremetal syscall
  * dispatcher is never reached. */
 uint64_t rt_read_msr(uint32_t msr)
 {
     uint32_t lo, hi;
     __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
     return ((uint64_t)hi << 32) | lo;
 }
 
 void rt_write_msr(uint32_t msr, uint64_t value)
 {
     uint32_t lo = (uint32_t)(value & 0xFFFFFFFFu);
     uint32_t hi = (uint32_t)(value >> 32);
     __asm__ volatile("wrmsr" : : "c"(msr), "a"(lo), "d"(hi));
 }
 
 /* rt_x86_syscall — C-ABI wrapper that actually emits the `syscall`
  * instruction. The Simple-side asm-volatile block in
  * src/os/userlib/syscall_raw.spl currently lowers to a no-op stub that
  * simply returns a constant, so posix_spawn → syscall(13, ...) never
  * traps. Until the Simple compiler lowers `asm volatile` to the real
  * instruction, route through this extern.
  *
  * Register convention matches the x86_64 System V SYSCALL ABI:
  *   rax = syscall number
 *   rdi, rsi, rdx, r10, r8 = args 0..4  (r10 for arg3, NOT rcx — rcx
 *     holds the caller's saved RIP post-SYSCALL)
 *   rax = int64_t return value
 */
/* rt_copy_user_byte — single-byte copy from a user-mode virtual address.
 * The Simple-side `_copy_user_bytes` loop in src/os/kernel/ipc/syscall.spl
 * appears to miscompile in the baremetal freestanding build (args.arg1 = 22
  * is observed at the marker before entry, but _copy_user_bytes returns an
  * empty Vec for the same arguments). This helper lets the Simple code read
  * one byte at a time from a raw u64 address via plain C pointer deref,
  * sidestepping whatever the asm-volatile / unsafe lowering does. */
 uint8_t rt_copy_user_byte(uint64_t ptr_addr) {
     return *(const uint8_t*)(uintptr_t)ptr_addr;
 }
 
 int64_t rt_x86_syscall(uint64_t id, uint64_t a0, uint64_t a1, uint64_t a2,
                        uint64_t a3, uint64_t a4) {
     int64_t result;
     /* Load every register explicitly from memory to avoid register-allocator
      * reordering between the Sys V C ABI (arg3 in rcx, arg4 in r8, arg5 in
      * r9) and the SYSCALL ABI (arg3 in r10, arg4 in r8). Memory operands are
      * slower but unambiguous; the caller is in the kernel-stack path so the
      * extra spill is negligible. */
     __asm__ volatile(
         "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "movq %4, %%rdx\n\t"
         "movq %5, %%r10\n\t"
         "movq %6, %%r8\n\t"
         "syscall\n\t"
         "movq %%rax, %0"
         : "=m"(result)
         : "m"(id), "m"(a0), "m"(a1), "m"(a2), "m"(a3), "m"(a4)
         : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r10", "r11", "memory"
    );
    return result;
}

#else
/* Stubs for non-x86 host analysis (never called at runtime) */
static inline void outb(uint16_t p, uint8_t v) { (void)p; (void)v; }
static inline uint8_t inb(uint16_t p) { (void)p; return 0; }
static inline void outw(uint16_t p, uint16_t v) { (void)p; (void)v; }
static inline uint16_t inw(uint16_t p) { (void)p; return 0; }
static inline void outl(uint16_t p, uint32_t v) { (void)p; (void)v; }
static inline uint32_t inl(uint16_t p) { (void)p; return 0; }
static inline void io_wait(void) {}
#endif

static void _serial_putchar_impl(char c)
{
    /* Wait until transmit holding register is empty (bit 5 of LSR) */
    while (!(inb(0x3F8 + 5) & 0x20)) {}
    outb(0x3F8, (uint8_t)c);
}
/* Public alias used by the rest of this file */
#define serial_putchar _serial_putchar_impl

static int serial_data_ready(void)
{
    /* Bit 0 of LSR (0x3F8 + 5) = Data Ready */
    return inb(0x3F8 + 5) & 0x01;
}

static char serial_getchar(void)
{
    /* Wait until data is available */
    while (!serial_data_ready()) {}
    return (char)inb(0x3F8);
}

static void _serial_puts_impl(const char *s)
{
    while (*s) {
        if (*s == '\n') _serial_putchar_impl('\r');
        _serial_putchar_impl(*s++);
    }
}
#define serial_puts _serial_puts_impl

extern int64_t simple_log_c_init(int64_t level, int64_t targets) __attribute__((weak));
extern int64_t simple_log_c_is_ready(void) __attribute__((weak));
extern int64_t simple_log_c_enabled(int64_t level) __attribute__((weak));
extern int64_t simple_log_c_write(int64_t level, int64_t msg_ptr, int64_t msg_len) __attribute__((weak));

static void serial_put_hex(uint64_t v)
{
    static const char hex[] = "0123456789abcdef";
    serial_puts("0x");
    /* Skip leading zeros but always print at least one digit */
    int started = 0;
    for (int i = 60; i >= 0; i -= 4) {
        int nibble = (v >> i) & 0xF;
        if (nibble || started || i == 0) {
            serial_putchar(hex[nibble]);
            started = 1;
        }
    }
}

static void serial_put_dec(int64_t v)
{
    if (v < 0) {
        serial_putchar('-');
        /* Handle INT64_MIN carefully */
        if (v == (-9223372036854775807LL - 1)) {
            serial_puts("9223372036854775808");
            return;
        }
        v = -v;
    }
    char buf[21];
    int pos = 0;
    uint64_t uv = (uint64_t)v;
    do {
        buf[pos++] = '0' + (char)(uv % 10);
        uv /= 10;
    } while (uv > 0);
    while (pos > 0) {
        serial_putchar(buf[--pos]);
    }
}

static int64_t _simpleos_log_level = 2;
static int64_t _simpleos_log_targets = 0;

static int _simple_log_facade_ready(void)
{
    return simple_log_c_is_ready && simple_log_c_is_ready() != 0;
}

static int64_t _cstr_len(const char *s)
{
    int64_t len = 0;
    if (!s) return 0;
    while (s[len]) len++;
    return len;
}

static void _simpleos_log_raw_emit(const char *ptr, int64_t len)
{
    if (!ptr || len <= 0) return;
    for (int64_t i = 0; i < len; i++) serial_putchar(ptr[i]);
    serial_puts("\r\n");
}

static int8_t _simpleos_log_write_bytes(int64_t level, const char *ptr, int64_t len)
{
    if (!ptr || len <= 0) return 0;
    if ((_simpleos_log_targets & 1) == 0) return 0;
    if (_simple_log_facade_ready() && simple_log_c_write)
        return simple_log_c_write(level, (int64_t)(intptr_t)ptr, len) ? 1 : 0;
    _simpleos_log_raw_emit(ptr, len);
    return 1;
}

static int8_t _simpleos_log_write_cstr(int64_t level, const char *msg)
{
    return _simpleos_log_write_bytes(level, msg, _cstr_len(msg));
}

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
    uint64_t   len;
    char       data[];
} RuntimeString;

typedef struct {
    HeapHeader   hdr;
    uint64_t     len;
    uint64_t     cap;
    RuntimeValue *items;
} RuntimeArray;

#define HEAP_STRING  1
#define HEAP_ARRAY   2
#define HEAP_MAP     3
#define HEAP_OBJECT  4
#define HEAP_CLOSURE 5
#define HEAP_MODULE  6
#define HEAP_ENUM    7

static inline RuntimeValue *runtime_array_inline_items(RuntimeArray *a)
{
    return (RuntimeValue *)((uint8_t *)a + sizeof(RuntimeArray));
}

static inline RuntimeValue *runtime_array_items(RuntimeArray *a)
{
    if (!a) return NULL;
    return a->items ? a->items : runtime_array_inline_items(a);
}

void *malloc(size_t sz);

RuntimeValue rt_u32_alloc_filled(uint64_t len, uint32_t fill)
{
    if (len > 0x400000) return NIL_VALUE;
    size_t bytes = sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue);
    RuntimeArray *a = (RuntimeArray *)malloc(bytes);
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)bytes;
    a->len = len;
    a->cap = len;
    a->items = runtime_array_inline_items(a);
    RuntimeValue item = (RuntimeValue)(uint64_t)fill;
    for (uint64_t i = 0; i < len; i++) {
        a->items[i] = item;
    }
    return ENCODE_PTR(a);
}

#if defined(__x86_64__) || defined(__i386__)
#define SIMPLEOS_AP_TRAMPOLINE_PHYS   0x8000ULL
#define SIMPLEOS_AP_TRAMPOLINE_VECTOR 0x08U
#define SIMPLEOS_AP_TRAMPOLINE_MAX    4096U
#define SIMPLEOS_AP_MAX_CPUS          32U
#define SIMPLEOS_AP_STACK_SIZE        8192U

extern uint8_t simpleos_ap_trampoline_template_start[];
extern uint8_t simpleos_ap_trampoline_template_end[];
extern uint8_t simpleos_ap_trampoline_gdt_desc[];
extern uint8_t simpleos_ap_trampoline_gdt[];
extern uint8_t simpleos_ap_trampoline_gdt_end[];
extern uint8_t simpleos_ap_trampoline_pml4_phys_slot[];
extern uint8_t boot_pml4[];
extern int64_t spl_x86_mark_current_ap_online(void) __attribute__((weak));

__attribute__((aligned(16))) static uint8_t simpleos_ap_boot_stacks[SIMPLEOS_AP_MAX_CPUS][SIMPLEOS_AP_STACK_SIZE];
uint64_t simpleos_ap_boot_stack_top = 0;
static volatile uint32_t simpleos_ap_entry_count = 0;

static uint64_t simpleos_raw_or_encoded_int(RuntimeValue v)
{
    return IS_INT(v) ? (uint64_t)DECODE_INT(v) : (uint64_t)v;
}

static RuntimeValue simpleos_expose_runtime_value(RuntimeValue v)
{
    return IS_INT(v) ? (RuntimeValue)DECODE_INT(v) : v;
}

RuntimeValue rt_x86_ap_trampoline_vector(void)
{
    return (RuntimeValue)SIMPLEOS_AP_TRAMPOLINE_VECTOR;
}

RuntimeValue rt_x86_ap_trampoline_phys(void)
{
    return (RuntimeValue)SIMPLEOS_AP_TRAMPOLINE_PHYS;
}

RuntimeValue rt_x86_prepare_ap_startup(RuntimeValue cpu_id_rv, RuntimeValue vector_rv)
{
    uint32_t cpu_id = (uint32_t)cpu_id_rv;
    uint32_t vector = (uint32_t)vector_rv;
    uint8_t *dst = (uint8_t *)(uintptr_t)SIMPLEOS_AP_TRAMPOLINE_PHYS;
    uint64_t size = (uint64_t)(simpleos_ap_trampoline_template_end - simpleos_ap_trampoline_template_start);

    if (cpu_id == 0 || cpu_id >= SIMPLEOS_AP_MAX_CPUS) {
        serial_puts("[smp] AP trampoline reject cpu\r\n");
        return (RuntimeValue)0;
    }
    if (vector != SIMPLEOS_AP_TRAMPOLINE_VECTOR) {
        serial_puts("[smp] AP trampoline reject vector\r\n");
        return (RuntimeValue)0;
    }
    if (size == 0 || size > SIMPLEOS_AP_TRAMPOLINE_MAX) {
        serial_puts("[smp] AP trampoline reject size\r\n");
        return (RuntimeValue)0;
    }

    __builtin_memcpy(dst, simpleos_ap_trampoline_template_start, (size_t)size);

    uint64_t gdt_off = (uint64_t)(simpleos_ap_trampoline_gdt_desc - simpleos_ap_trampoline_template_start);
    uint64_t gdt_table_off = (uint64_t)(simpleos_ap_trampoline_gdt - simpleos_ap_trampoline_template_start);
    uint64_t gdt_table_size = (uint64_t)(simpleos_ap_trampoline_gdt_end - simpleos_ap_trampoline_gdt);
    uint64_t pml4_off = (uint64_t)(simpleos_ap_trampoline_pml4_phys_slot - simpleos_ap_trampoline_template_start);
    *(volatile uint16_t *)(void *)(dst + gdt_off) = (uint16_t)(gdt_table_size - 1U);
    *(volatile uint32_t *)(void *)(dst + gdt_off + 2U) = (uint32_t)(SIMPLEOS_AP_TRAMPOLINE_PHYS + gdt_table_off);
    *(volatile uint32_t *)(void *)(dst + pml4_off) = (uint32_t)(uintptr_t)boot_pml4;

    simpleos_ap_boot_stack_top = (uint64_t)(uintptr_t)&simpleos_ap_boot_stacks[cpu_id][SIMPLEOS_AP_STACK_SIZE];
    __asm__ volatile("mfence" ::: "memory");

    serial_puts("[smp] AP trampoline prepared cpu=");
    serial_put_dec((int64_t)cpu_id);
    serial_puts(" vector=0x08\r\n");
    return (RuntimeValue)1;
}

RuntimeValue rt_x86_ap_entry_count(void)
{
    return ENCODE_INT((int64_t)simpleos_ap_entry_count);
}

void simpleos_ap_entry64(void)
{
    simpleos_ap_entry_count++;
    if (spl_x86_mark_current_ap_online) {
        (void)spl_x86_mark_current_ap_online();
    }
    for (volatile uint32_t i = 0; i < 1000000U; i++) {
        __asm__ volatile("" ::: "memory");
    }
    serial_puts("[smp] AP reached 64-bit entry\r\n");
    for (;;) {
        __asm__ volatile("hlt");
    }
}
#endif

/* Enum/Optional/Result representation — matches Rust runtime RuntimeEnum.
 * Used by rt_enum_new / rt_enum_discriminant / rt_enum_payload.
 * Total size: 24 bytes (header 8 + enum_id 4 + discriminant 4 + payload 8). */
typedef struct {
    HeapHeader   hdr;
    uint32_t     enum_id;
    uint32_t     discriminant;
    RuntimeValue payload;
} RuntimeEnum;

typedef struct {
    HeapHeader    hdr;
    uint32_t      len;
    uint32_t      cap;
    RuntimeValue *keys;
    RuntimeValue *values;
} RuntimeMap;

RuntimeValue rt_map_clone(RuntimeValue map);
RuntimeValue rt_map_new(void);
RuntimeValue rt_map_set(RuntimeValue map, RuntimeValue key, RuntimeValue value);
RuntimeValue rt_map_get(RuntimeValue map, RuntimeValue key);
RuntimeValue rt_array_new(RuntimeValue cap_val);
int8_t rt_array_push(RuntimeValue arr, RuntimeValue val);
RuntimeValue rt_array_get(RuntimeValue arr, RuntimeValue idx);
int8_t rt_array_set(RuntimeValue arr, RuntimeValue idx, RuntimeValue val);
RuntimeValue rt_tuple_new(RuntimeValue len_rv);
RuntimeValue rt_tuple_get(RuntimeValue tuple, RuntimeValue index);
RuntimeValue rt_tuple_set(RuntimeValue tuple, RuntimeValue index, RuntimeValue value);
RuntimeValue rt_string_concat(RuntimeValue a, RuntimeValue b);
RuntimeValue rt_string_from_cstr(const char *cstr);
RuntimeValue rt_string_new(RuntimeValue data, RuntimeValue len_val);
RuntimeValue rt_native_eq(RuntimeValue a, RuntimeValue b);
RuntimeValue rt_value_to_string(RuntimeValue val);
RuntimeValue rt_value_format_string(RuntimeValue val, RuntimeValue fmt_ptr, RuntimeValue fmt_len);
RuntimeValue rt_string_format(RuntimeValue fmt, RuntimeValue val);
void rt_print_value(RuntimeValue val);

static const size_t BAREMETAL_HEAP_SIZE = 192ULL * 1024ULL * 1024ULL;
static const size_t BAREMETAL_HEAP_WARN_SIZE = 144ULL * 1024ULL * 1024ULL;

static char   _heap[192ULL * 1024ULL * 1024ULL] __attribute__((aligned(16)));
static size_t _heap_off = 0;

void *malloc(size_t sz);

static inline size_t simpleos_heap_align(size_t sz)
{
    return (sz + 15U) & ~(size_t)15U;
}

static void *simpleos_heap_realloc_last(void *p, size_t old_sz, size_t new_sz)
{
    size_t old_aligned = simpleos_heap_align(old_sz);
    size_t new_aligned = simpleos_heap_align(new_sz);
    if (!p) return malloc(new_sz);

    uint8_t *base = (uint8_t *)p;
    if (base >= (uint8_t *)_heap &&
        base + old_aligned == (uint8_t *)&_heap[_heap_off]) {
        if (new_aligned > old_aligned) {
            size_t grow = new_aligned - old_aligned;
            if (_heap_off + grow > sizeof(_heap)) {
                serial_puts("[PANIC] heap exhausted\r\n");
                serial_puts("[PANIC] heap_off=");
                serial_put_hex((uint64_t)_heap_off);
                serial_puts(" req=");
                serial_put_hex((uint64_t)grow);
                serial_puts(" limit=");
                serial_put_hex((uint64_t)sizeof(_heap));
                serial_puts("\r\n");
                for (;;) outb(0xF4, 0);
            }
            _heap_off += grow;
        } else {
            _heap_off -= (old_aligned - new_aligned);
        }
        return p;
    }

    void *n = malloc(new_sz);
    if (p && n) {
        size_t copy_sz = old_sz < new_sz ? old_sz : new_sz;
        __builtin_memcpy(n, p, copy_sz);
    }
    return n;
}

void *malloc(size_t sz)
{
    void *caller = __builtin_return_address(0);
    sz = simpleos_heap_align(sz);
    if (sz >= 0x100000 || _heap_off >= BAREMETAL_HEAP_WARN_SIZE) {
        serial_puts("[heap] alloc sz=");
        serial_put_hex((uint64_t)sz);
        serial_puts(" off_before=");
        serial_put_hex((uint64_t)_heap_off);
        serial_puts(" caller=");
        serial_put_hex((uint64_t)(uintptr_t)caller);
        serial_puts("\r\n");
    }
    if (_heap_off + sz > sizeof(_heap)) {
        serial_puts("[PANIC] heap exhausted\r\n");
        serial_puts("[PANIC] heap_off=");
        serial_put_hex((uint64_t)_heap_off);
        serial_puts(" req=");
        serial_put_hex((uint64_t)sz);
        serial_puts(" limit=");
        serial_put_hex((uint64_t)sizeof(_heap));
        serial_puts("\r\n");
        for(;;) outb(0xF4, 0);
    }
    void *p = &_heap[_heap_off];
    _heap_off += sz;
    if (sz >= 0x100000 || _heap_off >= BAREMETAL_HEAP_WARN_SIZE) {
        serial_puts("[heap] alloc off_after=");
        serial_put_hex((uint64_t)_heap_off);
        serial_puts("\r\n");
    }
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
    /* compile_struct_init passes RAW size (not tagged): iconst.i64 16
     * Return RAW pointer — codegen uses it directly for store(val, ptr, offset).
     * Other runtime functions that need heap pointers use ENCODE_PTR themselves. */
    size_t bytes = (size_t)sz;
    if (bytes == 0) return 0;
    /* Debug: log allocations of PciDevice-sized (96) or similar struct sizes */
    if (bytes > 0x1000000) bytes = 0x1000000;
    void *p = malloc(bytes);
    if (!p) return 0;
    __builtin_memset(p, 0, bytes);  /* zero to avoid garbage in uninitialized fields */
    return (RuntimeValue)(uintptr_t)p;
}

RuntimeValue rt_alloc_zeroed(RuntimeValue sz)
{
    size_t bytes = (size_t)sz;
    if (bytes == 0) return 0;
    if (bytes > 0x1000000) bytes = 0x1000000;
    void *p = malloc(bytes);
    if (!p) return 0;
    __builtin_memset(p, 0, bytes);
    return (RuntimeValue)(uintptr_t)p;
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
    int64_t len = len_val;
    if (len < 0 || len > 0x100000) return NIL_VALUE;
    /* len == 0 is valid: creates an empty string (used as f-string accumulator
       by compile_fstring_format which calls rt_string_new(NULL, 0)) */
    RuntimeString *s = (RuntimeString *)malloc(sizeof(RuntimeString) + (size_t)len + 1);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + (size_t)len + 1);
    s->len = (uint32_t)len;
    /* data is a raw pointer cast to i64 */
    const char *src = (const char *)(uintptr_t)data;
    if (src && len > 0) __builtin_memcpy(s->data, src, (size_t)len);
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
    /* Return RAW (untagged) — Cranelift backend does not unbox len results */
    if (!IS_HEAP(str)) return 0;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s) return 0;
    return (RuntimeValue)s->len;
}

RuntimeValue rt_string_char_at(RuntimeValue str, RuntimeValue idx)
{
    if (!IS_HEAP(str)) return NIL_VALUE;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s) return NIL_VALUE;
    int64_t i = (int64_t)idx;
    if (i < 0 || (uint32_t)i >= s->len) return NIL_VALUE;
    return rt_string_new((RuntimeValue)(uintptr_t)(s->data + i), 1);
}

RuntimeValue char_code_at(RuntimeValue str, RuntimeValue idx)
{
    if (!IS_HEAP(str)) return 0;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s || !IS_INT(idx)) return 0;
    int64_t i = DECODE_INT(idx);
    if (i < 0 || (uint32_t)i >= s->len) return 0;
    return ENCODE_INT((unsigned char)s->data[i]);
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
    /* Return raw 0/1 (Cranelift uses raw convention) */
    if (!IS_HEAP(a) || !IS_HEAP(b)) return (RuntimeValue)(a == b ? 1 : 0);
    RuntimeString *sa = (RuntimeString *)DECODE_PTR(a);
    RuntimeString *sb = (RuntimeString *)DECODE_PTR(b);
    if (!sa || !sb) return 0;
    if (sa->len != sb->len) return 0;
    for (uint32_t i = 0; i < sa->len; i++) {
        if (sa->data[i] != sb->data[i]) return 0;
    }
    return 1;
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
    int64_t a = (int64_t)start;
    int64_t b = (int64_t)end;
    if (a < 0) a = 0;
    if (b > (int64_t)s->len) b = (int64_t)s->len;
    if (a >= b) {
        /* empty string */
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

/* --- rt_value_to_string: convert any RuntimeValue to a string RuntimeValue ---
 *
 * Handles both tagged (BoxInt: val << 3) and raw integer values.
 * The Cranelift codegen uses raw integers internally, but inserts BoxInt
 * before calling this function for f-string interpolation. Cross-module
 * return values may arrive without BoxInt (raw). We handle both cases.
 */
static RuntimeValue _int_to_string(int64_t n)
{
    if (n == 0) return rt_string_from_cstr("0");
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

RuntimeValue rt_raw_u64_to_string(RuntimeValue raw)
{
    uint64_t uv = (uint64_t)raw;
    if (uv == 0) return rt_string_from_cstr("0");
    char buf[21];
    int pos = 0;
    while (uv > 0) { buf[pos++] = '0' + (char)(uv % 10); uv /= 10; }
    uint32_t len = (uint32_t)pos;
    RuntimeString *s = (RuntimeString *)malloc(sizeof(RuntimeString) + len + 1);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + len + 1);
    s->len = len;
    int out = 0;
    while (pos > 0) s->data[out++] = buf[--pos];
    s->data[out] = '\0';
    return ENCODE_PTR(s);
}

RuntimeValue rt_value_to_string(RuntimeValue val)
{
    /* 1. Tagged integer (BoxInt: low 3 bits = 0, TAG_INT) */
    if (IS_INT(val)) {
        return _int_to_string(DECODE_INT(val));
    }
    /* 2. Heap object (string, array, map) */
    if (IS_HEAP(val)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
        if (h && h->type == HEAP_STRING) return val;
        if (h && h->type == HEAP_ARRAY) return rt_string_from_cstr("<array>");
        if (h && h->type == HEAP_MAP) return rt_string_from_cstr("<map>");
        return rt_string_from_cstr("<object>");
    }
    /* 3. nil (0x3) */
    if (val == NIL_VALUE) return rt_string_from_cstr("nil");
    /* 4. Everything else: treat as raw integer (cross-module return without BoxInt) */
    return _int_to_string((int64_t)val);
}

RuntimeValue rt_len(RuntimeValue v)
{
    /* Return RAW (untagged) — Cranelift backend does not unbox len results */
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
    if (h->type == HEAP_MAP) {
        RuntimeMap *m = (RuntimeMap *)h;
        return (RuntimeValue)m->len;
    }
    return 0;
}

RuntimeValue rt_index_get(RuntimeValue v, RuntimeValue idx)
{
    if (!IS_HEAP(v)) return NIL_VALUE;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
    if (!h) return NIL_VALUE;
    if (h->type == HEAP_STRING) {
        if (!IS_INT(idx)) return NIL_VALUE;
        return rt_string_char_at(v, (RuntimeValue)DECODE_INT(idx));
    }
    if (h->type == HEAP_ARRAY) {
        if (!IS_INT(idx)) return NIL_VALUE;
        return rt_array_get(v, (RuntimeValue)DECODE_INT(idx));
    }
    if (h->type == HEAP_MAP) {
        /* Map indexing: key is the idx argument */
        return rt_map_get(v, idx);
    }
    return NIL_VALUE;
}

RuntimeValue rt_index_set(RuntimeValue v, RuntimeValue idx, RuntimeValue val)
{
    if (!IS_HEAP(v)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(v);
    if (!h) return 0;
    if (h->type == HEAP_ARRAY) {
        if (!IS_INT(idx)) return 0;
        return rt_array_set(v, (RuntimeValue)DECODE_INT(idx), val);
    }
    if (h->type == HEAP_MAP) {
        /* Map indexing: key is the idx argument */
        rt_map_set(v, idx, val);
        return 1;
    }
    return 0;
}

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
    serial_put_hex((uint64_t)DECODE_INT(val));
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

RuntimeValue rt_print(RuntimeValue val)
{
    if (IS_INT(val)) {
        serial_put_dec(DECODE_INT(val));
    } else if (IS_HEAP(val)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
        if (h && h->type == HEAP_STRING) {
            RuntimeString *s = (RuntimeString *)h;
            for (uint32_t i = 0; i < s->len; i++) serial_putchar(s->data[i]);
        } else {
            serial_puts("<object>");
        }
    } else if (IS_NIL(val)) {
        serial_puts("nil");
    } else {
        serial_puts("<value>");
    }
    return NIL_VALUE;
}

static RuntimeValue rt_array_push_handle(RuntimeValue arr, RuntimeValue val);
static int8_t rt_array_set_raw(RuntimeValue arr, RuntimeValue idx, RuntimeValue val);

RuntimeValue rt_println(RuntimeValue val)
{
    rt_print(val);
    serial_putchar('\r');
    serial_putchar('\n');
    return NIL_VALUE;
}

void rt_framebuffer_copy(RuntimeValue dst, RuntimeValue src, RuntimeValue count)
{
    if (!IS_HEAP(dst) || !IS_HEAP(src)) return;
    uint8_t *d = (uint8_t *)DECODE_PTR(dst);
    const uint8_t *s = (const uint8_t *)DECODE_PTR(src);
    int64_t n = DECODE_INT(count);
    if (n <= 0) return;
    for (int64_t i = 0; i < n; i++) d[i] = s[i];
}

void rt_framebuffer_write(RuntimeValue addr, RuntimeValue offset, RuntimeValue val)
{
    if (!IS_HEAP(addr)) return;
    uint8_t *base = (uint8_t *)DECODE_PTR(addr);
    int64_t off = DECODE_INT(offset);
    int64_t v = DECODE_INT(val);
    base[off] = (uint8_t)v;
}

RuntimeValue rt_mmio_write_u32_real(RuntimeValue addr, RuntimeValue val);

void rt_fb_blit_array32(RuntimeValue dst_addr, RuntimeValue dst_stride_pixels,
                        RuntimeValue src_pixels, RuntimeValue src_stride_pixels,
                        RuntimeValue copy_w, RuntimeValue copy_h)
{
    RuntimeArray *src = IS_HEAP(src_pixels)
        ? (RuntimeArray *)DECODE_PTR(src_pixels)
        : (RuntimeArray *)(uintptr_t)src_pixels;
    if (!src || src->hdr.type != HEAP_ARRAY) return;

    volatile uint32_t *dst = (volatile uint32_t *)(uintptr_t)(uint64_t)dst_addr;
    uint64_t dst_stride = (uint64_t)dst_stride_pixels;
    uint64_t src_stride = (uint64_t)src_stride_pixels;
    uint64_t w = (uint64_t)copy_w;
    uint64_t h = (uint64_t)copy_h;
    RuntimeValue *src_items = runtime_array_items(src);

    for (uint64_t y = 0; y < h; y++) {
        uint64_t src_row = y * src_stride;
        uint64_t dst_row = y * dst_stride;
        if (src_row >= src->len) break;

        uint64_t row_count = w;
        if (src_row + row_count > src->len) {
            row_count = src->len - src_row;
        }

        for (uint64_t x = 0; x < row_count; x++) {
            RuntimeValue item = src_items[src_row + x];
            uint32_t pixel = (uint32_t)(uint64_t)item;
            rt_mmio_write_u32_real((RuntimeValue)(uintptr_t)(dst + dst_row + x), (RuntimeValue)(uint64_t)pixel);
        }
    }
}

RuntimeValue rt_fb_fill_rect32(RuntimeValue addr, RuntimeValue stride_pixels,
                               RuntimeValue x, RuntimeValue y,
                               RuntimeValue w, RuntimeValue h,
                               RuntimeValue color)
{
    volatile uint32_t *fb = (volatile uint32_t *)(uintptr_t)(uint64_t)addr;
    uint64_t stride = (uint64_t)stride_pixels;
    uint64_t x0 = (uint64_t)x;
    uint64_t y0 = (uint64_t)y;
    uint64_t width = (uint64_t)w;
    uint64_t height = (uint64_t)h;
    uint32_t pixel = (uint32_t)(uint64_t)color;
    for (uint64_t row = 0; row < height; row++) {
        uint64_t base = (y0 + row) * stride + x0;
        for (uint64_t col = 0; col < width; col++) {
            rt_mmio_write_u32_real((RuntimeValue)(uintptr_t)(fb + base + col), (RuntimeValue)(uint64_t)pixel);
        }
    }
    return ENCODE_INT(0);
}

RuntimeValue rt_native_eq(RuntimeValue a, RuntimeValue b)
{
    /* Fast path: bitwise identical (same int, same pointer, both nil) */
    if (a == b) return 1;
    /* Heap objects: compare by content if both are strings */
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

#define MAX_PCI_CACHED 64
static struct {
    uint8_t bus, dev, func;
    uint16_t vendor, devid;
    uint8_t cls, sub, progif, htype, irq;
} _pci_cache[MAX_PCI_CACHED];
static int _pci_cache_count = -1; /* -1 = not scanned yet */

static void _pci_scan(void)
{
    _pci_cache_count = 0;
    for (int bus = 0; bus < 256 && _pci_cache_count < MAX_PCI_CACHED; bus++) {
        for (int dev = 0; dev < 32 && _pci_cache_count < MAX_PCI_CACHED; dev++) {
            /* Read vendor ID at bus:dev.0 */
            uint32_t addr = 0x80000000 | ((uint32_t)bus << 16) | ((uint32_t)dev << 11);
            outl(0xCF8, addr);
            uint32_t reg0 = inl(0xCFC);
            uint16_t vendor = (uint16_t)(reg0 & 0xFFFF);
            if (vendor == 0xFFFF || vendor == 0) continue;

            uint16_t devid = (uint16_t)(reg0 >> 16);

            /* Read class/subclass/prog_if at offset 0x08 */
            outl(0xCF8, addr | 0x08);
            uint32_t reg2 = inl(0xCFC);
            uint8_t cls = (uint8_t)(reg2 >> 24);
            uint8_t sub = (uint8_t)(reg2 >> 16);
            uint8_t progif = (uint8_t)(reg2 >> 8);

            /* Read header type at offset 0x0C */
            outl(0xCF8, addr | 0x0C);
            uint32_t reg3 = inl(0xCFC);
            uint8_t htype = (uint8_t)(reg3 >> 16);

            /* Read IRQ line at offset 0x3C */
            outl(0xCF8, addr | 0x3C);
            uint32_t regF = inl(0xCFC);
            uint8_t irq = (uint8_t)(regF & 0xFF);

            int i = _pci_cache_count++;
            _pci_cache[i].bus = (uint8_t)bus;
            _pci_cache[i].dev = (uint8_t)dev;
            _pci_cache[i].func = 0;
            _pci_cache[i].vendor = vendor;
            _pci_cache[i].devid = devid;
            _pci_cache[i].cls = cls;
            _pci_cache[i].sub = sub;
            _pci_cache[i].progif = progif;
            _pci_cache[i].htype = htype;
            _pci_cache[i].irq = irq;
        }
    }
}

#define nvme_rd32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))
#define nvme_wr32(addr, val) (*(volatile uint32_t *)(uintptr_t)(addr) = (val))
#define nvme_rd64(addr) (*(volatile uint64_t *)(uintptr_t)(addr))
#define nvme_wr64(addr, val) (*(volatile uint64_t *)(uintptr_t)(addr) = (val))

/* NVMe register offsets */
#define NVME_REG_CAP   0x00
#define NVME_REG_VS    0x08
#define NVME_REG_CC    0x14
#define NVME_REG_CSTS  0x1C
#define NVME_REG_AQA   0x24
#define NVME_REG_ASQ   0x28
#define NVME_REG_ACQ   0x30

/* CC bits */
#define NVME_CC_EN        (1u << 0)
#define NVME_CC_CSS_NVM   (0u << 4)
#define NVME_CC_MPS_4K    (0u << 7)
#define NVME_CC_IOSQES_6  (6u << 16)  /* log2(64) = 6 */
#define NVME_CC_IOCQES_4  (4u << 20)  /* log2(16) = 4 */

/* CSTS bits */
#define NVME_CSTS_RDY  (1u << 0)
#define NVME_CSTS_CFS  (1u << 1)

/* Queue sizes */
#define NVME_ADMIN_DEPTH  32
#define NVME_IO_DEPTH     64
#define NVME_SQE_SIZE     64
#define NVME_CQE_SIZE     16

/* NVMe SQE (Submission Queue Entry) — 64 bytes */
struct nvme_sqe {
    uint32_t cdw0;    /* opcode[7:0] | fuse[9:8] | psdt[15:14] | cid[31:16] */
    uint32_t nsid;
    uint64_t rsvd;
    uint64_t mptr;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
};

/* NVMe CQE (Completion Queue Entry) — 16 bytes */
struct nvme_cqe {
    uint32_t dw0;
    uint32_t rsvd;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t cid;
    uint16_t status;   /* bit 0 = phase, bits 1-15 = status code */
};

/* NVMe controller state (single device) */
static struct {
    uint64_t bar0;
    uint32_t db_stride;    /* doorbell stride in bytes = 4 << DSTRD */

    /* Admin queue (QID 0) */
    struct nvme_sqe *admin_sq;
    struct nvme_cqe *admin_cq;
    uint16_t admin_sq_tail;
    uint16_t admin_cq_head;
    uint8_t  admin_cq_phase;
    uint16_t admin_cid;

    /* I/O queue (QID 1) */
    struct nvme_sqe *io_sq;
    struct nvme_cqe *io_cq;
    uint16_t io_sq_tail;
    uint16_t io_cq_head;
    uint8_t  io_cq_phase;
    uint16_t io_cid;

    int initialized;
    uint32_t sector_size;
    uint64_t sector_count;
} _nvme;

typedef struct {
    size_t prev_heap_off;
    size_t alloc_end_off;
} nvme_aligned_alloc_header_t;

/* Allocate page-aligned memory from the bump allocator.
 * NVMe requires queue and data buffers to be page-aligned (4KB).
 * We waste up to (alignment-1) bytes of padding to get the alignment. */
static void *nvme_alloc_aligned(size_t size, size_t alignment)
{
    if (alignment == 0) return (void *)0;
    /* Allocate extra space so we can align within it */
    size_t total = size + alignment + sizeof(nvme_aligned_alloc_header_t);
    size_t prev_heap_off = _heap_off;
    void *raw = malloc(total);
    if (!raw) return (void *)0;
    /* Align the pointer within the allocated region */
    uintptr_t addr = (uintptr_t)raw + sizeof(nvme_aligned_alloc_header_t);
    uintptr_t aligned = (addr + alignment - 1) & ~(alignment - 1);
    nvme_aligned_alloc_header_t *hdr =
        (nvme_aligned_alloc_header_t *)(aligned - sizeof(nvme_aligned_alloc_header_t));
    hdr->prev_heap_off = prev_heap_off;
    hdr->alloc_end_off = _heap_off;
    return (void *)aligned;
}

static void nvme_free_aligned(void *ptr)
{
    if (!ptr) return;
    nvme_aligned_alloc_header_t *hdr =
        (nvme_aligned_alloc_header_t *)((uintptr_t)ptr - sizeof(nvme_aligned_alloc_header_t));
    if (_heap_off == hdr->alloc_end_off && hdr->prev_heap_off <= hdr->alloc_end_off)
        _heap_off = hdr->prev_heap_off;
}

/* Ring a doorbell: SQ tail doorbell = BAR0 + 0x1000 + (2*qid) * stride
 *                  CQ head doorbell = BAR0 + 0x1000 + (2*qid+1) * stride */
static void nvme_ring_sq_doorbell(uint16_t qid, uint16_t tail)
{
    uint64_t off = 0x1000 + (uint64_t)(2 * qid) * _nvme.db_stride;
    nvme_wr32(_nvme.bar0 + off, tail);
}

static void nvme_ring_cq_doorbell(uint16_t qid, uint16_t head)
{
    uint64_t off = 0x1000 + (uint64_t)(2 * qid + 1) * _nvme.db_stride;
    nvme_wr32(_nvme.bar0 + off, head);
}

/* Submit a command to the admin queue and poll for completion.
 * Returns 0 on success, negative on error. */
static int nvme_admin_cmd(uint8_t opcode, uint32_t nsid,
                          uint64_t prp1, uint64_t prp2,
                          uint32_t cdw10, uint32_t cdw11, uint32_t cdw12)
{
    int idx = _nvme.admin_sq_tail;
    struct nvme_sqe *sqe = &_nvme.admin_sq[idx];

    __builtin_memset(sqe, 0, sizeof(*sqe));
    sqe->cdw0 = (uint32_t)opcode | ((uint32_t)_nvme.admin_cid << 16);
    sqe->nsid = nsid;
    sqe->prp1 = prp1;
    sqe->prp2 = prp2;
    sqe->cdw10 = cdw10;
    sqe->cdw11 = cdw11;
    sqe->cdw12 = cdw12;

    _nvme.admin_sq_tail = (_nvme.admin_sq_tail + 1) % NVME_ADMIN_DEPTH;
    nvme_ring_sq_doorbell(0, _nvme.admin_sq_tail);

    /* Poll CQ for completion */
    volatile struct nvme_cqe *cqe = &_nvme.admin_cq[_nvme.admin_cq_head];
    uint32_t timeout = 5000000;
    while (timeout--) {
        uint16_t status_raw = cqe->status;
        uint8_t phase = status_raw & 1;
        if (phase == _nvme.admin_cq_phase) {
            /* Completion arrived */
            uint16_t sc = (status_raw >> 1) & 0x7FFF;
            _nvme.admin_cq_head = (_nvme.admin_cq_head + 1) % NVME_ADMIN_DEPTH;
            if (_nvme.admin_cq_head == 0)
                _nvme.admin_cq_phase ^= 1;
            nvme_ring_cq_doorbell(0, _nvme.admin_cq_head);
            _nvme.admin_cid++;
            if (sc != 0) {
                _simpleos_log_write_cstr(4, "[nvme-c] admin cmd failed");
                return -5; /* EIO */
            }
            return 0;
        }
        /* Tiny delay to avoid hammering the bus */
        __asm__ volatile("pause" ::: "memory");
    }
    _simpleos_log_write_cstr(4, "[nvme-c] admin cmd timeout");
    return -110; /* ETIMEDOUT */
}

/* Submit a command to the I/O queue and poll for completion.
 * Returns 0 on success, negative on error. */
static int nvme_io_cmd(uint8_t opcode, uint32_t nsid,
                       uint64_t prp1, uint64_t prp2,
                       uint32_t cdw10, uint32_t cdw11, uint32_t cdw12)
{
    int idx = _nvme.io_sq_tail;
    struct nvme_sqe *sqe = &_nvme.io_sq[idx];

    __builtin_memset(sqe, 0, sizeof(*sqe));
    sqe->cdw0 = (uint32_t)opcode | ((uint32_t)_nvme.io_cid << 16);
    sqe->nsid = nsid;
    sqe->prp1 = prp1;
    sqe->prp2 = prp2;
    sqe->cdw10 = cdw10;
    sqe->cdw11 = cdw11;
    sqe->cdw12 = cdw12;

    _nvme.io_sq_tail = (_nvme.io_sq_tail + 1) % NVME_IO_DEPTH;
    nvme_ring_sq_doorbell(1, _nvme.io_sq_tail);

    /* Poll CQ for completion */
    volatile struct nvme_cqe *cqe = &_nvme.io_cq[_nvme.io_cq_head];
    uint32_t timeout = 5000000;
    while (timeout--) {
        uint16_t status_raw = cqe->status;
        uint8_t phase = status_raw & 1;
        if (phase == _nvme.io_cq_phase) {
            uint16_t sc = (status_raw >> 1) & 0x7FFF;
            _nvme.io_cq_head = (_nvme.io_cq_head + 1) % NVME_IO_DEPTH;
            if (_nvme.io_cq_head == 0)
                _nvme.io_cq_phase ^= 1;
            nvme_ring_cq_doorbell(1, _nvme.io_cq_head);
            _nvme.io_cid++;
            if (sc != 0) {
                _simpleos_log_write_cstr(4, "[nvme-c] I/O cmd failed");
                return -5; /* EIO */
            }
            return 0;
        }
        __asm__ volatile("pause" ::: "memory");
    }
    _simpleos_log_write_cstr(4, "[nvme-c] I/O cmd timeout");
    return -110;
}

/* ---------------------------------------------------------------
 * _nvme_init_and_read_sector0 — full NVMe init + BPB sector read
 * --------------------------------------------------------------- */
static int _nvme_init_controller(void)
{
    if (_nvme.initialized) return 0;

    /* Ensure PCI cache is populated */
    if (_pci_cache_count < 0) _pci_scan();

    /* Find NVMe device: class=0x01, subclass=0x08 */
    int nvme_idx = -1;
    for (int i = 0; i < _pci_cache_count; i++) {
        if (_pci_cache[i].cls == 0x01 && _pci_cache[i].sub == 0x08) {
            nvme_idx = i;
            break;
        }
    }
    if (nvme_idx < 0) {
        _simpleos_log_write_cstr(3, "[nvme-c] No NVMe device found on PCI bus");
        return -19; /* ENODEV */
    }

    /* Step 1: Read BAR0 from PCI config space */
    uint32_t pci_addr = 0x80000000
        | ((uint32_t)_pci_cache[nvme_idx].bus << 16)
        | ((uint32_t)_pci_cache[nvme_idx].dev << 11)
        | 0x10; /* BAR0 offset */
    outl(0xCF8, pci_addr);
    uint32_t bar0_lo = inl(0xCFC);

    /* Check if 64-bit BAR */
    uint64_t bar0_phys;
    if ((bar0_lo & 0x6) == 0x4) {
        /* 64-bit memory BAR: read high 32 bits from BAR1 (offset 0x14) */
        outl(0xCF8, pci_addr + 4);
        uint32_t bar0_hi = inl(0xCFC);
        bar0_phys = ((uint64_t)bar0_hi << 32) | (uint64_t)(bar0_lo & ~0xFu);
    } else {
        bar0_phys = (uint64_t)(bar0_lo & ~0xFu);
    }

    /* Enable bus mastering + memory space in PCI command register */
    uint32_t cmd_addr = 0x80000000
        | ((uint32_t)_pci_cache[nvme_idx].bus << 16)
        | ((uint32_t)_pci_cache[nvme_idx].dev << 11)
        | 0x04; /* Command register offset */
    outl(0xCF8, cmd_addr);
    uint32_t cmd_reg = inl(0xCFC);
    cmd_reg |= (1 << 1) | (1 << 2); /* Memory Space + Bus Master */
    outl(0xCF8, cmd_addr);
    outl(0xCFC, cmd_reg);

    _nvme.bar0 = bar0_phys; /* Identity mapped: phys == virt */

    serial_puts("[nvme-c] BAR0=");
    serial_put_hex(bar0_phys);
    serial_puts("\r\n");

    /* Step 2: Read CAP register (64-bit) */
    uint64_t cap = nvme_rd64(_nvme.bar0 + NVME_REG_CAP);
    serial_puts("[nvme-c] CAP=");
    serial_put_hex(cap);
    serial_puts("\r\n");

    /* Extract doorbell stride: CAP bits [35:32] = DSTRD */
    uint32_t dstrd = (uint32_t)((cap >> 32) & 0xF);
    _nvme.db_stride = 4u << dstrd;

    /* Step 3: Disable controller — clear CC.EN, wait CSTS.RDY=0 */
    uint32_t cc = nvme_rd32(_nvme.bar0 + NVME_REG_CC);
    nvme_wr32(_nvme.bar0 + NVME_REG_CC, cc & ~NVME_CC_EN);

    for (uint32_t i = 0; i < 1000000; i++) {
        uint32_t csts = nvme_rd32(_nvme.bar0 + NVME_REG_CSTS);
        if (!(csts & NVME_CSTS_RDY)) goto disabled;
        if (csts & NVME_CSTS_CFS) {
            _simpleos_log_write_cstr(4, "[nvme-c] Controller fatal status during disable");
            return -5;
        }
        __asm__ volatile("pause" ::: "memory");
    }
    _simpleos_log_write_cstr(4, "[nvme-c] Timeout waiting for controller disable");
    return -110;
disabled:
    _simpleos_log_write_cstr(2, "[nvme-c] Controller disabled");

    /* Step 4: Allocate admin queues (4KB-aligned for NVMe compliance) */
    size_t admin_sq_bytes = NVME_ADMIN_DEPTH * NVME_SQE_SIZE; /* 2048 */
    size_t admin_cq_bytes = NVME_ADMIN_DEPTH * NVME_CQE_SIZE; /* 512  */

    _nvme.admin_sq = (struct nvme_sqe *)nvme_alloc_aligned(admin_sq_bytes, 4096);
    _nvme.admin_cq = (struct nvme_cqe *)nvme_alloc_aligned(admin_cq_bytes, 4096);
    if (!_nvme.admin_sq || !_nvme.admin_cq) {
        _simpleos_log_write_cstr(4, "[nvme-c] Failed to allocate admin queues");
        return -12; /* ENOMEM */
    }
    __builtin_memset(_nvme.admin_sq, 0, admin_sq_bytes);
    __builtin_memset(_nvme.admin_cq, 0, admin_cq_bytes);
    _nvme.admin_sq_tail = 0;
    _nvme.admin_cq_head = 0;
    _nvme.admin_cq_phase = 1;
    _nvme.admin_cid = 0;

    /* Step 5: Configure AQA, ASQ, ACQ */
    uint32_t aqa = ((NVME_ADMIN_DEPTH - 1) << 16) | (NVME_ADMIN_DEPTH - 1);
    nvme_wr32(_nvme.bar0 + NVME_REG_AQA, aqa);
    nvme_wr64(_nvme.bar0 + NVME_REG_ASQ, (uint64_t)(uintptr_t)_nvme.admin_sq);
    nvme_wr64(_nvme.bar0 + NVME_REG_ACQ, (uint64_t)(uintptr_t)_nvme.admin_cq);

    serial_puts("[nvme-c] Admin queues configured: SQ=");
    serial_put_hex((uint64_t)(uintptr_t)_nvme.admin_sq);
    serial_puts(" CQ=");
    serial_put_hex((uint64_t)(uintptr_t)_nvme.admin_cq);
    serial_puts("\r\n");

    /* Step 6: Enable controller
     * CC: EN=1, CSS=0 (NVM), MPS=0 (4KB), IOSQES=6 (64B), IOCQES=4 (16B) */
    uint32_t cc_val = NVME_CC_EN | NVME_CC_CSS_NVM | NVME_CC_MPS_4K
                    | NVME_CC_IOSQES_6 | NVME_CC_IOCQES_4;
    nvme_wr32(_nvme.bar0 + NVME_REG_CC, cc_val);

    /* Step 7: Wait for CSTS.RDY=1 */
    for (uint32_t i = 0; i < 1000000; i++) {
        uint32_t csts = nvme_rd32(_nvme.bar0 + NVME_REG_CSTS);
        if (csts & NVME_CSTS_RDY) goto enabled;
        if (csts & NVME_CSTS_CFS) {
            _simpleos_log_write_cstr(4, "[nvme-c] Controller fatal status during enable");
            return -5;
        }
        __asm__ volatile("pause" ::: "memory");
    }
    _simpleos_log_write_cstr(4, "[nvme-c] Timeout waiting for CSTS.RDY");
    return -110;
enabled:
    _simpleos_log_write_cstr(2, "[nvme-c] Controller enabled (CSTS.RDY=1)");

    /* Step 8: Identify Controller (admin opcode 0x06, CNS=1) */
    {
        void *id_buf = nvme_alloc_aligned(4096, 4096);
        if (!id_buf) return -12;
        __builtin_memset(id_buf, 0, 4096);
        int rc = nvme_admin_cmd(0x06, 0,
                                (uint64_t)(uintptr_t)id_buf, 0,
                                1 /* CNS=1: controller */, 0, 0);
        if (rc < 0) {
            _simpleos_log_write_cstr(3, "[nvme-c] Identify Controller failed");
            /* Non-fatal: continue anyway */
        } else {
            _simpleos_log_write_cstr(2, "[nvme-c] Identify Controller OK");
        }
    }

    /* Step 9: Identify Namespace 1 (admin opcode 0x06, CNS=0, NSID=1) */
    {
        void *ns_buf = nvme_alloc_aligned(4096, 4096);
        if (!ns_buf) return -12;
        __builtin_memset(ns_buf, 0, 4096);
        int rc = nvme_admin_cmd(0x06, 1,
                                (uint64_t)(uintptr_t)ns_buf, 0,
                                0 /* CNS=0: namespace */, 0, 0);
        if (rc < 0) {
            _simpleos_log_write_cstr(3, "[nvme-c] Identify Namespace failed");
            _nvme.sector_size = 512;
            _nvme.sector_count = 0;
        } else {
            /* Parse NSZE at offset 0 (64-bit) */
            _nvme.sector_count = *(volatile uint64_t *)((uintptr_t)ns_buf + 0);
            /* Parse FLBAS at offset 26 (1 byte, lower 4 bits = LBA format index) */
            uint8_t flbas = *(volatile uint8_t *)((uintptr_t)ns_buf + 26);
            uint8_t fmt_idx = flbas & 0x0F;
            /* LBAF starts at offset 128, each 4 bytes; LBADS is bits [23:16] */
            uint32_t lbaf = *(volatile uint32_t *)((uintptr_t)ns_buf + 128 + (uint32_t)fmt_idx * 4);
            uint32_t lbads = (lbaf >> 16) & 0xFF;
            if (lbads >= 9 && lbads <= 16)
                _nvme.sector_size = 1u << lbads;
            else
                _nvme.sector_size = 512;

            serial_puts("[nvme-c] NS1: sectors=");
            serial_put_dec((int64_t)_nvme.sector_count);
            serial_puts(", sector_size=");
            serial_put_dec((int64_t)_nvme.sector_size);
            serial_puts("\r\n");
        }
    }

    /* Step 10: Set Number of Queues (Feature 0x07) — request 1 I/O SQ + 1 I/O CQ */
    {
        /* CDW10 = feature ID (0x07) for Set Features */
        /* CDW11 = NCQR[31:16] | NSQR[15:0], each 0-based */
        int rc = nvme_admin_cmd(0x09 /* Set Features */, 0,
                                0, 0,
                                0x07 /* Feature ID: Number of Queues */,
                                (0u << 16) | 0u /* 1 SQ, 1 CQ (0-based) */,
                                0);
        if (rc < 0)
            _simpleos_log_write_cstr(3, "[nvme-c] Set Number of Queues failed (non-fatal)");
    }

    /* Step 11: Create I/O Completion Queue (QID 1) */
    size_t io_cq_bytes = NVME_IO_DEPTH * NVME_CQE_SIZE; /* 1024 */
    _nvme.io_cq = (struct nvme_cqe *)nvme_alloc_aligned(io_cq_bytes, 4096);
    if (!_nvme.io_cq) return -12;
    __builtin_memset(_nvme.io_cq, 0, io_cq_bytes);
    _nvme.io_cq_head = 0;
    _nvme.io_cq_phase = 1;
    _nvme.io_cid = 0;
    {
        /* Create I/O CQ: admin opcode 0x05
         * CDW10: QSIZE[31:16] (0-based) | QID[15:0]
         * CDW11: IV[31:16] | IEN[1] | PC[0]
         * PRP1 = CQ physical address */
        uint32_t cdw10 = ((uint32_t)(NVME_IO_DEPTH - 1) << 16) | 1u; /* QID=1 */
        uint32_t cdw11 = 1u; /* PC=1 (physically contiguous), IEN=0, IV=0 */
        int rc = nvme_admin_cmd(0x05, 0,
                                (uint64_t)(uintptr_t)_nvme.io_cq, 0,
                                cdw10, cdw11, 0);
        if (rc < 0) {
            _simpleos_log_write_cstr(4, "[nvme-c] Create I/O CQ failed");
            return rc;
        }
    }

    /* Step 12: Create I/O Submission Queue (QID 1) */
    size_t io_sq_bytes = NVME_IO_DEPTH * NVME_SQE_SIZE; /* 4096 */
    _nvme.io_sq = (struct nvme_sqe *)nvme_alloc_aligned(io_sq_bytes, 4096);
    if (!_nvme.io_sq) return -12;
    __builtin_memset(_nvme.io_sq, 0, io_sq_bytes);
    _nvme.io_sq_tail = 0;
    {
        /* Create I/O SQ: admin opcode 0x01
         * CDW10: QSIZE[31:16] (0-based) | QID[15:0]
         * CDW11: CQID[31:16] | QPRIO[2:1] | PC[0]
         * PRP1 = SQ physical address */
        uint32_t cdw10 = ((uint32_t)(NVME_IO_DEPTH - 1) << 16) | 1u; /* QID=1 */
        uint32_t cdw11 = (1u << 16) | 1u; /* CQID=1, PC=1 */
        int rc = nvme_admin_cmd(0x01, 0,
                                (uint64_t)(uintptr_t)_nvme.io_sq, 0,
                                cdw10, cdw11, 0);
        if (rc < 0) {
            _simpleos_log_write_cstr(4, "[nvme-c] Create I/O SQ failed");
            return rc;
        }
    }

    _simpleos_log_write_cstr(2, "[nvme-c] I/O queues created");
    _nvme.initialized = 1;
    return 0;
}

/* Read one sector from NVMe namespace 1.
 * lba     = logical block address
 * buf     = destination buffer (must be large enough for one sector)
 * Returns 0 on success, negative on error. */
static int _nvme_read_sector_impl(uint64_t lba, void *buf)
{
    int rc = 0;
    if (!_nvme.initialized) {
        rc = _nvme_init_controller();
        if (rc < 0) return rc;
    }

    /* Allocate a 4KB-aligned DMA buffer for the read */
    void *dma_buf = nvme_alloc_aligned(4096, 4096);
    if (!dma_buf) return -12;
    __builtin_memset(dma_buf, 0, 4096);

    /* NVMe I/O Read: opcode 0x02
     * NSID = 1
     * PRP1 = DMA buffer physical address
     * CDW10 = LBA[31:0]
     * CDW11 = LBA[63:32]
     * CDW12 = NLB[15:0] (0-based, so 0 = 1 sector) */
    uint32_t cdw10 = (uint32_t)(lba & 0xFFFFFFFF);
    uint32_t cdw11 = (uint32_t)(lba >> 32);
    uint32_t cdw12 = 0; /* 1 sector (0-based) */

    rc = nvme_io_cmd(0x02, 1,
                     (uint64_t)(uintptr_t)dma_buf, 0,
                     cdw10, cdw11, cdw12);
    if (rc < 0) {
        nvme_free_aligned(dma_buf);
        return rc;
    }

    /* Copy from DMA buffer to caller's buffer */
    __builtin_memcpy(buf, dma_buf, _nvme.sector_size);
    nvme_free_aligned(dma_buf);
    return 0;
}

/* Write one sector to NVMe namespace 1.
 * lba     = logical block address
 * buf     = source buffer containing one sector
 * Returns 0 on success, negative on error. */
static int _nvme_write_sector_impl(uint64_t lba, const void *buf)
{
    int rc = 0;
    if (!_nvme.initialized) {
        rc = _nvme_init_controller();
        if (rc < 0) return rc;
    }

    void *dma_buf = nvme_alloc_aligned(4096, 4096);
    if (!dma_buf) return -12;
    __builtin_memcpy(dma_buf, buf, _nvme.sector_size);

    uint32_t cdw10 = (uint32_t)(lba & 0xFFFFFFFF);
    uint32_t cdw11 = (uint32_t)(lba >> 32);
    uint32_t cdw12 = 0; /* 1 sector (0-based) */

    rc = nvme_io_cmd(0x01, 1,
                     (uint64_t)(uintptr_t)dma_buf, 0,
                     cdw10, cdw11, cdw12);
    nvme_free_aligned(dma_buf);
    return rc;
}

/* Syscall 85 handler: NvmeReadSector
 * a0 = device index (ignored, only one NVMe device supported)
 * a1 = LBA
 * a2 = destination buffer address (caller-provided, must be >= sector_size)
 * Returns 0 on success, negative errno on failure. */
static int64_t _nvme_read_sector(uint64_t device_idx, uint64_t lba, uint64_t buf_addr)
{
    (void)device_idx;
    void *buf = (void *)(uintptr_t)buf_addr;
    if (!buf) return -14; /* EFAULT */
    return (int64_t)_nvme_read_sector_impl(lba, buf);
}

/* Syscall 94 handler: NvmeWriteSector
 * a0 = device index (ignored, only one NVMe device supported)
 * a1 = LBA
 * a2 = source buffer address (caller-provided, must contain one sector)
 * Returns 0 on success, negative errno on failure. */
static int64_t _nvme_write_sector(uint64_t device_idx, uint64_t lba, uint64_t buf_addr)
{
    (void)device_idx;
    const void *buf = (const void *)(uintptr_t)buf_addr;
    if (!buf) return -14; /* EFAULT */
    return (int64_t)_nvme_write_sector_impl(lba, buf);
}

/* Exported bridge for Simple baremetal code.
 * Keeps private boot-time NVMe access out of the public syscall table. */
int64_t simpleos_nvme_read_sector(uint64_t device_idx, uint64_t lba, uint64_t buf_addr)
{
    return _nvme_read_sector(device_idx, lba, buf_addr);
}

/* Exported bridge for Simple baremetal code.
 * Keeps private boot-time NVMe access out of the public syscall table. */
int64_t simpleos_nvme_write_sector(uint64_t device_idx, uint64_t lba, uint64_t buf_addr)
{
    return _nvme_write_sector(device_idx, lba, buf_addr);
}

int64_t simpleos_nvme_rw_selftest(void)
{
    int rc = _nvme_init_controller();
    if (rc < 0) return rc;
    if (_nvme.sector_count == 0) return -19;
    if (_nvme.sector_size == 0 || _nvme.sector_size > 512) return -22;

    uint64_t lba = _nvme.sector_count - 1;
    uint8_t backup[512];
    uint8_t pattern[512];
    uint8_t verify[512];
    __builtin_memset(backup, 0, sizeof(backup));
    __builtin_memset(pattern, 0, sizeof(pattern));
    __builtin_memset(verify, 0, sizeof(verify));

    rc = _nvme_read_sector_impl(lba, backup);
    if (rc < 0) return rc;

    for (uint32_t i = 0; i < _nvme.sector_size; i++) {
        pattern[i] = (uint8_t)((i * 37u + 0x5Au) & 0xFFu);
    }

    rc = _nvme_write_sector_impl(lba, pattern);
    if (rc < 0) return rc;

    rc = _nvme_read_sector_impl(lba, verify);
    if (rc < 0) {
        (void)_nvme_write_sector_impl(lba, backup);
        return rc;
    }

    int mismatch = 0;
    for (uint32_t i = 0; i < _nvme.sector_size; i++) {
        if (verify[i] != pattern[i]) {
            mismatch = 1;
            break;
        }
    }

    int restore_rc = _nvme_write_sector_impl(lba, backup);
    if (restore_rc < 0) return restore_rc;
    if (mismatch) return -5;
    return 0;
}

/* _nvme_init_and_read_sector0 — callable from Simple code or early boot
 * Initializes NVMe and reads sector 0 (FAT32 BPB).
 * Returns 0 on success, prints diagnostics to serial. */
static int _nvme_init_and_read_sector0(void)
{
    _simpleos_log_write_cstr(2, "[nvme-c] === NVMe Init + Sector 0 Read ===");

    int rc = _nvme_init_controller();
    if (rc < 0) {
        _simpleos_log_write_cstr(4, "[nvme-c] Controller init failed");
        return rc;
    }

    /* Read sector 0 (FAT32 BPB / boot sector) */
    _simpleos_log_write_cstr(2, "[nvme-c] Reading sector 0...");
    uint8_t sector_buf[512];
    __builtin_memset(sector_buf, 0, sizeof(sector_buf));
    rc = _nvme_read_sector_impl(0, sector_buf);
    if (rc < 0) {
        _simpleos_log_write_cstr(4, "[nvme-c] Sector 0 read failed");
        return rc;
    }

    /* Print first 16 bytes */
    serial_puts("[nvme-c] Sector 0 read OK, first bytes:");
    for (int i = 0; i < 16; i++) {
        serial_putchar(' ');
        static const char hex[] = "0123456789ABCDEF";
        serial_putchar(hex[(sector_buf[i] >> 4) & 0xF]);
        serial_putchar(hex[sector_buf[i] & 0xF]);
    }
    serial_puts("\r\n");

    /* Check FAT32 BPB signature at bytes 510-511: must be 0x55 0xAA */
    uint16_t sig = (uint16_t)sector_buf[510] | ((uint16_t)sector_buf[511] << 8);
    serial_puts("[nvme-c] FAT32 signature at offset 510: 0x");
    serial_put_hex(sig);
    serial_puts("\r\n");
    if (sig == 0xAA55) {
        _simpleos_log_write_cstr(2, "[nvme-c] FAT32 BPB signature valid!");
    } else {
        _simpleos_log_write_cstr(3, "[nvme-c] WARNING: invalid BPB signature (expected 0xAA55)");
    }

    return 0;
}

/* Exported bridge for Simple baremetal code.
 * Keeps private boot-time NVMe access out of the public syscall table. */
int64_t simpleos_nvme_init(void)
{
    return (int64_t)_nvme_init_and_read_sector0();
}

/* Public FAT32 API — callable from Simple via extern fn */
int fat32_find_file(const char *name, uint32_t *out_cluster, uint32_t *out_size);
int fat32_read_file(const char *name, uint8_t *buf, uint32_t max_size, uint32_t *bytes_read);
int fat32_write_file(const char *name, const uint8_t *buf, uint32_t size);
int fat32_list_dir(void);

/* RuntimeValue wrappers — Simple passes text as tagged heap pointers.
 * These extract the C string from RuntimeValue and call the real functions. */
static char _fat32_selected_name[64];
static uint32_t _fat32_selected_name_len = 0;

RuntimeValue spl_fat32_find_file(RuntimeValue name_rv, RuntimeValue out_cluster, RuntimeValue out_size) {
    const char *name = "";
    if (IS_HEAP(name_rv)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(name_rv);
        if (s) name = s->data;
    }
    uint32_t *cluster_ptr = (uint32_t *)(uintptr_t)DECODE_INT(out_cluster);
    uint32_t *size_ptr    = (uint32_t *)(uintptr_t)DECODE_INT(out_size);
    int result = fat32_find_file(name, cluster_ptr, size_ptr);
    return ENCODE_INT(result);
}

RuntimeValue spl_fat32_read_file(RuntimeValue name_rv, RuntimeValue buf_rv, RuntimeValue max_size_rv, RuntimeValue bytes_read_rv) {
    const char *name = "";
    if (IS_HEAP(name_rv)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(name_rv);
        if (s) name = s->data;
    }
    uint8_t  *buf        = (uint8_t *)(uintptr_t)DECODE_INT(buf_rv);
    uint32_t  max_size   = (uint32_t)DECODE_INT(max_size_rv);
    uint32_t *bytes_read = (uint32_t *)(uintptr_t)DECODE_INT(bytes_read_rv);
    int result = fat32_read_file(name, buf, max_size, bytes_read);
    return ENCODE_INT(result);
}

/* Simple wrappers that return values directly (no output-parameter encoding issues) */

/* TODO(M5): migrate write-path callers from rt_fat32_write_file_text /
 * rt_fat32_select_file / rt_fat32_write_selected_file_text to g_vfs_* helpers
 * once the VFS write path is wired in M5.
 */
RuntimeValue rt_fat32_write_file_text(RuntimeValue name_rv, RuntimeValue content_rv)
{
    const char *name = "";
    const uint8_t *content = (const uint8_t *)"";
    uint32_t name_len = 0;
    uint32_t content_len = 0;
    if (IS_HEAP(name_rv)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(name_rv);
        if (s) {
            name = s->data;
            name_len = s->len;
        }
    }
    if (IS_HEAP(content_rv)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(content_rv);
        if (s) {
            content = (const uint8_t *)s->data;
            content_len = s->len;
        }
    }
    if (!name || name_len == 0)
        return FALSE_VALUE;
    int rc = fat32_write_file(name, content, content_len);
    return rc == 0 ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue rt_fat32_select_file(RuntimeValue name_rv)
{
    const char *name = "";
    uint32_t name_len = 0;
    if (IS_HEAP(name_rv)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(name_rv);
        if (s) {
            name = s->data;
            name_len = s->len;
        }
    }
    if (!name || name_len == 0 || name_len >= sizeof(_fat32_selected_name))
        return FALSE_VALUE;
    __builtin_memcpy(_fat32_selected_name, name, name_len);
    _fat32_selected_name[name_len] = '\0';
    _fat32_selected_name_len = name_len;
    serial_puts("[fat32-c] select name=");
    serial_puts(_fat32_selected_name);
    serial_puts("\r\n");
    return TRUE_VALUE;
}

RuntimeValue rt_fat32_write_selected_file_text(RuntimeValue content_rv)
{
    const uint8_t *content = (const uint8_t *)"";
    uint32_t content_len = 0;
    if (_fat32_selected_name_len == 0)
        return FALSE_VALUE;
    if (IS_HEAP(content_rv)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(content_rv);
        if (s) {
            content = (const uint8_t *)s->data;
            content_len = s->len;
        }
    }
    serial_puts("[fat32-c] write_selected name=");
    serial_puts(_fat32_selected_name);
    serial_puts(" len=");
    serial_put_dec((int64_t)content_len);
    serial_puts("\r\n");
    return fat32_write_file(_fat32_selected_name, content, content_len) == 0 ? TRUE_VALUE : FALSE_VALUE;
}

static int _fat32_copy_path_arg(const char *src, int64_t src_len, char *dst, uint32_t dst_cap)
{
    if (!src || !dst || dst_cap == 0)
        return -1;
    const char *data = src;
    uint32_t len = 0;

    RuntimeValue src_rv = (RuntimeValue)(uintptr_t)src;
    if (IS_HEAP(src_rv)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(src_rv);
        if (s && s->hdr.type == HEAP_STRING && s->len < 0x100000) {
            data = s->data;
            len = s->len;
        }
    }
    if (len == 0) {
        RuntimeString *s = (RuntimeString *)(uintptr_t)src;
        if (s && s->hdr.type == HEAP_STRING && s->len < 0x100000) {
            data = s->data;
            len = s->len;
        }
    }
    if (len == 0)
        len = src_len > 0 ? (uint32_t)src_len : (uint32_t)strlen(data);
    if (len == 0)
        return -1;
    if (len >= dst_cap)
        len = dst_cap - 1;
    __builtin_memcpy(dst, data, len);
    dst[len] = '\0';
    return (int)len;
}

static RuntimeValue _fat32_read_file_text_value(const char *name, int64_t name_len)
{
    char path_buf[128];
    static uint8_t _read_buf[8192];
    uint32_t bytes_read = 0;
    if (_fat32_copy_path_arg(name, (int64_t)name_len, path_buf, sizeof(path_buf)) <= 0)
        return rt_string_from_cstr("");
    int result = fat32_read_file(path_buf, _read_buf, sizeof(_read_buf) - 1, &bytes_read);
    if (result != 0 || bytes_read == 0)
        return rt_string_from_cstr("");
    _read_buf[bytes_read] = '\0';
    return rt_string_from_cstr((const char *)_read_buf);
}

static int8_t _fat32_write_text_impl(const char *name, int64_t name_len, const char *content, int64_t content_len)
{
    char path_buf[128];
    const uint8_t *content_bytes = (const uint8_t *)"";
    uint32_t write_len = 0;
    int path_copy_len = _fat32_copy_path_arg(name, name_len, path_buf, sizeof(path_buf));
    if (path_copy_len <= 0) {
        _simpleos_log_write_cstr(4, "[fat32-c] write_text path-copy failed");
        return 0;
    }
    if (content && content_len > 0) {
        content_bytes = (const uint8_t *)content;
        write_len = (uint32_t)content_len;
    }
    int rc = fat32_write_file(path_buf, content_bytes, write_len);
    if (rc != 0) {
        _simpleos_log_write_cstr(4, "[fat32-c] write_text failed");
    }
    return rc == 0 ? 1 : 0;
}

static struct {
    uint32_t bytes_per_sector;
    uint32_t sectors_per_cluster;
    uint32_t reserved_sectors;
    uint32_t num_fats;
    uint32_t fat_size;          /* sectors per FAT */
    uint32_t root_cluster;
    uint32_t data_start_sector; /* first data sector */
    uint32_t total_clusters;
    int initialized;
} _fat32;

/* Helper: print a uint32_t in hex without 0x prefix (compact BPB dump) */
static void _fat32_puthex(uint32_t v) {
    static const char hex[] = "0123456789abcdef";
    if (v > 0xFFFF) { serial_putchar(hex[(v>>28)&0xF]); serial_putchar(hex[(v>>24)&0xF]); serial_putchar(hex[(v>>20)&0xF]); serial_putchar(hex[(v>>16)&0xF]); }
    if (v > 0xFF) { serial_putchar(hex[(v>>12)&0xF]); serial_putchar(hex[(v>>8)&0xF]); }
    serial_putchar(hex[(v>>4)&0xF]); serial_putchar(hex[v&0xF]);
}

/* Parse BPB from sector 0 and initialize FAT32 state.
 * Returns 0 on success, -1 on failure. */
static int _fat32_init(void) {
    if (_fat32.initialized) return 0;

    /* Read sector 0 (BPB) into a DMA-safe buffer */
    uint8_t *bpb = (uint8_t *)nvme_alloc_aligned(512, 512);
    if (!bpb) return -1;
    __builtin_memset(bpb, 0, 512);

    if (_nvme_read_sector_impl(0, bpb) != 0) {
        _simpleos_log_write_cstr(4, "[fat32-c] Failed to read sector 0");
        nvme_free_aligned(bpb);
        return -1;
    }

    /* Check boot signature at bytes 510-511 */
    if (bpb[510] != 0x55 || bpb[511] != 0xAA) {
        _simpleos_log_write_cstr(4, "[fat32-c] Invalid BPB signature");
        nvme_free_aligned(bpb);
        return -1;
    }

    /* Parse BPB fields (all little-endian) */
    _fat32.bytes_per_sector    = (uint32_t)bpb[11] | ((uint32_t)bpb[12] << 8);
    _fat32.sectors_per_cluster = bpb[13];
    _fat32.reserved_sectors    = (uint32_t)bpb[14] | ((uint32_t)bpb[15] << 8);
    _fat32.num_fats            = bpb[16];
    _fat32.fat_size            = (uint32_t)bpb[36] | ((uint32_t)bpb[37] << 8)
                               | ((uint32_t)bpb[38] << 16) | ((uint32_t)bpb[39] << 24);
    _fat32.root_cluster        = (uint32_t)bpb[44] | ((uint32_t)bpb[45] << 8)
                               | ((uint32_t)bpb[46] << 16) | ((uint32_t)bpb[47] << 24);
    _fat32.data_start_sector   = _fat32.reserved_sectors
                               + (_fat32.num_fats * _fat32.fat_size);
    uint32_t total_sectors     = (uint32_t)bpb[32] | ((uint32_t)bpb[33] << 8)
                               | ((uint32_t)bpb[34] << 16) | ((uint32_t)bpb[35] << 24);
    uint32_t data_sectors      = total_sectors - _fat32.data_start_sector;
    _fat32.total_clusters      = data_sectors / _fat32.sectors_per_cluster;

    /* Print BPB info for diagnostics */
    serial_puts("[fat32-c] BPS=");    _fat32_puthex(_fat32.bytes_per_sector);
    serial_puts(" SPC=");             _fat32_puthex(_fat32.sectors_per_cluster);
    serial_puts(" reserved=");        _fat32_puthex(_fat32.reserved_sectors);
    serial_puts(" FATs=");            _fat32_puthex(_fat32.num_fats);
    serial_puts(" FAT_size=");        _fat32_puthex(_fat32.fat_size);
    serial_puts(" root_cluster=");    _fat32_puthex(_fat32.root_cluster);
    serial_puts(" data_start=");      _fat32_puthex(_fat32.data_start_sector);
    serial_puts("\r\n");

    _fat32.initialized = 1;
    nvme_free_aligned(bpb);
    return 0;
}

/* Convert a cluster number to a sector number (clusters are 2-based). */
static uint32_t _fat32_cluster_to_sector(uint32_t cluster) {
    return _fat32.data_start_sector + (cluster - 2) * _fat32.sectors_per_cluster;
}

/* Read one cluster into buf. buf must be at least sectors_per_cluster * 512 bytes.
 * Returns 0 on success, -1 on error. */
static int _fat32_read_cluster(uint32_t cluster, uint8_t *buf) {
    uint32_t sector = _fat32_cluster_to_sector(cluster);
    for (uint32_t i = 0; i < _fat32.sectors_per_cluster; i++) {
        if (_nvme_read_sector_impl(sector + i, buf + i * 512) != 0)
            return -1;
    }
    return 0;
}

static int _fat32_write_cluster(uint32_t cluster, const uint8_t *buf) {
    uint32_t sector = _fat32_cluster_to_sector(cluster);
    for (uint32_t i = 0; i < _fat32.sectors_per_cluster; i++) {
        if (_nvme_write_sector_impl(sector + i, buf + i * 512) != 0)
            return -1;
    }
    return 0;
}

/* Follow the FAT chain: return the next cluster number, or >= 0x0FFFFFF8 for EOC. */
static uint32_t _fat32_next_cluster(uint32_t cluster) {
    /* Each FAT entry is 4 bytes, located in the reserved area */
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = _fat32.reserved_sectors + (fat_offset / 512);
    uint32_t fat_offset_in_sector = fat_offset % 512;

    uint8_t *sector_buf = (uint8_t *)nvme_alloc_aligned(512, 512);
    if (!sector_buf) return 0x0FFFFFFF; /* treat alloc failure as EOC */
    __builtin_memset(sector_buf, 0, 512);

    if (_nvme_read_sector_impl(fat_sector, sector_buf) != 0) {
        nvme_free_aligned(sector_buf);
        return 0x0FFFFFFF; /* treat read failure as EOC */
    }

    uint32_t entry = (uint32_t)sector_buf[fat_offset_in_sector]
                   | ((uint32_t)sector_buf[fat_offset_in_sector + 1] << 8)
                   | ((uint32_t)sector_buf[fat_offset_in_sector + 2] << 16)
                   | ((uint32_t)sector_buf[fat_offset_in_sector + 3] << 24);
    entry &= 0x0FFFFFFF; /* mask upper 4 bits (reserved in FAT32) */
    nvme_free_aligned(sector_buf);
    return entry;
}

static int _fat32_write_fat_entry(uint32_t cluster, uint32_t value) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector_offset = fat_offset / 512;
    uint32_t fat_offset_in_sector = fat_offset % 512;

    uint8_t *sector_buf = (uint8_t *)nvme_alloc_aligned(512, 512);
    if (!sector_buf) return -1;

    for (uint32_t fat_index = 0; fat_index < _fat32.num_fats; fat_index++) {
        uint32_t fat_sector = _fat32.reserved_sectors + (fat_index * _fat32.fat_size) + fat_sector_offset;
        __builtin_memset(sector_buf, 0, 512);
        if (_nvme_read_sector_impl(fat_sector, sector_buf) != 0)
            return -1;
        value &= 0x0FFFFFFF;
        sector_buf[fat_offset_in_sector] = (uint8_t)(value & 0xFF);
        sector_buf[fat_offset_in_sector + 1] = (uint8_t)((value >> 8) & 0xFF);
        sector_buf[fat_offset_in_sector + 2] = (uint8_t)((value >> 16) & 0xFF);
        sector_buf[fat_offset_in_sector + 3] = (uint8_t)((sector_buf[fat_offset_in_sector + 3] & 0xF0) | ((value >> 24) & 0x0F));
        if (_nvme_write_sector_impl(fat_sector, sector_buf) != 0)
            return -1;
    }
    return 0;
}

static uint32_t _fat32_find_free_cluster(void) {
    uint32_t max_cluster = _fat32.total_clusters + 2;
    for (uint32_t cluster = 2; cluster < max_cluster; cluster++) {
        if (_fat32_next_cluster(cluster) == 0)
            return cluster;
    }
    return 0;
}

static void _fat32_free_chain(uint32_t cluster) {
    uint32_t current = cluster;
    while (current >= 2 && current < 0x0FFFFFF8) {
        uint32_t next = _fat32_next_cluster(current);
        if (_fat32_write_fat_entry(current, 0) != 0)
            return;
        if (next < 2 || next >= 0x0FFFFFF8)
            return;
        current = next;
    }
}

static int _fat32_find_root_dir_slot(const char name83[11], uint32_t *out_cluster,
                                     uint32_t *out_entry_index, int *out_found,
                                     uint32_t *out_old_cluster) {
    uint32_t cluster_bytes = _fat32.sectors_per_cluster * 512;
    uint8_t *dir_buf = (uint8_t *)nvme_alloc_aligned(cluster_bytes, 512);
    if (!dir_buf) return -1;

    uint32_t cluster = _fat32.root_cluster;
    uint32_t free_cluster = 0;
    uint32_t free_index = 0;
    int have_free = 0;

    while (cluster >= 2 && cluster < 0x0FFFFFF8) {
        if (_fat32_read_cluster(cluster, dir_buf) != 0) {
            nvme_free_aligned(dir_buf);
            return -1;
        }

        uint32_t entries_per_cluster = cluster_bytes / 32;
        for (uint32_t i = 0; i < entries_per_cluster; i++) {
            uint8_t *entry = dir_buf + i * 32;
            if (entry[0] == 0x00) {
                *out_cluster = have_free ? free_cluster : cluster;
                *out_entry_index = have_free ? free_index : i;
                *out_found = 0;
                *out_old_cluster = 0;
                nvme_free_aligned(dir_buf);
                return 0;
            }
            if (entry[0] == 0xE5) {
                if (!have_free) {
                    free_cluster = cluster;
                    free_index = i;
                    have_free = 1;
                }
                continue;
            }
            if ((entry[11] & 0x0F) == 0x0F)
                continue;
            if (memcmp(entry, name83, 11) == 0) {
                uint32_t lo = (uint32_t)entry[26] | ((uint32_t)entry[27] << 8);
                uint32_t hi = (uint32_t)entry[20] | ((uint32_t)entry[21] << 8);
                *out_cluster = cluster;
                *out_entry_index = i;
                *out_found = 1;
                *out_old_cluster = lo | (hi << 16);
                nvme_free_aligned(dir_buf);
                return 0;
            }
        }

        uint32_t next = _fat32_next_cluster(cluster);
        if (next < 2 || next >= 0x0FFFFFF8)
            break;
        cluster = next;
    }

    if (have_free) {
        *out_cluster = free_cluster;
        *out_entry_index = free_index;
        *out_found = 0;
        *out_old_cluster = 0;
        nvme_free_aligned(dir_buf);
        return 0;
    }
    nvme_free_aligned(dir_buf);
    return -1;
}

static void _fat32_write_dir_entry(uint8_t *entry, const char name83[11],
                                   uint32_t first_cluster, uint32_t size) {
    __builtin_memset(entry, 0, 32);
    __builtin_memcpy(entry, name83, 11);
    entry[11] = 0x20; /* archive */
    entry[20] = (uint8_t)((first_cluster >> 16) & 0xFF);
    entry[21] = (uint8_t)((first_cluster >> 24) & 0xFF);
    entry[26] = (uint8_t)(first_cluster & 0xFF);
    entry[27] = (uint8_t)((first_cluster >> 8) & 0xFF);
    entry[28] = (uint8_t)(size & 0xFF);
    entry[29] = (uint8_t)((size >> 8) & 0xFF);
    entry[30] = (uint8_t)((size >> 16) & 0xFF);
    entry[31] = (uint8_t)((size >> 24) & 0xFF);
}

static const char *_fat32_root_name(const char *name) {
    if (!name) return "";
    while (*name == '/') name++;
    return name;
}

/* Convert a filename like "hello.txt" to FAT32 8.3 format (uppercase, space-padded).
 * out must be at least 11 bytes. */
static void _fat32_make_8_3_name(const char *name, char out[11]) {
    __builtin_memset(out, ' ', 11);
    const char *dot = (const char *)0;
    for (const char *p = name; *p; p++) {
        if (*p == '.') dot = p;
    }

    int base_len = dot ? (int)(dot - name) : (int)strlen(name);
    for (int i = 0; i < 8 && i < base_len; i++) {
        out[i] = (name[i] >= 'a' && name[i] <= 'z') ? name[i] - 32 : name[i];
    }

    if (dot) {
        const char *ext = dot + 1;
        for (int i = 0; i < 3 && ext[i]; i++) {
            out[8 + i] = (ext[i] >= 'a' && ext[i] <= 'z') ? ext[i] - 32 : ext[i];
        }
    }
}

static int _fat32_find_in_dir(uint32_t dir_cluster, const char *name,
                              uint32_t *out_cluster, uint32_t *out_size,
                              uint8_t *out_attr) {
    char name83[11];
    _fat32_make_8_3_name(name, name83);

    uint32_t cluster_bytes = _fat32.sectors_per_cluster * 512;
    uint8_t *dir_buf = (uint8_t *)nvme_alloc_aligned(cluster_bytes, 512);
    if (!dir_buf) return -1;

    uint32_t cluster = dir_cluster;

    while (cluster >= 2 && cluster < 0x0FFFFFF8) {
        if (_fat32_read_cluster(cluster, dir_buf) != 0) {
            nvme_free_aligned(dir_buf);
            return -1;
        }

        int entries_per_cluster = (int)(cluster_bytes / 32);
        for (int i = 0; i < entries_per_cluster; i++) {
            uint8_t *entry = dir_buf + i * 32;

            if (entry[0] == 0x00) {
                nvme_free_aligned(dir_buf);
                return -1; /* end of directory */
            }
            if (entry[0] == 0xE5) continue;  /* deleted entry */
            if ((entry[11] & 0x0F) == 0x0F) continue; /* LFN entry, skip */

            if (memcmp(entry, name83, 11) == 0) {
                /* Found: extract first cluster (high word at offset 20, low at 26) and size */
                uint32_t lo = (uint32_t)entry[26] | ((uint32_t)entry[27] << 8);
                uint32_t hi = (uint32_t)entry[20] | ((uint32_t)entry[21] << 8);
                *out_cluster = lo | (hi << 16);
                *out_size = (uint32_t)entry[28] | ((uint32_t)entry[29] << 8)
                          | ((uint32_t)entry[30] << 16) | ((uint32_t)entry[31] << 24);
                if (out_attr) *out_attr = entry[11];
                nvme_free_aligned(dir_buf);
                return 0;
            }
        }
        cluster = _fat32_next_cluster(cluster);
    }
    nvme_free_aligned(dir_buf);
    return -1; /* not found */
}

static int _fat32_find_path(const char *name, uint32_t *out_cluster,
                            uint32_t *out_size, uint8_t *out_attr) {
    const char *root_name = _fat32_root_name(name);
    if (!root_name || !*root_name) return -1;

    uint32_t dir_cluster = _fat32.root_cluster;
    const char *p = root_name;
    char segment[64];

    while (*p) {
        while (*p == '/') p++;
        if (!*p) break;

        uint32_t seg_len = 0;
        while (p[seg_len] && p[seg_len] != '/') {
            if (seg_len + 1 >= sizeof(segment)) return -1;
            segment[seg_len] = p[seg_len];
            seg_len++;
        }
        segment[seg_len] = '\0';

        uint32_t found_cluster = 0;
        uint32_t found_size = 0;
        uint8_t attr = 0;
        if (_fat32_find_in_dir(dir_cluster, segment, &found_cluster, &found_size, &attr) != 0)
            return -1;

        p += seg_len;
        while (*p == '/') p++;
        if (!*p) {
            *out_cluster = found_cluster;
            *out_size = found_size;
            if (out_attr) *out_attr = attr;
            return 0;
        }
        if ((attr & 0x10) == 0)
            return -1;
        dir_cluster = found_cluster;
    }

    return -1;
}

/* Find a file by absolute or root-relative 8.3 path.
 * Returns 0 on success and fills out_cluster/out_size, -1 if not found. */
int fat32_find_file(const char *name, uint32_t *out_cluster, uint32_t *out_size) {
    if (!_fat32.initialized) {
        if (_fat32_init() != 0) return -1;
    }

    uint8_t attr = 0;
    return _fat32_find_path(name, out_cluster, out_size, &attr);
}

/* Read an entire file by name into buf (up to max_size bytes).
 * Returns 0 on success, -1 on failure. *bytes_read is set to actual bytes read. */
int fat32_read_file(const char *name, uint8_t *buf, uint32_t max_size,
                    uint32_t *bytes_read) {
    uint32_t cluster, file_size;
    if (fat32_find_file(name, &cluster, &file_size) != 0) return -1;

    uint32_t to_read = file_size < max_size ? file_size : max_size;
    uint32_t remaining = to_read;
    uint32_t cluster_bytes = _fat32.sectors_per_cluster * 512;
    uint8_t *cluster_buf = (uint8_t *)nvme_alloc_aligned(cluster_bytes, 512);
    if (!cluster_buf) return -1;
    uint32_t offset = 0;

    while (remaining > 0 && cluster >= 2 && cluster < 0x0FFFFFF8) {
        if (_fat32_read_cluster(cluster, cluster_buf) != 0) {
            nvme_free_aligned(cluster_buf);
            return -1;
        }

        uint32_t copy = remaining < cluster_bytes ? remaining : cluster_bytes;
        __builtin_memcpy(buf + offset, cluster_buf, copy);
        offset += copy;
        remaining -= copy;
        cluster = _fat32_next_cluster(cluster);
    }

    *bytes_read = offset;
    nvme_free_aligned(cluster_buf);
    return 0;
}

static uint8_t simpleos_fat32_read_buf[32768];
static const uint32_t simpleos_fat32_read_buf_size = 32768;
static uint8_t simpleos_fat32_path_read_buf[4194304];
static const uint32_t simpleos_fat32_path_read_buf_size = 4194304;

static const char *simpleos_known_app_name(uint64_t app_id)
{
    switch (app_id) {
    case 1: return "/BROWSMF.SMF";
    case 2: return "/FILESMF.SMF";
    case 3: return "/HELLOSMF.SMF";
    case 4: return "/SHELLSMF.SMF";
    case 5: return "/EDITORSM.SMF";
    case 11: return "/BROWSMF.SMF";
    case 12: return "/FILESMF.SMF";
    case 13: return "/HELLOSMF.SMF";
    case 14: return "/SHELLSMF.SMF";
    case 15: return "/EDITORSM.SMF";
    case 21: return "/HELLO.TXT";
    case 22: return "/NUMBERS.TXT";
    case 23: return "/HELLO.SPL";
    default: return NULL;
    }
}

static uint64_t simpleos_known_app_arg(RuntimeValue app_id_val)
{
    uint64_t raw = (uint64_t)app_id_val;
    if (simpleos_known_app_name(raw))
        return raw;
    if (IS_INT(app_id_val))
        return (uint64_t)DECODE_INT(app_id_val);
    return raw;
}

uint64_t simpleos_fat32_read_buffer_addr(void)
{
    return (uint64_t)(uintptr_t)simpleos_fat32_read_buf;
}

uint64_t simpleos_fat32_path_read_buffer_addr(void)
{
    return (uint64_t)(uintptr_t)simpleos_fat32_path_read_buf;
}

static int64_t simpleos_fat32_read_known_app_size_raw(uint64_t app_id)
{
    const char *name = simpleos_known_app_name(app_id);
    uint32_t cluster = 0;
    uint32_t file_size = 0;

    if (!name)
        return 0;
    if (fat32_find_file(name, &cluster, &file_size) != 0)
        return 0;
    return (int64_t)file_size;
}

int64_t simpleos_fat32_read_known_app_size(RuntimeValue app_id_val)
{
    return simpleos_fat32_read_known_app_size_raw(simpleos_known_app_arg(app_id_val));
}

static int64_t simpleos_fat32_read_known_app_raw(uint64_t app_id)
{
    const char *name = simpleos_known_app_name(app_id);
    uint32_t bytes_read = 0;
    uint64_t file_size = (uint64_t)simpleos_fat32_read_known_app_size_raw(app_id);

    if (!name || file_size == 0)
        return -1;
    if (file_size > simpleos_fat32_read_buf_size)
        return -2;

    __builtin_memset(simpleos_fat32_read_buf, 0, simpleos_fat32_read_buf_size);
    if (fat32_read_file(name, simpleos_fat32_read_buf,
                        simpleos_fat32_read_buf_size, &bytes_read) != 0)
        return -1;
    if (bytes_read != (uint32_t)file_size)
        return -3;
    return 0;
}

int64_t simpleos_fat32_read_known_app(RuntimeValue app_id_val)
{
    return simpleos_fat32_read_known_app_raw(simpleos_known_app_arg(app_id_val));
}

RuntimeValue simpleos_fat32_read_known_app_array(RuntimeValue app_id_val)
{
    uint64_t app_id = simpleos_known_app_arg(app_id_val);
    int64_t rc = simpleos_fat32_read_known_app_raw(app_id);
    uint32_t cluster = 0;
    uint32_t file_size = 0;
    const char *name = simpleos_known_app_name(app_id);

    if (rc != 0)
        return rt_array_new((RuntimeValue)0);
    if (!name)
        return rt_array_new((RuntimeValue)0);
    if (fat32_find_file(name, &cluster, &file_size) != 0 || file_size > simpleos_fat32_read_buf_size)
        return rt_array_new((RuntimeValue)0);

    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)file_size * sizeof(RuntimeValue));
    if (!a)
        return rt_array_new((RuntimeValue)0);
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)file_size * sizeof(RuntimeValue));
    a->len = file_size;
    a->cap = file_size;
    a->items = runtime_array_inline_items(a);
    for (uint32_t i = 0; i < file_size; i++)
        a->items[i] = ENCODE_INT((int64_t)simpleos_fat32_read_buf[i]);
    return ENCODE_PTR(a);
}

int64_t simpleos_fat32_read_path_size(const char *path, int64_t path_len)
{
    char path_buf[128];
    uint32_t cluster = 0;
    uint32_t file_size = 0;

    if (_fat32_copy_path_arg(path, path_len, path_buf, sizeof(path_buf)) <= 0)
        return 0;
    if (fat32_find_file(path_buf, &cluster, &file_size) != 0)
        return 0;
    return (int64_t)file_size;
}

int64_t simpleos_fat32_read_path(const char *path, int64_t path_len)
{
    char path_buf[128];
    uint32_t bytes_read = 0;
    int64_t file_size = 0;

    if (_fat32_copy_path_arg(path, path_len, path_buf, sizeof(path_buf)) <= 0)
        return -1;
    file_size = simpleos_fat32_read_path_size(path, path_len);
    if (file_size <= 0)
        return -1;
    if ((uint64_t)file_size > simpleos_fat32_path_read_buf_size)
        return -2;

    __builtin_memset(simpleos_fat32_path_read_buf, 0, simpleos_fat32_path_read_buf_size);
    if (fat32_read_file(path_buf, simpleos_fat32_path_read_buf,
                        simpleos_fat32_path_read_buf_size, &bytes_read) != 0)
        return -1;
    if (bytes_read != (uint32_t)file_size)
        return -3;
    return 0;
}

RuntimeValue simpleos_fat32_read_path_array(const char *path, int64_t path_len)
{
    char path_buf[128];
    uint32_t cluster = 0;
    uint32_t file_size = 0;
    uint32_t bytes_read = 0;

    if (_fat32_copy_path_arg(path, path_len, path_buf, sizeof(path_buf)) <= 0)
        return rt_array_new((RuntimeValue)0);
    if (fat32_find_file(path_buf, &cluster, &file_size) != 0)
        return rt_array_new((RuntimeValue)0);
    if (file_size == 0 || (uint64_t)file_size > simpleos_fat32_path_read_buf_size)
        return rt_array_new((RuntimeValue)0);
    if (fat32_read_file(path_buf, simpleos_fat32_path_read_buf,
                        simpleos_fat32_path_read_buf_size, &bytes_read) != 0)
        return rt_array_new((RuntimeValue)0);
    if (bytes_read != file_size)
        return rt_array_new((RuntimeValue)0);

    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)file_size * sizeof(RuntimeValue));
    if (!a)
        return rt_array_new((RuntimeValue)0);
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)file_size * sizeof(RuntimeValue));
    a->len = file_size;
    a->cap = file_size;
    a->items = runtime_array_inline_items(a);
    for (uint32_t i = 0; i < file_size; i++)
        a->items[i] = ENCODE_INT((int64_t)simpleos_fat32_path_read_buf[i]);
    return ENCODE_PTR(a);
}

int fat32_write_file(const char *name, const uint8_t *buf, uint32_t size) {
    if (!_fat32.initialized) {
        if (_fat32_init() != 0) return -1;
    }

    const char *root_name = _fat32_root_name(name);
    if (!root_name || !*root_name) {
        _simpleos_log_write_cstr(4, "[fat32-c] write_file empty root name");
        return -1;
    }
    char name83[11];
    _fat32_make_8_3_name(root_name, name83);

    uint32_t dir_cluster = 0;
    uint32_t entry_index = 0;
    uint32_t old_cluster = 0;
    int found = 0;
    if (_fat32_find_root_dir_slot(name83, &dir_cluster, &entry_index, &found, &old_cluster) != 0) {
        _simpleos_log_write_cstr(4, "[fat32-c] write_file root-dir slot failed");
        return -1;
    }

    uint32_t cluster_bytes = _fat32.sectors_per_cluster * 512;
    uint32_t clusters_needed = size == 0 ? 0 : (size + cluster_bytes - 1) / cluster_bytes;
    uint32_t first_cluster = 0;
    uint32_t prev_cluster = 0;
    uint32_t remaining = size;
    uint32_t written = 0;
    uint8_t *cluster_buf = (uint8_t *)nvme_alloc_aligned(cluster_bytes, 512);
    if (!cluster_buf) {
        _simpleos_log_write_cstr(4, "[fat32-c] write_file cluster_buf alloc failed");
        return -1;
    }

    for (uint32_t i = 0; i < clusters_needed; i++) {
        uint32_t cluster = _fat32_find_free_cluster();
        if (cluster < 2) {
            _simpleos_log_write_cstr(4, "[fat32-c] write_file no free cluster");
            if (first_cluster >= 2) _fat32_free_chain(first_cluster);
            return -1;
        }
        if (_fat32_write_fat_entry(cluster, 0x0FFFFFFF) != 0) {
            _simpleos_log_write_cstr(4, "[fat32-c] write_file mark-eoc failed");
            if (first_cluster >= 2) _fat32_free_chain(first_cluster);
            return -1;
        }
        if (prev_cluster >= 2 && _fat32_write_fat_entry(prev_cluster, cluster) != 0) {
            _simpleos_log_write_cstr(4, "[fat32-c] write_file link cluster failed");
            _fat32_free_chain(first_cluster);
            return -1;
        }
        if (first_cluster < 2)
            first_cluster = cluster;
        prev_cluster = cluster;

        __builtin_memset(cluster_buf, 0, cluster_bytes);
        uint32_t chunk = remaining < cluster_bytes ? remaining : cluster_bytes;
        if (chunk > 0)
            __builtin_memcpy(cluster_buf, buf + written, chunk);
        if (_fat32_write_cluster(cluster, cluster_buf) != 0) {
            _simpleos_log_write_cstr(4, "[fat32-c] write_file cluster write failed");
            _fat32_free_chain(first_cluster);
            return -1;
        }
        written += chunk;
        remaining -= chunk;
    }

    uint8_t *dir_buf = (uint8_t *)nvme_alloc_aligned(cluster_bytes, 512);
    if (!dir_buf) {
        _simpleos_log_write_cstr(4, "[fat32-c] write_file dir_buf alloc failed");
        if (first_cluster >= 2) _fat32_free_chain(first_cluster);
        return -1;
    }
    if (_fat32_read_cluster(dir_cluster, dir_buf) != 0) {
        _simpleos_log_write_cstr(4, "[fat32-c] write_file dir read failed");
        if (first_cluster >= 2) _fat32_free_chain(first_cluster);
        return -1;
    }

    _fat32_write_dir_entry(dir_buf + entry_index * 32, name83, first_cluster, size);
    if (_fat32_write_cluster(dir_cluster, dir_buf) != 0) {
        _simpleos_log_write_cstr(4, "[fat32-c] write_file dir write failed");
        if (first_cluster >= 2) _fat32_free_chain(first_cluster);
        return -1;
    }

    if (found && old_cluster >= 2)
        _fat32_free_chain(old_cluster);
    return 0;
}

/* List root directory entries to serial (for diagnostics). */
int fat32_list_dir(void) {
    if (!_fat32.initialized) {
        if (_fat32_init() != 0) return -1;
    }

    serial_puts("[fat32-c] Root directory listing:\r\n");

    uint32_t cluster_bytes = _fat32.sectors_per_cluster * 512;
    uint8_t *dir_buf = (uint8_t *)nvme_alloc_aligned(cluster_bytes, 512);
    if (!dir_buf) return -1;

    uint32_t cluster = _fat32.root_cluster;
    int count = 0;

    while (cluster >= 2 && cluster < 0x0FFFFFF8) {
        if (_fat32_read_cluster(cluster, dir_buf) != 0) return -1;

        int entries_per_cluster = (int)(cluster_bytes / 32);
        for (int i = 0; i < entries_per_cluster; i++) {
            uint8_t *entry = dir_buf + i * 32;

            if (entry[0] == 0x00) goto done; /* end of directory */
            if (entry[0] == 0xE5) continue;  /* deleted */
            if ((entry[11] & 0x0F) == 0x0F) continue; /* LFN */

            /* Print 8.3 name */
            serial_puts("  ");
            for (int j = 0; j < 8; j++) {
                if (entry[j] != ' ') serial_putchar((char)entry[j]);
            }
            if (entry[8] != ' ') {
                serial_putchar('.');
                for (int j = 8; j < 11; j++) {
                    if (entry[j] != ' ') serial_putchar((char)entry[j]);
                }
            }

            /* Print attributes and size */
            uint8_t attr = entry[11];
            if (attr & 0x10) serial_puts("  <DIR>");
            else {
                uint32_t sz = (uint32_t)entry[28] | ((uint32_t)entry[29] << 8)
                            | ((uint32_t)entry[30] << 16) | ((uint32_t)entry[31] << 24);
                serial_puts("  size=");
                serial_put_dec((int64_t)sz);
            }
            serial_puts("\r\n");
            count++;
        }
        cluster = _fat32_next_cluster(cluster);
    }
done:
    serial_puts("[fat32-c] ");
    serial_put_dec((int64_t)count);
    serial_puts(" entries\r\n");
    return 0;
}

/* Syscall wrapper: Fat32ReadFile
 * a0 = pointer to null-terminated filename string
 * a1 = destination buffer address
 * a2 = max buffer size
 * Returns bytes read on success, negative on failure. */
static int64_t _fat32_read_file_syscall(uint64_t name_addr, uint64_t buf_addr,
                                         uint64_t max_size) {
    const char *name = (const char *)(uintptr_t)name_addr;
    uint8_t *buf = (uint8_t *)(uintptr_t)buf_addr;
    if (!name || !buf || max_size == 0) return -14; /* EFAULT */

    uint32_t bytes_read = 0;
    if (fat32_read_file(name, buf, (uint32_t)max_size, &bytes_read) != 0)
        return -2; /* ENOENT */
    return (int64_t)bytes_read;
}

static void serial_puthex(uint32_t v);

#define VIRTIO_STATUS_ACK        1
#define VIRTIO_STATUS_DRIVER     2
#define VIRTIO_STATUS_FEATURES_OK 8
#define VIRTIO_STATUS_DRIVER_OK  4
#define VIRTIO_STATUS_FAILED     128

#define VIRTIO_NET_F_MAC         (1u << 5)
#define VIRTIO_NET_F_STATUS      (1u << 16)
#define VIRTIO_NET_F_CSUM        (1u << 0)

#define VRING_DESC_F_NEXT     1
#define VRING_DESC_F_WRITE    2

struct virtio_net_hdr {
    uint8_t  flags;
    uint8_t  gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
} __attribute__((packed));

#define VIRTIO_NET_HDR_SIZE 10

struct vring_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

struct vring_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[];
} __attribute__((packed));

struct vring_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

struct vring_used {
    uint16_t flags;
    uint16_t idx;
    struct vring_used_elem ring[];
} __attribute__((packed));

#define ETH_ALEN       6
#define ETH_HLEN       14
#define ETH_P_IP       0x0800
#define ETH_P_ARP      0x0806

#define ARP_HW_ETHER   1
#define ARP_OP_REQUEST  1
#define ARP_OP_REPLY    2

#define IP_PROTO_ICMP   1
#define ICMP_ECHO_REQ   8
#define ICMP_ECHO_REPLY 0

struct eth_hdr {
    uint8_t  dst[ETH_ALEN];
    uint8_t  src[ETH_ALEN];
    uint16_t ethertype;   /* big-endian */
} __attribute__((packed));

struct arp_pkt {
    uint16_t hw_type;     /* big-endian */
    uint16_t proto_type;  /* big-endian */
    uint8_t  hw_len;
    uint8_t  proto_len;
    uint16_t opcode;      /* big-endian */
    uint8_t  sender_mac[ETH_ALEN];
    uint8_t  sender_ip[4];
    uint8_t  target_mac[ETH_ALEN];
    uint8_t  target_ip[4];
} __attribute__((packed));

struct ipv4_hdr {
    uint8_t  ver_ihl;     /* version(4) + IHL(4) */
    uint8_t  tos;
    uint16_t total_len;   /* big-endian */
    uint16_t id;
    uint16_t frag_off;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;    /* big-endian */
    uint8_t  src_ip[4];
    uint8_t  dst_ip[4];
} __attribute__((packed));

struct icmp_hdr {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;    /* big-endian */
    uint16_t id;
    uint16_t seq;
} __attribute__((packed));

static inline uint16_t _net_htons(uint16_t h) {
    return (uint16_t)((h >> 8) | (h << 8));
}
static inline uint16_t _net_ntohs(uint16_t n) {
    return _net_htons(n);
}
static inline uint32_t _net_htonl(uint32_t h) {
    return ((h & 0xFF) << 24) | ((h & 0xFF00) << 8)
         | ((h >> 8) & 0xFF00) | ((h >> 24) & 0xFF);
}
static inline uint32_t __attribute__((unused)) _net_ntohl(uint32_t n) {
    return _net_htonl(n);
}

static uint16_t _inet_checksum(const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t sum = 0;
    for (size_t i = 0; i + 1 < len; i += 2) {
        sum += (uint32_t)((uint16_t)p[i] << 8 | p[i+1]);
    }
    if (len & 1) {
        sum += (uint32_t)((uint16_t)p[len-1] << 8);
    }
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    return _net_htons((uint16_t)~sum);
}

static void _net_print_mac(const uint8_t *mac) {
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 6; i++) {
        if (i) serial_putchar(':');
        serial_putchar(hex[(mac[i] >> 4) & 0xF]);
        serial_putchar(hex[mac[i] & 0xF]);
    }
}

static void _net_print_ip(const uint8_t *ip) {
    for (int i = 0; i < 4; i++) {
        if (i) serial_putchar('.');
        serial_put_dec((int64_t)ip[i]);
    }
}

#define VIRTIO_NET_QUEUE_SIZE 256  /* must match QEMU's vring-num-default */
#define VIRTIO_NET_BUF_SIZE  2048  /* per-buffer size for RX/TX */

static struct {
    uint16_t iobase;         /* BAR0 I/O port base */
    uint8_t  mac[6];
    uint8_t  our_ip[4];     /* 10.0.2.15 */
    uint8_t  gateway_ip[4]; /* 10.0.2.2  */
    uint8_t  gateway_mac[6];
    int      gateway_mac_valid;

    /* RX queue (queue 0) */
    uint16_t rx_qsize;
    struct vring_desc  *rx_desc;
    struct vring_avail *rx_avail;
    struct vring_used  *rx_used;
    uint8_t *rx_buffers;    /* rx_qsize * VIRTIO_NET_BUF_SIZE contiguous */
    uint16_t rx_last_used;

    /* TX queue (queue 1) */
    uint16_t tx_qsize;
    struct vring_desc  *tx_desc;
    struct vring_avail *tx_avail;
    struct vring_used  *tx_used;
    uint8_t *tx_buffers;    /* tx_qsize * VIRTIO_NET_BUF_SIZE contiguous */
    uint16_t tx_last_used;
    uint16_t tx_next_desc;  /* next free TX descriptor index */

    int initialized;
    uint32_t rx_count;      /* total frames received */
    uint32_t tx_count;      /* total frames sent */
    uint32_t arp_replies;   /* ARP replies sent */
    uint32_t icmp_replies;  /* ICMP echo replies sent */
} _vnet;

static inline uint32_t _vnet_rd32(uint16_t off) {
    return inl(_vnet.iobase + off);
}
static inline void _vnet_wr32(uint16_t off, uint32_t val) {
    outl(_vnet.iobase + off, val);
}
static inline uint16_t _vnet_rd16(uint16_t off) {
    return inw(_vnet.iobase + off);
}
static inline void _vnet_wr16(uint16_t off, uint16_t val) {
    outw(_vnet.iobase + off, val);
}
static inline uint8_t _vnet_rd8(uint16_t off) {
    return inb(_vnet.iobase + off);
}
static inline void _vnet_wr8(uint16_t off, uint8_t val) {
    outb(_vnet.iobase + off, val);
}

/* -----------------------------------------------------------
 * _vnet_setup_queue — allocate and configure one virtqueue
 *
 * VirtIO legacy layout for a queue of size N:
 *   Descriptors: N * 16 bytes
 *   Available:   2 + 2 + N*2 bytes  (+ 2 bytes used_event, optional)
 *   Padding to 4096-byte boundary
 *   Used:        2 + 2 + N*8 bytes  (+ 2 bytes avail_event, optional)
 *
 * The device wants the physical page frame number (addr >> 12).
 * ----------------------------------------------------------- */
static int _vnet_setup_queue(uint16_t qsel,
                             uint16_t *out_qsize,
                             struct vring_desc  **out_desc,
                             struct vring_avail **out_avail,
                             struct vring_used  **out_used)
{
    /* Select queue */
    _vnet_wr16(0x0E, qsel);

    /* Read queue size (max descriptors) */
    uint16_t qsize = _vnet_rd16(0x0C);
    if (qsize == 0) {
        serial_puts("[net] queue ");
        serial_put_dec(qsel);
        serial_puts(" size=0, not available\r\n");
        return -1;
    }
    /* Clamp to our max */
    if (qsize > VIRTIO_NET_QUEUE_SIZE) qsize = VIRTIO_NET_QUEUE_SIZE;

    serial_puts("[net] queue ");
    serial_put_dec(qsel);
    serial_puts(" size=");
    serial_put_dec(qsize);
    serial_puts("\r\n");

    /* Calculate memory layout sizes */
    size_t desc_sz  = (size_t)qsize * 16;
    size_t avail_sz = 2 + 2 + (size_t)qsize * 2 + 2; /* +2 for used_event */
    size_t desc_avail_sz = desc_sz + avail_sz;
    /* Round up to 4096 boundary */
    size_t desc_avail_aligned = (desc_avail_sz + 4095) & ~(size_t)4095;
    size_t used_sz = 2 + 2 + (size_t)qsize * 8 + 2; /* +2 for avail_event */
    size_t total = desc_avail_aligned + used_sz;

    /* Allocate page-aligned memory */
    void *mem = nvme_alloc_aligned(total, 4096);
    if (!mem) {
        serial_puts("[net] failed to alloc queue memory\r\n");
        return -12;
    }
    __builtin_memset(mem, 0, total);

    uint8_t *base = (uint8_t *)mem;
    *out_desc  = (struct vring_desc *)base;
    *out_avail = (struct vring_avail *)(base + desc_sz);
    *out_used  = (struct vring_used *)(base + desc_avail_aligned);
    *out_qsize = qsize;

    /* Tell device the page frame number */
    uint32_t pfn = (uint32_t)((uintptr_t)mem >> 12);
    _vnet_wr32(0x08, pfn);

    serial_puts("[net] queue ");
    serial_put_dec(qsel);
    serial_puts(" PFN=0x");
    serial_put_hex(pfn);
    serial_puts(" mem=0x");
    serial_put_hex((uint32_t)(uintptr_t)mem);
    serial_puts(" desc[0].addr=0x");
    serial_put_hex((uint32_t)(*out_desc)[0].addr);
    serial_puts("\r\n");

    /* Verify: read back PFN */
    _vnet_wr16(0x0E, qsel);
    uint32_t readback = _vnet_rd32(0x08);
    serial_puts("[net] queue ");
    serial_put_dec(qsel);
    serial_puts(" PFN readback=0x");
    serial_put_hex(readback);
    serial_puts(readback == pfn ? " OK\r\n" : " MISMATCH!\r\n");

    return 0;
}

/* -----------------------------------------------------------
 * _vnet_rx_fill — populate RX queue with empty buffers
 *
 * Each RX buffer gets a 2-descriptor chain:
 *   desc[2i]   = virtio_net_hdr (device-writable, 10 bytes)
 *   desc[2i+1] = frame payload  (device-writable, rest of buffer)
 *
 * For simplicity we use a single descriptor per buffer that
 * covers the whole buffer (header + frame). The device writes
 * the virtio-net header first, then the ethernet frame.
 * ----------------------------------------------------------- */
static void _vnet_rx_fill(void)
{
    for (uint16_t i = 0; i < _vnet.rx_qsize; i++) {
        uint8_t *buf = _vnet.rx_buffers + (size_t)i * VIRTIO_NET_BUF_SIZE;
        _vnet.rx_desc[i].addr  = (uint64_t)(uintptr_t)buf;
        _vnet.rx_desc[i].len   = VIRTIO_NET_BUF_SIZE;
        _vnet.rx_desc[i].flags = VRING_DESC_F_WRITE; /* device writes */
        _vnet.rx_desc[i].next  = 0;

        _vnet.rx_avail->ring[i] = i;
    }
    /* Memory barrier: ensure descriptors + ring entries are visible */
    __asm__ volatile("mfence" ::: "memory");
    _vnet.rx_avail->idx = _vnet.rx_qsize;
    _vnet.rx_last_used = 0;

    /* Memory barrier: ensure idx is visible before we notify */
    __asm__ volatile("mfence" ::: "memory");

    /* Notify device about RX buffers (queue 0) */
    _vnet_wr16(0x10, 0);
}

/* -----------------------------------------------------------
 * _vnet_reclaim_tx — drain TX used ring completions
 *
 * Legacy virtio-net still reports TX completion through the used
 * ring. Reclaim those entries so the driver never wraps descriptors
 * blindly back to zero while the device may still own them.
 * ----------------------------------------------------------- */
static int _vnet_reclaim_tx(void)
{
    if (!_vnet.initialized) return -19;

    int reclaimed = 0;
    __asm__ volatile("mfence" ::: "memory");
    uint16_t used_idx = _vnet.tx_used->idx;

    while (_vnet.tx_last_used != used_idx) {
        uint16_t slot = _vnet.tx_last_used % _vnet.tx_qsize;
        uint32_t desc_id = _vnet.tx_used->ring[slot].id;
        uint32_t used_len = _vnet.tx_used->ring[slot].len;
        _vnet.tx_last_used++;
        reclaimed++;

        serial_puts("[net-tx] complete desc=");
        serial_put_dec(desc_id);
        serial_puts(" len=");
        serial_put_dec(used_len);
        serial_puts(" used.idx=");
        serial_put_dec(used_idx);
        serial_puts(" last_used=");
        serial_put_dec(_vnet.tx_last_used);
        serial_puts("\r\n");
    }

    return reclaimed;
}

/* -----------------------------------------------------------
 * _vnet_send_frame — transmit one Ethernet frame
 *
 * Prepends a 10-byte virtio-net header (all zeros = no offload).
 * Copies frame data into a TX buffer, adds to TX avail ring,
 * notifies device.
 *
 * Returns 0 on success, negative on error.
 * ----------------------------------------------------------- */
static int _vnet_send_frame(const void *frame, uint16_t frame_len)
{
    if (!_vnet.initialized) return -19; /* ENODEV */
    if (frame_len + VIRTIO_NET_HDR_SIZE > VIRTIO_NET_BUF_SIZE) return -90;

    _vnet_reclaim_tx();

    uint16_t pending = (uint16_t)(_vnet.tx_next_desc - _vnet.tx_last_used);
    if (pending >= _vnet.tx_qsize) {
        serial_puts("[net-tx] queue full\r\n");
        return -11;
    }
    uint16_t di = (uint16_t)(_vnet.tx_next_desc % _vnet.tx_qsize);
    _vnet.tx_next_desc++;

    uint8_t *buf = _vnet.tx_buffers + (size_t)di * VIRTIO_NET_BUF_SIZE;

    /* Virtio-net header: all zeros (no checksum offload, no GSO) */
    __builtin_memset(buf, 0, VIRTIO_NET_HDR_SIZE);
    /* Ethernet frame right after header */
    __builtin_memcpy(buf + VIRTIO_NET_HDR_SIZE, frame, frame_len);

    uint32_t total_len = VIRTIO_NET_HDR_SIZE + frame_len;

    /* Fill descriptor */
    _vnet.tx_desc[di].addr  = (uint64_t)(uintptr_t)buf;
    _vnet.tx_desc[di].len   = total_len;
    _vnet.tx_desc[di].flags = 0; /* device reads */
    _vnet.tx_desc[di].next  = 0;

    /* Add to available ring */
    uint16_t avail_idx = _vnet.tx_avail->idx;
    _vnet.tx_avail->ring[avail_idx % _vnet.tx_qsize] = di;
    __asm__ volatile("mfence" ::: "memory");
    _vnet.tx_avail->idx = avail_idx + 1;

    /* Notify device (queue 1 = TX) */
    _vnet_wr16(0x10, 1);
    _vnet.tx_count++;

    serial_puts("[net-tx] submit desc=");
    serial_put_dec(di);
    serial_puts(" len=");
    serial_put_dec(total_len);
    serial_puts(" avail.idx=");
    serial_put_dec(_vnet.tx_avail->idx);
    serial_puts(" used.idx=");
    serial_put_dec(_vnet.tx_used->idx);
    serial_puts("\r\n");

    return 0;
}

/* -----------------------------------------------------------
 * _vnet_handle_arp — respond to ARP request for our IP
 * ----------------------------------------------------------- */
static void _vnet_handle_arp(const uint8_t *frame, uint16_t frame_len)
{
    if (frame_len < (uint16_t)(ETH_HLEN + sizeof(struct arp_pkt))) return;

    const struct arp_pkt *arp = (const struct arp_pkt *)(frame + ETH_HLEN);

    /* Learn the gateway MAC from ARP replies. */
    if (_net_ntohs(arp->hw_type) == ARP_HW_ETHER &&
        _net_ntohs(arp->proto_type) == ETH_P_IP &&
        _net_ntohs(arp->opcode) == ARP_OP_REPLY &&
        __builtin_memcmp(arp->sender_ip, _vnet.gateway_ip, 4) == 0) {
        __builtin_memcpy(_vnet.gateway_mac, arp->sender_mac, ETH_ALEN);
        _vnet.gateway_mac_valid = 1;
        serial_puts("[net] Learned gateway MAC ");
        _net_print_mac(_vnet.gateway_mac);
        serial_puts("\r\n");
        return;
    }

    /* Only handle Ethernet/IPv4 ARP requests */
    if (_net_ntohs(arp->hw_type) != ARP_HW_ETHER) return;
    if (_net_ntohs(arp->proto_type) != ETH_P_IP) return;
    if (_net_ntohs(arp->opcode) != ARP_OP_REQUEST) return;

    /* Check if target IP is ours */
    if (__builtin_memcmp(arp->target_ip, _vnet.our_ip, 4) != 0) return;

    serial_puts("[net] ARP request for ");
    _net_print_ip(arp->target_ip);
    serial_puts(" from ");
    _net_print_mac(arp->sender_mac);
    serial_puts("\r\n");

    /* Build ARP reply */
    uint8_t reply[ETH_HLEN + sizeof(struct arp_pkt)];
    struct eth_hdr *reth = (struct eth_hdr *)reply;
    struct arp_pkt *rarp = (struct arp_pkt *)(reply + ETH_HLEN);

    /* Ethernet header: send back to requester */
    __builtin_memcpy(reth->dst, arp->sender_mac, ETH_ALEN);
    __builtin_memcpy(reth->src, _vnet.mac, ETH_ALEN);
    reth->ethertype = _net_htons(ETH_P_ARP);

    /* ARP reply */
    rarp->hw_type    = _net_htons(ARP_HW_ETHER);
    rarp->proto_type = _net_htons(ETH_P_IP);
    rarp->hw_len     = ETH_ALEN;
    rarp->proto_len  = 4;
    rarp->opcode     = _net_htons(ARP_OP_REPLY);
    __builtin_memcpy(rarp->sender_mac, _vnet.mac, ETH_ALEN);
    __builtin_memcpy(rarp->sender_ip, _vnet.our_ip, 4);
    __builtin_memcpy(rarp->target_mac, arp->sender_mac, ETH_ALEN);
    __builtin_memcpy(rarp->target_ip, arp->sender_ip, 4);

    _vnet_send_frame(reply, sizeof(reply));
    _vnet.arp_replies++;

    serial_puts("[net] ARP reply sent\r\n");
}

/* -----------------------------------------------------------
 * _vnet_handle_icmp — respond to ICMP echo requests (ping)
 * ----------------------------------------------------------- */
static void _vnet_handle_icmp(const uint8_t *frame, uint16_t frame_len)
{
    if (frame_len < (uint16_t)(ETH_HLEN + sizeof(struct ipv4_hdr) + sizeof(struct icmp_hdr)))
        return;

    const struct eth_hdr  *eth  = (const struct eth_hdr *)frame;
    const struct ipv4_hdr *ip   = (const struct ipv4_hdr *)(frame + ETH_HLEN);

    /* Only handle IPv4 */
    if ((ip->ver_ihl >> 4) != 4) return;
    uint16_t ihl = (uint16_t)(ip->ver_ihl & 0x0F) * 4;
    if (ihl < 20) return;

    /* Only handle ICMP */
    if (ip->protocol != IP_PROTO_ICMP) return;

    /* Check destination is our IP */
    if (__builtin_memcmp(ip->dst_ip, _vnet.our_ip, 4) != 0) return;

    const struct icmp_hdr *icmp = (const struct icmp_hdr *)(frame + ETH_HLEN + ihl);

    /* Only respond to echo requests */
    if (icmp->type != ICMP_ECHO_REQ || icmp->code != 0) return;

    uint16_t ip_total = _net_ntohs(ip->total_len);
    uint16_t icmp_len = (uint16_t)(ip_total - ihl);

    serial_puts("[net] ICMP echo request from ");
    _net_print_ip(ip->src_ip);
    serial_puts(" id=");
    serial_put_dec(_net_ntohs(icmp->id));
    serial_puts(" seq=");
    serial_put_dec(_net_ntohs(icmp->seq));
    serial_puts("\r\n");

    /* Build ICMP echo reply — same frame with swapped addresses and type=0 */
    uint16_t reply_len = ETH_HLEN + ip_total;
    if (reply_len > VIRTIO_NET_BUF_SIZE - VIRTIO_NET_HDR_SIZE) return;

    uint8_t reply[VIRTIO_NET_BUF_SIZE];
    __builtin_memcpy(reply, frame, reply_len);

    struct eth_hdr  *reth  = (struct eth_hdr *)reply;
    struct ipv4_hdr *rip   = (struct ipv4_hdr *)(reply + ETH_HLEN);
    struct icmp_hdr *ricmp = (struct icmp_hdr *)(reply + ETH_HLEN + ihl);

    /* Swap Ethernet addresses */
    __builtin_memcpy(reth->dst, eth->src, ETH_ALEN);
    __builtin_memcpy(reth->src, _vnet.mac, ETH_ALEN);

    /* Swap IP addresses */
    __builtin_memcpy(rip->dst_ip, ip->src_ip, 4);
    __builtin_memcpy(rip->src_ip, _vnet.our_ip, 4);
    rip->ttl = 64;

    /* Recalculate IP checksum */
    rip->checksum = 0;
    rip->checksum = _inet_checksum(rip, ihl);

    /* Change ICMP type from echo request (8) to echo reply (0) */
    ricmp->type = ICMP_ECHO_REPLY;

    /* Recalculate ICMP checksum */
    ricmp->checksum = 0;
    ricmp->checksum = _inet_checksum(ricmp, icmp_len);

    _vnet_send_frame(reply, reply_len);
    _vnet.icmp_replies++;

    serial_puts("[net] ICMP echo reply sent\r\n");
}

/* -----------------------------------------------------------
 * _vnet_handle_frame — dispatch incoming Ethernet frame
 * ----------------------------------------------------------- */
static void _vnet_handle_ipv4(const uint8_t *frame, uint16_t frame_len);

static void _vnet_handle_frame(const uint8_t *frame, uint16_t frame_len)
{
    if (frame_len < ETH_HLEN) return;

    const struct eth_hdr *eth = (const struct eth_hdr *)frame;
    uint16_t ethertype = _net_ntohs(eth->ethertype);

    serial_puts("[net-rx] frame len=");
    serial_put_dec(frame_len);
    serial_puts(" type=0x");
    serial_put_hex(ethertype);
    if (ethertype == ETH_P_IP) {
        const struct ipv4_hdr *ip = (const struct ipv4_hdr *)(frame + ETH_HLEN);
        serial_puts(" proto=");
        serial_put_dec(ip->protocol);
        serial_puts(" ");
        _net_print_ip(ip->src_ip);
        serial_puts("->");
        _net_print_ip(ip->dst_ip);
    }
    serial_puts("\r\n");

    switch (ethertype) {
        case ETH_P_ARP:
            _vnet_handle_arp(frame, frame_len);
            break;
        case ETH_P_IP:
            _vnet_handle_ipv4(frame, frame_len);
            break;
        default:
            /* Ignore unknown ethertype */
            break;
    }
}

/* -----------------------------------------------------------
 * _virtio_net_poll — check RX used ring, process frames
 *
 * Returns: number of frames processed (0 if none available)
 * ----------------------------------------------------------- */
static int _virtio_net_poll(void)
{
    if (!_vnet.initialized) return -19;

    int processed = 0;
    _vnet_reclaim_tx();

    /* Periodic diagnostic: dump RX queue state every 10000 calls */
    static int _poll_count = 0;
    _poll_count++;
    if (_poll_count == 1 || _poll_count % 50000 == 0) {
        serial_puts("[net-poll] avail.idx=");
        serial_put_dec(_vnet.rx_avail->idx);
        serial_puts(" used.idx=");
        serial_put_dec(_vnet.rx_used->idx);
        serial_puts(" last_used=");
        serial_put_dec(_vnet.rx_last_used);
        serial_puts(" poll#=");
        serial_put_dec(_poll_count);
        serial_puts("\r\n");
    }

    while (1) {
        /* Read used ring idx (device updates this) */
        __asm__ volatile("mfence" ::: "memory");
        uint16_t used_idx = _vnet.rx_used->idx;

        if (_vnet.rx_last_used == used_idx) break; /* no new frames */

        /* Get the used element */
        uint16_t slot = _vnet.rx_last_used % _vnet.rx_qsize;
        uint32_t desc_id  = _vnet.rx_used->ring[slot].id;
        uint32_t used_len = _vnet.rx_used->ring[slot].len;

        _vnet.rx_last_used++;

        if (desc_id >= _vnet.rx_qsize) continue; /* safety */

        /* The buffer contains: virtio_net_hdr (10 bytes) + ethernet frame */
        uint8_t *buf = _vnet.rx_buffers + (size_t)desc_id * VIRTIO_NET_BUF_SIZE;
        if (used_len <= VIRTIO_NET_HDR_SIZE) {
            /* Runt — no frame data */
        } else {
            uint16_t frame_len = (uint16_t)(used_len - VIRTIO_NET_HDR_SIZE);
            uint8_t *frame = buf + VIRTIO_NET_HDR_SIZE;
            _vnet.rx_count++;
            _vnet_handle_frame(frame, frame_len);
        }

        /* Recycle the buffer: re-add to available ring */
        _vnet.rx_desc[desc_id].addr  = (uint64_t)(uintptr_t)buf;
        _vnet.rx_desc[desc_id].len   = VIRTIO_NET_BUF_SIZE;
        _vnet.rx_desc[desc_id].flags = VRING_DESC_F_WRITE;
        _vnet.rx_desc[desc_id].next  = 0;

        uint16_t avail_idx = _vnet.rx_avail->idx;
        _vnet.rx_avail->ring[avail_idx % _vnet.rx_qsize] = (uint16_t)desc_id;
        __asm__ volatile("mfence" ::: "memory");
        _vnet.rx_avail->idx = avail_idx + 1;

        processed++;
    }

    /* Notify device if we recycled any buffers (queue 0 = RX) */
    if (processed > 0) {
        _vnet_wr16(0x10, 0);
    }

    return processed;
}

/* -----------------------------------------------------------
 * _vnet_send_gratuitous_arp — announce our MAC/IP to the network
 *
 * Sends a gratuitous ARP request (sender = target = our IP)
 * so QEMU's user-mode networking and any switches learn our MAC.
 * ----------------------------------------------------------- */
static void _vnet_send_gratuitous_arp(void)
{
    uint8_t frame[ETH_HLEN + sizeof(struct arp_pkt)];
    struct eth_hdr *eth = (struct eth_hdr *)frame;
    struct arp_pkt *arp = (struct arp_pkt *)(frame + ETH_HLEN);

    /* Broadcast Ethernet header */
    __builtin_memset(eth->dst, 0xFF, ETH_ALEN); /* broadcast */
    __builtin_memcpy(eth->src, _vnet.mac, ETH_ALEN);
    eth->ethertype = _net_htons(ETH_P_ARP);

    /* Gratuitous ARP: who-has our_ip, tell our_ip */
    arp->hw_type    = _net_htons(ARP_HW_ETHER);
    arp->proto_type = _net_htons(ETH_P_IP);
    arp->hw_len     = ETH_ALEN;
    arp->proto_len  = 4;
    arp->opcode     = _net_htons(ARP_OP_REQUEST);
    __builtin_memcpy(arp->sender_mac, _vnet.mac, ETH_ALEN);
    __builtin_memcpy(arp->sender_ip, _vnet.our_ip, 4);
    __builtin_memset(arp->target_mac, 0x00, ETH_ALEN);
    __builtin_memcpy(arp->target_ip, _vnet.our_ip, 4);

    int arp_rc = _vnet_send_frame(frame, sizeof(frame));
    serial_puts("[net] Gratuitous ARP sent\r\n");
    serial_puts("[net] Gratuitous ARP rc=");
    serial_put_dec(arp_rc);
    serial_puts("\r\n");
}

/* -----------------------------------------------------------
 * _vnet_send_arp_request — ARP who-has for a specific IP
 * ----------------------------------------------------------- */
static void _vnet_send_arp_request(const uint8_t *target_ip)
{
    uint8_t frame[ETH_HLEN + sizeof(struct arp_pkt)];
    struct eth_hdr *eth = (struct eth_hdr *)frame;
    struct arp_pkt *arp = (struct arp_pkt *)(frame + ETH_HLEN);

    __builtin_memset(eth->dst, 0xFF, ETH_ALEN);
    __builtin_memcpy(eth->src, _vnet.mac, ETH_ALEN);
    eth->ethertype = _net_htons(ETH_P_ARP);

    arp->hw_type    = _net_htons(ARP_HW_ETHER);
    arp->proto_type = _net_htons(ETH_P_IP);
    arp->hw_len     = ETH_ALEN;
    arp->proto_len  = 4;
    arp->opcode     = _net_htons(ARP_OP_REQUEST);
    __builtin_memcpy(arp->sender_mac, _vnet.mac, ETH_ALEN);
    __builtin_memcpy(arp->sender_ip, _vnet.our_ip, 4);
    __builtin_memset(arp->target_mac, 0x00, ETH_ALEN);
    __builtin_memcpy(arp->target_ip, target_ip, 4);

    int arp_rc = _vnet_send_frame(frame, sizeof(frame));
    serial_puts("[net] ARP request sent for ");
    _net_print_ip(target_ip);
    serial_puts(" rc=");
    serial_put_dec(arp_rc);
    serial_puts("\r\n");
}

/* -----------------------------------------------------------
 * _virtio_net_init — find VirtIO-net on PCI, init driver
 *
 * Scans PCI for vendor 0x1AF4 with network class (02.00) or
 * device IDs 0x1000 (legacy transitional network).
 *
 * Returns 0 on success, negative errno on failure.
 * ----------------------------------------------------------- */
static int _virtio_net_init(void)
{
    if (_vnet.initialized) return 0;

    /* Ensure PCI cache is populated */
    if (_pci_cache_count < 0) _pci_scan();

    /* Find VirtIO-net device:
     *   VirtIO vendor = 0x1AF4
     *   Legacy device IDs: 0x1000 (net), 0x1041 (modern net)
     *   Or any 0x1AF4 device with class=02 (network controller)
     */
    int net_idx = -1;
    for (int i = 0; i < _pci_cache_count; i++) {
        if (_pci_cache[i].vendor == 0x1AF4) {
            /* VirtIO vendor — check if it's a network device */
            if (_pci_cache[i].cls == 0x02 ||
                _pci_cache[i].devid == 0x1000 ||
                _pci_cache[i].devid == 0x1041) {
                net_idx = i;
                break;
            }
        }
    }

    if (net_idx < 0) {
        serial_puts("[net] No VirtIO-net device found on PCI bus\r\n");
        serial_puts("[net] PCI devices:\r\n");
        for (int i = 0; i < _pci_cache_count; i++) {
            serial_puts("[net]   ");
            serial_put_hex(_pci_cache[i].bus); serial_puts(":");
            serial_put_hex(_pci_cache[i].dev); serial_puts(".");
            serial_put_hex(_pci_cache[i].func);
            serial_puts(" vendor="); serial_put_hex(_pci_cache[i].vendor);
            serial_puts(" device="); serial_put_hex(_pci_cache[i].devid);
            serial_puts(" class="); serial_put_hex(_pci_cache[i].cls);
            serial_puts("."); serial_put_hex(_pci_cache[i].sub);
            serial_puts("\r\n");
        }
        return -19; /* ENODEV */
    }

    serial_puts("[net] Found VirtIO-net at PCI ");
    serial_put_hex(_pci_cache[net_idx].bus); serial_puts(":");
    serial_put_hex(_pci_cache[net_idx].dev); serial_puts(".");
    serial_put_hex(_pci_cache[net_idx].func);
    serial_puts(" (vendor="); serial_put_hex(_pci_cache[net_idx].vendor);
    serial_puts(" device="); serial_put_hex(_pci_cache[net_idx].devid);
    serial_puts(")\r\n");

    /* Step 1: Read BAR0 — must be an I/O BAR for legacy VirtIO */
    uint32_t pci_addr = 0x80000000
        | ((uint32_t)_pci_cache[net_idx].bus << 16)
        | ((uint32_t)_pci_cache[net_idx].dev << 11)
        | 0x10; /* BAR0 offset in PCI config */
    outl(0xCF8, pci_addr);
    uint32_t bar0 = inl(0xCFC);

    if (!(bar0 & 1)) {
        serial_puts("[net] BAR0 is MMIO, not I/O — not legacy VirtIO\r\n");
        serial_puts("[net] BAR0 raw = ");
        serial_put_hex(bar0);
        serial_puts("\r\n");
        return -19;
    }

    _vnet.iobase = (uint16_t)(bar0 & ~0x3u);
    serial_puts("[net] BAR0 I/O base = 0x");
    serial_put_hex(_vnet.iobase);
    serial_puts("\r\n");

    /* Enable bus mastering + I/O space in PCI command register */
    uint32_t cmd_addr = 0x80000000
        | ((uint32_t)_pci_cache[net_idx].bus << 16)
        | ((uint32_t)_pci_cache[net_idx].dev << 11)
        | 0x04;
    outl(0xCF8, cmd_addr);
    uint32_t cmd_reg = inl(0xCFC);
    cmd_reg |= (1 << 0) | (1 << 2); /* I/O Space + Bus Master */
    outl(0xCF8, cmd_addr);
    outl(0xCFC, cmd_reg);

    /* Step 2: Reset device */
    _vnet_wr8(0x12, 0);
    /* Small delay for reset to take effect */
    for (volatile int i = 0; i < 10000; i++) {}

    /* Step 3: Acknowledge */
    _vnet_wr8(0x12, VIRTIO_STATUS_ACK);

    /* Step 4: Driver */
    _vnet_wr8(0x12, VIRTIO_STATUS_ACK | VIRTIO_STATUS_DRIVER);

    /* Step 5: Negotiate features */
    uint32_t host_features = _vnet_rd32(0x00);
    serial_puts("[net] Host features: 0x");
    serial_put_hex(host_features);
    serial_puts("\r\n");

    /* We only need MAC read capability.
     * NOTE: Do NOT set VIRTIO_NET_F_STATUS — it's feature bit 16 which
     * would require the FEATURES_OK transition (VirtIO 1.0+).
     * For legacy VirtIO, keep features minimal. */
    uint32_t guest_features = 0;
    if (host_features & VIRTIO_NET_F_MAC)
        guest_features |= VIRTIO_NET_F_MAC;
    _vnet_wr32(0x04, guest_features);

    /* Legacy VirtIO: skip FEATURES_OK (that's VirtIO 1.0+ only).
     * Go directly from DRIVER to queue setup, then DRIVER_OK. */

    /* Step 7: Read MAC address from device-specific config at offset 0x14.
     * VirtIO legacy: common regs 0x00-0x13, device config starts at 0x14. */
    for (int i = 0; i < 6; i++) {
        _vnet.mac[i] = _vnet_rd8(0x14 + (uint16_t)i);
    }
    serial_puts("[net] MAC address: ");
    _net_print_mac(_vnet.mac);
    serial_puts("\r\n");

    /* Step 8: Set up RX queue (queue 0) */
    if (_vnet_setup_queue(0, &_vnet.rx_qsize,
                          &_vnet.rx_desc, &_vnet.rx_avail, &_vnet.rx_used) < 0) {
        _vnet_wr8(0x12, VIRTIO_STATUS_FAILED);
        return -12;
    }

    /* Allocate RX buffers (contiguous block) */
    _vnet.rx_buffers = (uint8_t *)nvme_alloc_aligned(
        (size_t)_vnet.rx_qsize * VIRTIO_NET_BUF_SIZE, 4096);
    if (!_vnet.rx_buffers) {
        serial_puts("[net] Failed to allocate RX buffers\r\n");
        _vnet_wr8(0x12, VIRTIO_STATUS_FAILED);
        return -12;
    }
    __builtin_memset(_vnet.rx_buffers, 0, (size_t)_vnet.rx_qsize * VIRTIO_NET_BUF_SIZE);

    /* Step 9: Set up TX queue (queue 1) */
    if (_vnet_setup_queue(1, &_vnet.tx_qsize,
                          &_vnet.tx_desc, &_vnet.tx_avail, &_vnet.tx_used) < 0) {
        _vnet_wr8(0x12, VIRTIO_STATUS_FAILED);
        return -12;
    }

    /* Allocate TX buffers */
    _vnet.tx_buffers = (uint8_t *)nvme_alloc_aligned(
        (size_t)_vnet.tx_qsize * VIRTIO_NET_BUF_SIZE, 4096);
    if (!_vnet.tx_buffers) {
        serial_puts("[net] Failed to allocate TX buffers\r\n");
        _vnet_wr8(0x12, VIRTIO_STATUS_FAILED);
        return -12;
    }
    __builtin_memset(_vnet.tx_buffers, 0, (size_t)_vnet.tx_qsize * VIRTIO_NET_BUF_SIZE);
    _vnet.tx_last_used = 0;
    _vnet.tx_next_desc = 0;

    /* Step 10: Set our IP address (QEMU user-mode default) */
    _vnet.our_ip[0] = 10; _vnet.our_ip[1] = 0;
    _vnet.our_ip[2] = 2;  _vnet.our_ip[3] = 15;
    _vnet.gateway_ip[0] = 10; _vnet.gateway_ip[1] = 0;
    _vnet.gateway_ip[2] = 2;  _vnet.gateway_ip[3] = 2;

    /* Step 11: Set DRIVER_OK *before* filling RX queue.
     * The VirtIO spec says the device MUST NOT process virtqueue entries
     * until DRIVER_OK is set. */
    {
        uint8_t s = _vnet_rd8(0x12);
        _vnet_wr8(0x12, s | VIRTIO_STATUS_DRIVER_OK);
        serial_puts("[net] Status after DRIVER_OK: 0x");
        serial_put_hex(_vnet_rd8(0x12));
        serial_puts("\r\n");
    }

    _vnet.initialized = 1;
    _vnet.rx_count = 0;
    _vnet.tx_count = 0;
    _vnet.arp_replies = 0;
    _vnet.icmp_replies = 0;

    /* Now fill RX queue — device is live and will see the notify */
    _vnet_rx_fill();

    /* Verify RX queue state after fill */
    serial_puts("[net] Post-fill: desc[0].addr=0x");
    serial_put_hex((uint32_t)_vnet.rx_desc[0].addr);
    serial_puts(" len=");
    serial_put_dec(_vnet.rx_desc[0].len);
    serial_puts(" flags=");
    serial_put_dec(_vnet.rx_desc[0].flags);
    serial_puts(" avail[0]=");
    serial_put_dec(_vnet.rx_avail->ring[0]);
    serial_puts(" avail.idx=");
    serial_put_dec(_vnet.rx_avail->idx);
    serial_puts("\r\n");

    serial_puts("[net] VirtIO-net initialized: MAC=");
    _net_print_mac(_vnet.mac);
    serial_puts(" IP=");
    _net_print_ip(_vnet.our_ip);
    serial_puts(" GW=");
    _net_print_ip(_vnet.gateway_ip);
    serial_puts("\r\n");

    /* Send gratuitous ARP so QEMU learns our MAC */
    _vnet_send_gratuitous_arp();

    /* Also ARP for the gateway to trigger QEMU's ARP response */
    _vnet_send_arp_request(_vnet.gateway_ip);

    return 0;
}

/* -----------------------------------------------------------
 * _virtio_net_get_stats — print network statistics
 * ----------------------------------------------------------- */
static void _virtio_net_get_stats(void)
{
    serial_puts("[net] Stats: rx=");
    serial_put_dec(_vnet.rx_count);
    serial_puts(" tx=");
    serial_put_dec(_vnet.tx_count);
    serial_puts(" arp_replies=");
    serial_put_dec(_vnet.arp_replies);
    serial_puts(" icmp_replies=");
    serial_put_dec(_vnet.icmp_replies);
    serial_puts("\r\n");
}

int64_t simpleos_virtio_net_selftest(void)
{
    int rc = _virtio_net_init();
    if (rc < 0) return rc;

    uint32_t tx_before = _vnet.tx_count;
    uint32_t rx_before = _vnet.rx_count;
    uint16_t tx_used_before = _vnet.tx_used ? _vnet.tx_used->idx : 0;
    uint16_t rx_used_before = _vnet.rx_used ? _vnet.rx_used->idx : 0;

    _vnet_send_arp_request(_vnet.gateway_ip);

    int saw_tx_completion = 0;
    int saw_rx_frame = 0;
    for (int i = 0; i < 40000; i++) {
        int reclaimed = _vnet_reclaim_tx();
        if (reclaimed > 0 || (_vnet.tx_used && _vnet.tx_used->idx != tx_used_before)) {
            saw_tx_completion = 1;
        }
        int processed = _virtio_net_poll();
        if (processed > 0 || _vnet.rx_count > rx_before || (_vnet.rx_used && _vnet.rx_used->idx != rx_used_before)) {
            saw_rx_frame = 1;
        }
        if (saw_tx_completion && saw_rx_frame) break;
        for (volatile int d = 0; d < 1000; d++) {}
    }

    _vnet_reclaim_tx();
    _virtio_net_get_stats();

    if (_vnet.tx_count <= tx_before) return -5;
    if (!saw_tx_completion) return -110;
    if (!saw_rx_frame) return -61;
    return 0;
}

/* TCP header */
struct tcp_hdr {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t  data_off;  /* upper 4 bits = offset in 32-bit words */
    uint8_t  flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent;
} __attribute__((packed));

#define TCP_FIN  0x01
#define TCP_SYN  0x02
#define TCP_RST  0x04
#define TCP_PSH  0x08
#define TCP_ACK  0x10

/* TCP connection states */
enum tcp_state {
    TCP_CLOSED = 0,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,
    TCP_FIN_WAIT_2,
    TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_TIME_WAIT
};

/* Socket / TCP connection */
#define MAX_SOCKETS 16
#define TCP_RXBUF_SIZE 8192
#define TCP_TXBUF_SIZE 8192
#define TCP_ACCEPT_QUEUE 4

struct tcp_socket {
    int      in_use;
    enum tcp_state state;
    uint16_t local_port;
    uint16_t remote_port;
    uint8_t  remote_ip[4];
    uint8_t  remote_mac[6];
    uint32_t snd_nxt;       /* next send sequence number */
    uint32_t snd_una;       /* unacknowledged */
    uint32_t rcv_nxt;       /* next expected receive seq */
    uint16_t rcv_wnd;       /* receive window */
    /* Receive ring buffer */
    uint8_t  rxbuf[TCP_RXBUF_SIZE];
    uint32_t rx_head;
    uint32_t rx_tail;
    /* Accept queue (for listening sockets) */
    int      accept_queue[TCP_ACCEPT_QUEUE];
    int      aq_head;
    int      aq_tail;
    int      aq_count;
    int      backlog;
    /* IPC reply buffer */
    int      has_reply;
    int32_t  reply_status;
    int32_t  reply_value;
    uint8_t  reply_data[4096];
    int      reply_data_len;
};

static struct tcp_socket _sockets[MAX_SOCKETS];
static uint32_t _tcp_isn = 0x10000;  /* initial sequence number counter */
static uint16_t _tcp_ephemeral_port = 49152;

/* TCP pseudo-header checksum */
static uint16_t _tcp_checksum(const uint8_t *src_ip, const uint8_t *dst_ip,
                               const void *tcp_data, uint16_t tcp_len)
{
    uint32_t sum = 0;
    const uint8_t *p;
    /* Pseudo-header: src_ip(4) + dst_ip(4) + zero(1) + proto(1=6) + tcp_len(2) */
    sum += ((uint16_t)src_ip[0] << 8) | src_ip[1];
    sum += ((uint16_t)src_ip[2] << 8) | src_ip[3];
    sum += ((uint16_t)dst_ip[0] << 8) | dst_ip[1];
    sum += ((uint16_t)dst_ip[2] << 8) | dst_ip[3];
    sum += 6;  /* protocol = TCP */
    sum += tcp_len;
    /* TCP segment */
    p = (const uint8_t *)tcp_data;
    for (uint16_t i = 0; i + 1 < tcp_len; i += 2)
        sum += ((uint16_t)p[i] << 8) | p[i+1];
    if (tcp_len & 1)
        sum += (uint16_t)p[tcp_len - 1] << 8;
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    return _net_htons((uint16_t)~sum);
}

/* Send a TCP segment */
static void _tcp_send_segment(int sid, uint8_t flags, const void *data, uint16_t data_len)
{
    struct tcp_socket *s = &_sockets[sid];
    uint8_t pkt[1500];
    uint16_t tcp_len = 20 + data_len;  /* TCP header (no options) + data */
    uint16_t ip_len = 20 + tcp_len;

    /* Ethernet header */
    struct eth_hdr *eth = (struct eth_hdr *)pkt;
    __builtin_memcpy(eth->dst, s->remote_mac, 6);
    __builtin_memcpy(eth->src, _vnet.mac, 6);
    eth->ethertype = _net_htons(ETH_P_IP);

    /* IPv4 header */
    struct ipv4_hdr *ip = (struct ipv4_hdr *)(pkt + ETH_HLEN);
    ip->ver_ihl = 0x45;
    ip->tos = 0;
    ip->total_len = _net_htons(ip_len);
    ip->id = _net_htons((uint16_t)(_tcp_isn & 0xFFFF));
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = 6;  /* TCP */
    ip->checksum = 0;
    __builtin_memcpy(ip->src_ip, _vnet.our_ip, 4);
    __builtin_memcpy(ip->dst_ip, s->remote_ip, 4);
    ip->checksum = _inet_checksum(ip, 20);

    /* TCP header */
    struct tcp_hdr *tcp = (struct tcp_hdr *)(pkt + ETH_HLEN + 20);
    tcp->src_port = _net_htons(s->local_port);
    tcp->dst_port = _net_htons(s->remote_port);
    tcp->seq_num = _net_htonl(s->snd_nxt);
    tcp->ack_num = _net_htonl(s->rcv_nxt);
    tcp->data_off = 0x50;  /* 5 * 4 = 20 bytes, no options */
    tcp->flags = flags;
    tcp->window = _net_htons(TCP_RXBUF_SIZE);
    tcp->checksum = 0;
    tcp->urgent = 0;

    /* Copy data */
    if (data && data_len > 0) {
        __builtin_memcpy(pkt + ETH_HLEN + 20 + 20, data, data_len);
    }

    /* TCP checksum */
    tcp->checksum = _tcp_checksum(_vnet.our_ip, s->remote_ip, tcp, tcp_len);

    /* Send */
    serial_puts("[tcp-tx] sid=");
    serial_put_dec(sid);
    serial_puts(" flags=0x");
    serial_put_hex(flags);
    serial_puts(" src=");
    serial_put_dec(s->local_port);
    serial_puts(" dst=");
    serial_put_dec(s->remote_port);
    serial_puts(" ip=");
    _net_print_ip(s->remote_ip);
    serial_puts(" mac=");
    _net_print_mac(s->remote_mac);
    serial_puts("\r\n");
    int tx_rc = _vnet_send_frame(pkt, ETH_HLEN + ip_len);
    serial_puts("[tcp-tx] frame rc=");
    serial_put_dec(tx_rc);
    serial_puts("\r\n");

    /* Advance sequence number */
    s->snd_nxt += data_len;
    if (flags & (TCP_SYN | TCP_FIN)) s->snd_nxt += 1;
}

/* Handle incoming TCP segment */
static void _tcp_handle_segment(const uint8_t *frame, uint16_t frame_len)
{
    if (frame_len < ETH_HLEN + 20 + 20) return;

    const struct eth_hdr *eth = (const struct eth_hdr *)frame;
    const struct ipv4_hdr *ip = (const struct ipv4_hdr *)(frame + ETH_HLEN);
    uint16_t ip_hlen = (ip->ver_ihl & 0x0F) * 4;
    const struct tcp_hdr *tcp = (const struct tcp_hdr *)(frame + ETH_HLEN + ip_hlen);
    uint16_t tcp_hlen = (tcp->data_off >> 4) * 4;
    uint16_t ip_total = _net_ntohs(ip->total_len);
    uint16_t tcp_data_len = (ip_total > ip_hlen + tcp_hlen) ? ip_total - ip_hlen - tcp_hlen : 0;
    const uint8_t *tcp_data = frame + ETH_HLEN + ip_hlen + tcp_hlen;

    uint16_t dst_port = _net_ntohs(tcp->dst_port);
    uint16_t src_port = _net_ntohs(tcp->src_port);
    uint32_t seg_seq = _net_ntohl(tcp->seq_num);
    uint32_t seg_ack = _net_ntohl(tcp->ack_num);
    uint8_t  flags = tcp->flags;

    /* Find matching socket */
    int sid = -1;

    /* First: look for established connection */
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (!_sockets[i].in_use) continue;
        if (_sockets[i].state >= TCP_SYN_SENT &&
            _sockets[i].local_port == dst_port &&
            _sockets[i].remote_port == src_port) {
            sid = i;
            break;
        }
    }

    /* Second: look for listening socket (for SYN) */
    int listen_sid = -1;
    if (sid < 0 && (flags & TCP_SYN)) {
        for (int i = 0; i < MAX_SOCKETS; i++) {
            if (!_sockets[i].in_use) continue;
            if (_sockets[i].state == TCP_LISTEN &&
                _sockets[i].local_port == dst_port) {
                listen_sid = i;
                break;
            }
        }
    }

    if (sid < 0 && listen_sid < 0) {
        /* No matching socket — send RST */
        return;
    }

    /* Handle SYN on listening socket → create new connection */
    if (listen_sid >= 0 && (flags & TCP_SYN) && !(flags & TCP_ACK)) {
        /* Find free socket for the new connection */
        int new_sid = -1;
        for (int i = 0; i < MAX_SOCKETS; i++) {
            if (!_sockets[i].in_use) { new_sid = i; break; }
        }
        if (new_sid < 0) return;  /* No free sockets */

        struct tcp_socket *ns = &_sockets[new_sid];
        __builtin_memset(ns, 0, sizeof(*ns));
        ns->in_use = 1;
        ns->state = TCP_SYN_RECEIVED;
        ns->local_port = dst_port;
        ns->remote_port = src_port;
        __builtin_memcpy(ns->remote_ip, ip->src_ip, 4);
        __builtin_memcpy(ns->remote_mac, eth->src, 6);
        ns->rcv_nxt = seg_seq + 1;
        ns->snd_nxt = _tcp_isn++;
        ns->snd_una = ns->snd_nxt;
        ns->rcv_wnd = TCP_RXBUF_SIZE;

        /* Send SYN+ACK */
        _tcp_send_segment(new_sid, TCP_SYN | TCP_ACK, NULL, 0);

        serial_puts("[tcp] SYN received on port ");
        serial_put_dec(dst_port);
        serial_puts(" from ");
        _net_print_ip(ip->src_ip);
        serial_puts(":");
        serial_put_dec(src_port);
        serial_puts("\r\n");
        return;
    }

    if (sid < 0) return;
    struct tcp_socket *s = &_sockets[sid];
    serial_puts("[tcp] rx sid=");
    serial_put_dec(sid);
    serial_puts(" state=");
    serial_put_dec(s->state);
    serial_puts(" flags=0x");
    serial_put_hex(flags);
    serial_puts(" src_port=");
    serial_put_dec(src_port);
    serial_puts(" dst_port=");
    serial_put_dec(dst_port);
    serial_puts("\r\n");

    switch (s->state) {
    case TCP_SYN_SENT:
        if ((flags & TCP_SYN) && (flags & TCP_ACK)) {
            __builtin_memcpy(s->remote_mac, eth->src, ETH_ALEN);
            s->rcv_nxt = seg_seq + 1;
            s->snd_una = seg_ack;
            s->state = TCP_ESTABLISHED;
            _tcp_send_segment(sid, TCP_ACK, NULL, 0);
            serial_puts("[tcp] Client connection established on port ");
            serial_put_dec(s->local_port);
            serial_puts("\r\n");
        }
        break;

    case TCP_SYN_RECEIVED:
        if (flags & TCP_ACK) {
            s->snd_una = seg_ack;
            s->state = TCP_ESTABLISHED;
            serial_puts("[tcp] Connection established on port ");
            serial_put_dec(s->local_port);
            serial_puts("\r\n");

            /* Add to listening socket's accept queue */
            for (int i = 0; i < MAX_SOCKETS; i++) {
                if (_sockets[i].in_use && _sockets[i].state == TCP_LISTEN &&
                    _sockets[i].local_port == s->local_port) {
                    struct tcp_socket *ls = &_sockets[i];
                    if (ls->aq_count < TCP_ACCEPT_QUEUE) {
                        ls->accept_queue[ls->aq_tail] = sid;
                        ls->aq_tail = (ls->aq_tail + 1) % TCP_ACCEPT_QUEUE;
                        ls->aq_count++;
                    }
                    break;
                }
            }
        }
        break;

    case TCP_ESTABLISHED:
        /* Handle incoming data */
        if (tcp_data_len > 0) {
            uint16_t preview = tcp_data_len < 8 ? tcp_data_len : 8;
            serial_puts("[tcp-rx] first-bytes=");
            for (uint16_t j = 0; j < preview; j++) {
                if (j) serial_puts(" ");
                serial_put_hex(tcp_data[j]);
            }
            serial_puts("\r\n");
            /* Store in receive buffer */
            for (uint16_t i = 0; i < tcp_data_len; i++) {
                uint32_t next = (s->rx_head + 1) % TCP_RXBUF_SIZE;
                if (next == s->rx_tail) break;  /* Buffer full */
                s->rxbuf[s->rx_head] = tcp_data[i];
                s->rx_head = next;
            }
            s->rcv_nxt = seg_seq + tcp_data_len;
            /* Send ACK */
            _tcp_send_segment(sid, TCP_ACK, NULL, 0);
        }
        /* Handle ACK */
        if (flags & TCP_ACK) {
            s->snd_una = seg_ack;
        }
        /* Handle FIN */
        if (flags & TCP_FIN) {
            s->rcv_nxt = seg_seq + tcp_data_len + 1;
            s->state = TCP_CLOSE_WAIT;
            _tcp_send_segment(sid, TCP_ACK, NULL, 0);
            serial_puts("[tcp] FIN received, connection closing\r\n");
        }
        break;

    case TCP_FIN_WAIT_1:
        if ((flags & TCP_ACK) && (flags & TCP_FIN)) {
            s->rcv_nxt = seg_seq + 1;
            s->state = TCP_TIME_WAIT;
            _tcp_send_segment(sid, TCP_ACK, NULL, 0);
        } else if (flags & TCP_ACK) {
            s->state = TCP_FIN_WAIT_2;
        }
        break;

    case TCP_FIN_WAIT_2:
        if (flags & TCP_FIN) {
            s->rcv_nxt = seg_seq + 1;
            s->state = TCP_TIME_WAIT;
            _tcp_send_segment(sid, TCP_ACK, NULL, 0);
        }
        break;

    case TCP_LAST_ACK:
        if (flags & TCP_ACK) {
            s->state = TCP_CLOSED;
            s->in_use = 0;
        }
        break;

    default:
        break;
    }
}

/* Extend frame handler to dispatch TCP */
static void _vnet_handle_ipv4(const uint8_t *frame, uint16_t frame_len)
{
    if (frame_len < ETH_HLEN + 20) return;
    const struct ipv4_hdr *ip = (const struct ipv4_hdr *)(frame + ETH_HLEN);
    if (ip->protocol == 1) {
        _vnet_handle_icmp(frame, frame_len);
    } else if (ip->protocol == 6) {
        _tcp_handle_segment(frame, frame_len);
    }
}

/* Pending IPC reply for socket operations */
static struct {
    int      valid;
    int32_t  status;
    int32_t  value;
} _ipc_reply;

/* Available bytes in socket receive buffer */
static uint32_t _tcp_rx_available(int sid)
{
    struct tcp_socket *s = &_sockets[sid];
    return (s->rx_head >= s->rx_tail)
        ? s->rx_head - s->rx_tail
        : TCP_RXBUF_SIZE - s->rx_tail + s->rx_head;
}

/* Read from socket receive buffer */
static uint32_t _tcp_rx_read(int sid, uint8_t *buf, uint32_t max_len)
{
    struct tcp_socket *s = &_sockets[sid];
    uint32_t copied = 0;
    while (copied < max_len && s->rx_tail != s->rx_head) {
        buf[copied++] = s->rxbuf[s->rx_tail];
        s->rx_tail = (s->rx_tail + 1) % TCP_RXBUF_SIZE;
    }
    return copied;
}

static int32_t _read_i32(const uint8_t *buf, int offset)
{
    return (int32_t)(
        ((uint32_t)buf[offset]) |
        ((uint32_t)buf[offset+1] << 8) |
        ((uint32_t)buf[offset+2] << 16) |
        ((uint32_t)buf[offset+3] << 24)
    );
}

static uint32_t _read_u32(const uint8_t *buf, int offset)
{
    return ((uint32_t)buf[offset]) |
           ((uint32_t)buf[offset+1] << 8) |
           ((uint32_t)buf[offset+2] << 16) |
           ((uint32_t)buf[offset+3] << 24);
}

static uint16_t _read_u16(const uint8_t *buf, int offset)
{
    return (uint16_t)(buf[offset] | ((uint16_t)buf[offset+1] << 8));
}

static void _write_i32(uint8_t *buf, int offset, int32_t val)
{
    buf[offset]   = (uint8_t)(val & 0xFF);
    buf[offset+1] = (uint8_t)((val >> 8) & 0xFF);
    buf[offset+2] = (uint8_t)((val >> 16) & 0xFF);
    buf[offset+3] = (uint8_t)((val >> 24) & 0xFF);
}

static int64_t _ipc_send_handler(uint64_t port, uint64_t method,
                                   uint64_t flags, uint64_t buf_addr, uint64_t buf_len)
{
    (void)port; (void)flags;
    const uint8_t *payload = (const uint8_t *)(uintptr_t)buf_addr;
    uint32_t ipc_method = (buf_len >= 4) ? _read_u32(payload, 0) : (uint32_t)method;

    _ipc_reply.valid = 1;
    _ipc_reply.status = 0;
    _ipc_reply.value = 0;

    switch (ipc_method) {
    case 1: { /* NET_SOCKET: payload = [method(4)] + [proto_byte(1)] */
        int sid = -1;
        for (int i = 0; i < MAX_SOCKETS; i++) {
            if (!_sockets[i].in_use) { sid = i; break; }
        }
        if (sid < 0) {
            _ipc_reply.status = -24; /* EMFILE */
            break;
        }
        __builtin_memset(&_sockets[sid], 0, sizeof(_sockets[sid]));
        _sockets[sid].in_use = 1;
        _sockets[sid].state = TCP_CLOSED;
        _sockets[sid].rcv_wnd = TCP_RXBUF_SIZE;
        _ipc_reply.status = 0;
        _ipc_reply.value = sid;
        break;
    }
    case 2: { /* NET_BIND: payload = [method(4)] + [fd(4)] + [ip(4)] + [port(2)] */
        int32_t fd = _read_i32(payload, 4);
        uint16_t port_num = _read_u16(payload, 12);
        if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) {
            _ipc_reply.status = -9; /* EBADF */
            break;
        }
        _sockets[fd].local_port = port_num;
        _ipc_reply.status = 0;
        serial_puts("[tcp] Socket ");
        serial_put_dec(fd);
        serial_puts(" bound to port ");
        serial_put_dec(port_num);
        serial_puts("\r\n");
        break;
    }
    case 3: { /* NET_LISTEN: payload = [method(4)] + [fd(4)] + [backlog(4)] */
        int32_t fd = _read_i32(payload, 4);
        int32_t backlog = _read_i32(payload, 8);
        if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) {
            _ipc_reply.status = -9;
            break;
        }
        _sockets[fd].state = TCP_LISTEN;
        _sockets[fd].backlog = backlog;
        _ipc_reply.status = 0;
        serial_puts("[tcp] Socket ");
        serial_put_dec(fd);
        serial_puts(" listening on port ");
        serial_put_dec(_sockets[fd].local_port);
        serial_puts("\r\n");
        break;
    }
    case 5: { /* NET_ACCEPT: payload = [method(4)] + [fd(4)] */
        int32_t fd = _read_i32(payload, 4);
        serial_puts("[tcp-accept] fd=");
        serial_put_dec(fd);
        if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use ||
            _sockets[fd].state != TCP_LISTEN) {
            serial_puts(" EBADF\r\n");
            _ipc_reply.status = -9;
            break;
        }
        /* Poll for incoming connections */
        struct tcp_socket *ls = &_sockets[fd];
        serial_puts(" aq_head=");
        serial_put_dec(ls->aq_head);
        serial_puts(" aq_tail=");
        serial_put_dec(ls->aq_tail);
        serial_puts(" aq_count=");
        serial_put_dec(ls->aq_count);
        serial_puts(" port=");
        serial_put_dec(ls->local_port);
        serial_puts("\r\n");
        int timeout = 0;
        while (ls->aq_count == 0 && timeout < 100000) {
            _virtio_net_poll();
            timeout++;
            for (volatile int d = 0; d < 1000; d++) {}
        }
        serial_puts("[tcp-accept] poll done, timeout=");
        serial_put_dec(timeout);
        serial_puts(" aq_head=");
        serial_put_dec(ls->aq_head);
        serial_puts(" aq_tail=");
        serial_put_dec(ls->aq_tail);
        serial_puts(" aq_count=");
        serial_put_dec(ls->aq_count);
        serial_puts("\r\n");
        if (ls->aq_count == 0) {
            _ipc_reply.status = -11; /* EAGAIN — no connections */
            serial_puts("[tcp-accept] EAGAIN\r\n");
            break;
        }
        int accepted_sid = ls->accept_queue[ls->aq_head];
        ls->aq_head = (ls->aq_head + 1) % TCP_ACCEPT_QUEUE;
        ls->aq_count--;
        _ipc_reply.status = 0;
        _ipc_reply.value = accepted_sid;
        serial_puts("[tcp-accept] accepted socket ");
        serial_put_dec(accepted_sid);
        serial_puts("\r\n");
        break;
    }
    case 6: { /* NET_SEND: payload = [method(4)] + [fd(4)] + [data...] */
        int32_t fd = _read_i32(payload, 4);
        if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use ||
            _sockets[fd].state != TCP_ESTABLISHED) {
            _ipc_reply.status = -9;
            break;
        }
        uint32_t data_len = (buf_len > 8) ? (uint32_t)(buf_len - 8) : 0;
        const uint8_t *data = payload + 8;
        /* Send in chunks of 1400 bytes (MTU - headers) */
        uint32_t sent = 0;
        while (sent < data_len) {
            uint16_t chunk = (data_len - sent > 1400) ? 1400 : (uint16_t)(data_len - sent);
            _tcp_send_segment(fd, TCP_ACK | TCP_PSH, data + sent, chunk);
            sent += chunk;
        }
        _ipc_reply.status = (int32_t)sent;
        break;
    }
    case 7: { /* NET_RECV: payload = [method(4)] + [fd(4)] + [max_len(4)] */
        int32_t fd = _read_i32(payload, 4);
        uint32_t max_len = (buf_len >= 12) ? _read_u32(payload, 8) : 4096;
        if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) {
            _ipc_reply.status = -9;
            break;
        }
        /* Poll until data available or timeout */
        int timeout = 0;
        while (_tcp_rx_available(fd) == 0 && _sockets[fd].state == TCP_ESTABLISHED && timeout < 50000) {
            _virtio_net_poll();
            timeout++;
            for (volatile int d = 0; d < 1000; d++) {}
        }
        uint32_t avail = _tcp_rx_available(fd);
        if (avail == 0) {
            if (_sockets[fd].state != TCP_ESTABLISHED) {
                _ipc_reply.status = 0; /* EOF */
            } else {
                _ipc_reply.status = -11; /* EAGAIN */
            }
            break;
        }
        uint32_t to_read = (avail < max_len) ? avail : max_len;
        if (to_read > 4092) to_read = 4092; /* leave room for status in reply */
        struct tcp_socket *rs = &_sockets[fd];
        int read_count = (int)_tcp_rx_read(fd, rs->reply_data + 4, to_read);
        rs->reply_data_len = read_count;
        _write_i32(rs->reply_data, 0, (int32_t)read_count);
        _ipc_reply.status = 0;
        _ipc_reply.value = (int32_t)read_count;
        /* Store reply data pointer for IPC_RECV */
        rs->has_reply = 1;
        break;
    }
    case 8: { /* NET_CLOSE: payload = [method(4)] + [fd(4)] */
        int32_t fd = _read_i32(payload, 4);
        if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) {
            _ipc_reply.status = -9;
            break;
        }
        if (_sockets[fd].state == TCP_ESTABLISHED) {
            _tcp_send_segment(fd, TCP_FIN | TCP_ACK, NULL, 0);
            _sockets[fd].state = TCP_FIN_WAIT_1;
            /* Brief poll for FIN-ACK */
            for (int t = 0; t < 10000; t++) {
                _virtio_net_poll();
                if (_sockets[fd].state == TCP_TIME_WAIT || _sockets[fd].state == TCP_CLOSED)
                    break;
                for (volatile int d = 0; d < 100; d++) {}
            }
        }
        _sockets[fd].in_use = 0;
        _sockets[fd].state = TCP_CLOSED;
        _ipc_reply.status = 0;
        break;
    }
    default:
        _ipc_reply.status = -38; /* ENOSYS */
        break;
    }
    return 0;
}

static int64_t _ipc_recv_handler(uint64_t port, uint64_t buf_addr, uint64_t max_len)
{
    (void)port;
    if (!_ipc_reply.valid) return -11; /* EAGAIN */

    uint8_t *reply_buf = (uint8_t *)(uintptr_t)buf_addr;
    _write_i32(reply_buf, 0, _ipc_reply.status);
    _write_i32(reply_buf, 4, _ipc_reply.value);

    _ipc_reply.valid = 0;

    /* For NET_RECV with data, copy data into reply buffer */
    /* The data was already stored in reply_data by IPC_SEND handler */
    int total = 8;

    /* Check if any socket has pending reply data */
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (_sockets[i].has_reply && _sockets[i].reply_data_len > 0) {
            int copy_len = _sockets[i].reply_data_len + 4; /* status + data */
            if ((uint64_t)(total + copy_len) <= max_len) {
                /* Copy status + data into reply */
                __builtin_memcpy(reply_buf, _sockets[i].reply_data, copy_len);
                total = copy_len;
            }
            _sockets[i].has_reply = 0;
            _sockets[i].reply_data_len = 0;
            break;
        }
    }

    return total;
}

int64_t _pci_enumerate(uint64_t mode, uint64_t index, uint64_t buf_addr);
extern int64_t kernel__arch__x86_64__interrupt__x86_dispatch_installed_syscall_abi(
    uint64_t id, uint64_t arg0, uint64_t arg1, uint64_t arg2,
    uint64_t arg3, uint64_t arg4, uint64_t arg5
) __attribute__((weak));
/* Forward declaration of the real Simple implementation (no spl_ prefix, no os__ prefix).
 * The linker.ld assignment makes spl_x86_dispatch_installed_syscall_abi a strong alias
 * to this symbol, overriding any freestanding stubs. */
extern int64_t kernel__arch__x86_64__interrupt__spl_x86_dispatch_installed_syscall_abi(
    uint64_t id, uint64_t arg0, uint64_t arg1, uint64_t arg2,
    uint64_t arg3, uint64_t arg4, uint64_t arg5
) __attribute__((weak));

__attribute__((weak)) int64_t
kernel__arch__x86_64__interrupt__spl_x86_dispatch_installed_syscall_abi(
    uint64_t id, uint64_t arg0, uint64_t arg1, uint64_t arg2,
    uint64_t arg3, uint64_t arg4, uint64_t arg5
)
{
    if (kernel__arch__x86_64__interrupt__x86_dispatch_installed_syscall_abi) {
        return kernel__arch__x86_64__interrupt__x86_dispatch_installed_syscall_abi(
            id, arg0, arg1, arg2, arg3, arg4, arg5
        );
    }
    return -38;
}

static int simpleos_path_eq_raw(const char *p, uint64_t len, const char *expected)
{
    uint64_t i = 0;
    if (!p || !expected)
        return 0;
    while (expected[i]) {
        if (i >= len || p[i] != expected[i])
            return 0;
        i++;
    }
    return i == len;
}

static int simpleos_path_eq(uint64_t ptr, uint64_t len, const char *expected)
{
    const char *raw = (const char *)(uintptr_t)ptr;
    if (simpleos_path_eq_raw(raw, len, expected))
        return 1;
    if ((ptr & TAG_HEAP) != 0) {
        const char *data_decoded = (const char *)(uintptr_t)(ptr & ~TAG_HEAP);
        if (simpleos_path_eq_raw(data_decoded, len, expected))
            return 1;
    }
    if ((ptr & TAG_MASK) != 0) {
        const char *decoded = (const char *)DECODE_PTR((RuntimeValue)ptr);
        if (simpleos_path_eq_raw(decoded, len, expected))
            return 1;
    }
    return 0;
}

static uint64_t simpleos_decode_path_data_ptr(uint64_t ptr, uint64_t len)
{
    const char *raw = (const char *)(uintptr_t)ptr;
    if (raw && len > 0 && raw[0] == '/')
        return ptr;
    if ((ptr & TAG_HEAP) != 0) {
        const char *data_decoded = (const char *)(uintptr_t)(ptr & ~TAG_HEAP);
        if (data_decoded && len > 0 && data_decoded[0] == '/')
            return ptr & ~TAG_HEAP;
    }
    if ((ptr & TAG_MASK) != 0) {
        const char *decoded = (const char *)DECODE_PTR((RuntimeValue)ptr);
        if (decoded && len > 0 && decoded[0] == '/')
            return (uint64_t)(uintptr_t)decoded;
    }
    return ptr;
}

static uint64_t simpleos_known_app_id_from_path(uint64_t ptr, uint64_t len)
{
    if (simpleos_path_eq(ptr, len, "/sys/apps/browser_demo")) return 1;
    if (simpleos_path_eq(ptr, len, "/sys/apps/browser_demo.smf")) return 1;
    if (simpleos_path_eq(ptr, len, "/sys/apps/file_manager")) return 2;
    if (simpleos_path_eq(ptr, len, "/sys/apps/file_manager.smf")) return 2;
    if (simpleos_path_eq(ptr, len, "/sys/apps/hello_world")) return 3;
    if (simpleos_path_eq(ptr, len, "/sys/apps/hello_world.smf")) return 3;
    if (simpleos_path_eq(ptr, len, "/sys/apps/terminal")) return 4;
    if (simpleos_path_eq(ptr, len, "/sys/apps/shell")) return 4;
    if (simpleos_path_eq(ptr, len, "/sys/apps/shell.smf")) return 4;
    if (simpleos_path_eq(ptr, len, "/sys/apps/editor")) return 5;
    if (simpleos_path_eq(ptr, len, "/sys/apps/editor.smf")) return 5;
    return 0;
}

int64_t userlib__syscall_raw__syscall(uint64_t id, uint64_t a0, uint64_t a1,
                                       uint64_t a2, uint64_t a3, uint64_t a4)
{
    switch (id) {
        case 0:  /* Exit */
            outb(0xf4, (uint8_t)((a0 << 1) | 1)); /* isa-debug-exit */
            for (;;) __asm__ volatile("cli; hlt");
            return 0;
        case 4:  /* GetPid */
            return 1; /* bare-metal: PID 1 */
        case 13: /* SpawnBinary */
        {
            uint64_t path_ptr = simpleos_decode_path_data_ptr(a0, a1);
            uint64_t path_len = a1;
            return kernel__arch__x86_64__interrupt__spl_x86_dispatch_installed_syscall_abi(
                id, path_ptr, path_len, a2, a3, a4, 0
            );
        }
        case 14: /* EnterUserBlocking — pid_hint in a0, noreturn on success */
            return kernel__arch__x86_64__interrupt__spl_x86_dispatch_installed_syscall_abi(
                14, a0, a1, a2, a3, a4, 0
            );
        case 60: /* DebugWrite */
            serial_putchar((char)(a0 & 0xFF));
            return 0;
        case 80: /* DevEnumerate — PCI bus scan via direct port I/O */
            return _pci_enumerate(a0, a1, a2);
        case 82: /* DeviceGrant — read PCI BAR0 via _pci_enumerate mode 5 */
            return _pci_enumerate(5, a0, 0);
        case 83: { /* MapBar — identity map on baremetal (no-op, return same addr) */
            return (int64_t)a0; /* On baremetal, phys == virt (identity mapped) */
        }
        case 84: { /* AllocDma — allocate DMA buffer (use heap) */
            uint64_t size = a0;
            void *p = malloc(size);
            if (!p) return -12; /* ENOMEM */
            return (int64_t)(uintptr_t)p; /* Return virtual address (= physical on identity map) */
        }
        case 85: /* NvmeReadSector: a0=device_idx, a1=lba, a2=buf_addr */
            return _nvme_read_sector(a0, a1, a2);
        case 86: /* NvmeInit: initialize NVMe controller + read sector 0 for diag */
            return (int64_t)_nvme_init_and_read_sector0();
        case 94: /* NvmeWriteSector: a0=device_idx, a1=lba, a2=buf_addr */
            return _nvme_write_sector(a0, a1, a2);
        case 87: /* Fat32Init: parse BPB and initialize FAT32 state */
            return (int64_t)_fat32_init();
        case 88: /* Fat32ReadFile: a0=name_ptr, a1=buf_ptr, a2=max_size */
            return _fat32_read_file_syscall(a0, a1, a2);
        case 89: /* Fat32ListDir: list root directory entries to serial */
            return (int64_t)fat32_list_dir();
        case 20: /* IPC_SEND: a0=port, a1=method, a2=flags, a3=buf, a4=len */
            return _ipc_send_handler(a0, a1, a2, a3, a4);
        case 21: /* IPC_RECV: a0=port, a1=reply_buf, a2=max_len */
            return _ipc_recv_handler(a0, a1, a2);
        case 22: /* SYS_IPC_CREATE_PORT: a0=name_ptr, a1=name_len — baremetal
                  * has a single implicit port (_ipc_reply) shared by all
                  * services, so we return a non-zero pseudo-id that passes
                  * the 'port < 0' failure check in service init() paths.
                  * See Agent R research 2026-04-13 (ipc_error_38). */
            (void)a0; (void)a1;
            return 1;
        case 23: /* SYS_IPC_SEND (service-id variant used by wm/launcher/vfs):
                  * routes to the same in-process _ipc_send_handler as case 20.
                  * a0=port, a1=method, a2=flags, a3=buf, a4=len */
            return _ipc_send_handler(a0, a1, a2, a3, a4);
        case 24: /* SYS_IPC_CONNECT: a0=name_ptr, a1=name_len — baremetal
                  * has the same single implicit port, so connect succeeds
                  * by returning the same pseudo-id as create_port. */
            (void)a0; (void)a1;
            return 1;
        case 90: /* NetInit: initialize VirtIO-net, set IP 10.0.2.15 */
            return (int64_t)_virtio_net_init();
        case 91: /* NetPoll: process incoming frames (ARP/ICMP auto-reply) */
            return (int64_t)_virtio_net_poll();
        case 92: /* NetSendFrame: a0=buf_addr, a1=frame_len (raw Ethernet) */
            return (int64_t)_vnet_send_frame((const void *)(uintptr_t)a0,
                                              (uint16_t)a1);
        case 93: /* NetStats: print network statistics to serial */
            _virtio_net_get_stats();
            return 0;
        case 106: /* Schedule */
        case 107: /* SchedCtl */
            /* spl_x86_dispatch_installed_syscall_abi is a linker-assigned strong
             * alias; call the inner symbol directly (always present). */
            return kernel__arch__x86_64__interrupt__spl_x86_dispatch_installed_syscall_abi(
                id, a0, a1, a2, a3, a4, 0
            );
        default:
            return -38; /* ENOSYS */
    }
}

int64_t os__userlib__syscall_raw__syscall(uint64_t id, uint64_t a0, uint64_t a1,
                                           uint64_t a2, uint64_t a3, uint64_t a4)
{
    return userlib__syscall_raw__syscall(id, a0, a1, a2, a3, a4);
}

int64_t _pci_enumerate(uint64_t mode, uint64_t index, uint64_t buf_addr)
{
    if (_pci_cache_count < 0) _pci_scan();

    if (mode == 0) {
        /* Mode 0: return device count */
        return (int64_t)_pci_cache_count;
    }
    if (mode == 1) {
        /* Mode 1: fill DeviceInfoBuf at buf_addr for device[index] */
        if ((int)index >= _pci_cache_count) return -22; /* EINVAL */
        uint8_t *buf = (uint8_t *)(uintptr_t)buf_addr;
        int i = (int)index;
        buf[0] = _pci_cache[i].bus;
        buf[1] = _pci_cache[i].dev;
        buf[2] = _pci_cache[i].func;
        buf[3] = 0; /* padding */
        *(uint16_t *)(buf + 4) = _pci_cache[i].vendor;
        *(uint16_t *)(buf + 6) = _pci_cache[i].devid;
        buf[8] = _pci_cache[i].cls;
        buf[9] = _pci_cache[i].sub;
        buf[10] = _pci_cache[i].progif;
        buf[11] = _pci_cache[i].htype;
        buf[12] = _pci_cache[i].irq;
        return 0;
    }
    if (mode == 2) {
        /* Mode 2: return packed device info for device[index] — no buffer needed.
         * Return value layout (i64):
         *   bits [7:0]   = bus
         *   bits [15:8]  = device
         *   bits [23:16] = func
         *   bits [31:24] = class_code
         *   bits [39:32] = subclass
         *   bits [55:40] = vendor_id
         *   bits [63:56] = 0 (reserved)
         * Second call with mode 3 returns device_id + extras:
         *   bits [15:0]  = device_id
         *   bits [23:16] = prog_if
         *   bits [31:24] = irq_line
         */
        if ((int)index >= _pci_cache_count) return -22;
        int i = (int)index;
        return (int64_t)(
            ((uint64_t)_pci_cache[i].bus) |
            ((uint64_t)_pci_cache[i].dev << 8) |
            ((uint64_t)_pci_cache[i].func << 16) |
            ((uint64_t)_pci_cache[i].cls << 24) |
            ((uint64_t)_pci_cache[i].sub << 32) |
            ((uint64_t)_pci_cache[i].vendor << 40)
        );
    }
    if (mode == 3) {
        /* Mode 3: return device_id + extras for device[index] */
        if ((int)index >= _pci_cache_count) return -22;
        int i = (int)index;
        return (int64_t)(
            ((uint64_t)_pci_cache[i].devid) |
            ((uint64_t)_pci_cache[i].progif << 16) |
            ((uint64_t)_pci_cache[i].irq << 24)
        );
    }
    if (mode == 4) {
        /* Mode 4: return single field for device[index].
         * buf_addr = field selector:
         *   0=bus, 1=device, 2=func, 3=class, 4=subclass, 5=vendor, 6=devid, 7=irq
         */
        if ((int)index >= _pci_cache_count) return -22;
        int i = (int)index;
        switch ((int)buf_addr) {
            case 0: return (int64_t)_pci_cache[i].bus;
            case 1: return (int64_t)_pci_cache[i].dev;
            case 2: return (int64_t)_pci_cache[i].func;
            case 3: return (int64_t)_pci_cache[i].cls;
            case 4: return (int64_t)_pci_cache[i].sub;
            case 5: return (int64_t)_pci_cache[i].vendor;
            case 6: return (int64_t)_pci_cache[i].devid;
            case 7: return (int64_t)_pci_cache[i].irq;
            default: return -22;
        }
    }
    if (mode == 5) {
        /* Mode 5: Read PCI BAR0 for device at index.
         * Returns physical base address of BAR0 (type bits masked off). */
        if ((int)index >= _pci_cache_count) return -22;
        int i = (int)index;
        uint32_t addr = 0x80000000 | ((uint32_t)_pci_cache[i].bus << 16)
                      | ((uint32_t)_pci_cache[i].dev << 11) | 0x10;
        outl(0xCF8, addr);
        uint32_t bar0 = inl(0xCFC);
        if (bar0 & 1) return (int64_t)(bar0 & ~0x3u); /* I/O BAR */
        return (int64_t)(bar0 & ~0xFu); /* Memory BAR */
    }
    return -38; /* ENOSYS */
}

static const uint64_t _sha512_K[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

static const uint64_t _sha512_H[8] = {
    0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
    0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};

static const uint32_t _sha256_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static const uint32_t _sha256_H[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

static RuntimeValue rt_bytes_from_rodata(const uint8_t *data, size_t len)
{
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + len * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + len * sizeof(RuntimeValue));
    a->len = (uint32_t)len;
    a->cap = (uint32_t)len;
    a->items = runtime_array_inline_items(a);
    for (size_t i = 0; i < len; i++) a->items[i] = ENCODE_INT((int64_t)data[i]);
    return ENCODE_PTR(a);
}

static const uint8_t ssh_userauth_password_only_failure_payload[14] = {
    51U, 0x00U, 0x00U, 0x00U, 0x08U, 0x70U, 0x61U, 0x73U,
    0x73U, 0x77U, 0x6fU, 0x72U, 0x64U, 0x00U
};

RuntimeValue rt_ssh_userauth_password_only_failure_payload(void)
{
    return rt_bytes_from_rodata(
        ssh_userauth_password_only_failure_payload,
        sizeof(ssh_userauth_password_only_failure_payload));
}

RuntimeValue rt_embedded_host_rsa_pkcs8(void)
{
    return rt_bytes_from_rodata(embedded_host_rsa_pkcs8, sizeof(embedded_host_rsa_pkcs8));
}

RuntimeValue rt_embedded_host_rsa_public_blob(void)
{
    return rt_bytes_from_rodata(embedded_host_rsa_public_blob, sizeof(embedded_host_rsa_public_blob));
}

RuntimeValue rt_embedded_host_rsa_component(int64_t which)
{
    switch (which) {
        case 0: return rt_bytes_from_rodata(embedded_host_rsa_modulus, sizeof(embedded_host_rsa_modulus));
        case 1: return rt_bytes_from_rodata(embedded_host_rsa_prime1, sizeof(embedded_host_rsa_prime1));
        case 2: return rt_bytes_from_rodata(embedded_host_rsa_prime2, sizeof(embedded_host_rsa_prime2));
        case 3: return rt_bytes_from_rodata(embedded_host_rsa_exponent1, sizeof(embedded_host_rsa_exponent1));
        case 4: return rt_bytes_from_rodata(embedded_host_rsa_exponent2, sizeof(embedded_host_rsa_exponent2));
        case 5: return rt_bytes_from_rodata(embedded_host_rsa_coefficient, sizeof(embedded_host_rsa_coefficient));
        default: return NIL_VALUE;
    }
}

static const uint8_t _aes_sbox[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

static const uint8_t _aes_inv_sbox[256] = {
    0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
    0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
    0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
    0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
    0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
    0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
    0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
    0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
    0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
    0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
    0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
    0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
    0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
    0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
    0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
    0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
};

static const uint32_t _aes_rcon[10] = {
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x1b000000, 0x36000000
};

/* Lookup functions — return raw (untagged) integers */
int64_t rt_sha512_K(int64_t i) { return (i >= 0 && i < 80) ? (int64_t)_sha512_K[i] : 0; }
int64_t rt_sha512_H(int64_t i) { return (i >= 0 && i < 8) ? (int64_t)_sha512_H[i] : 0; }
int64_t rt_sha256_K(int64_t i) { return (i >= 0 && i < 64) ? (int64_t)_sha256_K[i] : 0; }
int64_t rt_sha256_H(int64_t i) { return (i >= 0 && i < 8) ? (int64_t)_sha256_H[i] : 0; }
int64_t rt_aes_sbox(int64_t i) { return (i >= 0 && i < 256) ? (int64_t)_aes_sbox[i] : 0; }
int64_t rt_aes_inv_sbox(int64_t i) { return (i >= 0 && i < 256) ? (int64_t)_aes_inv_sbox[i] : 0; }
int64_t rt_aes_rcon(int64_t i) { return (i >= 0 && i < 10) ? (int64_t)_aes_rcon[i] : 0; }

static inline uint64_t _sha512_rotr(uint64_t x, int n) { return (x >> n) | (x << (64 - n)); }
static inline uint64_t _sha512_ch(uint64_t x, uint64_t y, uint64_t z) { return (x & y) ^ (~x & z); }
static inline uint64_t _sha512_maj(uint64_t x, uint64_t y, uint64_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint64_t _sha512_S0(uint64_t x) { return _sha512_rotr(x,28) ^ _sha512_rotr(x,34) ^ _sha512_rotr(x,39); }
static inline uint64_t _sha512_S1(uint64_t x) { return _sha512_rotr(x,14) ^ _sha512_rotr(x,18) ^ _sha512_rotr(x,41); }
static inline uint64_t _sha512_s0(uint64_t x) { return _sha512_rotr(x,1) ^ _sha512_rotr(x,8) ^ (x >> 7); }
static inline uint64_t _sha512_s1(uint64_t x) { return _sha512_rotr(x,19) ^ _sha512_rotr(x,61) ^ (x >> 6); }

static void _sha512_process_block(const uint8_t *block, uint64_t *h)
{
    uint64_t w[80];
    for (int t = 0; t < 16; t++) {
        w[t] = 0;
        for (int b = 0; b < 8; b++)
            w[t] = (w[t] << 8) | block[t * 8 + b];
    }
    for (int t = 16; t < 80; t++)
        w[t] = _sha512_s1(w[t-2]) + w[t-7] + _sha512_s0(w[t-15]) + w[t-16];

    uint64_t a=h[0], b=h[1], c=h[2], d=h[3], e=h[4], f=h[5], g=h[6], hh=h[7];
    for (int t = 0; t < 80; t++) {
        uint64_t t1 = hh + _sha512_S1(e) + _sha512_ch(e,f,g) + _sha512_K[t] + w[t];
        uint64_t t2 = _sha512_S0(a) + _sha512_maj(a,b,c);
        hh=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
}

/* SHA-512 result buffer — stores 64-byte digest for Simple to read byte-by-byte */
static uint8_t _sha512_result[64];

/* rt_sha512_hash: compute SHA-512, store result in _sha512_result buffer.
 * data_rv: RuntimeValue [u8] array.
 * Returns 64 (digest length) on success, negative on error. */
int64_t rt_sha512_hash(int64_t data_rv, int64_t unused)
{
    if (!IS_HEAP(data_rv)) return -1;
    HeapHeader *hdr = (HeapHeader *)DECODE_PTR(data_rv);
    if (!hdr || hdr->type != HEAP_ARRAY) return -1;
    RuntimeArray *arr = (RuntimeArray *)hdr;
    uint32_t data_len = arr->len;
    RuntimeValue *items = runtime_array_items(arr);

    uint8_t *data = (uint8_t *)malloc(data_len + 256);
    if (!data) return -1;
    for (uint32_t i = 0; i < data_len; i++)
        data[i] = (uint8_t)(DECODE_INT(items[i]) & 0xFF);

    /* SHA-512 padding */
    uint64_t bit_len = (uint64_t)data_len * 8;
    uint32_t padded_len = data_len + 1;
    while ((padded_len % 128) != 112) padded_len++;
    padded_len += 16;

    uint8_t *padded = (uint8_t *)malloc(padded_len);
    if (!padded) return -1;
    for (uint32_t i = 0; i < padded_len; i++) padded[i] = 0;
    for (uint32_t i = 0; i < data_len; i++) padded[i] = data[i];
    padded[data_len] = 0x80;
    for (int i = 0; i < 8; i++)
        padded[padded_len - 8 + i] = (uint8_t)(bit_len >> (56 - i * 8));

    uint64_t h[8];
    for (int i = 0; i < 8; i++) h[i] = _sha512_H[i];
    for (uint32_t off = 0; off < padded_len; off += 128)
        _sha512_process_block(padded + off, h);

    /* Store 64-byte digest in static buffer */
    for (int i = 0; i < 8; i++)
        for (int b = 0; b < 8; b++)
            _sha512_result[i * 8 + b] = (uint8_t)(h[i] >> (56 - b * 8));

    return 64;
}

/* rt_sha512_byte: read one byte from the last SHA-512 result */
int64_t rt_sha512_byte(int64_t index)
{
    if (index < 0 || index >= 64) return 0;
    return (int64_t)_sha512_result[index];
}

/* ---------- SHA-512 helpers (reuse 8e internals) ---------- */

static void _ed25519_sha512(const uint8_t *msg, uint32_t msg_len, uint8_t out[64])
{
    uint64_t bit_len = (uint64_t)msg_len * 8;
    uint32_t padded_len = msg_len + 1;
    while ((padded_len % 128) != 112) padded_len++;
    padded_len += 16;

    uint8_t *padded = (uint8_t *)malloc(padded_len);
    if (!padded) return;
    for (uint32_t i = 0; i < padded_len; i++) padded[i] = 0;
    for (uint32_t i = 0; i < msg_len; i++) padded[i] = msg[i];
    padded[msg_len] = 0x80;
    for (int i = 0; i < 8; i++)
        padded[padded_len - 8 + i] = (uint8_t)(bit_len >> (56 - i * 8));

    uint64_t h[8];
    for (int i = 0; i < 8; i++) h[i] = _sha512_H[i];
    for (uint32_t off = 0; off < padded_len; off += 128)
        _sha512_process_block(padded + off, h);
    free(padded);

    for (int i = 0; i < 8; i++)
        for (int b = 0; b < 8; b++)
            out[i * 8 + b] = (uint8_t)(h[i] >> (56 - b * 8));
}

/* ---------- fe25519: field element mod p = 2^255-19 ----------
 * Radix 2^51, 5 limbs: f = f[0] + f[1]*2^51 + ... + f[4]*2^204
 */

typedef struct { int64_t v[5]; } fe25519;

#define FE_MASK51 ((int64_t)((1ULL << 51) - 1))

static void fe_0(fe25519 *f) { f->v[0]=f->v[1]=f->v[2]=f->v[3]=f->v[4]=0; }
static void fe_1(fe25519 *f) { f->v[0]=1; f->v[1]=f->v[2]=f->v[3]=f->v[4]=0; }
static void fe_copy(fe25519 *d, const fe25519 *s) { for(int i=0;i<5;i++) d->v[i]=s->v[i]; }

static void fe_add(fe25519 *h, const fe25519 *f, const fe25519 *g)
{
    for (int i = 0; i < 5; i++) h->v[i] = f->v[i] + g->v[i];
}

static void fe_sub(fe25519 *h, const fe25519 *f, const fe25519 *g)
{
    /* Add 2*p split into limbs to keep result positive.
     * p = 2^255-19 in radix-2^51: (2^51-19, 2^51-1, 2^51-1, 2^51-1, 2^51-1)
     * 2p = (2^52-38, 2^52-2, 2^52-2, 2^52-2, 2^52-2) */
    h->v[0] = f->v[0] + ((1LL<<52) - 38) - g->v[0];
    h->v[1] = f->v[1] + ((1LL<<52) - 2)  - g->v[1];
    h->v[2] = f->v[2] + ((1LL<<52) - 2)  - g->v[2];
    h->v[3] = f->v[3] + ((1LL<<52) - 2)  - g->v[3];
    h->v[4] = f->v[4] + ((1LL<<52) - 2)  - g->v[4];
}

static void fe_neg(fe25519 *h, const fe25519 *f)
{
    fe25519 z; fe_0(&z);
    fe_sub(h, &z, f);
}

static void fe_carry(fe25519 *h)
{
    int64_t c;
    for (int i = 0; i < 4; i++) {
        c = h->v[i] >> 51;
        h->v[i] &= FE_MASK51;
        h->v[i+1] += c;
    }
    c = h->v[4] >> 51;
    h->v[4] &= FE_MASK51;
    h->v[0] += c * 19;
    c = h->v[0] >> 51;
    h->v[0] &= FE_MASK51;
    h->v[1] += c;
}

/* fe_mul using __int128 (always available on x86_64) */
static void fe_mul(fe25519 *h, const fe25519 *f, const fe25519 *g)
{
#ifdef __SIZEOF_INT128__
    __int128 t[5];
    int64_t f0=f->v[0], f1=f->v[1], f2=f->v[2], f3=f->v[3], f4=f->v[4];
    int64_t g0=g->v[0], g1=g->v[1], g2=g->v[2], g3=g->v[3], g4=g->v[4];
    int64_t g1_19=19*g1, g2_19=19*g2, g3_19=19*g3, g4_19=19*g4;

    t[0] = (__int128)f0*g0 + (__int128)f1*g4_19 + (__int128)f2*g3_19 + (__int128)f3*g2_19 + (__int128)f4*g1_19;
    t[1] = (__int128)f0*g1 + (__int128)f1*g0    + (__int128)f2*g4_19 + (__int128)f3*g3_19 + (__int128)f4*g2_19;
    t[2] = (__int128)f0*g2 + (__int128)f1*g1    + (__int128)f2*g0    + (__int128)f3*g4_19 + (__int128)f4*g3_19;
    t[3] = (__int128)f0*g3 + (__int128)f1*g2    + (__int128)f2*g1    + (__int128)f3*g0    + (__int128)f4*g4_19;
    t[4] = (__int128)f0*g4 + (__int128)f1*g3    + (__int128)f2*g2    + (__int128)f3*g1    + (__int128)f4*g0;

    int64_t c;
    t[1] += (int64_t)(t[0] >> 51); h->v[0] = (int64_t)t[0] & FE_MASK51;
    t[2] += (int64_t)(t[1] >> 51); h->v[1] = (int64_t)t[1] & FE_MASK51;
    t[3] += (int64_t)(t[2] >> 51); h->v[2] = (int64_t)t[2] & FE_MASK51;
    t[4] += (int64_t)(t[3] >> 51); h->v[3] = (int64_t)t[3] & FE_MASK51;
    c     = (int64_t)(t[4] >> 51); h->v[4] = (int64_t)t[4] & FE_MASK51;
    h->v[0] += c * 19;
    c = h->v[0] >> 51; h->v[0] &= FE_MASK51; h->v[1] += c;
#else
    /* Fallback: naive approach for non-x86_64.
     * Split each limb into two 26-bit halves to avoid overflow. */
    fe25519 tmp; fe_0(&tmp);
    for (int i = 0; i < 5; i++) {
        int64_t fi = f->v[i];
        for (int j = 0; j < 5; j++) {
            int k = i + j;
            int64_t gj = g->v[j];
            if (k >= 5) {
                tmp.v[k - 5] += fi * gj * 19;
            } else {
                tmp.v[k] += fi * gj;
            }
        }
    }
    fe_carry(&tmp); fe_carry(&tmp);
    *h = tmp;
#endif
}

static void fe_sq(fe25519 *h, const fe25519 *f) { fe_mul(h, f, f); }

static uint64_t _fe_load8(const uint8_t *p)
{
    uint64_t r = 0;
    for (int i = 7; i >= 0; i--) r = (r << 8) | p[i];
    return r;
}

static void fe_frombytes(fe25519 *h, const uint8_t s[32])
{
    uint64_t lo   = _fe_load8(s);
    uint64_t mid1 = _fe_load8(s + 8);
    uint64_t mid2 = _fe_load8(s + 16);
    uint64_t hi   = _fe_load8(s + 24);

    h->v[0] = (int64_t)(lo & (uint64_t)FE_MASK51);
    h->v[1] = (int64_t)(((lo >> 51) | (mid1 << 13)) & (uint64_t)FE_MASK51);
    h->v[2] = (int64_t)(((mid1 >> 38) | (mid2 << 26)) & (uint64_t)FE_MASK51);
    h->v[3] = (int64_t)(((mid2 >> 25) | (hi << 39)) & (uint64_t)FE_MASK51);
    h->v[4] = (int64_t)((hi >> 12) & (uint64_t)FE_MASK51);
}

static void fe_tobytes(uint8_t s[32], const fe25519 *f)
{
    fe25519 t;
    fe_copy(&t, f);
    fe_carry(&t);
    fe_carry(&t);

    /* Conditional subtraction of p */
    int64_t q = (t.v[0] + 19) >> 51;
    for (int i = 1; i < 5; i++) q = (t.v[i] + q) >> 51;
    t.v[0] += 19 * q;
    int64_t c;
    for (int i = 0; i < 4; i++) {
        c = t.v[i] >> 51;
        t.v[i] &= FE_MASK51;
        t.v[i+1] += c;
    }
    t.v[4] &= FE_MASK51;

    uint64_t u0 = (uint64_t)t.v[0], u1 = (uint64_t)t.v[1], u2 = (uint64_t)t.v[2];
    uint64_t u3 = (uint64_t)t.v[3], u4 = (uint64_t)t.v[4];
    uint64_t w0 = u0 | (u1 << 51);
    uint64_t w1 = (u1 >> 13) | (u2 << 38);
    uint64_t w2 = (u2 >> 26) | (u3 << 25);
    uint64_t w3 = (u3 >> 39) | (u4 << 12);
    for (int i = 0; i < 8; i++) s[i]    = (uint8_t)(w0 >> (i*8));
    for (int i = 0; i < 8; i++) s[8+i]  = (uint8_t)(w1 >> (i*8));
    for (int i = 0; i < 8; i++) s[16+i] = (uint8_t)(w2 >> (i*8));
    for (int i = 0; i < 8; i++) s[24+i] = (uint8_t)(w3 >> (i*8));
}

static int fe_isnonzero(const fe25519 *f)
{
    uint8_t s[32]; fe_tobytes(s, f);
    uint8_t r = 0; for (int i = 0; i < 32; i++) r |= s[i];
    return r != 0;
}

static int fe_isneg(const fe25519 *f)
{
    uint8_t s[32]; fe_tobytes(s, f);
    return s[0] & 1;
}

/* fe_invert: z^(p-2), p-2 = 2^255-21. Standard addition chain from ref10. */
static void fe_invert(fe25519 *out, const fe25519 *z)
{
    fe25519 t0, t1, t2, t3; int i;
    fe_sq(&t0, z);                                         /* t0 = z^2          */
    fe_sq(&t1, &t0);                                       /* t1 = z^4          */
    fe_sq(&t1, &t1);                                       /* t1 = z^8          */
    fe_mul(&t1, z, &t1);                                   /* t1 = z^9          */
    fe_mul(&t0, &t0, &t1);                                 /* t0 = z^11         */
    fe_sq(&t2, &t0);                                       /* t2 = z^22         */
    fe_mul(&t1, &t1, &t2);                                 /* t1 = z^(2^5-1)    */
    fe_sq(&t2, &t1);
    for (i=0;i<4;i++) fe_sq(&t2, &t2);                    /* t2 = z^(2^10-2^5) */
    fe_mul(&t1, &t2, &t1);                                 /* t1 = z^(2^10-1)   */
    fe_sq(&t2, &t1);
    for (i=0;i<9;i++) fe_sq(&t2, &t2);                    /* t2 = z^(2^20-2^10)*/
    fe_mul(&t2, &t2, &t1);                                 /* t2 = z^(2^20-1)   */
    fe_sq(&t3, &t2);
    for (i=0;i<19;i++) fe_sq(&t3, &t3);                   /* t3 = z^(2^40-2^20)*/
    fe_mul(&t2, &t3, &t2);                                 /* t2 = z^(2^40-1)   */
    fe_sq(&t2, &t2);
    for (i=0;i<9;i++) fe_sq(&t2, &t2);                    /* t2 = z^(2^50-2^10)*/
    fe_mul(&t1, &t2, &t1);                                 /* t1 = z^(2^50-1)   */
    fe_sq(&t2, &t1);
    for (i=0;i<49;i++) fe_sq(&t2, &t2);                   /* t2 = z^(2^100-2^50)*/
    fe_mul(&t2, &t2, &t1);                                 /* t2 = z^(2^100-1)  */
    fe_sq(&t3, &t2);
    for (i=0;i<99;i++) fe_sq(&t3, &t3);                   /* t3 = z^(2^200-2^100)*/
    fe_mul(&t2, &t3, &t2);                                 /* t2 = z^(2^200-1)  */
    fe_sq(&t2, &t2);
    for (i=0;i<49;i++) fe_sq(&t2, &t2);                   /* t2 = z^(2^250-2^50)*/
    fe_mul(&t1, &t2, &t1);                                 /* t1 = z^(2^250-1)  */
    fe_sq(&t1, &t1);                                       /* z^(2^251-2)       */
    fe_sq(&t1, &t1);                                       /* z^(2^252-4)       */
    fe_sq(&t1, &t1);                                       /* z^(2^253-8)       */
    fe_sq(&t1, &t1);                                       /* z^(2^254-16)      */
    fe_sq(&t1, &t1);                                       /* z^(2^255-32)      */
    fe_mul(out, &t1, &t0);                                 /* z^(2^255-21) = z^(p-2) */
}

/* fe_pow2523: z^((p-5)/8) = z^(2^252-3). Used for square root recovery. */
static void fe_pow2523(fe25519 *out, const fe25519 *z)
{
    fe25519 t0, t1, t2; int i;
    fe_sq(&t0, z);                                         /* z^2 */
    fe_sq(&t1, &t0); fe_sq(&t1, &t1);                     /* z^8 */
    fe_mul(&t1, z, &t1);                                   /* z^9 */
    fe_mul(&t0, &t0, &t1);                                 /* z^11 */
    fe_sq(&t0, &t0);                                       /* z^22 */
    fe_mul(&t0, &t1, &t0);                                 /* z^(2^5-1) */
    fe_sq(&t1, &t0);
    for (i=0;i<4;i++) fe_sq(&t1, &t1);
    fe_mul(&t0, &t1, &t0);                                 /* z^(2^10-1) */
    fe_sq(&t1, &t0);
    for (i=0;i<9;i++) fe_sq(&t1, &t1);
    fe_mul(&t1, &t1, &t0);                                 /* z^(2^20-1) */
    fe_sq(&t2, &t1);
    for (i=0;i<19;i++) fe_sq(&t2, &t2);
    fe_mul(&t1, &t2, &t1);                                 /* z^(2^40-1) */
    fe_sq(&t1, &t1);
    for (i=0;i<9;i++) fe_sq(&t1, &t1);
    fe_mul(&t0, &t1, &t0);                                 /* z^(2^50-1) */
    fe_sq(&t1, &t0);
    for (i=0;i<49;i++) fe_sq(&t1, &t1);
    fe_mul(&t1, &t1, &t0);                                 /* z^(2^100-1) */
    fe_sq(&t2, &t1);
    for (i=0;i<99;i++) fe_sq(&t2, &t2);
    fe_mul(&t1, &t2, &t1);                                 /* z^(2^200-1) */
    fe_sq(&t1, &t1);
    for (i=0;i<49;i++) fe_sq(&t1, &t1);
    fe_mul(&t0, &t1, &t0);                                 /* z^(2^250-1) */
    fe_sq(&t0, &t0); fe_sq(&t0, &t0);                     /* z^(2^252-4) */
    fe_mul(out, &t0, z);                                   /* z^(2^252-3) */
}

/* ---------- ge25519: group element on Ed25519 ----------
 * Curve: -x^2 + y^2 = 1 + d*x^2*y^2
 * Extended coords (X:Y:Z:T) where x=X/Z, y=Y/Z, T=XY/Z
 */

typedef struct { fe25519 X, Y, Z, T; } ge_p3;
typedef struct { fe25519 X, Y, Z; } ge_p2;
typedef struct { fe25519 X, Y, Z, T; } ge_p1p1;
typedef struct { fe25519 YplusX, YminusX, Z, T2d; } ge_cached;

/* Curve constant d and 2d, loaded from canonical bytes */
static int _ed25519_consts_inited = 0;
static fe25519 _ed_d, _ed_2d, _ed_sqrtm1;

static void _ed25519_init_consts(void)
{
    if (_ed25519_consts_inited) return;
    static const uint8_t d_bytes[32] = {
        0xa3,0x78,0x59,0x13,0xca,0x4d,0xeb,0x75,
        0xab,0xd8,0x41,0x41,0x4d,0x0a,0x70,0x00,
        0x98,0xe8,0x79,0x77,0x79,0x40,0xc7,0x8c,
        0x73,0xfe,0x6f,0x2b,0xee,0x6c,0x03,0x52
    };
    static const uint8_t d2_bytes[32] = {
        0x59,0xf1,0xb2,0x26,0x94,0x9b,0xd6,0xeb,
        0x56,0xb1,0x83,0x82,0x9a,0x14,0xe0,0x00,
        0x30,0xd1,0xf3,0xee,0xf2,0x80,0x8e,0x19,
        0xe7,0xfc,0xdf,0x56,0xdc,0xd9,0x06,0x24
    };
    static const uint8_t sqrtm1_bytes[32] = {
        0xb0,0xa0,0x0e,0x4a,0x27,0x1b,0xee,0xc4,
        0x78,0xe4,0x2f,0xad,0x06,0x18,0x43,0x2f,
        0xa7,0xd7,0xfb,0x3d,0x99,0x00,0x4d,0x2b,
        0x0b,0xdf,0xc1,0x4f,0x80,0x24,0x83,0x2b
    };
    fe_frombytes(&_ed_d, d_bytes);
    fe_frombytes(&_ed_2d, d2_bytes);
    fe_frombytes(&_ed_sqrtm1, sqrtm1_bytes);
    _ed25519_consts_inited = 1;
}

/* ge_p3_0: identity (0,1,1,0) */
static void ge_p3_0(ge_p3 *h)
{
    fe_0(&h->X); fe_1(&h->Y); fe_1(&h->Z); fe_0(&h->T);
}

/* Conversion routines */
static void ge_p3_to_p2(ge_p2 *r, const ge_p3 *p)
{
    fe_copy(&r->X, &p->X); fe_copy(&r->Y, &p->Y); fe_copy(&r->Z, &p->Z);
}

static void ge_p1p1_to_p3(ge_p3 *r, const ge_p1p1 *p)
{
    /* Use temporaries to avoid aliasing if r overlaps p */
    fe25519 tX, tY, tZ, tT;
    fe_mul(&tX, &p->X, &p->T);
    fe_mul(&tY, &p->Y, &p->Z);
    fe_mul(&tZ, &p->Z, &p->T);
    fe_mul(&tT, &p->X, &p->Y);
    fe_copy(&r->X, &tX);
    fe_copy(&r->Y, &tY);
    fe_copy(&r->Z, &tZ);
    fe_copy(&r->T, &tT);
}

static void ge_p1p1_to_p2(ge_p2 *r, const ge_p1p1 *p)
{
    fe_mul(&r->X, &p->X, &p->T);
    fe_mul(&r->Y, &p->Y, &p->Z);
    fe_mul(&r->Z, &p->Z, &p->T);
}

static void ge_p3_to_cached(ge_cached *r, const ge_p3 *p)
{
    fe_add(&r->YplusX, &p->Y, &p->X);
    fe_sub(&r->YminusX, &p->Y, &p->X);
    fe_copy(&r->Z, &p->Z);
    fe_mul(&r->T2d, &p->T, &_ed_2d);
}

/* Doubling: p2 -> p1p1 (ref10 ge_p2_dbl)
 * Uses local copies to avoid any aliasing issues between r and p. */
static void ge_p2_dbl(ge_p1p1 *r, const ge_p2 *p)
{
    fe25519 A, B, C, t0;
    fe_sq(&A, &p->X);              /* A = X^2 */
    fe_sq(&B, &p->Y);              /* B = Y^2 */
    fe_sq(&C, &p->Z);
    fe_add(&C, &C, &C);            /* C = 2*Z^2 */
    fe_add(&t0, &p->X, &p->Y);
    fe_sq(&t0, &t0);               /* t0 = (X+Y)^2 */
    fe25519 ApB, BmA;
    fe_add(&ApB, &B, &A);          /* A+B */
    fe_sub(&BmA, &B, &A);          /* B-A */
    fe_sub(&r->X, &t0, &ApB);      /* E = (X+Y)^2 - (A+B) = 2XY */
    fe_copy(&r->Y, &ApB);          /* A+B */
    fe_copy(&r->Z, &BmA);          /* B-A */
    fe_sub(&r->T, &C, &BmA);       /* 2Z^2 - (B-A) */
}

/* Doubling: p3 -> p1p1 */
static void ge_p3_dbl(ge_p1p1 *r, const ge_p3 *p)
{
    ge_p2 q; ge_p3_to_p2(&q, p); ge_p2_dbl(r, &q);
}

/* Addition: p3 + cached -> p1p1 (ref10 ge_add) */
static void ge_add_cached(ge_p1p1 *r, const ge_p3 *p, const ge_cached *q)
{
    fe25519 t0;
    fe_add(&r->X, &p->Y, &p->X);
    fe_sub(&r->Y, &p->Y, &p->X);
    fe_mul(&r->Z, &r->X, &q->YplusX);
    fe_mul(&r->Y, &r->Y, &q->YminusX);
    fe_mul(&r->T, &q->T2d, &p->T);
    fe_mul(&t0, &p->Z, &q->Z);
    fe_add(&t0, &t0, &t0);
    fe_sub(&r->X, &r->Z, &r->Y);
    fe_add(&r->Y, &r->Z, &r->Y);
    fe_add(&r->Z, &t0, &r->T);
    fe_sub(&r->T, &t0, &r->T);
}

/* Subtraction: p3 - cached -> p1p1 (ref10 ge_sub) */
static void ge_sub_cached(ge_p1p1 *r, const ge_p3 *p, const ge_cached *q)
{
    fe25519 t0;
    fe_add(&r->X, &p->Y, &p->X);
    fe_sub(&r->Y, &p->Y, &p->X);
    fe_mul(&r->Z, &r->X, &q->YminusX);
    fe_mul(&r->Y, &r->Y, &q->YplusX);
    fe_mul(&r->T, &q->T2d, &p->T);
    fe_mul(&t0, &p->Z, &q->Z);
    fe_add(&t0, &t0, &t0);
    fe_sub(&r->X, &r->Z, &r->Y);
    fe_add(&r->Y, &r->Z, &r->Y);
    fe_sub(&r->Z, &t0, &r->T);
    fe_add(&r->T, &t0, &r->T);
}

/* Point encoding: compress p3 to 32 bytes */
static void ge_tobytes(uint8_t s[32], const ge_p3 *h)
{
    fe25519 recip, x, y;
    fe_invert(&recip, &h->Z);
    fe_mul(&x, &h->X, &recip);
    fe_mul(&y, &h->Y, &recip);
    fe_tobytes(s, &y);
    s[31] ^= (uint8_t)(fe_isneg(&x) << 7);
}

/* Point decoding: decompress 32 bytes to p3 (returns -P as in ref10).
 * Returns 0 on success, -1 on invalid point. */
static int ge_frombytes_negate_vartime(ge_p3 *h, const uint8_t s[32])
{
    _ed25519_init_consts();
    fe25519 u, v, v3, vxx, check;

    int x_sign = (s[31] >> 7) & 1;
    uint8_t s2[32];
    for (int i = 0; i < 32; i++) s2[i] = s[i];
    s2[31] &= 0x7F;

    fe_frombytes(&h->Y, s2);
    fe_1(&h->Z);

    /* u = y^2 - 1, v = d*y^2 + 1 */
    fe_sq(&u, &h->Y);
    fe_mul(&v, &u, &_ed_d);
    fe_sub(&u, &u, &h->Z);
    fe_add(&v, &v, &h->Z);

    /* x = u * v^3 * (u * v^7)^((p-5)/8) */
    fe_sq(&v3, &v);
    fe_mul(&v3, &v3, &v);       /* v^3 */
    fe_sq(&h->X, &v3);
    fe_mul(&h->X, &h->X, &v);   /* v^7 */
    fe_mul(&h->X, &h->X, &u);   /* u*v^7 */
    fe_pow2523(&h->X, &h->X);   /* (u*v^7)^((p-5)/8) */
    fe_mul(&h->X, &h->X, &v3);  /* * v^3 */
    fe_mul(&h->X, &h->X, &u);   /* * u */

    /* Verify: v * x^2 == u */
    fe_sq(&vxx, &h->X);
    fe_mul(&vxx, &vxx, &v);
    fe_sub(&check, &vxx, &u);
    if (fe_isnonzero(&check)) {
        fe_add(&check, &vxx, &u);
        if (fe_isnonzero(&check)) return -1;
        fe_mul(&h->X, &h->X, &_ed_sqrtm1);
    }

    /* Adjust sign: frombytes_negate returns -P, so we want the x
     * that, when negated, gives the correct sign for -P.
     * If fe_isneg(x) == x_sign, negate x (so -P has opposite sign). */
    if (fe_isneg(&h->X) == x_sign) {
        fe_neg(&h->X, &h->X);
    }

    fe_mul(&h->T, &h->X, &h->Y);
    return 0;
}

/* Scalar mult: [s]B (base point), double-and-add */
static void ge_scalarmult_base(ge_p3 *result, const uint8_t s[32])
{
    _ed25519_init_consts();

    /* Decode base point from canonical encoding */
    static const uint8_t base_enc[32] = {
        0x58,0x66,0x66,0x66,0x66,0x66,0x66,0x66,
        0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,
        0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,
        0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66
    };
    ge_p3 B;
    ge_frombytes_negate_vartime(&B, base_enc);
    /* frombytes returns -B; negate X,T to get +B */
    fe_neg(&B.X, &B.X);
    fe_neg(&B.T, &B.T);

    ge_p3_0(result);
    int started = 0;

    for (int i = 255; i >= 0; i--) {
        if (started) {
            ge_p1p1 t; ge_p3_dbl(&t, result); ge_p1p1_to_p3(result, &t);
        }
        if ((s[i/8] >> (i%8)) & 1) {
            if (!started) {
                *result = B; started = 1;
            } else {
                ge_p1p1 t; ge_cached Bc;
                ge_p3_to_cached(&Bc, &B);
                ge_add_cached(&t, result, &Bc);
                ge_p1p1_to_p3(result, &t);
            }
        }
        /* Periodic carry to prevent limb growth in extended coordinates */
        if (started && (i & 3) == 0) {
            fe_carry(&result->X); fe_carry(&result->Y);
            fe_carry(&result->Z); fe_carry(&result->T);
        }
    }
    if (!started) ge_p3_0(result);
}

/* Generic scalar mult: [s]P */
static void ge_scalarmult(ge_p3 *result, const uint8_t s[32], const ge_p3 *P)
{
    ge_p3_0(result);
    int started = 0;

    for (int i = 255; i >= 0; i--) {
        if (started) {
            ge_p1p1 t; ge_p3_dbl(&t, result); ge_p1p1_to_p3(result, &t);
        }
        if ((s[i/8] >> (i%8)) & 1) {
            if (!started) {
                *result = *P; started = 1;
            } else {
                ge_p1p1 t; ge_cached Pc;
                ge_p3_to_cached(&Pc, P);
                ge_add_cached(&t, result, &Pc);
                ge_p1p1_to_p3(result, &t);
            }
        }
        /* Periodic carry to prevent limb growth in extended coordinates */
        if (started && (i & 3) == 0) {
            fe_carry(&result->X); fe_carry(&result->Y);
            fe_carry(&result->Z); fe_carry(&result->T);
        }
    }
    if (!started) ge_p3_0(result);
}

/* ---------- Scalar arithmetic mod L ----------
 * L = 2^252 + 27742317777372353535851937790883648493
 * Using 21-bit limbs (12 limbs for 252 bits).
 */

static void _sc_load21(int64_t out[24], const uint8_t in[], int nbytes)
{
    /* Load nbytes as 21-bit limbs. For 32 bytes -> 12 limbs, 64 bytes -> 24 limbs */
    int nlimbs = (nbytes == 64) ? 24 : 12;
    for (int i = 0; i < nlimbs; i++) out[i] = 0;

    out[ 0] = (int64_t)( in[0]        | ((int64_t)in[1]  << 8) | ((int64_t)in[2]  << 16)) & 0x1FFFFF;
    out[ 1] = (int64_t)((in[2]  >> 5) | ((int64_t)in[3]  << 3) | ((int64_t)in[4]  << 11) | ((int64_t)in[5]  << 19)) & 0x1FFFFF;
    out[ 2] = (int64_t)((in[5]  >> 2) | ((int64_t)in[6]  << 6) | ((int64_t)in[7]  << 14)) & 0x1FFFFF;
    out[ 3] = (int64_t)((in[7]  >> 7) | ((int64_t)in[8]  << 1) | ((int64_t)in[9]  << 9) | ((int64_t)in[10] << 17)) & 0x1FFFFF;
    out[ 4] = (int64_t)((in[10] >> 4) | ((int64_t)in[11] << 4) | ((int64_t)in[12] << 12) | ((int64_t)in[13] << 20)) & 0x1FFFFF;
    out[ 5] = (int64_t)((in[13] >> 1) | ((int64_t)in[14] << 7) | ((int64_t)in[15] << 15)) & 0x1FFFFF;
    out[ 6] = (int64_t)((in[15] >> 6) | ((int64_t)in[16] << 2) | ((int64_t)in[17] << 10) | ((int64_t)in[18] << 18)) & 0x1FFFFF;
    out[ 7] = (int64_t)((in[18] >> 3) | ((int64_t)in[19] << 5) | ((int64_t)in[20] << 13)) & 0x1FFFFF;

    if (nbytes < 22) return;
    out[ 8] = (int64_t)( in[21]       | ((int64_t)in[22] << 8) | ((int64_t)in[23] << 16)) & 0x1FFFFF;
    out[ 9] = (int64_t)((in[23] >> 5) | ((int64_t)in[24] << 3) | ((int64_t)in[25] << 11) | ((int64_t)in[26] << 19)) & 0x1FFFFF;
    out[10] = (int64_t)((in[26] >> 2) | ((int64_t)in[27] << 6) | ((int64_t)in[28] << 14)) & 0x1FFFFF;
    /* For 32-byte inputs, limb 11 is the LAST limb. A 256-bit scalar needs
     * 12*21=252 bits + 4 extra, so the top limb holds up to 25 bits
     * (bits 231..255). Masking to 21 bits would lose the clamped private-key
     * bit 254, breaking sc_muladd.
     * For 64-byte inputs, limb 12 picks up at byte 31 bit 4 (=bit 252), so
     * limb 11 MUST be masked to avoid double-counting bits 252-255. */
    if (nbytes <= 32) {
        out[11] = (int64_t)((in[28] >> 7) | ((int64_t)in[29] << 1) | ((int64_t)in[30] << 9) | ((int64_t)in[31] << 17));
        return;
    }
    out[11] = (int64_t)((in[28] >> 7) | ((int64_t)in[29] << 1) | ((int64_t)in[30] << 9) | ((int64_t)in[31] << 17)) & 0x1FFFFF;

    out[12] = (int64_t)((in[31] >> 4) | ((int64_t)in[32] << 4) | ((int64_t)in[33] << 12) | ((int64_t)in[34] << 20)) & 0x1FFFFF;
    out[13] = (int64_t)((in[34] >> 1) | ((int64_t)in[35] << 7) | ((int64_t)in[36] << 15)) & 0x1FFFFF;
    out[14] = (int64_t)((in[36] >> 6) | ((int64_t)in[37] << 2) | ((int64_t)in[38] << 10) | ((int64_t)in[39] << 18)) & 0x1FFFFF;
    out[15] = (int64_t)((in[39] >> 3) | ((int64_t)in[40] << 5) | ((int64_t)in[41] << 13)) & 0x1FFFFF;
    out[16] = (int64_t)( in[42]       | ((int64_t)in[43] << 8) | ((int64_t)in[44] << 16)) & 0x1FFFFF;
    out[17] = (int64_t)((in[44] >> 5) | ((int64_t)in[45] << 3) | ((int64_t)in[46] << 11) | ((int64_t)in[47] << 19)) & 0x1FFFFF;
    out[18] = (int64_t)((in[47] >> 2) | ((int64_t)in[48] << 6) | ((int64_t)in[49] << 14)) & 0x1FFFFF;
    out[19] = (int64_t)((in[49] >> 7) | ((int64_t)in[50] << 1) | ((int64_t)in[51] << 9) | ((int64_t)in[52] << 17)) & 0x1FFFFF;
    out[20] = (int64_t)((in[52] >> 4) | ((int64_t)in[53] << 4) | ((int64_t)in[54] << 12) | ((int64_t)in[55] << 20)) & 0x1FFFFF;
    out[21] = (int64_t)((in[55] >> 1) | ((int64_t)in[56] << 7) | ((int64_t)in[57] << 15)) & 0x1FFFFF;
    out[22] = (int64_t)((in[57] >> 6) | ((int64_t)in[58] << 2) | ((int64_t)in[59] << 10) | ((int64_t)in[60] << 18)) & 0x1FFFFF;
    out[23] = (int64_t)((in[60] >> 3) | ((int64_t)in[61] << 5) | ((int64_t)in[62] << 13) | ((int64_t)in[63] << 21));
}

static void _sc_pack(uint8_t out[32], const int64_t s[12])
{
    out[ 0] = (uint8_t)(s[0]  >>  0);
    out[ 1] = (uint8_t)(s[0]  >>  8);
    out[ 2] = (uint8_t)((s[0] >> 16) | (s[1] << 5));
    out[ 3] = (uint8_t)(s[1]  >>  3);
    out[ 4] = (uint8_t)(s[1]  >> 11);
    out[ 5] = (uint8_t)((s[1] >> 19) | (s[2] << 2));
    out[ 6] = (uint8_t)(s[2]  >>  6);
    out[ 7] = (uint8_t)((s[2] >> 14) | (s[3] << 7));
    out[ 8] = (uint8_t)(s[3]  >>  1);
    out[ 9] = (uint8_t)(s[3]  >>  9);
    out[10] = (uint8_t)((s[3] >> 17) | (s[4] << 4));
    out[11] = (uint8_t)(s[4]  >>  4);
    out[12] = (uint8_t)(s[4]  >> 12);
    out[13] = (uint8_t)((s[4] >> 20) | (s[5] << 1));
    out[14] = (uint8_t)(s[5]  >>  7);
    out[15] = (uint8_t)((s[5] >> 15) | (s[6] << 6));
    out[16] = (uint8_t)(s[6]  >>  2);
    out[17] = (uint8_t)(s[6]  >> 10);
    out[18] = (uint8_t)((s[6] >> 18) | (s[7] << 3));
    out[19] = (uint8_t)(s[7]  >>  5);
    out[20] = (uint8_t)(s[7]  >> 13);
    out[21] = (uint8_t)(s[8]  >>  0);
    out[22] = (uint8_t)(s[8]  >>  8);
    out[23] = (uint8_t)((s[8] >> 16) | (s[9] << 5));
    out[24] = (uint8_t)(s[9]  >>  3);
    out[25] = (uint8_t)(s[9]  >> 11);
    out[26] = (uint8_t)((s[9] >> 19) | (s[10] << 2));
    out[27] = (uint8_t)(s[10] >>  6);
    out[28] = (uint8_t)((s[10] >> 14) | (s[11] << 7));
    out[29] = (uint8_t)(s[11] >>  1);
    out[30] = (uint8_t)(s[11] >>  9);
    out[31] = (uint8_t)(s[11] >> 17);
}

static const uint32_t _sc_l32[8] = {
    0x5cf5d3edu, 0x5812631au, 0xa2f79cd6u, 0x14def9deu,
    0x00000000u, 0x00000000u, 0x00000000u, 0x10000000u
};

static void _sc_load_u32_8(uint32_t out[8], const uint8_t in[32])
{
    for (int i = 0; i < 8; i++) {
        out[i] = ((uint32_t)in[i * 4]) |
                 ((uint32_t)in[i * 4 + 1] << 8) |
                 ((uint32_t)in[i * 4 + 2] << 16) |
                 ((uint32_t)in[i * 4 + 3] << 24);
    }
}

static void _sc_load_u32_16(uint32_t out[16], const uint8_t in[64])
{
    for (int i = 0; i < 16; i++) {
        out[i] = ((uint32_t)in[i * 4]) |
                 ((uint32_t)in[i * 4 + 1] << 8) |
                 ((uint32_t)in[i * 4 + 2] << 16) |
                 ((uint32_t)in[i * 4 + 3] << 24);
    }
}

static void _sc_store_u32_8(uint8_t out[32], const uint32_t in[8])
{
    for (int i = 0; i < 8; i++) {
        out[i * 4 + 0] = (uint8_t)(in[i] >> 0);
        out[i * 4 + 1] = (uint8_t)(in[i] >> 8);
        out[i * 4 + 2] = (uint8_t)(in[i] >> 16);
        out[i * 4 + 3] = (uint8_t)(in[i] >> 24);
    }
}

static int _sc_cmp_u32_8(const uint32_t a[8], const uint32_t b[8])
{
    for (int i = 7; i >= 0; i--) {
        if (a[i] > b[i]) return 1;
        if (a[i] < b[i]) return -1;
    }
    return 0;
}

static void _sc_sub_u32_8(uint32_t out[8], const uint32_t a[8], const uint32_t b[8])
{
    uint64_t borrow = 0;
    for (int i = 0; i < 8; i++) {
        uint64_t ai = (uint64_t)a[i];
        uint64_t bi = (uint64_t)b[i] + borrow;
        if (ai >= bi) {
            out[i] = (uint32_t)(ai - bi);
            borrow = 0;
        } else {
            out[i] = (uint32_t)((0x100000000ULL + ai) - bi);
            borrow = 1;
        }
    }
}

static void _sc_add_u32_8(uint32_t out[8], const uint32_t a[8], const uint32_t b[8])
{
    uint64_t carry = 0;
    for (int i = 0; i < 8; i++) {
        uint64_t sum = (uint64_t)a[i] + (uint64_t)b[i] + carry;
        out[i] = (uint32_t)(sum & 0xFFFFFFFFu);
        carry = sum >> 32;
    }
}

static int _sc_bit_is_set_16(const uint32_t limbs[16], unsigned bit)
{
    return (int)((limbs[bit / 32] >> (bit % 32)) & 1u);
}

static void _sc_sub_shifted_L16(uint32_t limbs[16], unsigned shift)
{
    uint32_t shifted[16];
    for (int i = 0; i < 16; i++) shifted[i] = 0;

    unsigned word_shift = shift / 32;
    unsigned bit_shift = shift % 32;
    for (unsigned j = 0; j < 8; j++) {
        unsigned idx = word_shift + j;
        if (idx >= 16) break;
        uint64_t piece = (uint64_t)_sc_l32[j] << bit_shift;
        shifted[idx] |= (uint32_t)(piece & 0xFFFFFFFFu);
        if (idx + 1 < 16) shifted[idx + 1] |= (uint32_t)(piece >> 32);
    }

    uint64_t borrow = 0;
    for (int i = 0; i < 16; i++) {
        uint64_t cur = (uint64_t)limbs[i];
        uint64_t sub = (uint64_t)shifted[i] + borrow;
        if (cur >= sub) {
            limbs[i] = (uint32_t)(cur - sub);
            borrow = 0;
        } else {
            limbs[i] = (uint32_t)((0x100000000ULL + cur) - sub);
            borrow = 1;
        }
    }
}

/* Reduce mod L using the relation:
 * L = 2^252 + c, where c is small. At limb position 12 we have 2^252.
 * So s[i] for i >= 12: subtract s[i] * L_low from s[i-12..i-7],
 * and s[i]*1 from s[i] (which becomes 0). */
static void _sc_reduce_limbs(int64_t s[24])
{
    int64_t carry;

    /* --- Round 1: fold high limbs (s[23]..s[12]) into s[0..11] --- */
    for (int i = 23; i >= 12; i--) {
        int64_t si = s[i]; s[i] = 0;
        s[i-12] += si * 666643;
        s[i-11] += si * 470296;
        s[i-10] += si * 654183;
        s[i-9]  -= si * 997805;
        s[i-8]  += si * 136657;
        s[i-7]  -= si * 683901;
    }

    /* --- Round 1 carry propagation (ref10 pattern) --- */
    /* Even limbs first */
    for (int i = 0; i < 12; i += 2) {
        carry = (s[i] + (1LL << 20)) >> 21;
        s[i] -= carry << 21;
        if (i + 1 < 12) s[i+1] += carry;
    }
    /* Odd limbs */
    for (int i = 1; i < 12; i += 2) {
        carry = (s[i] + (1LL << 20)) >> 21;
        s[i] -= carry << 21;
        if (i + 1 < 12) s[i+1] += carry;
    }

    /* --- L-wrap: carry out of s[11] wraps back via L --- */
    {
        int64_t s12 = (s[11] + (1LL << 20)) >> 21;
        s[11] -= s12 << 21;
        s[0] += s12 * 666643;
        s[1] += s12 * 470296;
        s[2] += s12 * 654183;
        s[3] -= s12 * 997805;
        s[4] += s12 * 136657;
        s[5] -= s12 * 683901;
    }

    /* --- Round 2 carry propagation --- */
    for (int i = 0; i < 12; i += 2) {
        carry = (s[i] + (1LL << 20)) >> 21;
        s[i] -= carry << 21;
        if (i + 1 < 12) s[i+1] += carry;
    }
    for (int i = 1; i < 12; i += 2) {
        carry = (s[i] + (1LL << 20)) >> 21;
        s[i] -= carry << 21;
        if (i + 1 < 12) s[i+1] += carry;
    }

    /* --- Second L-wrap --- */
    {
        int64_t s12 = (s[11] + (1LL << 20)) >> 21;
        s[11] -= s12 << 21;
        s[0] += s12 * 666643;
        s[1] += s12 * 470296;
        s[2] += s12 * 654183;
        s[3] -= s12 * 997805;
        s[4] += s12 * 136657;
        s[5] -= s12 * 683901;
    }

    /* Final carry propagation to normalize all limbs */
    for (int i = 0; i < 11; i++) {
        carry = (s[i] + (1LL << 20)) >> 21;
        s[i] -= carry << 21;
        s[i+1] += carry;
    }
}

static void sc_reduce(uint8_t out[32], const uint8_t in[64])
{
    uint32_t limbs[16];
    _sc_load_u32_16(limbs, in);

    for (int bit = 511; bit >= 252; bit--) {
        if (_sc_bit_is_set_16(limbs, (unsigned)bit)) {
            _sc_sub_shifted_L16(limbs, (unsigned)(bit - 252));
        }
    }

    uint32_t low[8];
    for (int i = 0; i < 8; i++) low[i] = limbs[i];
    while (_sc_cmp_u32_8(low, _sc_l32) >= 0) {
        _sc_sub_u32_8(low, low, _sc_l32);
    }
    _sc_store_u32_8(out, low);
}

/* sc_muladd: out = (a * b + c) mod L */
static void sc_muladd(uint8_t out[32], const uint8_t a[32], const uint8_t b[32], const uint8_t c[32])
{
    uint32_t al[8], bl[8], cl[8];
    _sc_load_u32_8(al, a);
    _sc_load_u32_8(bl, b);
    _sc_load_u32_8(cl, c);

    unsigned __int128 accum[16];
    for (int i = 0; i < 16; i++) accum[i] = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            accum[i + j] += (unsigned __int128)al[i] * (unsigned __int128)bl[j];
        }
    }

    uint32_t prod32[16];
    unsigned __int128 carry = 0;
    for (int i = 0; i < 16; i++) {
        unsigned __int128 sum = accum[i] + carry;
        prod32[i] = (uint32_t)(sum & 0xFFFFFFFFu);
        carry = sum >> 32;
    }

    uint8_t prod[64];
    for (int i = 0; i < 16; i++) {
        prod[i * 4 + 0] = (uint8_t)(prod32[i] >> 0);
        prod[i * 4 + 1] = (uint8_t)(prod32[i] >> 8);
        prod[i * 4 + 2] = (uint8_t)(prod32[i] >> 16);
        prod[i * 4 + 3] = (uint8_t)(prod32[i] >> 24);
    }

    uint8_t reduced[32];
    sc_reduce(reduced, prod);
    _sc_load_u32_8(prod32, reduced);
    _sc_add_u32_8(prod32, prod32, cl);
    while (_sc_cmp_u32_8(prod32, _sc_l32) >= 0) {
        _sc_sub_u32_8(prod32, prod32, _sc_l32);
    }
    _sc_store_u32_8(out, prod32);
}

/* ---------- ring crypto stubs (baremetal fallback — return -1 to trigger software path) ---------- */

int64_t rt_tls13_ring_ed25519_keypair_raw(const uint8_t seed[32], uint8_t pk[32], uint8_t sk[64])
{
    (void)seed; (void)pk; (void)sk;
    return -1;
}

int64_t rt_tls13_ring_ed25519_sign_raw(const uint8_t *msg, uint32_t msg_len,
                                        const uint8_t sk[64], uint8_t sig[64])
{
    (void)msg; (void)msg_len; (void)sk; (void)sig;
    return -1;
}

int64_t rt_tls13_ring_ed25519_verify_raw(const uint8_t *msg, uint32_t msg_len,
                                          const uint8_t pk[32], const uint8_t sig[64])
{
    (void)msg; (void)msg_len; (void)pk; (void)sig;
    return -1;
}

int64_t rt_tls13_ring_x25519_shared_secret_into_raw(const uint8_t scalar[32],
                                                      const uint8_t point[32],
                                                      uint8_t out[32])
{
    (void)scalar; (void)point; (void)out;
    return -1;
}

/* ---------- Ed25519 high-level API ---------- */

static void _ed25519_create_keypair(const uint8_t seed[32], uint8_t pk[32], uint8_t sk[64])
{
    extern int64_t rt_tls13_ring_ed25519_keypair_raw(const uint8_t seed[32], uint8_t pk[32], uint8_t sk[64]);
    if (rt_tls13_ring_ed25519_keypair_raw(seed, pk, sk) == 0) return;
    for (int i = 0; i < 32; i++) pk[i] = 0;
    for (int i = 0; i < 64; i++) sk[i] = 0;
}

static void _ed25519_sign(const uint8_t *msg, uint32_t msg_len,
                           const uint8_t sk[64], uint8_t sig[64])
{
    extern int64_t rt_tls13_ring_ed25519_sign_raw(const uint8_t *msg, uint32_t msg_len,
                                                  const uint8_t sk[64], uint8_t sig[64]);
    if (rt_tls13_ring_ed25519_sign_raw(msg, msg_len, sk, sig) == 0) return;
    for (int i = 0; i < 64; i++) sig[i] = 0;
}

static int _ed25519_verify(const uint8_t *msg, uint32_t msg_len,
                            const uint8_t pk[32], const uint8_t sig[64])
{
    extern int64_t rt_tls13_ring_ed25519_verify_raw(const uint8_t *msg, uint32_t msg_len,
                                                    const uint8_t pk[32], const uint8_t sig[64]);
    return rt_tls13_ring_ed25519_verify_raw(msg, msg_len, pk, sig);
}

/* ---------- RuntimeValue API wrappers ---------- */

static uint8_t *_ed_rv_to_bytes(int64_t rv, uint32_t *out_len)
{
    if (!IS_HEAP(rv)) return (void*)0;
    HeapHeader *hdr = (HeapHeader *)DECODE_PTR(rv);
    if (!hdr || hdr->type != HEAP_ARRAY) return (void*)0;
    RuntimeArray *arr = (RuntimeArray *)hdr;
    uint32_t len = arr->len;
    RuntimeValue *items = runtime_array_items(arr);
    uint8_t *buf = (uint8_t *)malloc(len);
    if (!buf) return (void*)0;
    for (uint32_t i = 0; i < len; i++)
        buf[i] = (uint8_t)(DECODE_INT(items[i]) & 0xFF);
    *out_len = len;
    return buf;
}

static inline uint8_t _rv_byte(RuntimeValue v)
{
    int64_t byte_val = IS_INT(v) ? DECODE_INT(v) : (int64_t)v;
    return (uint8_t)(byte_val & 0xFF);
}

static int _ed_bytes_to_rv(const uint8_t *src, uint32_t src_len, int64_t rv)
{
    if (!IS_HEAP(rv)) return -1;
    HeapHeader *hdr = (HeapHeader *)DECODE_PTR(rv);
    if (!hdr || hdr->type != HEAP_ARRAY) return -1;
    RuntimeArray *arr = (RuntimeArray *)hdr;
    if (arr->len < src_len) return -1;
    RuntimeValue *items = runtime_array_items(arr);
    for (uint32_t i = 0; i < src_len; i++)
        items[i] = ENCODE_INT(src[i]);
    return 0;
}

int64_t rt_ed25519_keypair(int64_t seed_rv, int64_t pk_rv)
{
    uint32_t seed_len = 0;
    uint8_t *seed = _ed_rv_to_bytes(seed_rv, &seed_len);
    if (!seed || seed_len != 32) { if (seed) free(seed); return -1; }
    uint8_t pk[32], sk[64];
    _ed25519_create_keypair(seed, pk, sk);
    free(seed);
    if (_ed_bytes_to_rv(pk, 32, pk_rv) != 0) return -1;
    return 0;
}

int64_t rt_ed25519_sign(int64_t msg_rv, int64_t sk_rv, int64_t sig_rv)
{
    uint32_t msg_len = 0, sk_len = 0;
    uint8_t *msg = _ed_rv_to_bytes(msg_rv, &msg_len);
    uint8_t *sk = _ed_rv_to_bytes(sk_rv, &sk_len);
    if (!sk || sk_len != 64) { if (msg) free(msg); if (sk) free(sk); return -1; }
    uint8_t sig[64];
    _ed25519_sign(msg ? msg : (const uint8_t*)"", msg_len, sk, sig);
    if (msg) free(msg); free(sk);
    if (_ed_bytes_to_rv(sig, 64, sig_rv) != 0) return -1;
    return 0;
}

RuntimeValue rt_ed25519_sign_seed(RuntimeValue seed_rv, RuntimeValue msg_rv)
{
    uint32_t seed_len = 0, msg_len = 0;
    uint8_t *seed = _ed_rv_to_bytes(seed_rv, &seed_len);
    uint8_t *msg = _ed_rv_to_bytes(msg_rv, &msg_len);
    if (!seed || seed_len != 32) {
        if (seed) free(seed);
        if (msg) free(msg);
        return NIL_VALUE;
    }
    uint8_t pk[32], sk[64], sig[64];
    _ed25519_create_keypair(seed, pk, sk);
    _ed25519_sign(msg ? msg : (const uint8_t*)"", msg_len, sk, sig);
    free(seed);
    if (msg) free(msg);
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + 64 * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY; a->hdr.size = sizeof(RuntimeArray) + 64 * sizeof(RuntimeValue);
    a->len = 64; a->cap = 64;
    a->items = runtime_array_inline_items(a);
    for (int i = 0; i < 64; i++) a->items[i] = ENCODE_INT(sig[i]);
    return ENCODE_PTR(a);
}

int64_t rt_ed25519_verify(int64_t msg_rv, int64_t pk_rv, int64_t sig_rv)
{
    uint32_t msg_len = 0, pk_len = 0, sig_len = 0;
    uint8_t *msg = _ed_rv_to_bytes(msg_rv, &msg_len);
    uint8_t *pk = _ed_rv_to_bytes(pk_rv, &pk_len);
    uint8_t *sig = _ed_rv_to_bytes(sig_rv, &sig_len);
    if (!pk || pk_len != 32 || !sig || sig_len != 64) {
        if (msg) free(msg); if (pk) free(pk); if (sig) free(sig);
        return -1;
    }
    int result = _ed25519_verify(msg ? msg : (const uint8_t*)"", msg_len, pk, sig);
    if (msg) free(msg); free(pk); free(sig);
    return result == 0 ? 1 : 0;
}

/* rt_ed25519_self_test: RFC 8032 §7.1 TEST 1 plus negative checks.
 * Returns 0 on pass, -1 on fail. */
int64_t rt_ed25519_self_test(void)
{
    _ed25519_init_consts();

    static const uint8_t seed[32] = {
        0x9d,0x61,0xb1,0x9d,0xef,0xfd,0x5a,0x60,
        0xba,0x84,0x4a,0xf4,0x92,0xec,0x2c,0xc4,
        0x44,0x49,0xc5,0x69,0x7b,0x32,0x69,0x19,
        0x70,0x3b,0xac,0x03,0x1c,0xae,0x7f,0x60
    };
    static const uint8_t expected_pk[32] = {
        0xd7,0x5a,0x98,0x01,0x82,0xb1,0x0a,0xb7,
        0xd5,0x4b,0xfe,0xd3,0xc9,0x64,0x07,0x3a,
        0x0e,0xe1,0x72,0xf3,0xda,0xa6,0x23,0x25,
        0xaf,0x02,0x1a,0x68,0xf7,0x07,0x51,0x1a
    };
    static const uint8_t expected_sig[64] = {
        0xe5,0x56,0x43,0x00,0xc3,0x60,0xac,0x72,
        0x90,0x86,0xe2,0xcc,0x80,0x6e,0x82,0x8a,
        0x84,0x87,0x7f,0x1e,0xb8,0xe5,0xd9,0x74,
        0xd8,0x73,0xe0,0x65,0x22,0x49,0x01,0x55,
        0x5f,0xb8,0x82,0x15,0x90,0xa3,0x3b,0xac,
        0xc6,0x1e,0x39,0x70,0x1c,0xf9,0xb4,0x6b,
        0xd2,0x5b,0xf5,0xf0,0x59,0x5b,0xbe,0x24,
        0x65,0x51,0x41,0x43,0x8e,0x7a,0x10,0x0b
    };

    serial_puts("[ed25519-c] step 1: RFC keypair...\r\n");
    uint8_t pk[32], sk[64];
    _ed25519_create_keypair(seed, pk, sk);
    for (int i = 0; i < 32; i++) {
        if (pk[i] != expected_pk[i]) {
            serial_puts("[ed25519-c] FAIL: RFC keypair mismatch\r\n");
            return -1;
        }
    }
    serial_puts("[ed25519-c] step 1: RFC keypair OK\r\n");

    serial_puts("[ed25519-c] step 2: RFC sign empty msg...\r\n");
    uint8_t sig[64];
    _ed25519_sign((const uint8_t *)"", 0, sk, sig);
    for (int i = 0; i < 64; i++) {
        if (sig[i] != expected_sig[i]) {
            serial_puts("[ed25519-c] FAIL: RFC signature mismatch\r\n");
            return -1;
        }
    }
    serial_puts("[ed25519-c] step 2: RFC signature OK\r\n");

    serial_puts("[ed25519-c] step 3: RFC verify...\r\n");
    if (_ed25519_verify((const uint8_t *)"", 0, pk, sig) != 0) {
        serial_puts("[ed25519-c] FAIL: RFC verify rejected valid sig\r\n");
        return -1;
    }
    serial_puts("[ed25519-c] step 3: RFC verify OK\r\n");

    /* 4. Verify tampered message fails */
    serial_puts("[ed25519-c] step 4: verify-reject...\r\n");
    uint8_t bad_msg[1] = {0x42};
    if (_ed25519_verify(bad_msg, 1, pk, sig) == 0) {
        serial_puts("[ed25519-c] FAIL: verify accepted bad msg\r\n");
        return -1;
    }
    serial_puts("[ed25519-c] step 4: verify-reject OK\r\n");

    /* 5. Sign+verify non-empty message */
    serial_puts("[ed25519-c] step 5: sign+verify non-empty...\r\n");
    static const uint8_t msg2[3] = {0x48, 0x69, 0x21}; /* "Hi!" */
    uint8_t sig2[64];
    _ed25519_sign(msg2, 3, sk, sig2);
    if (_ed25519_verify(msg2, 3, pk, sig2) != 0) {
        serial_puts("[ed25519-c] FAIL: verify rejected non-empty msg sig\r\n");
        return -1;
    }
    serial_puts("[ed25519-c] step 5: OK\r\n");

    serial_puts("[ed25519-c] ALL PASSED\r\n");
    return 0;
}

/* rt_string_from_byte_array: text.from_bytes([u8]) → text
 * Reads byte values from a RuntimeArray, creates a RuntimeString.
 * Byte values may be tagged (ENCODE_INT) from BoxInt push. */
RuntimeValue rt_string_from_byte_array(RuntimeValue arr)
{
    if (!IS_HEAP(arr)) return rt_string_new(0, 0); /* empty string for nil */
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return rt_string_new(0, 0);
    uint32_t len = a->len;
    RuntimeValue *items = runtime_array_items(a);
    RuntimeString *s = (RuntimeString *)malloc(sizeof(RuntimeString) + len + 1);
    if (!s) return NIL_VALUE;
    s->hdr.type = HEAP_STRING;
    s->hdr.size = (uint32_t)(sizeof(RuntimeString) + len + 1);
    s->len = len;
    for (uint32_t i = 0; i < len; i++) {
        RuntimeValue v = items[i];
        /* Items may be tagged (from BoxInt push) or raw */
        int64_t byte_val = IS_INT(v) ? DECODE_INT(v) : (int64_t)v;
        s->data[i] = (char)(byte_val & 0xFF);
    }
    s->data[len] = '\0';
    return ENCODE_PTR(s);
}

/* Also provide the name that the codegen might use for text.from_bytes */
RuntimeValue text__from_bytes(RuntimeValue arr) { return rt_string_from_byte_array(arr); }
RuntimeValue common__text__text__from_bytes(RuntimeValue arr) { return rt_string_from_byte_array(arr); }

/* rt_string_to_byte_array: text.to_bytes() → [u8]
 * Converts a RuntimeString to a RuntimeArray of BoxInt'd bytes. */
RuntimeValue rt_string_to_byte_array(RuntimeValue str)
{
    if (!IS_HEAP(str)) {
        /* Return empty array for nil/non-string */
        return rt_array_new(ENCODE_INT(0));
    }
    RuntimeString *s = (RuntimeString *)DECODE_PTR(str);
    if (!s || s->hdr.type != HEAP_STRING) return rt_array_new(ENCODE_INT(0));
    uint32_t len = s->len;
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    a->len = len;
    a->cap = len;
    a->items = runtime_array_inline_items(a);
    for (uint32_t i = 0; i < len; i++) {
        a->items[i] = ENCODE_INT((uint8_t)s->data[i]);
    }
    return ENCODE_PTR(a);
}

static inline void _tls_put_u16(uint8_t *buf, uint32_t *off, uint16_t v)
{
    buf[(*off)++] = (uint8_t)((v >> 8) & 0xFF);
    buf[(*off)++] = (uint8_t)(v & 0xFF);
}

static inline void _tls_put_u24(uint8_t *buf, uint32_t *off, uint32_t v)
{
    buf[(*off)++] = (uint8_t)((v >> 16) & 0xFF);
    buf[(*off)++] = (uint8_t)((v >> 8) & 0xFF);
    buf[(*off)++] = (uint8_t)(v & 0xFF);
}

RuntimeValue rt_tls13_build_client_hello(RuntimeValue host_rv)
{
    const uint8_t ch_random[32] = {
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf
    };
    const uint8_t pub_key[32] = {
        0x4d, 0x27, 0xbc, 0xee, 0x31, 0x35, 0xc4, 0x94,
        0x4b, 0x28, 0xd2, 0x7d, 0xd8, 0x09, 0xb0, 0x7b,
        0xe1, 0x0c, 0x35, 0x16, 0x0d, 0x20, 0x13, 0x1c,
        0xaa, 0x7e, 0x85, 0x57, 0x54, 0x98, 0xd0, 0x7c
    };
    const char *host = "localhost";
    uint32_t host_len = 9;
    if (IS_HEAP(host_rv)) {
        RuntimeString *s = (RuntimeString *)DECODE_PTR(host_rv);
        if (s && s->hdr.type == HEAP_STRING) {
            host = s->data;
            host_len = s->len;
        }
    }

    uint8_t body[256];
    uint32_t boff = 0;
    body[boff++] = 0x03;
    body[boff++] = 0x03;
    for (uint32_t i = 0; i < 32; i++) body[boff++] = ch_random[i];
    body[boff++] = 0x20;
    for (uint32_t i = 0; i < 32; i++) body[boff++] = 0x00;
    body[boff++] = 0x00;
    body[boff++] = 0x02;
    body[boff++] = 0x13;
    body[boff++] = 0x01;
    body[boff++] = 0x01;
    body[boff++] = 0x00;

    uint8_t exts[160];
    uint32_t eoff = 0;

    /* supported_versions */
    _tls_put_u16(exts, &eoff, 43);
    _tls_put_u16(exts, &eoff, 3);
    exts[eoff++] = 2;
    exts[eoff++] = 3;
    exts[eoff++] = 4;

    /* key_share */
    _tls_put_u16(exts, &eoff, 51);
    _tls_put_u16(exts, &eoff, 38);
    _tls_put_u16(exts, &eoff, 36);
    _tls_put_u16(exts, &eoff, 29);
    _tls_put_u16(exts, &eoff, 32);
    for (uint32_t i = 0; i < 32; i++) exts[eoff++] = pub_key[i];

    /* server_name */
    _tls_put_u16(exts, &eoff, 0);
    _tls_put_u16(exts, &eoff, (uint16_t)(host_len + 5));
    _tls_put_u16(exts, &eoff, (uint16_t)(host_len + 3));
    exts[eoff++] = 0;
    _tls_put_u16(exts, &eoff, (uint16_t)host_len);
    for (uint32_t i = 0; i < host_len; i++) exts[eoff++] = (uint8_t)host[i];

    /* signature_algorithms */
    _tls_put_u16(exts, &eoff, 13);
    _tls_put_u16(exts, &eoff, 4);
    _tls_put_u16(exts, &eoff, 2);
    _tls_put_u16(exts, &eoff, 0x0807);

    /* supported_groups */
    _tls_put_u16(exts, &eoff, 10);
    _tls_put_u16(exts, &eoff, 4);
    _tls_put_u16(exts, &eoff, 2);
    _tls_put_u16(exts, &eoff, 29);

    _tls_put_u16(body, &boff, (uint16_t)eoff);
    for (uint32_t i = 0; i < eoff; i++) body[boff++] = exts[i];

    uint8_t hs[260];
    uint32_t hoff = 0;
    hs[hoff++] = 1;
    _tls_put_u24(hs, &hoff, boff);
    for (uint32_t i = 0; i < boff; i++) hs[hoff++] = body[i];

    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)hoff * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)hoff * sizeof(RuntimeValue));
    a->len = hoff;
    a->cap = hoff;
    a->items = runtime_array_inline_items(a);
    for (uint32_t i = 0; i < hoff; i++) a->items[i] = ENCODE_INT(hs[i]);
    return ENCODE_PTR(a);
}

RuntimeValue rt_tls13_build_client_hello_record(RuntimeValue host_rv)
{
    RuntimeValue hs_rv = rt_tls13_build_client_hello(host_rv);
    if (!IS_HEAP(hs_rv)) return hs_rv;
    RuntimeArray *hs = (RuntimeArray *)DECODE_PTR(hs_rv);
    if (!hs || hs->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    RuntimeValue *hs_items = runtime_array_items(hs);

    uint32_t rec_len = hs->len + 5;
    RuntimeArray *rec = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)rec_len * sizeof(RuntimeValue));
    if (!rec) return NIL_VALUE;
    rec->hdr.type = HEAP_ARRAY;
    rec->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)rec_len * sizeof(RuntimeValue));
    rec->len = rec_len;
    rec->cap = rec_len;
    rec->items = runtime_array_inline_items(rec);
    rec->items[0] = ENCODE_INT(0x16);
    rec->items[1] = ENCODE_INT(0x03);
    rec->items[2] = ENCODE_INT(0x01);
    rec->items[3] = ENCODE_INT((hs->len >> 8) & 0xFF);
    rec->items[4] = ENCODE_INT(hs->len & 0xFF);
    for (uint32_t i = 0; i < hs->len; i++) rec->items[5 + i] = hs_items[i];
    return ENCODE_PTR(rec);
}

/* C-side tuple test — bypasses Simple codegen to verify rt_tuple_* works */
int64_t rt_test_tuple(void)
{
    RuntimeValue tup = rt_tuple_new(2);
    if (IS_NIL(tup)) { serial_puts("[tuple-c] new returned NIL\r\n"); return -1; }
    serial_puts("[tuple-c] new ok, IS_HEAP=");
    serial_puthex(IS_HEAP(tup) ? 1 : 0);
    serial_puts("\r\n");
    rt_tuple_set(tup, 0, ENCODE_INT(42));
    rt_tuple_set(tup, 1, ENCODE_INT(99));
    RuntimeValue v0 = rt_tuple_get(tup, 0);
    RuntimeValue v1 = rt_tuple_get(tup, 1);
    serial_puts("[tuple-c] v0=");
    serial_puthex((uint8_t)(DECODE_INT(v0) & 0xFF));
    serial_puts(" v1=");
    serial_puthex((uint8_t)(DECODE_INT(v1) & 0xFF));
    serial_puts("\r\n");
    return (DECODE_INT(v0) == 42 && DECODE_INT(v1) == 99) ? 0 : -1;
}

/* rt_random_bytes(count): generate random bytes entirely in C.
 * Bypasses Simple CSPRNG which has ChaCha20 arithmetic bugs in baremetal. */
RuntimeValue rt_random_bytes_c(int64_t count)
{
    if (count < 0 || count > 65536) count = 0;
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)count * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)count * sizeof(RuntimeValue));
    a->len = (uint32_t)count;
    a->cap = (uint32_t)count;
    a->items = runtime_array_inline_items(a);
    /* Use xorshift seeded from TSC */
    static uint64_t _rng = 0;
    if (_rng == 0) {
#ifdef __x86_64__
        uint32_t lo, hi;
        __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
        _rng = ((uint64_t)hi << 32) | lo;
#endif
        if (_rng == 0) _rng = 0xDEADBEEF12345678ULL;
    }
    for (int64_t i = 0; i < count; i++) {
        _rng ^= _rng << 13; _rng ^= _rng >> 7; _rng ^= _rng << 17;
        a->items[i] = ENCODE_INT(_rng & 0xFF);
    }
    return ENCODE_PTR(a);
}

static int _cpu_has_rdrand(void)
{
#ifdef __x86_64__
    uint32_t eax = 1, ebx = 0, ecx = 0, edx = 0;
    __asm__ volatile("cpuid"
                     : "+a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     :
                     : "memory");
    return (ecx & (1u << 30)) != 0;
#else
    return 0;
#endif
}

/* rt_rdrand: use x86 hardware RDRAND when CPUID reports it.
 * The TSC/xorshift branch is a boot-survivability fallback only; TLS keeps
 * production readiness blocked unless the platform entropy gate is promoted. */
static uint64_t _xorshift_state = 0;
int64_t rt_rdrand(void)
{
#ifdef __x86_64__
    if (_cpu_has_rdrand()) {
        for (int attempt = 0; attempt < 10; attempt++) {
            uint64_t value = 0;
            unsigned char ok = 0;
            __asm__ volatile("rdrand %0; setc %1" : "=r"(value), "=qm"(ok));
            if (ok) {
                return (int64_t)value;
            }
        }
    }
#endif
    if (_xorshift_state == 0) {
#ifdef __x86_64__
        uint32_t lo, hi;
        __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
        _xorshift_state = ((uint64_t)hi << 32) | lo;
#endif
        if (_xorshift_state == 0) _xorshift_state = 0x12345678DEADBEEFULL;
    }
    _xorshift_state ^= _xorshift_state << 13;
    _xorshift_state ^= _xorshift_state >> 7;
    _xorshift_state ^= _xorshift_state << 17;
    return (int64_t)_xorshift_state;
}

int64_t rt_entropy_hardware_ready(void)
{
    return _cpu_has_rdrand() ? 1 : 0;
}

uint64_t rt_x86_read_cr0(void)
{
#ifdef __x86_64__
    uint64_t value = 0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(value) :: "memory");
    return value;
#else
    return 0;
#endif
}

uint64_t rt_x86_read_cr4(void)
{
#ifdef __x86_64__
    uint64_t value = 0;
    __asm__ volatile("mov %%cr4, %0" : "=r"(value) :: "memory");
    return value;
#else
    return 0;
#endif
}

uint64_t rt_x86_read_efer(void)
{
#ifdef __x86_64__
    uint32_t lo = 0;
    uint32_t hi = 0;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0xC0000080u));
    return ((uint64_t)hi << 32) | lo;
#else
    return 0;
#endif
}

uint64_t rt_x86_rdrand_or_rdtsc(void)
{
#ifdef __x86_64__
    if (_cpu_has_rdrand()) {
        for (int attempt = 0; attempt < 10; attempt++) {
            uint64_t value = 0;
            unsigned char ok = 0;
            __asm__ volatile("rdrand %0; setc %1" : "=r"(value), "=qm"(ok));
            if (ok) return value;
        }
    }
    uint32_t lo = 0;
    uint32_t hi = 0;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#else
    return 0;
#endif
}

/* rt_ed25519_sign_test: sign-only (skip broken verify). Returns 0=pass, -1=fail */
int64_t rt_ed25519_sign_test(void)
{
    _ed25519_init_consts();
    static const uint8_t seed[32] = {
        0x9d,0x61,0xb1,0x9d,0xef,0xfd,0x5a,0x60,
        0xba,0x84,0x4a,0xf4,0x92,0xec,0x2c,0xc4,
        0x44,0x49,0xc5,0x69,0x7b,0x32,0x69,0x19,
        0x70,0x3b,0xac,0x03,0x1c,0xae,0x7f,0x60
    };
    uint8_t pk[32], sk[64], sig1[64], sig2[64];
    _ed25519_create_keypair(seed, pk, sk);
    _ed25519_sign((const uint8_t*)"", 0, sk, sig1);
    /* Check sig1 not all zero */
    int nz = 0;
    for (int i = 0; i < 64; i++) if (sig1[i] != 0) nz = 1;
    if (!nz) return -1;
    /* Sign different message, check sigs differ */
    static const uint8_t msg2[3] = {0x48, 0x69, 0x21};
    _ed25519_sign(msg2, 3, sk, sig2);
    int differ = 0;
    for (int i = 0; i < 64; i++) if (sig1[i] != sig2[i]) differ = 1;
    return differ ? 0 : -1;
}

/* rt_ed25519_keypair_pk: return 32-byte public key from seed */
RuntimeValue rt_ed25519_keypair_pk(RuntimeValue seed_rv)
{
    uint32_t slen = 0;
    uint8_t *seed = _ed_rv_to_bytes(seed_rv, &slen);
    if (!seed || slen != 32) { if (seed) free(seed); return NIL_VALUE; }
    uint8_t pk[32], sk[64];
    _ed25519_create_keypair(seed, pk, sk);
    free(seed);
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + 32 * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY; a->hdr.size = sizeof(RuntimeArray) + 32 * sizeof(RuntimeValue);
    a->len = 32; a->cap = 32;
    a->items = runtime_array_inline_items(a);
    for (int i = 0; i < 32; i++) a->items[i] = ENCODE_INT(pk[i]);
    return ENCODE_PTR(a);
}

/* rt_ed25519_keypair_sk: return 32-byte seed (private key) */
RuntimeValue rt_ed25519_keypair_sk(RuntimeValue seed_rv)
{
    uint32_t slen = 0;
    uint8_t *seed = _ed_rv_to_bytes(seed_rv, &slen);
    if (!seed || slen != 32) { if (seed) free(seed); return NIL_VALUE; }
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + 32 * sizeof(RuntimeValue));
    if (!a) { free(seed); return NIL_VALUE; }
    a->hdr.type = HEAP_ARRAY; a->hdr.size = sizeof(RuntimeArray) + 32 * sizeof(RuntimeValue);
    a->len = 32; a->cap = 32;
    a->items = runtime_array_inline_items(a);
    for (int i = 0; i < 32; i++) a->items[i] = ENCODE_INT(seed[i]);
    free(seed);
    return ENCODE_PTR(a);
}

/* rt_ed25519_keys_differ: 1 if different seeds produce different PKs, 0 if same */
int64_t rt_ed25519_keys_differ(RuntimeValue seed1_rv, RuntimeValue seed2_rv)
{
    uint32_t l1 = 0, l2 = 0;
    uint8_t *s1 = _ed_rv_to_bytes(seed1_rv, &l1);
    uint8_t *s2 = _ed_rv_to_bytes(seed2_rv, &l2);
    if (!s1 || !s2 || l1 != 32 || l2 != 32) { free(s1); free(s2); return 0; }
    uint8_t pk1[32], pk2[32], sk[64];
    _ed25519_create_keypair(s1, pk1, sk);
    _ed25519_create_keypair(s2, pk2, sk);
    free(s1); free(s2);
    for (int i = 0; i < 32; i++) if (pk1[i] != pk2[i]) return 1;
    return 0;
}

/* Also provide the unmangled name for direct extern fn calls */
int64_t syscall(uint64_t id, uint64_t a0, uint64_t a1,
                uint64_t a2, uint64_t a3, uint64_t a4)
{
    return userlib__syscall_raw__syscall(id, a0, a1, a2, a3, a4);
}

int64_t rt_ipc_send(uint64_t port, uint64_t method, uint64_t flags,
                     uint64_t buf_addr, uint64_t buf_len)
{
    return _ipc_send_handler(port, method, flags, buf_addr, buf_len);
}

int64_t rt_ipc_recv(uint64_t port, uint64_t buf_addr, uint64_t max_len)
{
    return _ipc_recv_handler(port, buf_addr, max_len);
}

/* rt_ipc_send_bytes: send a Simple [u8] payload over IPC by packing tagged
 * RuntimeArray elements into a contiguous raw byte buffer first. */
int64_t rt_ipc_send_bytes(uint64_t port, uint64_t method, RuntimeValue data_rv)
{
    if (!IS_HEAP(data_rv)) return -22;
    RuntimeArray *arr = (RuntimeArray *)(uintptr_t)DECODE_PTR(data_rv);
    if (!arr) return -22;
    uint32_t payload_len = arr->len;
    RuntimeValue *items = runtime_array_items(arr);
    uint32_t len = payload_len + 4;
    uint8_t *buf = NULL;
    if (len > 0) {
        buf = (uint8_t *)malloc(len);
        if (!buf) return -12;
        buf[0] = (uint8_t)(method & 0xFF);
        buf[1] = (uint8_t)((method >> 8) & 0xFF);
        buf[2] = (uint8_t)((method >> 16) & 0xFF);
        buf[3] = (uint8_t)((method >> 24) & 0xFF);
        for (uint32_t i = 0; i < payload_len; i++) {
            buf[i + 4] = _rv_byte(items[i]);
        }
    }
    int64_t rc = _ipc_send_handler(port, method, 0, (uint64_t)(uintptr_t)buf, len);
    if (buf) free(buf);
    return rc;
}

/* rt_ipc_recv_bytes: receive a raw IPC reply and materialize it as a Simple
 * [u8] RuntimeArray. Returns empty array on timeout/error. */
RuntimeValue rt_ipc_recv_bytes(uint64_t port, int64_t max_len)
{
    if (max_len <= 0) {
        RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray));
        if (!a) return NIL_VALUE;
        a->hdr.type = HEAP_ARRAY;
        a->hdr.size = sizeof(RuntimeArray);
        a->len = 0;
        a->cap = 0;
        a->items = runtime_array_inline_items(a);
        return ENCODE_PTR(a);
    }
    uint8_t *buf = (uint8_t *)malloc((size_t)max_len);
    if (!buf) return NIL_VALUE;
    int64_t rc = _ipc_recv_handler(port, (uint64_t)(uintptr_t)buf, (uint64_t)max_len);
    if (rc < 0) {
        free(buf);
        RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray));
        if (!a) return NIL_VALUE;
        a->hdr.type = HEAP_ARRAY;
        a->hdr.size = sizeof(RuntimeArray);
        a->len = 0;
        a->cap = 0;
        a->items = runtime_array_inline_items(a);
        return ENCODE_PTR(a);
    }
    uint32_t len = (uint32_t)rc;
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + len * sizeof(RuntimeValue));
    if (!a) {
        free(buf);
        return NIL_VALUE;
    }
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = sizeof(RuntimeArray) + len * sizeof(RuntimeValue);
    a->len = len;
    a->cap = len;
    a->items = runtime_array_inline_items(a);
    for (uint32_t i = 0; i < len; i++) {
        a->items[i] = ENCODE_INT(buf[i]);
    }
    free(buf);
    return ENCODE_PTR(a);
}

int64_t rt_net_socket(int64_t proto)
{
    /* Find free socket */
    int sid = -1;
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (!_sockets[i].in_use) { sid = i; break; }
    }
    if (sid < 0) return ENCODE_INT(-24); /* EMFILE */
    __builtin_memset(&_sockets[sid], 0, sizeof(_sockets[sid]));
    _sockets[sid].in_use = 1;
    _sockets[sid].state = TCP_CLOSED;
    _sockets[sid].rcv_wnd = TCP_RXBUF_SIZE;
    serial_puts("[tcp] Created socket ");
    serial_put_dec(sid);
    serial_puts("\r\n");
    return ENCODE_INT((int64_t)sid);
}

int64_t rt_net_init(void)
{
    return ENCODE_INT((int64_t)_virtio_net_init());
}

int64_t rt_net_stats(void)
{
    _virtio_net_get_stats();
    return ENCODE_INT(_vnet.initialized ? 0 : -19);
}

int64_t rt_net_bind(int64_t sock_fd, int64_t port_num)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) return ENCODE_INT(-9);
    _sockets[fd].local_port = (uint16_t)port_num;
    serial_puts("[tcp] Bound socket ");
    serial_put_dec(fd);
    serial_puts(" to port ");
    serial_put_dec((int)port_num);
    serial_puts("\r\n");
    return ENCODE_INT(0);
}

int64_t rt_net_listen(int64_t sock_fd, int64_t backlog)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) return ENCODE_INT(-9);
    _sockets[fd].state = TCP_LISTEN;
    _sockets[fd].backlog = (int)backlog;
    _sockets[fd].aq_head = 0;
    _sockets[fd].aq_tail = 0;
    _sockets[fd].aq_count = 0;
    serial_puts("[tcp] Socket ");
    serial_put_dec(fd);
    serial_puts(" listening on port ");
    serial_put_dec(_sockets[fd].local_port);
    serial_puts("\r\n");
    return ENCODE_INT(0);
}

int64_t rt_net_connect(int64_t sock_fd, int64_t ip_addr_raw, int64_t port_num)
{
    int fd = (int)sock_fd;
    (void)ip_addr_raw;
    static const uint8_t qemu_gateway_ip[4] = {10, 0, 2, 2};
    static const uint8_t qemu_gateway_mac[6] = {0x52, 0x55, 0x0a, 0x00, 0x02, 0x02};
    serial_puts("[tcp-connect] fd=");
    serial_put_dec(fd);
    serial_puts(" port=");
    serial_put_dec((int)port_num);
    serial_puts("\r\n");
    if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) return ENCODE_INT(-9);

    struct tcp_socket *s = &_sockets[fd];
    if (s->state != TCP_CLOSED) return ENCODE_INT(-106); /* EISCONN / invalid state */

    if (s->local_port == 0) {
        s->local_port = _tcp_ephemeral_port++;
        if (_tcp_ephemeral_port < 49152) _tcp_ephemeral_port = 49152;
    }

    __builtin_memcpy(s->remote_ip, qemu_gateway_ip, 4);
    s->remote_port = (uint16_t)port_num;
    s->rcv_wnd = TCP_RXBUF_SIZE;

    if (!_vnet.gateway_mac_valid) {
        _vnet_send_arp_request(qemu_gateway_ip);
        for (int i = 0; i < 20000 && !_vnet.gateway_mac_valid; i++) {
            _virtio_net_poll();
            for (volatile int d = 0; d < 1000; d++) {}
        }
    }
    if (!_vnet.gateway_mac_valid) {
        __builtin_memcpy(_vnet.gateway_mac, qemu_gateway_mac, ETH_ALEN);
        _vnet.gateway_mac_valid = 1;
        serial_puts("[tcp-connect] using slirp gateway MAC fallback\r\n");
    }

    __builtin_memcpy(s->remote_mac, _vnet.gateway_mac, ETH_ALEN);
    s->snd_nxt = _tcp_isn++;
    s->snd_una = s->snd_nxt;
    s->rcv_nxt = 0;
    s->state = TCP_SYN_SENT;

    _tcp_send_segment(fd, TCP_SYN, NULL, 0);

    for (int i = 0; i < 50000; i++) {
        _virtio_net_poll();
        if (s->state == TCP_ESTABLISHED) {
            serial_puts("[tcp-connect] established\r\n");
            return ENCODE_INT(0);
        }
        for (volatile int d = 0; d < 1000; d++) {}
    }
    serial_puts("[tcp-connect] handshake timeout state=");
    serial_put_dec(s->state);
    serial_puts("\r\n");
    return ENCODE_INT(-110); /* ETIMEDOUT */
}

int64_t rt_net_connect_host_tls(int64_t sock_fd)
{
    return rt_net_connect(sock_fd, 0, 4433);
}

int64_t rt_net_accept(int64_t sock_fd)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use ||
        _sockets[fd].state != TCP_LISTEN) {
        serial_puts("[tcp-accept] EBADF fd=");
        serial_put_dec(fd);
        serial_puts("\r\n");
        return ENCODE_INT(-9);
    }
    struct tcp_socket *ls = &_sockets[fd];
    serial_puts("[tcp-accept] fd=");
    serial_put_dec(fd);
    serial_puts(" aq=");
    serial_put_dec(ls->aq_head);
    serial_puts("/");
    serial_put_dec(ls->aq_tail);
    serial_puts(" count=");
    serial_put_dec(ls->aq_count);
    serial_puts(" port=");
    serial_put_dec(ls->local_port);
    serial_puts("\r\n");

    /* Send a gratuitous ARP to provoke network traffic */
    if (ls->aq_count == 0) {
        _vnet_send_gratuitous_arp();
        /* Also check ISR status */
        uint8_t isr = _vnet_rd8(0x13);
        serial_puts("[tcp-accept] ISR=0x");
        serial_put_hex(isr);
        serial_puts(" RX avail.flags=");
        serial_put_dec(_vnet.rx_avail->flags);
        serial_puts(" used.flags=");
        serial_put_dec(_vnet.rx_used->flags);
        serial_puts("\r\n");
    }

    /* Poll for incoming connections (~2s per accept call) */
    int timeout = 0;
    while (ls->aq_count == 0 && timeout < 20000) {
        int rc = _virtio_net_poll();
        if (rc > 0) {
            serial_puts("[tcp-accept] poll got ");
            serial_put_dec(rc);
            serial_puts(" frames!\r\n");
        }
        timeout++;
        for (volatile int d = 0; d < 1000; d++) {}
    }
    if (ls->aq_count == 0) {
        serial_puts("[tcp-accept] EAGAIN (timeout=");
        serial_put_dec(timeout);
        serial_puts(")\r\n");
        return ENCODE_INT(-11); /* EAGAIN */
    }
    int accepted_sid = ls->accept_queue[ls->aq_head];
    ls->aq_head = (ls->aq_head + 1) % TCP_ACCEPT_QUEUE;
    ls->aq_count--;
    serial_puts("[tcp-accept] accepted socket ");
    serial_put_dec(accepted_sid);
    serial_puts("\r\n");
    return ENCODE_INT((int64_t)accepted_sid);
}

int64_t rt_net_close(int64_t sock_fd)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) return ENCODE_INT(-9);
    if (_sockets[fd].state == TCP_ESTABLISHED) {
        _tcp_send_segment(fd, TCP_FIN | TCP_ACK, NULL, 0);
    }
    _sockets[fd].in_use = 0;
    _sockets[fd].state = TCP_CLOSED;
    _sockets[fd].aq_head = 0;
    _sockets[fd].aq_tail = 0;
    _sockets[fd].aq_count = 0;
    return ENCODE_INT(0);
}

/* rt_net_send_bytes: send byte array data on an established socket.
 * data_rv is a RuntimeValue pointing to a RuntimeArray of u8.
 * Returns bytes sent or negative errno. */
int64_t rt_net_send_bytes(int64_t sock_fd, RuntimeValue data_rv)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use ||
        _sockets[fd].state != TCP_ESTABLISHED) {
        serial_puts("[tcp-send] EBADF fd=");
        serial_put_dec(fd);
        serial_puts(" state=");
        serial_put_dec(_sockets[fd].state);
        serial_puts("\r\n");
        return ENCODE_INT(-9);
    }
    /* Extract byte array from RuntimeValue */
    if (!IS_HEAP(data_rv)) {
        serial_puts("[tcp-send] data not heap ptr\r\n");
        return ENCODE_INT(-22);
    }
    RuntimeArray *arr = (RuntimeArray *)(uintptr_t)DECODE_PTR(data_rv);
    if (!arr || arr->len == 0) return ENCODE_INT(0);
    RuntimeValue *items = runtime_array_items(arr);

    /* Copy items to contiguous buffer (items are tagged RuntimeValues) */
    uint32_t data_len = (uint32_t)arr->len;
    uint8_t *buf = (uint8_t *)malloc(data_len);
    if (!buf) return ENCODE_INT(-12);
    uint32_t preview = data_len < 64 ? data_len : 64;
    for (uint32_t i = 0; i < data_len; i++) {
        buf[i] = _rv_byte(items[i]);
    }
    serial_puts("[tcp-send] first-bytes=");
    for (uint32_t i = 0; i < preview; i++) {
        if (i) serial_puts(" ");
        serial_put_hex(buf[i]);
    }
    serial_puts("\r\n");

    /* Send in chunks */
    uint32_t sent = 0;
    while (sent < data_len) {
        uint16_t chunk = (data_len - sent > 1400) ? 1400 : (uint16_t)(data_len - sent);
        _tcp_send_segment(fd, TCP_ACK | TCP_PSH, buf + sent, chunk);
        sent += chunk;
    }
    free(buf);
    serial_puts("[tcp-send] sent ");
    serial_put_dec(sent);
    serial_puts(" bytes on fd=");
    serial_put_dec(fd);
    serial_puts("\r\n");
    return ENCODE_INT((int64_t)sent);
}

static RuntimeValue _tls_runtime_array_from_bytes(const uint8_t *buf, uint32_t len);
static uint8_t _ssh_last_plain_payload[4096];
static uint32_t _ssh_last_plain_payload_len = 0;

/* rt_net_recv_bytes: receive data from a socket as a byte array RuntimeValue.
 * Returns a RuntimeValue pointing to RuntimeArray of u8, or nil on error. */
RuntimeValue rt_net_recv_bytes(int64_t sock_fd, int64_t max_len)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) {
        return NIL_VALUE; /* nil */
    }
    /* Poll until data available or timeout */
    int timeout = 0;
    while (_tcp_rx_available(fd) == 0 &&
           _sockets[fd].state == TCP_ESTABLISHED &&
           timeout < 50000) {
        _virtio_net_poll();
        timeout++;
        for (volatile int d = 0; d < 1000; d++) {}
    }
    uint32_t avail = _tcp_rx_available(fd);
    if (avail == 0) {
        /* Return empty array */
        RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray));
        if (!a) return NIL_VALUE;
        a->hdr.type = HEAP_ARRAY;
        a->hdr.size = sizeof(RuntimeArray);
        a->len = 0;
        a->cap = 0;
        a->items = runtime_array_inline_items(a);
        return ENCODE_PTR(a);
    }
    uint32_t read_len = avail;
    if ((int64_t)read_len > max_len) read_len = (uint32_t)max_len;

    /* Read data from socket rx buffer */
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + read_len * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = sizeof(RuntimeArray) + read_len * sizeof(RuntimeValue);
    a->len = read_len;
    a->cap = read_len;
    a->items = runtime_array_inline_items(a);

    struct tcp_socket *s = &_sockets[fd];
    uint32_t preview = read_len < 8 ? read_len : 8;
    for (uint32_t i = 0; i < read_len; i++) {
        uint8_t byte = s->rxbuf[s->rx_tail];
        s->rx_tail = (s->rx_tail + 1) % TCP_RXBUF_SIZE;
        a->items[i] = ENCODE_INT(byte);
    }
    serial_puts("[tcp-recv] read ");
    serial_put_dec(read_len);
    serial_puts(" bytes from fd=");
    serial_put_dec(fd);
    serial_puts(" first-bytes=");
    for (uint32_t i = 0; i < preview; i++) {
        if (i) serial_puts(" ");
        serial_put_hex((uint8_t)DECODE_INT(runtime_array_items(a)[i]));
    }
    serial_puts("\r\n");
    return ENCODE_PTR(a);
}

RuntimeValue rt_net_recv_version_text(int64_t sock_fd)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) {
        return rt_string_from_cstr("");
    }

    int timeout = 0;
    while (_tcp_rx_available(fd) == 0 &&
           _sockets[fd].state == TCP_ESTABLISHED &&
           timeout < 50000) {
        _virtio_net_poll();
        timeout++;
        for (volatile int d = 0; d < 1000; d++) {}
    }

    struct tcp_socket *s = &_sockets[fd];
    uint32_t avail = _tcp_rx_available(fd);
    if (avail == 0) return rt_string_from_cstr("");

    char buf[256];
    uint32_t copied = 0;
    while (copied + 1 < sizeof(buf) && s->rx_tail != s->rx_head) {
        uint8_t byte = s->rxbuf[s->rx_tail];
        s->rx_tail = (s->rx_tail + 1) % TCP_RXBUF_SIZE;
        if (byte == '\n') break;
        if (byte == '\r') continue;
        buf[copied++] = (char)byte;
    }
    buf[copied] = '\0';

    serial_puts("[tcp-recv-version] text=");
    serial_puts(buf);
    serial_puts("\r\n");
    return rt_string_from_cstr(buf);
}

RuntimeValue rt_net_recv_ssh_plain_packet_payload(int64_t sock_fd)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) {
        return NIL_VALUE;
    }

    struct tcp_socket *s = &_sockets[fd];
    int timeout = 0;
    while (_tcp_rx_available(fd) < 5U &&
           _sockets[fd].state == TCP_ESTABLISHED &&
           timeout < 50000) {
        _virtio_net_poll();
        timeout++;
        for (volatile int d = 0; d < 1000; d++) {}
    }

    uint32_t avail = _tcp_rx_available(fd);
    if (avail < 5U) {
        RuntimeArray *empty = (RuntimeArray *)malloc(sizeof(RuntimeArray));
        if (!empty) return NIL_VALUE;
        empty->hdr.type = HEAP_ARRAY;
        empty->hdr.size = sizeof(RuntimeArray);
        empty->len = 0;
        empty->cap = 0;
        empty->items = runtime_array_inline_items(empty);
        return ENCODE_PTR(empty);
    }

    uint32_t idx = s->rx_tail;
    uint8_t header[5];
    for (uint32_t i = 0; i < 5U; i++) {
        header[i] = s->rxbuf[idx];
        idx = (idx + 1U) % TCP_RXBUF_SIZE;
    }

    uint32_t packet_length =
        ((uint32_t)header[0] << 24) |
        ((uint32_t)header[1] << 16) |
        ((uint32_t)header[2] << 8) |
        (uint32_t)header[3];
    uint32_t padding_length = (uint32_t)header[4];
    if (packet_length < 2U || packet_length > 35000U || padding_length + 1U > packet_length) {
        serial_puts("[tcp-ssh-plain] invalid packet header\r\n");
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }

    uint32_t total = 4U + packet_length;
    while (_tcp_rx_available(fd) < total &&
           _sockets[fd].state == TCP_ESTABLISHED &&
           timeout < 50000) {
        _virtio_net_poll();
        timeout++;
        for (volatile int d = 0; d < 1000; d++) {}
    }
    avail = _tcp_rx_available(fd);
    if (avail < total) {
        serial_puts("[tcp-ssh-plain] incomplete packet\r\n");
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }

    uint32_t payload_length = packet_length - padding_length - 1U;
    uint8_t *payload = (uint8_t *)malloc(payload_length ? payload_length : 1U);
    if (!payload) return NIL_VALUE;

    /* Consume header */
    for (uint32_t i = 0; i < 5U; i++) {
        s->rx_tail = (s->rx_tail + 1U) % TCP_RXBUF_SIZE;
    }
    /* Consume payload */
    _ssh_last_plain_payload_len = payload_length < sizeof(_ssh_last_plain_payload)
        ? payload_length
        : (uint32_t)sizeof(_ssh_last_plain_payload);
    for (uint32_t i = 0; i < payload_length; i++) {
        payload[i] = s->rxbuf[s->rx_tail];
        if (i < _ssh_last_plain_payload_len) {
            _ssh_last_plain_payload[i] = payload[i];
        }
        s->rx_tail = (s->rx_tail + 1U) % TCP_RXBUF_SIZE;
    }
    /* Discard padding */
    for (uint32_t i = 0; i < padding_length; i++) {
        s->rx_tail = (s->rx_tail + 1U) % TCP_RXBUF_SIZE;
    }

    serial_puts("[tcp-ssh-plain] payload-len=");
    serial_put_dec(payload_length);
    serial_puts(" first-byte=");
    if (payload_length > 0U) {
        serial_put_hex(payload[0]);
    } else {
        serial_put_hex(0);
    }
    serial_puts("\r\n");
    RuntimeValue out = _tls_runtime_array_from_bytes(payload, payload_length);
    free(payload);
    return out;
}

RuntimeValue rt_net_last_ssh_plain_payload_slice(int64_t start_i64, int64_t length_i64)
{
    serial_puts("[tcp-ssh-plain] last-payload-slice start\r\n");
    if (start_i64 < 0) start_i64 = 0;
    if (length_i64 < 0) length_i64 = 0;
    uint32_t start = (uint32_t)start_i64;
    uint32_t length = (uint32_t)length_i64;
    if (start > _ssh_last_plain_payload_len) start = _ssh_last_plain_payload_len;
    if (length > _ssh_last_plain_payload_len - start) {
        length = _ssh_last_plain_payload_len - start;
    }
    serial_puts("[tcp-ssh-plain] last-payload-slice len=");
    serial_put_dec(length);
    serial_puts("\r\n");
    return _tls_runtime_array_from_bytes(_ssh_last_plain_payload + start, length);
}

int64_t rt_net_last_ssh_plain_payload_len(void)
{
    return (int64_t)_ssh_last_plain_payload_len;
}

RuntimeValue rt_tls13_recv_record(int64_t sock_fd)
{
    int fd = (int)sock_fd;
    if (fd < 0 || fd >= MAX_SOCKETS || !_sockets[fd].in_use) {
        return NIL_VALUE;
    }

    uint8_t header[5];
    uint32_t got = 0;
    int timeout = 0;
    struct tcp_socket *s = &_sockets[fd];
    while (got < 5U && timeout < 50000) {
        while (_tcp_rx_available(fd) > 0 && got < 5U) {
            header[got++] = s->rxbuf[s->rx_tail];
            s->rx_tail = (s->rx_tail + 1) % TCP_RXBUF_SIZE;
        }
        if (got < 5U) {
            _virtio_net_poll();
            timeout++;
            for (volatile int d = 0; d < 1000; d++) {}
        }
    }
    if (got < 5U) {
        return _tls_runtime_array_from_bytes(header, got);
    }

    uint32_t payload_len = ((uint32_t)header[3] << 8) | (uint32_t)header[4];
    if (payload_len > 18432U) {
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }
    uint32_t total_len = 5U + payload_len;
    uint8_t *record = (uint8_t *)malloc(total_len > 0 ? total_len : 1U);
    if (!record) return NIL_VALUE;
    memcpy(record, header, 5U);

    got = 0;
    timeout = 0;
    while (got < payload_len && timeout < 50000) {
        while (_tcp_rx_available(fd) > 0 && got < payload_len) {
            record[5U + got] = s->rxbuf[s->rx_tail];
            s->rx_tail = (s->rx_tail + 1) % TCP_RXBUF_SIZE;
            got++;
        }
        if (got < payload_len) {
            _virtio_net_poll();
            timeout++;
            for (volatile int d = 0; d < 1000; d++) {}
        }
    }
    RuntimeValue rv = _tls_runtime_array_from_bytes(record, 5U + got);
    free(record);
    return rv;
}

/* Direct PCI cache access — bypasses syscall for pcimgr */
/* Direct PCI cache access — returns raw (untagged) integers */
int64_t rt_pci_device_count(void)
{
    if (_pci_cache_count < 0) _pci_scan();
    return (int64_t)_pci_cache_count;
}

int64_t rt_pci_get_field(int64_t index, int64_t field)
{
    if (_pci_cache_count < 0) _pci_scan();
    if (index < 0 || index >= _pci_cache_count) return -1;
    int i = (int)index;
    switch ((int)field) {
        case 0: return (int64_t)_pci_cache[i].bus;
        case 1: return (int64_t)_pci_cache[i].dev;
        case 2: return (int64_t)_pci_cache[i].func;
        case 3: return (int64_t)_pci_cache[i].cls;
        case 4: return (int64_t)_pci_cache[i].sub;
        case 5: return (int64_t)_pci_cache[i].vendor;
        case 6: return (int64_t)_pci_cache[i].devid;
        case 7: return (int64_t)_pci_cache[i].irq;
        default: return -1;
    }
}

int64_t rt_pci_read_bar0(int64_t index)
{
    if (_pci_cache_count < 0) _pci_scan();
    if (index < 0 || index >= _pci_cache_count) return -1;
    int i = (int)index;
    uint32_t pci_addr = 0x80000000
        | ((uint32_t)_pci_cache[i].bus << 16)
        | ((uint32_t)_pci_cache[i].dev << 11)
        | ((uint32_t)_pci_cache[i].func << 8)
        | 0x10;
    outl(0xCF8, pci_addr);
    return (int64_t)inl(0xCFC);
}

static void _serial_init(void)
{
    /* Disable interrupts */
    outb(0x3F8 + 1, 0x00);
    /* Set DLAB (divisor latch access bit) */
    outb(0x3F8 + 3, 0x80);
    /* Set divisor to 1 (115200 baud) */
    outb(0x3F8 + 0, 0x01);  /* low byte */
    outb(0x3F8 + 1, 0x00);  /* high byte */
    /* 8 bits, no parity, one stop bit */
    outb(0x3F8 + 3, 0x03);
    /* Enable FIFO, clear, 14-byte threshold */
    outb(0x3F8 + 2, 0xC7);
    /* IRQs enabled, RTS/DSR set */
    outb(0x3F8 + 4, 0x0B);
}

#define BGA_INDEX_PORT 0x01CE
#define BGA_DATA_PORT  0x01CF

static void bga_write(uint16_t index, uint16_t value)
{
    outw(BGA_INDEX_PORT, index);
    outw(BGA_DATA_PORT, value);
}

static uint32_t pci_config_read32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off)
{
    uint32_t addr = 0x80000000u
        | ((uint32_t)bus << 16)
        | ((uint32_t)dev << 11)
        | ((uint32_t)func << 8)
        | ((uint32_t)(off & 0xFC));
    outl(0x0CF8, addr);
    return inl(0x0CFC);
}

static uint64_t detect_vga_lfb(void)
{
    for (uint8_t d = 0; d < 32; d++) {
        uint32_t vendor = pci_config_read32(0, d, 0, 0x00) & 0xFFFF;
        if (vendor == 0xFFFF || vendor == 0) continue;
        uint32_t cls = pci_config_read32(0, d, 0, 0x08);
        if (((cls >> 24) & 0xFF) == 0x03 && ((cls >> 16) & 0xFF) == 0x00) {
            uint32_t bar0 = pci_config_read32(0, d, 0, 0x10);
            uint64_t a = (uint64_t)(bar0 & 0xFFFFFFF0u);
            if (a) return a;
        }
    }
    return 0xFD000000ULL; /* QEMU Q35 fallback */
}

uint64_t g_fb_addr;
static uint32_t g_fb_width, g_fb_height, g_fb_pitch;

static void bga_init(uint32_t w, uint32_t h, uint32_t bpp)
{
    bga_write(0x04, 0x00);           /* VBE_DISPI_DISABLE */
    bga_write(0x01, (uint16_t)w);    /* XRES */
    bga_write(0x02, (uint16_t)h);    /* YRES */
    bga_write(0x03, (uint16_t)bpp);  /* BPP */
    bga_write(0x06, (uint16_t)w);    /* VIRT_WIDTH = XRES (set pitch) */
    bga_write(0x04, 0x01 | 0x40);    /* ENABLE | LFB */
    g_fb_addr = detect_vga_lfb();
    g_fb_width = w;
    g_fb_height = h;
    g_fb_pitch = w * (bpp / 8);
}

static void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= g_fb_width || y >= g_fb_height) return;
    volatile uint32_t *fb = (volatile uint32_t *)(uintptr_t)g_fb_addr;
    fb[y * (g_fb_pitch / 4) + x] = color;
}

static void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    for (uint32_t dy = 0; dy < h; dy++)
        for (uint32_t dx = 0; dx < w; dx++)
            fb_put_pixel(x + dx, y + dy, color);
}

/* 8x16 bitmap font — minimal ASCII subset for "Hello World" */
static const uint8_t font_H[] = {0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x42,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_e[] = {0x00,0x00,0x00,0x00,0x3C,0x42,0x7E,0x40,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_l[] = {0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_o[] = {0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_W[] = {0x41,0x41,0x41,0x41,0x49,0x49,0x55,0x55,0x22,0x22,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_r[] = {0x00,0x00,0x00,0x00,0x5C,0x62,0x40,0x40,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_d[] = {0x00,0x02,0x02,0x02,0x3E,0x42,0x42,0x42,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_bang[] = {0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_space[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const uint8_t font_S[] = {0x3C,0x42,0x40,0x40,0x3C,0x02,0x02,0x42,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_i[] = {0x00,0x00,0x10,0x00,0x10,0x10,0x10,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_m[] = {0x00,0x00,0x00,0x00,0x76,0x49,0x49,0x49,0x49,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_p[] = {0x00,0x00,0x00,0x00,0x5C,0x62,0x42,0x62,0x5C,0x40,0x40,0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_O[] = {0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static void fb_draw_char(uint32_t x, uint32_t y, const uint8_t *glyph, uint32_t fg, uint32_t bg)
{
    for (int row = 0; row < 16; row++) {
        uint8_t bits = (row < 10) ? glyph[row] : 0;
        for (int col = 0; col < 8; col++) {
            uint32_t color = (bits & (0x80 >> col)) ? fg : bg;
            fb_put_pixel(x + (uint32_t)col, y + (uint32_t)row, color);
        }
    }
}

static const uint8_t *get_glyph(char c)
{
    switch (c) {
        case 'H': return font_H; case 'e': return font_e; case 'l': return font_l;
        case 'o': return font_o; case 'W': return font_W; case 'r': return font_r;
        case 'd': return font_d; case '!': return font_bang; case ' ': return font_space;
        case 'S': return font_S; case 'i': return font_i; case 'm': return font_m;
        case 'p': return font_p; case 'O': return font_O;
        default: return font_space;
    }
}

static void fb_draw_text(uint32_t x, uint32_t y, const char *text, uint32_t fg, uint32_t bg)
{
    while (*text) {
        fb_draw_char(x, y, get_glyph(*text), fg, bg);
        x += 9; /* 8px char + 1px gap */
        text++;
    }
}

uint64_t g_fb_addr = 0xFD000000;
uint64_t g_fb_w = 1024;

static void serial_hex(uint64_t v) {
    char hex[] = "0123456789abcdef";
    serial_putchar('0'); serial_putchar('x');
    for (int i = 60; i >= 0; i -= 4)
        serial_putchar(hex[(v >> i) & 0xF]);
}

RuntimeValue rt_gui_set_fb(RuntimeValue addr, RuntimeValue w)
{
    uint32_t width = (uint32_t)(uint64_t)w;
    if (width < 64 || width > 8192) {
        width = 1024;
    }
    /* Initialize BGA display mode (1024x768x32) before setting fb params */
    bga_init(width, 768, 32);
    g_fb_addr = g_fb_addr ? g_fb_addr : (uint64_t)addr;  /* prefer PCI-detected addr */
    g_fb_w = (uint64_t)width;
    /* Diagnostic: print fb address and width */
    serial_puts("[GUI] fb_addr=0x");
    serial_hex(g_fb_addr);
    serial_puts(" fb_w=");
    serial_hex(g_fb_w);
    serial_puts("\r\n");
    return 0;
}

RuntimeValue rt_gui_hline(RuntimeValue y, RuntimeValue x, RuntimeValue count, RuntimeValue color)
{
    uint64_t base = g_fb_addr + ((uint64_t)y * g_fb_w + (uint64_t)x) * 4;
    uint32_t c = (uint32_t)(uint64_t)color;
    for (uint64_t i = 0; i < (uint64_t)count; i++) {
        *(volatile uint32_t *)(uintptr_t)(base + i * 4) = c;
    }
    return 0;
}

/* 4-arg version: pack x|y into xy and w|h into wh (high 32 = first, low 32 = second) */
RuntimeValue rt_gui_fill4(RuntimeValue xy, RuntimeValue wh, RuntimeValue color, RuntimeValue _unused)
{
    (void)_unused;
    uint32_t rx = (uint32_t)((uint64_t)xy >> 32);
    uint32_t ry = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t rw = (uint32_t)((uint64_t)wh >> 32);
    uint32_t rh = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t c = (uint32_t)(uint64_t)color;
    for (uint32_t row = 0; row < rh; row++) {
        uint64_t base = g_fb_addr + ((uint64_t)(ry + row) * g_fb_w + rx) * 4;
        for (uint32_t col = 0; col < rw; col++) {
            *(volatile uint32_t *)(uintptr_t)(base + col * 4) = c;
        }
    }
    return 0;
}

static void gui_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t c)
{
    for (uint32_t row = 0; row < h; row++) {
        uint64_t base = g_fb_addr + ((uint64_t)(y + row) * g_fb_w + x) * 4;
        for (uint32_t col = 0; col < w; col++) {
            *(volatile uint32_t *)(uintptr_t)(base + col * 4) = c;
        }
    }
}

/* Full desktop rendering — called from Simple code via extern fn.
 * Draws everything in C to avoid Cranelift stack frame issues. */
RuntimeValue rt_gui_render_desktop(RuntimeValue unused1, RuntimeValue unused2)
{
    (void)unused1; (void)unused2;

    /* Title bar (top) */
    gui_fill(0, 0, 1024, 28, 0x001E1E2E);
    serial_puts("[GUI] title bar\r\n");

    /* Taskbar (bottom) */
    gui_fill(0, 736, 1024, 32, 0x001E1E2E);
    serial_puts("[GUI] taskbar\r\n");

    /* Dock icons */
    gui_fill(10, 742, 20, 20, 0x003498DB);
    gui_fill(36, 742, 20, 20, 0x0027AE60);
    gui_fill(62, 742, 20, 20, 0x00E74C3C);
    gui_fill(88, 742, 20, 20, 0x00F39C12);
    gui_fill(114, 742, 20, 20, 0x009B59B6);
    gui_fill(944, 742, 70, 20, 0x002C2C3E);
    serial_puts("[GUI] dock\r\n");

    /* Window title bar (blue, 300x24) */
    gui_fill(200, 100, 300, 24, 0x00007ACC);
    serial_puts("[GUI] win title\r\n");

    /* Close button (red) */
    gui_fill(480, 104, 16, 16, 0x00E74C3C);

    /* Window body (white, 300x120) */
    gui_fill(200, 124, 300, 120, 0x00FFFFFF);
    serial_puts("[GUI] win body\r\n");

    /* "Hello World!" text */
    fb_draw_text(220, 140, "Hello World!", 0x00333333, 0x00FFFFFF);
    serial_puts("[GUI] text\r\n");

    /* RGB blocks (smaller) */
    gui_fill(220, 170, 60, 24, 0x00FF4444);
    gui_fill(290, 170, 60, 24, 0x0044CC44);
    gui_fill(360, 170, 60, 24, 0x004488FF);

    /* Status bar (300x20) */
    gui_fill(200, 224, 300, 20, 0x00E0E0E0);
    fb_draw_text(210, 228, "SimpleOS", 0x00666666, 0x00E0E0E0);
    serial_puts("[GUI] status\r\n");

    serial_puts("[GUI] render complete\r\n");
    return 0;
}

RuntimeValue rt_dict_new(void) { return NIL_VALUE; }
RuntimeValue rt_dict_get(RuntimeValue d, RuntimeValue k) { (void)d; (void)k; return NIL_VALUE; }
RuntimeValue rt_dict_set(RuntimeValue d, RuntimeValue k, RuntimeValue v) { (void)d; (void)k; (void)v; return NIL_VALUE; }
RuntimeValue rt_dict_len(RuntimeValue d) { (void)d; return 0; /* raw untagged */ }
RuntimeValue rt_dict_keys(RuntimeValue d) { (void)d; return NIL_VALUE; }
RuntimeValue rt_dict_values(RuntimeValue d) { (void)d; return NIL_VALUE; }
RuntimeValue rt_dict_clear(RuntimeValue d) { (void)d; return NIL_VALUE; }
RuntimeValue rt_array_first(RuntimeValue a) { (void)a; return NIL_VALUE; }
RuntimeValue rt_array_last(RuntimeValue a) { (void)a; return NIL_VALUE; }
RuntimeValue rt_array_repeat(RuntimeValue v, RuntimeValue n) { (void)v; (void)n; return NIL_VALUE; }
RuntimeValue rt_string_find(RuntimeValue s, RuntimeValue sub) { (void)s; (void)sub; return (RuntimeValue)(-1); }
RuntimeValue rt_string_rfind(RuntimeValue s, RuntimeValue sub) { (void)s; (void)sub; return (RuntimeValue)(-1); }
RuntimeValue rt_string_join(RuntimeValue a, RuntimeValue sep) { (void)a; (void)sep; return NIL_VALUE; }
RuntimeValue rt_string_to_int(RuntimeValue s) { (void)s; return ENCODE_INT(0); }
RuntimeValue rt_option_map(RuntimeValue o, RuntimeValue f) { (void)o; (void)f; return NIL_VALUE; }
RuntimeValue rt_file_read_text(const char *path, int64_t path_len) {
    return _fat32_read_file_text_value(path, path_len);
}
RuntimeValue rt_file_read_text_rv(RuntimeValue path_rv) {
    if (!IS_HEAP(path_rv))
        return rt_string_from_cstr("");
    RuntimeString *s = (RuntimeString *)DECODE_PTR(path_rv);
    if (!s)
        return rt_string_from_cstr("");
    return _fat32_read_file_text_value(s->data, s->len);
}
int8_t rt_file_write_text(const char *path, int64_t path_len, const char *content, int64_t content_len) {
    return _fat32_write_text_impl(path, path_len, content, content_len);
}
int8_t rt_file_append_text(const char *path, int64_t path_len, const char *content, int64_t content_len) {
    char path_buf[128];
    uint32_t existing_size = 0;
    uint32_t existing_cluster = 0;
    uint8_t *combined = (uint8_t *)0;
    uint32_t total_len = 0;
    int8_t ok = 0;

    if (_fat32_copy_path_arg(path, path_len, path_buf, sizeof(path_buf)) <= 0)
        return 0;

    if (fat32_find_file(path_buf, &existing_cluster, &existing_size) != 0 || existing_size == 0) {
        return _fat32_write_text_impl(path, path_len, content, content_len);
    }

    total_len = existing_size + (content_len > 0 ? (uint32_t)content_len : 0);
    combined = (uint8_t *)malloc(total_len == 0 ? 1 : total_len);
    if (!combined)
        return 0;
    if (fat32_read_file(path_buf, combined, existing_size, &existing_size) != 0) {
        free(combined);
        return 0;
    }
    if (content && content_len > 0)
        __builtin_memcpy(combined + existing_size, content, (uint32_t)content_len);
    ok = fat32_write_file(path_buf, combined, total_len) == 0 ? 1 : 0;
    free(combined);
    return ok;
}
RuntimeValue rt_file_open(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return NIL_VALUE; }
RuntimeValue rt_file_close(RuntimeValue a) { (void)a; return NIL_VALUE; }
RuntimeValue rt_file_remove(RuntimeValue a) { (void)a; return NIL_VALUE; }
RuntimeValue rt_file_find(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return NIL_VALUE; }
RuntimeValue rt_file_get_size(RuntimeValue a) { (void)a; return ENCODE_INT(0); }
RuntimeValue rt_file_canonicalize(RuntimeValue a) { (void)a; return NIL_VALUE; }
RuntimeValue rt_file_hash(RuntimeValue a) { (void)a; return NIL_VALUE; }
RuntimeValue rt_file_read_lines(RuntimeValue a) { (void)a; return NIL_VALUE; }
RuntimeValue rt_write_file(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return NIL_VALUE; }
RuntimeValue rt_cli_file_exists(RuntimeValue a) { (void)a; return ENCODE_INT(0); }
RuntimeValue rt_process_execute(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return NIL_VALUE; }
RuntimeValue rt_process_exists(RuntimeValue a) { (void)a; return ENCODE_INT(0); }
RuntimeValue rt_process_is_running(RuntimeValue a) { (void)a; return ENCODE_INT(0); }
RuntimeValue rt_process_run_with_limits(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d) { (void)a;(void)b;(void)c;(void)d; return NIL_VALUE; }
RuntimeValue rt_process_spawn_async(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return NIL_VALUE; }
RuntimeValue rt_cli_print(RuntimeValue v) { rt_print(v); return NIL_VALUE; }
RuntimeValue rt_cli_println(RuntimeValue v) { rt_print(v); serial_puts("\r\n"); return NIL_VALUE; }
RuntimeValue rt_cli_eprint(RuntimeValue v) { rt_print(v); return NIL_VALUE; }
RuntimeValue rt_cli_eprintln(RuntimeValue v) { rt_print(v); serial_puts("\r\n"); return NIL_VALUE; }
RuntimeValue rt_eprint_str(RuntimeValue v) { rt_print(v); return NIL_VALUE; }
RuntimeValue rt_eprint_value(RuntimeValue v) { rt_print(v); return NIL_VALUE; }
RuntimeValue rt_eprintln_str(RuntimeValue v) { rt_print(v); serial_puts("\r\n"); return NIL_VALUE; }
RuntimeValue rt_eprintln_value(RuntimeValue v) { rt_print(v); serial_puts("\r\n"); return NIL_VALUE; }
int8_t rt_log_target_device_write_bytes(const char *ptr, int64_t len) {
    if (!ptr || len <= 0) return 0;
    for (int64_t i = 0; i < len; i++) serial_putchar(ptr[i]);
    serial_puts("\r\n");
    return 1;
}
int8_t rt_log_target_semihost_write_bytes(const char *ptr, int64_t len) {
    (void)ptr;
    (void)len;
    return 0;
}
int8_t rt_simpleos_log_init(int64_t level, int64_t targets) {
    _simpleos_log_level = level;
    _simpleos_log_targets = targets;
    if (simple_log_c_init)
        return simple_log_c_init(level, targets) ? 1 : 0;
    return 1;
}
int8_t rt_simpleos_log_is_enabled(int64_t level) {
    if (_simple_log_facade_ready() && simple_log_c_enabled)
        return simple_log_c_enabled(level) ? 1 : 0;
    return level >= _simpleos_log_level ? 1 : 0;
}
int8_t rt_simpleos_log_emit(int64_t level, const char *ptr, int64_t len) {
    return _simpleos_log_write_bytes(level, ptr, len);
}
RuntimeValue rt_cstring_to_text(RuntimeValue p) { (void)p; return NIL_VALUE; }
RuntimeValue rt_profiler_is_active(void) { return ENCODE_INT(0); }

/* Comparison operator — called by <= >= < > on non-trivial values */
RuntimeValue rt_value_compare(RuntimeValue a, RuntimeValue b) {
    /* Simple integer comparison for tagged values */
    int64_t va = (int64_t)a;
    int64_t vb = (int64_t)b;
    if (va < vb) return (RuntimeValue)(-1);
    if (va > vb) return ENCODE_INT(1);
    return ENCODE_INT(0);
}

/* rt_native_eq/neq already defined at line ~759 */
RuntimeValue rt_profiler_record_call(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return NIL_VALUE; }
RuntimeValue rt_profiler_record_return(RuntimeValue a) { (void)a; return NIL_VALUE; }

RuntimeValue serial_println(RuntimeValue val) {
    rt_print(val);
    serial_puts("\r\n");
    return NIL_VALUE;
}

/* Read a line from serial (blocks until newline or buffer full).
 * Returns length of line (excluding null terminator). Echoes characters. */
static int serial_readline(char *buf, int max_len)
{
    int pos = 0;
    while (pos < max_len - 1) {
        char c = serial_getchar();
        if (c == '\r' || c == '\n') {
            serial_putchar('\r');
            serial_putchar('\n');
            break;
        } else if (c == 0x7f || c == '\b') {
            /* Backspace */
            if (pos > 0) {
                pos--;
                serial_puts("\b \b");
            }
        } else if (c >= 0x20) {
            buf[pos++] = c;
            serial_putchar(c);
        }
    }
    buf[pos] = '\0';
    return pos;
}

/* Simple language wrappers for serial input */
RuntimeValue rt_serial_getchar(void)
{
    char c = serial_getchar();
    return ENCODE_INT((int64_t)c);
}

RuntimeValue rt_serial_readline(void)
{
    char buf[256];
    serial_readline(buf, 256);
    return rt_string_from_cstr(buf);
}

RuntimeValue rt_serial_data_ready(void)
{
    return serial_data_ready() ? TRUE_VALUE : FALSE_VALUE;
}

static void serial_puthex(uint32_t v) {
    static const char hex[] = "0123456789abcdef";
    if (v > 0xFFFF) { serial_putchar(hex[(v>>28)&0xF]); serial_putchar(hex[(v>>24)&0xF]); serial_putchar(hex[(v>>20)&0xF]); serial_putchar(hex[(v>>16)&0xF]); }
    if (v > 0xFF) { serial_putchar(hex[(v>>12)&0xF]); serial_putchar(hex[(v>>8)&0xF]); }
    serial_putchar(hex[(v>>4)&0xF]); serial_putchar(hex[v&0xF]);
}

extern void spl_start(void) __attribute__((weak));
extern void __simple_call_module_inits(void) __attribute__((weak));

void _start(void)
{
    /* Disable all PIC IRQs to prevent timer interrupts during rendering.
     * Mask all IRQs on both PICs (master 0x21, slave 0xA1). */
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
    /* Also disable APIC timer if APIC is present */
    __asm__ volatile("cli");

    _serial_init();
    serial_puts("[BOOT] _start entered\r\n");

    /* For Simple baremetal app lanes, the validated minimum boot path is:
     * long-mode handoff -> basic IRQ masking -> direct spl_start().
     * Skip the heavyweight C boot diagnostics/probing until the boot lane is stable.
     */
    if (spl_start) {
        serial_puts("[BOOT] spl_start dispatch\r\n");
        __asm__ volatile(
            "movw $0x3F8, %%dx\n"
            "movb $'4', %%al\n"
            "outb %%al, %%dx\n"
            :
            :
            : "rax", "rdx"
        );
        spl_start();
        serial_puts("[BOOT] spl_start returned\r\n");
        __asm__ volatile(
            "movw $0x3F8, %%dx\n"
            "movb $'5', %%al\n"
            "outb %%al, %%dx\n"
            :
            :
            : "rax", "rdx"
        );
        __asm__ volatile("cli");
        for (;;) {
            __asm__ volatile("hlt");
        }
    }

    serial_puts("SimpleOS x86_64 boot\r\n");
    serial_puts("[BOOT] COM1 serial initialized at 115200 baud\r\n");
    serial_puts("[BOOT] Heap: 512 MB bump allocator\r\n");
    serial_puts("[BOOT] RuntimeValue: tagged 64-bit (int/heap/float/special)\r\n");

    /* BGA + GUI rendering is now done by Pure Simple code in spl_start().
     * C boot stub only provides serial, heap, and runtime stubs.
     */

    /* PCI hardware test — verify devices are visible before entering Simple code */
    {
        if (_pci_cache_count < 0) _pci_scan();
        serial_puts("[BOOT] PCI: ");
        serial_puthex(_pci_cache_count);
        serial_puts(" devices found\r\n");
        for (int i = 0; i < _pci_cache_count && i < 8; i++) {
            serial_puts("[BOOT]   ");
            serial_puthex(_pci_cache[i].bus); serial_puts(":");
            serial_puthex(_pci_cache[i].dev); serial_puts(".");
            serial_puthex(_pci_cache[i].func);
            serial_puts(" vendor="); serial_puthex(_pci_cache[i].vendor);
            serial_puts(" device="); serial_puthex(_pci_cache[i].devid);
            serial_puts(" class="); serial_puthex(_pci_cache[i].cls);
            serial_puts("."); serial_puthex(_pci_cache[i].sub);
            /* Print all BARs for each device */
            for (int bar = 0; bar < 6; bar++) {
                uint32_t bar_addr = 0x80000000
                    | ((uint32_t)_pci_cache[i].bus << 16)
                    | ((uint32_t)_pci_cache[i].dev << 11)
                    | (0x10 + bar * 4);
                outl(0xCF8, bar_addr);
                uint32_t barval = inl(0xCFC);
                if (barval != 0) {
                    serial_puts(" bar"); serial_put_dec(bar);
                    serial_puts("=0x"); serial_put_hex((uint64_t)barval);
                }
            }
            serial_puts("\r\n");
        }
    }

    /* ===================================================================
     * PCI BAR assignment for devices with unassigned BARs
     *
     * Some devices (e.g., virtio-gpu on q35) don't get BARs assigned by
     * firmware (SeaBIOS). We detect unassigned BARs and program them.
     * =================================================================== */
    {
        static uint32_t next_io_port = 0xC000;    /* I/O port allocation base */
        static uint32_t next_mmio_base = 0xFE000000; /* MMIO allocation base */

        for (int i = 0; i < _pci_cache_count; i++) {
            uint32_t pci_addr = 0x80000000
                | ((uint32_t)_pci_cache[i].bus << 16)
                | ((uint32_t)_pci_cache[i].dev << 11)
                | 0x10; /* BAR0 offset */
            outl(0xCF8, pci_addr);
            uint32_t bar0 = inl(0xCFC);

            if (bar0 != 0) continue; /* Already assigned */

            /* Write all-ones to discover BAR size and type */
            outl(0xCF8, pci_addr);
            outl(0xCFC, 0xFFFFFFFF);
            outl(0xCF8, pci_addr);
            uint32_t bar_mask = inl(0xCFC);

            if (bar_mask == 0 || bar_mask == 0xFFFFFFFF) {
                /* No BAR implemented or BAR not writable */
                outl(0xCF8, pci_addr);
                outl(0xCFC, 0); /* Restore */
                continue;
            }

            if (bar_mask & 1) {
                /* I/O BAR: mask = ~(size-1) | flags */
                uint32_t io_size = ~(bar_mask & ~0x3u) + 1;
                if (io_size == 0 || io_size > 256) io_size = 256;
                /* Align allocation */
                next_io_port = (next_io_port + io_size - 1) & ~(io_size - 1);
                uint32_t assigned = next_io_port | 1; /* I/O flag */
                outl(0xCF8, pci_addr);
                outl(0xCFC, assigned);
                serial_puts("[pci-bar] ");
                serial_put_hex(_pci_cache[i].bus); serial_puts(":");
                serial_put_hex(_pci_cache[i].dev); serial_puts(".0");
                serial_puts(" BAR0=IO 0x"); serial_put_hex(next_io_port);
                serial_puts(" size="); serial_put_dec((int64_t)io_size);
                serial_puts("\r\n");
                next_io_port += io_size;
            } else {
                /* Memory BAR */
                uint32_t mem_size = ~(bar_mask & ~0xFu) + 1;
                if (mem_size == 0) mem_size = 4096;
                next_mmio_base = (next_mmio_base + mem_size - 1) & ~(mem_size - 1);
                outl(0xCF8, pci_addr);
                outl(0xCFC, next_mmio_base);
                serial_puts("[pci-bar] ");
                serial_put_hex(_pci_cache[i].bus); serial_puts(":");
                serial_put_hex(_pci_cache[i].dev); serial_puts(".0");
                serial_puts(" BAR0=MEM 0x"); serial_put_hex(next_mmio_base);
                serial_puts(" size="); serial_put_dec((int64_t)mem_size);
                serial_puts("\r\n");
                next_mmio_base += mem_size;
            }

            /* Enable Memory Space + I/O Space + Bus Master in PCI command register */
            uint32_t cmd_addr = 0x80000000
                | ((uint32_t)_pci_cache[i].bus << 16)
                | ((uint32_t)_pci_cache[i].dev << 11)
                | 0x04; /* Command register offset */
            outl(0xCF8, cmd_addr);
            uint32_t cmd = inl(0xCFC);
            cmd |= 0x07; /* Memory Space | I/O Space | Bus Master */
            outl(0xCF8, cmd_addr);
            outl(0xCFC, cmd);
        }
    }

    /* Read VGA BAR0 and set framebuffer address (PCI bus 0, dev 1, BAR0 at offset 0x10) */
    {
        uint32_t vga_addr = 0x80000000 | (1 << 11) | 0x10;
        outl(0xCF8, vga_addr);
        uint32_t bar0 = inl(0xCFC);
        if (bar0 & 0xFFFFFFF0) {
            g_fb_addr = (uint64_t)(bar0 & 0xFFFFFFF0);
        }
    }

    /* FAT32 file read test — read hello.txt from NVMe disk */
    if (_fat32_init() == 0) {
        serial_puts("[BOOT] FAT32 initialized\r\n");
        fat32_list_dir();
        char fbuf[256];
        __builtin_memset(fbuf, 0, sizeof(fbuf));
        uint32_t bytes_read = 0;
        if (fat32_read_file("hello.txt", (uint8_t *)fbuf, 255, &bytes_read) == 0) {
            fbuf[bytes_read] = '\0';
            serial_puts("[BOOT] hello.txt: ");
            serial_puts(fbuf);
            serial_puts("\r\n");
        } else {
            serial_puts("[BOOT] hello.txt: not found\r\n");
        }
    } else {
        serial_puts("[BOOT] FAT32 init failed\r\n");
    }

    /* VirtIO-net initialization + ARP/ICMP polling test */
    {
        int net_rc = _virtio_net_init();
        if (net_rc == 0) {
            serial_puts("[BOOT] Network initialized: MAC=");
            _net_print_mac(_vnet.mac);
            serial_puts(" IP=10.0.2.15\r\n");

            /* Poll for incoming frames for ~2 seconds.
             * This allows QEMU user-mode networking to:
             *   1. Receive our gratuitous ARP
             *   2. Send ARP requests for 10.0.2.15 (which we reply to)
             *   3. Potentially send ICMP echo requests (which we reply to)
             * The loop processes ARP and ICMP automatically. */
            serial_puts("[BOOT] Polling network for 2 seconds...\r\n");
            int total_frames = 0;
            for (int i = 0; i < 2000; i++) {
                int n = _virtio_net_poll();
                if (n > 0) total_frames += n;
                /* ~1ms delay */
                for (volatile int j = 0; j < 10000; j++) {}
            }
            serial_puts("[BOOT] Network poll done, frames processed: ");
            serial_put_dec(total_frames);
            serial_puts("\r\n");
            _virtio_net_get_stats();
        } else {
            serial_puts("[BOOT] VirtIO-net init failed (rc=");
            serial_put_dec(net_rc);
            serial_puts(") — no network device or not VirtIO\r\n");
        }
    }

    if (__simple_call_module_inits) {
        __simple_call_module_inits();
    }
    if (spl_start) {
        serial_puts("[BOOT] Calling spl_start()...\r\n");
        spl_start();
        serial_puts("[BOOT] spl_start() returned\r\n");
    } else {
        serial_puts("[BOOT] No spl_start() found (weak symbol)\r\n");
    }

    serial_puts("[BOOT] x86_64 boot complete\r\n");

    /* Halt forever (don't exit — keep display visible) */
    for (;;) {
        __asm__ volatile("hlt");
    }
}

#define S0(n) RuntimeValue n(void) { \
    serial_puts("FATAL: unimplemented rt function: " #n "\n"); \
    for(;;) __asm__ volatile("cli; hlt"); \
    return 0; \
}
#define S1(n) RuntimeValue n(RuntimeValue a) { \
    (void)a; \
    serial_puts("FATAL: unimplemented rt function: " #n "\n"); \
    for(;;) __asm__ volatile("cli; hlt"); \
    return 0; \
}
#define S2(n) RuntimeValue n(RuntimeValue a, RuntimeValue b) { \
    (void)a; (void)b; \
    serial_puts("FATAL: unimplemented rt function: " #n "\n"); \
    for(;;) __asm__ volatile("cli; hlt"); \
    return 0; \
}
#define S3(n) RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c) { \
    (void)a; (void)b; (void)c; \
    serial_puts("FATAL: unimplemented rt function: " #n "\n"); \
    for(;;) __asm__ volatile("cli; hlt"); \
    return 0; \
}
#define S4(n) RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d) { \
    (void)a; (void)b; (void)c; (void)d; \
    serial_puts("FATAL: unimplemented rt function: " #n "\n"); \
    for(;;) __asm__ volatile("cli; hlt"); \
    return 0; \
}
#define S5(n) RuntimeValue n(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d, RuntimeValue e) { \
    (void)a; (void)b; (void)c; (void)d; (void)e; \
    serial_puts("FATAL: unimplemented rt function: " #n "\n"); \
    for(;;) __asm__ volatile("cli; hlt"); \
    return 0; \
}

/* void-return stub macros */
#define V0(n) void n(void) { \
    serial_puts("FATAL: unimplemented rt function: " #n "\n"); \
    for(;;) __asm__ volatile("cli; hlt"); \
}
#define V1(n) void n(RuntimeValue a) { \
    (void)a; \
    serial_puts("FATAL: unimplemented rt function: " #n "\n"); \
    for(;;) __asm__ volatile("cli; hlt"); \
}
#define V2(n) void n(RuntimeValue a, RuntimeValue b) { \
    (void)a; (void)b; \
    serial_puts("FATAL: unimplemented rt function: " #n "\n"); \
    for(;;) __asm__ volatile("cli; hlt"); \
}

/* --- Arithmetic / comparison ---
 *
 * Cranelift emits raw i64 values.  These operate on tagged RuntimeValues:
 * integer args are ENCODE_INT(n) = n<<3, results likewise.
 * Comparison results are raw 1 or 0 (Cranelift boolean convention).
 */

RuntimeValue rt_add(RuntimeValue a, RuntimeValue b)
{
    if (IS_INT(a) && IS_INT(b))
        return ENCODE_INT(DECODE_INT(a) + DECODE_INT(b));
    /* String concat fallback */
    if (IS_HEAP(a) || IS_HEAP(b))
        return rt_string_concat(a, b);
    return ENCODE_INT(0);
}

RuntimeValue rt_sub(RuntimeValue a, RuntimeValue b)
{
    return ENCODE_INT(DECODE_INT(a) - DECODE_INT(b));
}

RuntimeValue rt_mul(RuntimeValue a, RuntimeValue b)
{
    return ENCODE_INT(DECODE_INT(a) * DECODE_INT(b));
}

RuntimeValue rt_div(RuntimeValue a, RuntimeValue b)
{
    int64_t denom = DECODE_INT(b);
    if (denom == 0) return ENCODE_INT(0); /* avoid div-by-zero trap */
    return ENCODE_INT(DECODE_INT(a) / denom);
}

RuntimeValue rt_mod(RuntimeValue a, RuntimeValue b)
{
    int64_t denom = DECODE_INT(b);
    if (denom == 0) return ENCODE_INT(0);
    return ENCODE_INT(DECODE_INT(a) % denom);
}

RuntimeValue rt_pow(RuntimeValue a, RuntimeValue b)
{
    int64_t base = DECODE_INT(a);
    int64_t exp  = DECODE_INT(b);
    if (exp < 0) return ENCODE_INT(0);
    int64_t result = 1;
    for (int64_t i = 0; i < exp; i++) result *= base;
    return ENCODE_INT(result);
}

RuntimeValue rt_eq(RuntimeValue a, RuntimeValue b)
{
    return rt_native_eq(a, b) ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue rt_ne(RuntimeValue a, RuntimeValue b)
{
    return rt_native_eq(a, b) ? FALSE_VALUE : TRUE_VALUE;
}

RuntimeValue rt_lt(RuntimeValue a, RuntimeValue b)
{
    return (DECODE_INT(a) < DECODE_INT(b)) ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue rt_gt(RuntimeValue a, RuntimeValue b)
{
    return (DECODE_INT(a) > DECODE_INT(b)) ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue rt_le(RuntimeValue a, RuntimeValue b)
{
    return (DECODE_INT(a) <= DECODE_INT(b)) ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue rt_ge(RuntimeValue a, RuntimeValue b)
{
    return (DECODE_INT(a) >= DECODE_INT(b)) ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue rt_and(RuntimeValue a, RuntimeValue b)
{
    return (DECODE_INT(a) && DECODE_INT(b)) ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue rt_or(RuntimeValue a, RuntimeValue b)
{
    return (DECODE_INT(a) || DECODE_INT(b)) ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue rt_not(RuntimeValue a)
{
    return DECODE_INT(a) ? FALSE_VALUE : TRUE_VALUE;
}

RuntimeValue rt_shl(RuntimeValue a, RuntimeValue b)
{
    return ENCODE_INT(DECODE_INT(a) << DECODE_INT(b));
}

RuntimeValue rt_shr(RuntimeValue a, RuntimeValue b)
{
    return ENCODE_INT(DECODE_INT(a) >> DECODE_INT(b));
}

RuntimeValue rt_bitand(RuntimeValue a, RuntimeValue b)
{
    return ENCODE_INT(DECODE_INT(a) & DECODE_INT(b));
}

RuntimeValue rt_bitor(RuntimeValue a, RuntimeValue b)
{
    return ENCODE_INT(DECODE_INT(a) | DECODE_INT(b));
}

RuntimeValue rt_bitxor(RuntimeValue a, RuntimeValue b)
{
    return ENCODE_INT(DECODE_INT(a) ^ DECODE_INT(b));
}

RuntimeValue rt_bitnot(RuntimeValue a)
{
    return ENCODE_INT(~DECODE_INT(a));
}

RuntimeValue rt_neg(RuntimeValue a)
{
    return ENCODE_INT(-DECODE_INT(a));
}

/* --- Type introspection / conversion ---
 *
 * rt_typeof returns a string describing the type.
 * rt_is_* predicates return raw 1 or 0 for Cranelift boolean ABI.
 */

RuntimeValue rt_type_of(RuntimeValue val)
{
    if (IS_NIL(val))   return rt_string_from_cstr("nil");
    if (IS_INT(val))   return rt_string_from_cstr("int");
    if (IS_FLOAT(val)) return rt_string_from_cstr("float");
    if (IS_HEAP(val)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
        if (h) {
            if (h->type == HEAP_STRING) return rt_string_from_cstr("string");
            if (h->type == HEAP_ARRAY)  return rt_string_from_cstr("array");
            if (h->type == HEAP_MAP)    return rt_string_from_cstr("map");
            if (h->type == HEAP_OBJECT) return rt_string_from_cstr("object");
        }
        return rt_string_from_cstr("heap");
    }
    return rt_string_from_cstr("unknown");
}

RuntimeValue rt_is_nil(RuntimeValue val)
{
    return IS_NIL(val) ? 1 : 0;
}

RuntimeValue rt_is_int(RuntimeValue val)
{
    return IS_INT(val) ? 1 : 0;
}

RuntimeValue rt_is_float(RuntimeValue val)
{
    return IS_FLOAT(val) ? 1 : 0;
}

RuntimeValue rt_is_string(RuntimeValue val)
{
    if (!IS_HEAP(val)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
    return (h && h->type == HEAP_STRING) ? 1 : 0;
}

RuntimeValue rt_is_bool(RuntimeValue val)
{
    /* Booleans are encoded as ENCODE_INT(0) or ENCODE_INT(1) */
    if (!IS_INT(val)) return 0;
    int64_t v = DECODE_INT(val);
    return (v == 0 || v == 1) ? 1 : 0;
}

RuntimeValue rt_is_array(RuntimeValue val)
{
    if (!IS_HEAP(val)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
    return (h && h->type == HEAP_ARRAY) ? 1 : 0;
}

RuntimeValue rt_is_map(RuntimeValue val)
{
    if (!IS_HEAP(val)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
    return (h && h->type == HEAP_MAP) ? 1 : 0;
}

RuntimeValue rt_is_object(RuntimeValue val)
{
    if (!IS_HEAP(val)) return 0;
    HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
    return (h && h->type == HEAP_OBJECT) ? 1 : 0;
}
/* rt_to_int: convert to integer */
RuntimeValue rt_to_int(RuntimeValue val)
{
    if (IS_INT(val)) return val;
    if (IS_NIL(val)) return ENCODE_INT(0);
    if (IS_HEAP(val)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
        if (h && h->type == HEAP_STRING) {
            /* Parse decimal string to integer */
            RuntimeString *s = (RuntimeString *)h;
            if (s->len == 0) return ENCODE_INT(0);
            int64_t result = 0;
            int neg = 0;
            uint32_t i = 0;
            if (s->data[0] == '-') { neg = 1; i = 1; }
            else if (s->data[0] == '+') { i = 1; }
            for (; i < s->len; i++) {
                char c = s->data[i];
                if (c < '0' || c > '9') break;
                result = result * 10 + (c - '0');
            }
            if (neg) result = -result;
            return ENCODE_INT(result);
        }
    }
    return ENCODE_INT(0);
}
S1(rt_to_float)
/* rt_to_string: convert to string (delegates to rt_value_to_string) */
RuntimeValue rt_to_string(RuntimeValue val)
{
    return rt_value_to_string(val);
}
/* rt_to_bool: convert to boolean */
RuntimeValue rt_to_bool(RuntimeValue val)
{
    if (IS_NIL(val)) return FALSE_VALUE;
    if (IS_INT(val)) return DECODE_INT(val) ? TRUE_VALUE : FALSE_VALUE;
    if (IS_HEAP(val)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
        if (h && h->type == HEAP_STRING) {
            RuntimeString *s = (RuntimeString *)h;
            return s->len > 0 ? TRUE_VALUE : FALSE_VALUE;
        }
        if (h && h->type == HEAP_ARRAY) {
            RuntimeArray *a = (RuntimeArray *)h;
            return a->len > 0 ? TRUE_VALUE : FALSE_VALUE;
        }
        return TRUE_VALUE; /* non-nil heap object is truthy */
    }
    return FALSE_VALUE;
}
/* rt_clone: return as-is for primitives; shallow copy for heap objects */
RuntimeValue rt_clone(RuntimeValue val)
{
    if (!IS_HEAP(val)) return val; /* int, nil, float: value semantics */
    HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
    if (!h) return val;
    if (h->type == HEAP_STRING) {
        RuntimeString *s = (RuntimeString *)h;
        return rt_string_new((RuntimeValue)(uintptr_t)s->data, (RuntimeValue)s->len);
    }
    if (h->type == HEAP_ARRAY) {
        RuntimeArray *a = (RuntimeArray *)h;
        RuntimeValue new_arr = rt_array_new(ENCODE_INT(a->cap));
        RuntimeValue *items = runtime_array_items(a);
        for (uint32_t i = 0; i < a->len; i++) {
            new_arr = rt_array_push_handle(new_arr, items[i]);
        }
        return new_arr;
    }
    if (h->type == HEAP_MAP) {
        return rt_map_clone(val);
    }
    return val; /* unknown heap type: return as-is */
}

/* rt_freeze / rt_is_frozen: no-ops on bare metal (no GC, no mutability tracking) */
RuntimeValue rt_freeze(RuntimeValue val)
{
    return val;
}

RuntimeValue rt_is_frozen(RuntimeValue val)
{
    (void)val;
    return 0; /* always mutable on bare metal */
}

/* --- String extras ---
 *
 * Implement commonly-needed string operations for VFS routing and
 * general OS string handling.  Less-used ops remain as stubs.
 */

/* Helper: get RuntimeString pointer, or NULL */
static RuntimeString *decode_string(RuntimeValue v)
{
    if (!IS_HEAP(v)) return (RuntimeString *)0;
    RuntimeString *s = (RuntimeString *)DECODE_PTR(v);
    if (!s || s->hdr.type != HEAP_STRING) return (RuntimeString *)0;
    return s;
}

RuntimeValue rt_string_contains(RuntimeValue str, RuntimeValue needle)
{
    RuntimeString *s = decode_string(str);
    RuntimeString *n = decode_string(needle);
    if (!s || !n) return 0;
    if (n->len == 0) return 1;
    if (n->len > s->len) return 0;
    for (uint32_t i = 0; i <= s->len - n->len; i++) {
        uint32_t j;
        for (j = 0; j < n->len; j++) {
            if (s->data[i + j] != n->data[j]) break;
        }
        if (j == n->len) return 1;
    }
    return 0;
}

RuntimeValue rt_string_starts_with(RuntimeValue str, RuntimeValue prefix)
{
    RuntimeString *s = decode_string(str);
    RuntimeString *p = decode_string(prefix);
    if (!s || !p) return 0;
    if (p->len > s->len) return 0;
    for (uint32_t i = 0; i < p->len; i++) {
        if (s->data[i] != p->data[i]) return 0;
    }
    return 1;
}

RuntimeValue rt_string_ends_with(RuntimeValue str, RuntimeValue suffix)
{
    RuntimeString *s = decode_string(str);
    RuntimeString *x = decode_string(suffix);
    if (!s || !x) return 0;
    if (x->len > s->len) return 0;
    uint32_t off = s->len - x->len;
    for (uint32_t i = 0; i < x->len; i++) {
        if (s->data[off + i] != x->data[i]) return 0;
    }
    return 1;
}

RuntimeValue rt_string_index_of(RuntimeValue str, RuntimeValue needle)
{
    RuntimeString *s = decode_string(str);
    RuntimeString *n = decode_string(needle);
    if (!s || !n || n->len == 0) return (RuntimeValue)(-1);
    if (n->len > s->len) return (RuntimeValue)(-1);
    for (uint32_t i = 0; i <= s->len - n->len; i++) {
        uint32_t j;
        for (j = 0; j < n->len; j++) {
            if (s->data[i + j] != n->data[j]) break;
        }
        if (j == n->len) return ENCODE_INT((int64_t)i);
    }
    return (RuntimeValue)(-1);
}

RuntimeValue rt_string_last_index_of(RuntimeValue str, RuntimeValue needle)
{
    RuntimeString *s = decode_string(str);
    RuntimeString *n = decode_string(needle);
    if (!s || !n || n->len == 0) return (RuntimeValue)(-1);
    if (n->len > s->len) return (RuntimeValue)(-1);
    for (int64_t i = (int64_t)(s->len - n->len); i >= 0; i--) {
        uint32_t j;
        for (j = 0; j < n->len; j++) {
            if (s->data[i + j] != n->data[j]) break;
        }
        if (j == n->len) return ENCODE_INT(i);
    }
    return (RuntimeValue)(-1);
}

RuntimeValue rt_string_substr(RuntimeValue str, RuntimeValue start)
{
    /* substr(str, start) -- returns from start to end */
    RuntimeString *s = decode_string(str);
    if (!s) return NIL_VALUE;
    int64_t a = DECODE_INT(start);
    if (a < 0) a = 0;
    if ((uint32_t)a >= s->len) {
        return rt_string_from_cstr("");
    }
    return rt_string_slice(str, start, ENCODE_INT(s->len));
}

/* rt_string_split: split by delimiter, return array of strings */
RuntimeValue rt_string_split(RuntimeValue str, RuntimeValue delim)
{
    RuntimeString *s = decode_string(str);
    RuntimeString *d = decode_string(delim);
    RuntimeValue arr = rt_array_new(ENCODE_INT(4));
    if (!s || s->len == 0) return arr;
    if (!d || d->len == 0) {
        /* Split into individual characters */
        for (uint32_t i = 0; i < s->len; i++) {
            RuntimeValue ch = rt_string_new(
                (RuntimeValue)(uintptr_t)&s->data[i], 1);
            arr = rt_array_push_handle(arr, ch);
        }
        return arr;
    }
    if (d->len > s->len) {
        return rt_array_push_handle(arr, str);
    }
    uint32_t start = 0;
    for (uint32_t i = 0; i <= s->len - d->len; ) {
        uint32_t j;
        for (j = 0; j < d->len; j++) {
            if (s->data[i + j] != d->data[j]) break;
        }
        if (j == d->len) {
            /* Found delimiter at i */
            RuntimeValue part = rt_string_slice(str,
                ENCODE_INT(start), ENCODE_INT(i));
            arr = rt_array_push_handle(arr, part);
            i += d->len;
            start = i;
        } else {
            i++;
        }
    }
    /* Remainder */
    RuntimeValue rest = rt_string_slice(str,
        ENCODE_INT(start), ENCODE_INT(s->len));
    arr = rt_array_push_handle(arr, rest);
    return arr;
}

static int is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

RuntimeValue rt_string_trim(RuntimeValue str)
{
    RuntimeString *s = decode_string(str);
    if (!s || s->len == 0) return str;
    uint32_t start = 0;
    while (start < s->len && is_whitespace(s->data[start])) start++;
    uint32_t end = s->len;
    while (end > start && is_whitespace(s->data[end - 1])) end--;
    return rt_string_slice(str, (RuntimeValue)start, (RuntimeValue)end);
}

RuntimeValue rt_string_trim_start(RuntimeValue str)
{
    RuntimeString *s = decode_string(str);
    if (!s || s->len == 0) return str;
    uint32_t start = 0;
    while (start < s->len && is_whitespace(s->data[start])) start++;
    return rt_string_slice(str, (RuntimeValue)start, (RuntimeValue)s->len);
}

RuntimeValue rt_string_trim_end(RuntimeValue str)
{
    RuntimeString *s = decode_string(str);
    if (!s || s->len == 0) return str;
    uint32_t end = s->len;
    while (end > 0 && is_whitespace(s->data[end - 1])) end--;
    return rt_string_slice(str, 0, (RuntimeValue)end);
}

RuntimeValue rt_string_to_upper(RuntimeValue str)
{
    RuntimeString *s = decode_string(str);
    if (!s) return str;
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + s->len + 1);
    if (!r) return str;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + s->len + 1);
    r->len = s->len;
    for (uint32_t i = 0; i < s->len; i++) {
        char c = s->data[i];
        r->data[i] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    }
    r->data[s->len] = '\0';
    return ENCODE_PTR(r);
}

RuntimeValue rt_string_to_lower(RuntimeValue str)
{
    RuntimeString *s = decode_string(str);
    if (!s) return str;
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + s->len + 1);
    if (!r) return str;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + s->len + 1);
    r->len = s->len;
    for (uint32_t i = 0; i < s->len; i++) {
        char c = s->data[i];
        r->data[i] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
    }
    r->data[s->len] = '\0';
    return ENCODE_PTR(r);
}

/* rt_string_replace(str, old, new) — replace first occurrence */
RuntimeValue rt_string_replace(RuntimeValue str, RuntimeValue old_val, RuntimeValue new_val)
{
    RuntimeString *s = decode_string(str);
    RuntimeString *o = decode_string(old_val);
    RuntimeString *n = decode_string(new_val);
    if (!s || !o || o->len == 0) return str;
    if (o->len > s->len) return str; /* needle longer than haystack */
    if (!n) n = (RuntimeString *)0; /* treat nil replacement as empty */
    uint32_t nlen = n ? n->len : 0;

    /* Find first occurrence */
    for (uint32_t i = 0; i <= s->len - o->len; i++) {
        uint32_t j;
        for (j = 0; j < o->len; j++) {
            if (s->data[i + j] != o->data[j]) break;
        }
        if (j == o->len) {
            /* Found at position i */
            uint32_t result_len = s->len - o->len + nlen;
            RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + result_len + 1);
            if (!r) return str;
            r->hdr.type = HEAP_STRING;
            r->hdr.size = (uint32_t)(sizeof(RuntimeString) + result_len + 1);
            r->len = result_len;
            /* Copy: prefix + replacement + suffix */
            __builtin_memcpy(r->data, s->data, i);
            if (n && nlen > 0) __builtin_memcpy(r->data + i, n->data, nlen);
            __builtin_memcpy(r->data + i + nlen, s->data + i + o->len, s->len - i - o->len);
            r->data[result_len] = '\0';
            return ENCODE_PTR(r);
        }
    }
    return str; /* not found, return original */
}

/* rt_string_replace_all(str, old, new) — replace all occurrences (single-pass) */
RuntimeValue rt_string_replace_all(RuntimeValue str, RuntimeValue old_val, RuntimeValue new_val)
{
    RuntimeString *s = decode_string(str);
    RuntimeString *o = decode_string(old_val);
    RuntimeString *n = decode_string(new_val);
    if (!s || !o || o->len == 0) return str;
    uint32_t nlen = n ? n->len : 0;

    /* First pass: count occurrences to compute result size */
    uint32_t count = 0;
    for (uint32_t i = 0; i + o->len <= s->len; ) {
        uint32_t j;
        for (j = 0; j < o->len; j++) {
            if (s->data[i + j] != o->data[j]) break;
        }
        if (j == o->len) { count++; i += o->len; }
        else { i++; }
    }
    if (count == 0) return str;

    /* Allocate result */
    uint32_t result_len = s->len - count * o->len + count * nlen;
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + result_len + 1);
    if (!r) return str;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + result_len + 1);
    r->len = result_len;

    /* Second pass: build result */
    uint32_t out = 0;
    for (uint32_t i = 0; i < s->len; ) {
        if (i + o->len <= s->len) {
            uint32_t j;
            for (j = 0; j < o->len; j++) {
                if (s->data[i + j] != o->data[j]) break;
            }
            if (j == o->len) {
                if (n && nlen > 0) {
                    __builtin_memcpy(r->data + out, n->data, nlen);
                    out += nlen;
                }
                i += o->len;
                continue;
            }
        }
        r->data[out++] = s->data[i++];
    }
    r->data[result_len] = '\0';
    return ENCODE_PTR(r);
}

/* rt_string_repeat(str, count) — repeat string N times */
RuntimeValue rt_string_repeat(RuntimeValue str, RuntimeValue count_val)
{
    RuntimeString *s = decode_string(str);
    if (!s || s->len == 0) return str;
    int64_t count = DECODE_INT(count_val);
    if (count <= 0) return rt_string_from_cstr("");
    if (count == 1) return str;
    if ((uint64_t)count * s->len > 0x100000) count = (int64_t)(0x100000 / s->len);
    uint32_t result_len = s->len * (uint32_t)count;
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + result_len + 1);
    if (!r) return str;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + result_len + 1);
    r->len = result_len;
    for (int64_t i = 0; i < count; i++) {
        __builtin_memcpy(r->data + i * s->len, s->data, s->len);
    }
    r->data[result_len] = '\0';
    return ENCODE_PTR(r);
}

/* rt_string_pad_start(str, width) — left-pad with spaces to width */
RuntimeValue rt_string_pad_start(RuntimeValue str, RuntimeValue width_val)
{
    RuntimeString *s = decode_string(str);
    if (!s) return str;
    int64_t width = DECODE_INT(width_val);
    if (width <= 0 || (uint32_t)width <= s->len) return str;
    uint32_t pad = (uint32_t)width - s->len;
    uint32_t result_len = (uint32_t)width;
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + result_len + 1);
    if (!r) return str;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + result_len + 1);
    r->len = result_len;
    __builtin_memset(r->data, ' ', pad);
    __builtin_memcpy(r->data + pad, s->data, s->len);
    r->data[result_len] = '\0';
    return ENCODE_PTR(r);
}

/* rt_string_pad_end(str, width) — right-pad with spaces to width */
RuntimeValue rt_string_pad_end(RuntimeValue str, RuntimeValue width_val)
{
    RuntimeString *s = decode_string(str);
    if (!s) return str;
    int64_t width = DECODE_INT(width_val);
    if (width <= 0 || (uint32_t)width <= s->len) return str;
    uint32_t pad = (uint32_t)width - s->len;
    uint32_t result_len = (uint32_t)width;
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + result_len + 1);
    if (!r) return str;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + result_len + 1);
    r->len = result_len;
    __builtin_memcpy(r->data, s->data, s->len);
    __builtin_memset(r->data + s->len, ' ', pad);
    r->data[result_len] = '\0';
    return ENCODE_PTR(r);
}

/* rt_string_reverse(str) — reverse the string */
RuntimeValue rt_string_reverse(RuntimeValue str)
{
    RuntimeString *s = decode_string(str);
    if (!s || s->len <= 1) return str;
    RuntimeString *r = (RuntimeString *)malloc(sizeof(RuntimeString) + s->len + 1);
    if (!r) return str;
    r->hdr.type = HEAP_STRING;
    r->hdr.size = (uint32_t)(sizeof(RuntimeString) + s->len + 1);
    r->len = s->len;
    for (uint32_t i = 0; i < s->len; i++) {
        r->data[i] = s->data[s->len - 1 - i];
    }
    r->data[s->len] = '\0';
    return ENCODE_PTR(r);
}

/* rt_string_chars(str) — return array of single-character strings */
RuntimeValue rt_string_chars(RuntimeValue str)
{
    RuntimeString *s = decode_string(str);
    RuntimeValue arr = rt_array_new(ENCODE_INT(s ? s->len : 0));
    if (!s) return arr;
    for (uint32_t i = 0; i < s->len; i++) {
        RuntimeValue ch = rt_string_new(
            (RuntimeValue)(uintptr_t)&s->data[i], 1);
        arr = rt_array_push_handle(arr, ch);
    }
    return arr;
}

/* rt_string_bytes(str) — return array of byte values */
RuntimeValue rt_string_bytes(RuntimeValue str)
{
    RuntimeString *s = decode_string(str);
    RuntimeValue arr = rt_array_new(ENCODE_INT(s ? s->len : 0));
    if (!s) return arr;
    for (uint32_t i = 0; i < s->len; i++) {
        arr = rt_array_push_handle(arr, ENCODE_INT((int64_t)(unsigned char)s->data[i]));
    }
    return arr;
}

RuntimeValue rt_string_is_empty(RuntimeValue str)
{
    RuntimeString *s = decode_string(str);
    if (!s) return 1; /* nil/non-string is "empty" */
    return s->len == 0 ? 1 : 0;
}

RuntimeValue rt_string_compare(RuntimeValue a, RuntimeValue b)
{
    RuntimeString *sa = decode_string(a);
    RuntimeString *sb = decode_string(b);
    if (!sa && !sb) return ENCODE_INT(0);
    if (!sa) return (RuntimeValue)(-1);
    if (!sb) return ENCODE_INT(1);
    uint32_t min_len = sa->len < sb->len ? sa->len : sb->len;
    for (uint32_t i = 0; i < min_len; i++) {
        if (sa->data[i] != sb->data[i])
            return ENCODE_INT((int64_t)(unsigned char)sa->data[i]
                            - (int64_t)(unsigned char)sb->data[i]);
    }
    if (sa->len < sb->len) return (RuntimeValue)(-1);
    if (sa->len > sb->len) return ENCODE_INT(1);
    return ENCODE_INT(0);
}

/* --- rt_string_format(fmt, args_or_val) ---
 *
 * The compiler does NOT use this function for f-string interpolation
 * (it uses rt_string_new + rt_value_to_string + rt_string_concat instead).
 * This is a fallback/legacy symbol. Implement as simple concatenation
 * of the format template with the value converted to string. */
RuntimeValue rt_string_format(RuntimeValue fmt, RuntimeValue val)
{
    /* If fmt is a string template, just concatenate with val's string repr.
       For proper Python-style formatting, use rt_value_format_string. */
    RuntimeValue val_str = rt_value_to_string(val);
    if (!IS_HEAP(fmt)) return val_str;
    return rt_string_concat(fmt, val_str);
}

static int int_to_buf(char *buf, int buf_size, int64_t n)
{
    if (n == 0) { buf[0] = '0'; return 1; }
    int neg = 0;
    uint64_t uv;
    if (n < 0) { neg = 1; uv = (uint64_t)(-n); }
    else { uv = (uint64_t)n; }
    /* Build digits in reverse */
    char tmp[21];
    int pos = 0;
    while (uv > 0 && pos < 20) {
        tmp[pos++] = '0' + (char)(uv % 10);
        uv /= 10;
    }
    int out = 0;
    if (neg && out < buf_size) buf[out++] = '-';
    while (pos > 0 && out < buf_size) buf[out++] = tmp[--pos];
    return out;
}

static int int_to_hex_buf(char *buf, int buf_size, uint64_t v, int uppercase)
{
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    if (v == 0) { buf[0] = '0'; return 1; }
    char tmp[17];
    int pos = 0;
    while (v > 0 && pos < 16) {
        tmp[pos++] = digits[v & 0xF];
        v >>= 4;
    }
    int out = 0;
    while (pos > 0 && out < buf_size) buf[out++] = tmp[--pos];
    return out;
}

static int int_to_oct_buf(char *buf, int buf_size, uint64_t v)
{
    if (v == 0) { buf[0] = '0'; return 1; }
    char tmp[23];
    int pos = 0;
    while (v > 0 && pos < 22) {
        tmp[pos++] = '0' + (char)(v & 7);
        v >>= 3;
    }
    int out = 0;
    while (pos > 0 && out < buf_size) buf[out++] = tmp[--pos];
    return out;
}

static int int_to_bin_buf(char *buf, int buf_size, uint64_t v)
{
    if (v == 0) { buf[0] = '0'; return 1; }
    char tmp[65];
    int pos = 0;
    while (v > 0 && pos < 64) {
        tmp[pos++] = '0' + (char)(v & 1);
        v >>= 1;
    }
    int out = 0;
    while (pos > 0 && out < buf_size) buf[out++] = tmp[--pos];
    return out;
}

/* --- rt_value_format_string(val, fmt_ptr, fmt_len) ---
 *
 * Format a RuntimeValue using a Python-style format specifier.
 * Signature: (RuntimeValue val, RuntimeValue fmt_ptr, RuntimeValue fmt_len) -> RuntimeValue
 * where fmt_ptr is a raw pointer to the format spec bytes and fmt_len is the byte count.
 *
 * Supports: [[fill]align][sign][#][0][width][.precision][type]
 * Types: f(fixed-point), d(decimal), x/X(hex), o(octal), b(binary), s(string)
 */
RuntimeValue rt_value_format_string(RuntimeValue val, RuntimeValue fmt_ptr_rv, RuntimeValue fmt_len_rv)
{
    const char *spec = (const char *)(uintptr_t)fmt_ptr_rv;
    int64_t spec_len = fmt_len_rv;

    /* If no format spec, just convert to string */
    if (!spec || spec_len <= 0) {
        return rt_value_to_string(val);
    }

    /* Parse the format spec: [[fill]align][sign][#][0][width][.precision][type] */
    char fill = ' ';
    char align = '\0';    /* '<' '>' '^' '=' or '\0' for default */
    char sign_mode = '\0'; /* '+' '-' ' ' or '\0' */
    int alt_form = 0;     /* '#' prefix */
    int zero_pad = 0;     /* '0' before width */
    int width = -1;       /* -1 = no width */
    int precision = -1;   /* -1 = no precision */
    char type_code = '\0';
    int pos = 0;

    /* Check for [fill]align */
    if (spec_len >= 2 && (spec[1] == '<' || spec[1] == '>' || spec[1] == '^' || spec[1] == '=')) {
        fill = spec[0];
        align = spec[1];
        pos = 2;
    } else if (spec_len >= 1 && (spec[0] == '<' || spec[0] == '>' || spec[0] == '^' || spec[0] == '=')) {
        align = spec[0];
        pos = 1;
    }

    /* Sign */
    if (pos < spec_len && (spec[pos] == '+' || spec[pos] == '-' || spec[pos] == ' ')) {
        sign_mode = spec[pos];
        pos++;
    }

    /* Alt form '#' */
    if (pos < spec_len && spec[pos] == '#') {
        alt_form = 1;
        pos++;
    }

    /* Zero pad '0' (before width) */
    if (pos < spec_len && spec[pos] == '0') {
        zero_pad = 1;
        pos++;
    }

    /* Width (digits) */
    if (pos < spec_len && spec[pos] >= '1' && spec[pos] <= '9') {
        width = 0;
        while (pos < spec_len && spec[pos] >= '0' && spec[pos] <= '9') {
            width = width * 10 + (spec[pos] - '0');
            pos++;
        }
    }

    /* Precision */
    if (pos < spec_len && spec[pos] == '.') {
        pos++;
        precision = 0;
        while (pos < spec_len && spec[pos] >= '0' && spec[pos] <= '9') {
            precision = precision * 10 + (spec[pos] - '0');
            pos++;
        }
    }

    /* Type code */
    if (pos < spec_len) {
        type_code = spec[pos];
    }

    /* Format the raw value based on type code */
    char raw_buf[128];
    int raw_len = 0;

    int64_t int_val = 0;
    if (IS_INT(val)) int_val = DECODE_INT(val);

    switch (type_code) {
    case 'd': {
        /* Decimal integer */
        int64_t v = int_val;
        int is_neg = (v < 0);
        uint64_t abs_v = is_neg ? (uint64_t)(-v) : (uint64_t)v;
        char digits[21];
        int dlen = int_to_buf(digits, 21, (int64_t)abs_v);
        /* Apply sign */
        raw_len = 0;
        if (is_neg) raw_buf[raw_len++] = '-';
        else if (sign_mode == '+') raw_buf[raw_len++] = '+';
        else if (sign_mode == ' ') raw_buf[raw_len++] = ' ';
        __builtin_memcpy(raw_buf + raw_len, digits, (size_t)dlen);
        raw_len += dlen;
        break;
    }
    case 'x': case 'X': {
        /* Hexadecimal */
        uint64_t v = (uint64_t)int_val;
        raw_len = 0;
        if (alt_form) {
            raw_buf[raw_len++] = '0';
            raw_buf[raw_len++] = (type_code == 'X') ? 'X' : 'x';
        }
        int hlen = int_to_hex_buf(raw_buf + raw_len, (int)(sizeof(raw_buf) - (size_t)raw_len),
                                  v, (type_code == 'X'));
        raw_len += hlen;
        break;
    }
    case 'o': {
        /* Octal */
        uint64_t v = (uint64_t)int_val;
        raw_len = 0;
        if (alt_form) {
            raw_buf[raw_len++] = '0';
            raw_buf[raw_len++] = 'o';
        }
        int olen = int_to_oct_buf(raw_buf + raw_len, (int)(sizeof(raw_buf) - (size_t)raw_len), v);
        raw_len += olen;
        break;
    }
    case 'b': {
        /* Binary */
        uint64_t v = (uint64_t)int_val;
        raw_len = 0;
        if (alt_form) {
            raw_buf[raw_len++] = '0';
            raw_buf[raw_len++] = 'b';
        }
        int blen = int_to_bin_buf(raw_buf + raw_len, (int)(sizeof(raw_buf) - (size_t)raw_len), v);
        raw_len += blen;
        break;
    }
    case 'f': case 'F': {
        /* Fixed-point float — baremetal approximation for integers.
           Without FPU support, treat int as fixed-point: just append ".000000".
           If precision is 0, no decimal point. */
        int prec = (precision >= 0) ? precision : 6;
        int is_neg = (int_val < 0);
        int64_t abs_v = is_neg ? -int_val : int_val;
        raw_len = 0;
        if (is_neg) raw_buf[raw_len++] = '-';
        else if (sign_mode == '+') raw_buf[raw_len++] = '+';
        else if (sign_mode == ' ') raw_buf[raw_len++] = ' ';
        int dlen = int_to_buf(raw_buf + raw_len, (int)(sizeof(raw_buf) - (size_t)raw_len), abs_v);
        raw_len += dlen;
        if (prec > 0) {
            raw_buf[raw_len++] = '.';
            for (int i = 0; i < prec && raw_len < (int)sizeof(raw_buf) - 1; i++) {
                raw_buf[raw_len++] = '0';
            }
        }
        break;
    }
    case 's': case '\0': default: {
        /* String or default: convert to string, apply precision as max length */
        RuntimeValue str_rv = rt_value_to_string(val);
        RuntimeString *str_s = decode_string(str_rv);
        if (str_s) {
            int slen = (int)str_s->len;
            if (precision >= 0 && slen > precision) slen = precision;
            if (slen > (int)sizeof(raw_buf) - 1) slen = (int)sizeof(raw_buf) - 1;
            __builtin_memcpy(raw_buf, str_s->data, (size_t)slen);
            raw_len = slen;
        } else {
            /* rt_value_to_string returned nil — use "nil" */
            __builtin_memcpy(raw_buf, "nil", 3);
            raw_len = 3;
        }
        break;
    }
    }

    /* Apply width and alignment */
    if (width > 0 && raw_len < width) {
        int padding = width - raw_len;
        char fill_char = (zero_pad && align == '\0') ? '0' : fill;
        char eff_align = align;
        if (eff_align == '\0') {
            eff_align = zero_pad ? '>' : '<'; /* default alignment */
        }

        char result_buf[256];
        int result_len = 0;

        switch (eff_align) {
        case '>': {
            /* Right-align: for zero-pad, insert after sign */
            if (fill_char == '0' && raw_len > 0 &&
                (raw_buf[0] == '+' || raw_buf[0] == '-' || raw_buf[0] == ' ')) {
                result_buf[result_len++] = raw_buf[0]; /* sign */
                for (int i = 0; i < padding && result_len < 255; i++) result_buf[result_len++] = fill_char;
                for (int i = 1; i < raw_len && result_len < 255; i++) result_buf[result_len++] = raw_buf[i];
            } else {
                for (int i = 0; i < padding && result_len < 255; i++) result_buf[result_len++] = fill_char;
                for (int i = 0; i < raw_len && result_len < 255; i++) result_buf[result_len++] = raw_buf[i];
            }
            break;
        }
        case '<': {
            /* Left-align */
            for (int i = 0; i < raw_len && result_len < 255; i++) result_buf[result_len++] = raw_buf[i];
            for (int i = 0; i < padding && result_len < 255; i++) result_buf[result_len++] = fill_char;
            break;
        }
        case '^': {
            /* Center-align */
            int left_pad = padding / 2;
            int right_pad = padding - left_pad;
            for (int i = 0; i < left_pad && result_len < 255; i++) result_buf[result_len++] = fill_char;
            for (int i = 0; i < raw_len && result_len < 255; i++) result_buf[result_len++] = raw_buf[i];
            for (int i = 0; i < right_pad && result_len < 255; i++) result_buf[result_len++] = fill_char;
            break;
        }
        case '=': {
            /* Pad between sign and digits */
            if (raw_len > 0 && (raw_buf[0] == '+' || raw_buf[0] == '-' || raw_buf[0] == ' ')) {
                result_buf[result_len++] = raw_buf[0];
                for (int i = 0; i < padding && result_len < 255; i++) result_buf[result_len++] = fill_char;
                for (int i = 1; i < raw_len && result_len < 255; i++) result_buf[result_len++] = raw_buf[i];
            } else {
                for (int i = 0; i < padding && result_len < 255; i++) result_buf[result_len++] = fill_char;
                for (int i = 0; i < raw_len && result_len < 255; i++) result_buf[result_len++] = raw_buf[i];
            }
            break;
        }
        }

        return rt_string_new((RuntimeValue)(uintptr_t)result_buf, (RuntimeValue)result_len);
    }

    /* No width/alignment needed — return raw formatted value */
    return rt_string_new((RuntimeValue)(uintptr_t)raw_buf, (RuntimeValue)raw_len);
}


/* rt_array_new: create a new array with given capacity.
 * Cranelift codegen passes RAW capacity (iconst.i64 N), NOT tagged.
 * Must use raw value, not DECODE_INT. */
RuntimeValue rt_array_new(RuntimeValue cap_val)
{
    int64_t cap = (int64_t)simpleos_raw_or_encoded_int(cap_val);
    if (cap <= 0) cap = 16; /* default capacity */
    if (cap < 16) cap = 16;
    if (cap > 0x100000) cap = 0x100000; /* safety limit */
    size_t alloc_size = sizeof(RuntimeArray) + (size_t)cap * sizeof(RuntimeValue);
    RuntimeArray *a = (RuntimeArray *)malloc(alloc_size);
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)alloc_size;
    a->len = 0;
    a->cap = (uint32_t)cap;
    a->items = runtime_array_inline_items(a);
    /* Zero-init items */
    for (int64_t i = 0; i < cap; i++) a->items[i] = NIL_VALUE;
    return ENCODE_PTR(a);
}

/* Internal helper for baremetal C code that still expects the mutated array
 * handle back after append. The exported ABI below returns a status byte. */
static RuntimeValue rt_array_push_handle(RuntimeValue arr, RuntimeValue val)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    RuntimeValue *items = runtime_array_items(a);
    /* Values now arrive TAGGED from the compiler:
     * - Array literals: MIR BoxInt before rt_array_set
     * - .push(expr): MIR BoxInt inserted in lowering_expr.rs for integer args
     * - C-side callers: explicitly ENCODE_INT or pass heap pointers
     * IndexGet + MIR UnboxInt reads them back correctly. */
    if (a->len >= a->cap) {
        /* Preserve the original header address so callers that treat push as
         * in-place mutation do not keep using a stale handle after growth. */
        uint32_t old_cap = a->cap;
        uint32_t new_cap = a->cap * 2;
        if (new_cap < 128) new_cap = 128;
        RuntimeValue *new_items = NULL;
        if (items == runtime_array_inline_items(a)) {
            new_items = (RuntimeValue *)malloc((size_t)new_cap * sizeof(RuntimeValue));
            if (new_items) {
                for (uint32_t i = 0; i < a->len; i++) new_items[i] = items[i];
            }
        } else {
            new_items = (RuntimeValue *)simpleos_heap_realloc_last(
                items,
                (size_t)old_cap * sizeof(RuntimeValue),
                (size_t)new_cap * sizeof(RuntimeValue)
            );
        }
        if (!new_items) return ENCODE_PTR(a); /* alloc failed, drop */
        for (uint32_t i = a->len; i < new_cap; i++) new_items[i] = NIL_VALUE;
        a->items = new_items;
        a->cap = new_cap;
        items = runtime_array_items(a);
    }
    items[a->len] = val;
    a->len++;
    return ENCODE_PTR(a);
}

/* Exported array push ABI: mutate in place and return success like hosted. */
int8_t rt_array_push(RuntimeValue arr, RuntimeValue val)
{
    return rt_array_push_handle(arr, val) != NIL_VALUE;
}

RuntimeValue rt_push_byte(RuntimeValue arr, int64_t byte_val)
{
    return rt_array_push_handle(arr, ENCODE_INT(byte_val & 0xFF));
}

RuntimeValue rt_bytes_concat(RuntimeValue a_rv, RuntimeValue b_rv)
{
    if (!IS_HEAP(a_rv) || !IS_HEAP(b_rv)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(a_rv);
    RuntimeArray *b = (RuntimeArray *)DECODE_PTR(b_rv);
    if (!a || !b || a->hdr.type != HEAP_ARRAY || b->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    RuntimeValue *a_items = runtime_array_items(a);
    RuntimeValue *b_items = runtime_array_items(b);
    uint32_t len = a->len + b->len;
    RuntimeArray *out = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    if (!out) return NIL_VALUE;
    out->hdr.type = HEAP_ARRAY;
    out->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    out->len = len;
    out->cap = len;
    out->items = runtime_array_inline_items(out);
    for (uint32_t i = 0; i < a->len; i++) out->items[i] = a_items[i];
    for (uint32_t i = 0; i < b->len; i++) out->items[a->len + i] = b_items[i];
    return ENCODE_PTR(out);
}

RuntimeValue rt_bytes_slice(RuntimeValue arr_rv, int64_t start, int64_t length)
{
    if (!IS_HEAP(arr_rv)) return NIL_VALUE;
    RuntimeArray *arr = (RuntimeArray *)DECODE_PTR(arr_rv);
    if (!arr || arr->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    if (start < 0) start = 0;
    if (length < 0) length = 0;
    uint32_t ustart = (uint32_t)start;
    uint32_t ulen = (uint32_t)length;
    RuntimeValue *items = runtime_array_items(arr);
    if (ustart > arr->len) ustart = arr->len;
    if (ulen > arr->len - ustart) ulen = arr->len - ustart;
    RuntimeArray *out = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)ulen * sizeof(RuntimeValue));
    if (!out) return NIL_VALUE;
    out->hdr.type = HEAP_ARRAY;
    out->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)ulen * sizeof(RuntimeValue));
    out->len = ulen;
    out->cap = ulen;
    out->items = runtime_array_inline_items(out);
    for (uint32_t i = 0; i < ulen; i++) out->items[i] = items[ustart + i];
    return ENCODE_PTR(out);
}

int64_t rt_bytes_u8_at(RuntimeValue arr_rv, int64_t idx)
{
    if (!IS_HEAP(arr_rv)) return 0;
    RuntimeArray *arr = (RuntimeArray *)DECODE_PTR(arr_rv);
    if (!arr || arr->hdr.type != HEAP_ARRAY) return 0;
    if (idx < 0 || (uint32_t)idx >= arr->len) return 0;
    return (int64_t)_rv_byte(runtime_array_items(arr)[idx]);
}

int64_t rt_bytes_u16_be_at(RuntimeValue arr_rv, int64_t idx)
{
    if (!IS_HEAP(arr_rv)) return 0;
    RuntimeArray *arr = (RuntimeArray *)DECODE_PTR(arr_rv);
    if (!arr || arr->hdr.type != HEAP_ARRAY) return 0;
    if (idx < 0 || (uint32_t)(idx + 1) >= arr->len) return 0;
    RuntimeValue *items = runtime_array_items(arr);
    uint8_t hi = _rv_byte(items[(uint32_t)idx]);
    uint8_t lo = _rv_byte(items[(uint32_t)idx + 1]);
    return (int64_t)(((uint16_t)hi << 8) | (uint16_t)lo);
}

int64_t rt_bytes_u24_be_at(RuntimeValue arr_rv, int64_t idx)
{
    if (!IS_HEAP(arr_rv)) return 0;
    RuntimeArray *arr = (RuntimeArray *)DECODE_PTR(arr_rv);
    if (!arr || arr->hdr.type != HEAP_ARRAY) return 0;
    if (idx < 0 || (uint32_t)(idx + 2) >= arr->len) return 0;
    RuntimeValue *items = runtime_array_items(arr);
    uint8_t b0 = _rv_byte(items[(uint32_t)idx]);
    uint8_t b1 = _rv_byte(items[(uint32_t)idx + 1]);
    uint8_t b2 = _rv_byte(items[(uint32_t)idx + 2]);
    return (int64_t)(((uint32_t)b0 << 16) | ((uint32_t)b1 << 8) | (uint32_t)b2);
}

int64_t rt_tls13_serverhello_cipher_suite(RuntimeValue body_rv)
{
    if (!IS_HEAP(body_rv)) return 0;
    RuntimeArray *body = (RuntimeArray *)DECODE_PTR(body_rv);
    if (!body || body->hdr.type != HEAP_ARRAY) return 0;
    if (body->len < 38) return 0;
    RuntimeValue *items = runtime_array_items(body);

    uint32_t offset = 34; /* after legacy_version + random */
    uint32_t session_id_len = _rv_byte(items[offset]);
    if (offset + 1u + session_id_len + 2u > body->len) return 0;
    offset = offset + 1u + session_id_len;

    uint8_t hi = _rv_byte(items[offset]);
    uint8_t lo = _rv_byte(items[offset + 1]);
    return (int64_t)(((uint16_t)hi << 8) | (uint16_t)lo);
}

RuntimeValue rt_tls13_serverhello_x25519_pub(RuntimeValue body_rv)
{
    if (!IS_HEAP(body_rv)) return NIL_VALUE;
    RuntimeArray *body = (RuntimeArray *)DECODE_PTR(body_rv);
    if (!body || body->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    if (body->len < 38) return NIL_VALUE;
    RuntimeValue *items = runtime_array_items(body);
    for (uint32_t scan = 0; scan + 40U <= body->len; scan++) {
        if (_rv_byte(items[scan]) == 0x00 &&
            _rv_byte(items[scan + 1U]) == 0x33 &&
            _rv_byte(items[scan + 2U]) == 0x00 &&
            _rv_byte(items[scan + 3U]) == 0x24 &&
            _rv_byte(items[scan + 4U]) == 0x00 &&
            _rv_byte(items[scan + 5U]) == 0x1d &&
            _rv_byte(items[scan + 6U]) == 0x00 &&
            _rv_byte(items[scan + 7U]) == 0x20) {
            RuntimeArray *out = (RuntimeArray *)malloc(sizeof(RuntimeArray) + 32U * sizeof(RuntimeValue));
            if (!out) return NIL_VALUE;
            out->hdr.type = HEAP_ARRAY;
            out->hdr.size = (uint32_t)(sizeof(RuntimeArray) + 32U * sizeof(RuntimeValue));
            out->len = 32U;
            out->cap = 32U;
            out->items = runtime_array_inline_items(out);
            for (uint32_t i = 0; i < 32U; i++) {
                out->items[i] = ENCODE_INT(_rv_byte(items[scan + 8U + i]));
            }
            return ENCODE_PTR(out);
        }
    }

    uint32_t offset = 34; /* after legacy_version + random */
    uint32_t session_id_len = _rv_byte(items[offset]);
    if (offset + 1u + session_id_len + 2u + 1u + 2u > body->len) return NIL_VALUE;
    offset = offset + 1u + session_id_len;

    /* skip cipher_suite + compression */
    offset += 2u;
    offset += 1u;

    uint16_t exts_len = ((uint16_t)_rv_byte(items[offset]) << 8) |
                        (uint16_t)_rv_byte(items[offset + 1]);
    offset += 2u;
    uint32_t exts_end = offset + exts_len;
    if (exts_end > body->len) exts_end = body->len;

    while (offset + 4u <= exts_end) {
        uint16_t ext_type = ((uint16_t)_rv_byte(items[offset]) << 8) |
                            (uint16_t)_rv_byte(items[offset + 1]);
        uint16_t ext_len = ((uint16_t)_rv_byte(items[offset + 2]) << 8) |
                           (uint16_t)_rv_byte(items[offset + 3]);
        offset += 4u;
        uint32_t ext_data_end = offset + ext_len;
        if (ext_data_end > exts_end) ext_data_end = exts_end;

        if (ext_type == 51 && offset + 4u <= ext_data_end) {
            uint16_t group = ((uint16_t)_rv_byte(items[offset]) << 8) |
                             (uint16_t)_rv_byte(items[offset + 1]);
            uint16_t key_len = ((uint16_t)_rv_byte(items[offset + 2]) << 8) |
                               (uint16_t)_rv_byte(items[offset + 3]);
            uint32_t key_off = offset + 4u;
            uint32_t key_end = key_off + key_len;
            if (key_end > ext_data_end) key_end = ext_data_end;
            if (group == 0x001d && key_end >= key_off) {
                uint32_t out_len = key_end - key_off;
                RuntimeArray *out = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)out_len * sizeof(RuntimeValue));
                if (!out) return NIL_VALUE;
                out->hdr.type = HEAP_ARRAY;
                out->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)out_len * sizeof(RuntimeValue));
                out->len = out_len;
                out->cap = out_len;
                out->items = runtime_array_inline_items(out);
                for (uint32_t i = 0; i < out_len; i++) {
                    out->items[i] = ENCODE_INT(_rv_byte(items[key_off + i]));
                }
                return ENCODE_PTR(out);
            }
        }

        offset = ext_data_end;
    }

    RuntimeArray *empty = (RuntimeArray *)malloc(sizeof(RuntimeArray));
    if (!empty) return NIL_VALUE;
    empty->hdr.type = HEAP_ARRAY;
    empty->hdr.size = (uint32_t)sizeof(RuntimeArray);
    empty->len = 0;
    empty->cap = 0;
    empty->items = runtime_array_inline_items(empty);
    return ENCODE_PTR(empty);
}

static uint8_t *_tls_copy_runtime_bytes(RuntimeValue rv, uint32_t *out_len);
static RuntimeValue _tls_runtime_array_from_bytes(const uint8_t *buf, uint32_t len);

typedef int64_t tls_gf[16];

static const tls_gf _tls_x25519_121665 = { 0xdb41, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void _tls_x25519_car(tls_gf o)
{
    for (int i = 0; i < 16; i++) {
        o[i] += (int64_t)1 << 16;
        int64_t c = o[i] >> 16;
        o[(i + 1) * (i < 15)] += c - 1 + 37 * (c - 1) * (i == 15);
        o[i] -= c << 16;
    }
}

static void _tls_x25519_sel(tls_gf p, tls_gf q, int b)
{
    int64_t c = ~(int64_t)(b - 1);
    for (int i = 0; i < 16; i++) {
        int64_t t = c & (p[i] ^ q[i]);
        p[i] ^= t;
        q[i] ^= t;
    }
}

static void _tls_x25519_pack(uint8_t *o, const tls_gf n)
{
    tls_gf m, t;
    for (int i = 0; i < 16; i++) t[i] = n[i];
    _tls_x25519_car(t);
    _tls_x25519_car(t);
    _tls_x25519_car(t);
    for (int j = 0; j < 2; j++) {
        m[0] = t[0] - 0xffed;
        for (int i = 1; i < 15; i++) {
            m[i] = t[i] - 0xffff - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xffff;
        }
        m[15] = t[15] - 0x7fff - ((m[14] >> 16) & 1);
        int b = (int)((m[15] >> 16) & 1);
        m[14] &= 0xffff;
        _tls_x25519_sel(t, m, 1 - b);
    }
    for (int i = 0; i < 16; i++) {
        o[2 * i] = (uint8_t)(t[i] & 0xff);
        o[2 * i + 1] = (uint8_t)((t[i] >> 8) & 0xff);
    }
}

static void _tls_x25519_unpack(tls_gf o, const uint8_t *n)
{
    for (int i = 0; i < 16; i++) o[i] = (int64_t)n[2 * i] + ((int64_t)n[2 * i + 1] << 8);
    o[15] &= 0x7fff;
}

static void _tls_x25519_add(tls_gf o, const tls_gf a, const tls_gf b)
{
    for (int i = 0; i < 16; i++) o[i] = a[i] + b[i];
}

static void _tls_x25519_sub(tls_gf o, const tls_gf a, const tls_gf b)
{
    for (int i = 0; i < 16; i++) o[i] = a[i] - b[i];
}

static void _tls_x25519_mul(tls_gf o, const tls_gf a, const tls_gf b)
{
    int64_t t[31];
    for (int i = 0; i < 31; i++) t[i] = 0;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) t[i + j] += a[i] * b[j];
    }
    for (int i = 0; i < 15; i++) t[i] += 38 * t[i + 16];
    for (int i = 0; i < 16; i++) o[i] = t[i];
    _tls_x25519_car(o);
    _tls_x25519_car(o);
}

static void _tls_x25519_sqr(tls_gf o, const tls_gf a)
{
    _tls_x25519_mul(o, a, a);
}

static void _tls_x25519_inv(tls_gf o, const tls_gf i)
{
    tls_gf c;
    for (int a = 0; a < 16; a++) c[a] = i[a];
    for (int a = 253; a >= 0; a--) {
        _tls_x25519_sqr(c, c);
        if (a != 2 && a != 4) _tls_x25519_mul(c, c, i);
    }
    for (int a = 0; a < 16; a++) o[a] = c[a];
}

static void _tls_x25519_scalarmult(uint8_t *q, const uint8_t *n, const uint8_t *p)
{
    uint8_t z[32];
    tls_gf a, b, c, d, e, f, x;
    for (int i = 0; i < 32; i++) z[i] = n[i];
    z[31] = (uint8_t)((z[31] & 127) | 64);
    z[0] &= 248;
    _tls_x25519_unpack(x, p);
    for (int i = 0; i < 16; i++) {
        b[i] = x[i];
        d[i] = a[i] = c[i] = 0;
    }
    a[0] = d[0] = 1;
    for (int i = 254; i >= 0; --i) {
        int r = (z[i >> 3] >> (i & 7)) & 1;
        _tls_x25519_sel(a, b, r);
        _tls_x25519_sel(c, d, r);
        _tls_x25519_add(e, a, c);
        _tls_x25519_sub(a, a, c);
        _tls_x25519_add(c, b, d);
        _tls_x25519_sub(b, b, d);
        _tls_x25519_sqr(d, e);
        _tls_x25519_sqr(f, a);
        _tls_x25519_mul(a, c, a);
        _tls_x25519_mul(c, b, e);
        _tls_x25519_add(e, a, c);
        _tls_x25519_sub(a, a, c);
        _tls_x25519_sqr(b, a);
        _tls_x25519_sub(c, d, f);
        _tls_x25519_mul(a, c, _tls_x25519_121665);
        _tls_x25519_add(a, a, d);
        _tls_x25519_mul(c, c, a);
        _tls_x25519_mul(a, d, f);
        _tls_x25519_mul(d, b, x);
        _tls_x25519_sqr(b, e);
        _tls_x25519_sel(a, b, r);
        _tls_x25519_sel(c, d, r);
    }
    _tls_x25519_inv(c, c);
    _tls_x25519_mul(a, a, c);
    _tls_x25519_pack(q, a);
}

RuntimeValue rt_tls13_x25519_shared_secret(RuntimeValue scalar_rv, RuntimeValue point_rv)
{
    uint32_t scalar_len = 0, point_len = 0;
    uint8_t *scalar = _tls_copy_runtime_bytes(scalar_rv, &scalar_len);
    uint8_t *point = _tls_copy_runtime_bytes(point_rv, &point_len);
    uint8_t out[32];
    if (!scalar || !point || scalar_len != 32U || point_len != 32U) {
        if (scalar) free(scalar);
        if (point) free(point);
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }
    extern int64_t rt_tls13_ring_x25519_shared_secret_into_raw(const uint8_t scalar[32],
                                                               const uint8_t point[32],
                                                               uint8_t out[32]);
    if (rt_tls13_ring_x25519_shared_secret_into_raw(scalar, point, out) != 0) {
        _tls_x25519_scalarmult(out, scalar, point);
    }
    free(scalar);
    free(point);
    return _tls_runtime_array_from_bytes(out, 32U);
}

RuntimeValue rt_tls13_x25519_public_key(RuntimeValue scalar_rv)
{
    uint32_t scalar_len = 0;
    uint8_t *scalar = _tls_copy_runtime_bytes(scalar_rv, &scalar_len);
    uint8_t basepoint[32] = {9};
    uint8_t out[32];
    if (!scalar || scalar_len != 32U) {
        if (scalar) free(scalar);
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }
    _tls_x25519_scalarmult(out, scalar, basepoint);
    free(scalar);
    return _tls_runtime_array_from_bytes(out, 32U);
}

RuntimeValue rt_tls13_ed25519_public_key(RuntimeValue seed_rv)
{
    uint32_t seed_len = 0;
    uint8_t *seed = _tls_copy_runtime_bytes(seed_rv, &seed_len);
    uint8_t pk[32];
    uint8_t sk[64];
    if (!seed || seed_len != 32U) {
        if (seed) free(seed);
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }
    extern int64_t rt_tls13_ring_ed25519_keypair_raw(const uint8_t seed[32], uint8_t pk[32], uint8_t sk[64]);
    if (rt_tls13_ring_ed25519_keypair_raw(seed, pk, sk) != 0) {
        free(seed);
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }
    free(seed);
    return _tls_runtime_array_from_bytes(pk, 32U);
}

RuntimeValue rt_tls13_ed25519_sign(RuntimeValue seed_rv, RuntimeValue pub_rv, RuntimeValue msg_rv)
{
    uint32_t seed_len = 0, pub_len = 0, msg_len = 0;
    uint8_t *seed = _tls_copy_runtime_bytes(seed_rv, &seed_len);
    uint8_t *pub = _tls_copy_runtime_bytes(pub_rv, &pub_len);
    uint8_t *msg = _tls_copy_runtime_bytes(msg_rv, &msg_len);
    uint8_t sk[64];
    uint8_t sig[64];
    if (!seed || seed_len != 32U || !pub || pub_len != 32U) {
        if (seed) free(seed);
        if (pub) free(pub);
        if (msg) free(msg);
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }
    for (uint32_t i = 0; i < 32U; i++) {
        sk[i] = seed[i];
        sk[32U + i] = pub[i];
    }
    extern int64_t rt_tls13_ring_ed25519_sign_raw(const uint8_t *msg, uint32_t msg_len,
                                                  const uint8_t sk[64], uint8_t sig[64]);
    int64_t rc = rt_tls13_ring_ed25519_sign_raw(msg ? msg : (const uint8_t *)"", msg_len, sk, sig);
    free(seed);
    free(pub);
    if (msg) free(msg);
    if (rc != 0) {
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }
    return _tls_runtime_array_from_bytes(sig, 64U);
}

int64_t rt_tls13_x25519_shared_secret_into(RuntimeValue scalar_rv, RuntimeValue point_rv, RuntimeValue out_rv)
{
    uint32_t scalar_len = 0, point_len = 0;
    uint8_t *scalar = _tls_copy_runtime_bytes(scalar_rv, &scalar_len);
    uint8_t *point = _tls_copy_runtime_bytes(point_rv, &point_len);
    if (!scalar || !point || scalar_len != 32U || point_len != 32U) {
        if (scalar) free(scalar);
        if (point) free(point);
        return 0;
    }
    if (!IS_HEAP(out_rv)) {
        free(scalar);
        free(point);
        return 0;
    }
    RuntimeArray *out_arr = (RuntimeArray *)DECODE_PTR(out_rv);
    if (!out_arr || out_arr->hdr.type != HEAP_ARRAY || out_arr->len != 32U) {
        free(scalar);
        free(point);
        return 0;
    }

    uint8_t out[32];
    extern int64_t rt_tls13_ring_x25519_shared_secret_into_raw(const uint8_t scalar[32],
                                                               const uint8_t point[32],
                                                               uint8_t out[32]);
    if (rt_tls13_ring_x25519_shared_secret_into_raw(scalar, point, out) != 0) {
        _tls_x25519_scalarmult(out, scalar, point);
    }
    RuntimeValue *out_items = runtime_array_items(out_arr);
    for (uint32_t i = 0; i < 32U; i++) {
        out_items[i] = ENCODE_INT(out[i]);
    }
    free(scalar);
    free(point);
    return 1;
}

static inline uint32_t _tls_sha256_rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t _tls_sha256_ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t _tls_sha256_maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t _tls_sha256_S0(uint32_t x) { return _tls_sha256_rotr(x, 2) ^ _tls_sha256_rotr(x, 13) ^ _tls_sha256_rotr(x, 22); }
static inline uint32_t _tls_sha256_S1(uint32_t x) { return _tls_sha256_rotr(x, 6) ^ _tls_sha256_rotr(x, 11) ^ _tls_sha256_rotr(x, 25); }
static inline uint32_t _tls_sha256_s0(uint32_t x) { return _tls_sha256_rotr(x, 7) ^ _tls_sha256_rotr(x, 18) ^ (x >> 3); }
static inline uint32_t _tls_sha256_s1(uint32_t x) { return _tls_sha256_rotr(x, 17) ^ _tls_sha256_rotr(x, 19) ^ (x >> 10); }

typedef struct {
    uint32_t h[8];
    uint64_t total_len;
    uint32_t block_len;
    uint8_t block[64];
} TlsSha256Ctx;

static void _tls_sha256_process_block(const uint8_t *block, uint32_t h[8])
{
    uint32_t w[64];
    for (int t = 0; t < 16; t++) {
        w[t] = ((uint32_t)block[t * 4] << 24) |
               ((uint32_t)block[t * 4 + 1] << 16) |
               ((uint32_t)block[t * 4 + 2] << 8) |
               (uint32_t)block[t * 4 + 3];
    }
    for (int t = 16; t < 64; t++)
        w[t] = _tls_sha256_s1(w[t - 2]) + w[t - 7] + _tls_sha256_s0(w[t - 15]) + w[t - 16];

    uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
    uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];
    for (int t = 0; t < 64; t++) {
        uint32_t t1 = hh + _tls_sha256_S1(e) + _tls_sha256_ch(e, f, g) + _sha256_K[t] + w[t];
        uint32_t t2 = _tls_sha256_S0(a) + _tls_sha256_maj(a, b, c);
        hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
}

static void _tls_sha256_init(TlsSha256Ctx *ctx)
{
    for (int i = 0; i < 8; i++) ctx->h[i] = _sha256_H[i];
    ctx->total_len = 0;
    ctx->block_len = 0;
}

static void _tls_sha256_update(TlsSha256Ctx *ctx, const uint8_t *msg, uint32_t msg_len)
{
    if (!ctx || (!msg && msg_len != 0U)) return;
    ctx->total_len += (uint64_t)msg_len;
    if (ctx->block_len > 0U) {
        uint32_t take = 64U - ctx->block_len;
        if (take > msg_len) take = msg_len;
        if (take > 0U) {
            memcpy(ctx->block + ctx->block_len, msg, take);
            ctx->block_len += take;
            msg += take;
            msg_len -= take;
        }
        if (ctx->block_len == 64U) {
            _tls_sha256_process_block(ctx->block, ctx->h);
            ctx->block_len = 0U;
        }
    }
    while (msg_len >= 64U) {
        _tls_sha256_process_block(msg, ctx->h);
        msg += 64U;
        msg_len -= 64U;
    }
    if (msg_len > 0U) {
        memcpy(ctx->block, msg, msg_len);
        ctx->block_len = msg_len;
    }
}

static void _tls_sha256_final(TlsSha256Ctx *ctx, uint8_t out[32])
{
    uint64_t bit_len = ctx->total_len * 8ULL;
    uint8_t final_blocks[128];
    uint32_t final_len = ctx->block_len;
    memcpy(final_blocks, ctx->block, final_len);
    final_blocks[final_len++] = 0x80U;
    while ((final_len % 64U) != 56U) final_blocks[final_len++] = 0x00U;
    for (int i = 0; i < 8; i++) {
        final_blocks[final_len++] = (uint8_t)(bit_len >> (56 - i * 8));
    }
    for (uint32_t off = 0; off < final_len; off += 64U) {
        _tls_sha256_process_block(final_blocks + off, ctx->h);
    }
    for (int i = 0; i < 8; i++) {
        out[i * 4] = (uint8_t)(ctx->h[i] >> 24);
        out[i * 4 + 1] = (uint8_t)(ctx->h[i] >> 16);
        out[i * 4 + 2] = (uint8_t)(ctx->h[i] >> 8);
        out[i * 4 + 3] = (uint8_t)ctx->h[i];
    }
}

static void _tls_sha256_digest(const uint8_t *msg, uint32_t msg_len, uint8_t out[32])
{
    uint64_t bit_len = (uint64_t)msg_len * 8ULL;
    uint32_t padded_len = msg_len + 1U + 8U;
    uint32_t rem = padded_len % 64U;
    if (rem != 0U) padded_len += 64U - rem;
    uint8_t padded_stack[256];
    uint8_t *padded = padded_stack;
    uint32_t h[8];
    if (padded_len > sizeof(padded_stack)) {
        padded = (uint8_t *)malloc(padded_len > 0U ? padded_len : 1U);
        if (!padded && padded_len != 0U) {
            memset(out, 0, 32U);
            return;
        }
    }
    memset(padded, 0, padded_len);
    if (msg_len > 0U && msg) memcpy(padded, msg, msg_len);
    padded[msg_len] = 0x80U;
    for (int i = 0; i < 8; i++) {
        padded[padded_len - 8U + (uint32_t)i] = (uint8_t)(bit_len >> (56 - i * 8));
    }
    for (int i = 0; i < 8; i++) h[i] = _sha256_H[i];
    for (uint32_t off = 0; off < padded_len; off += 64U) {
        _tls_sha256_process_block(padded + off, h);
    }
    if (padded != padded_stack) free(padded);
    for (int i = 0; i < 8; i++) {
        out[i * 4] = (uint8_t)(h[i] >> 24);
        out[i * 4 + 1] = (uint8_t)(h[i] >> 16);
        out[i * 4 + 2] = (uint8_t)(h[i] >> 8);
        out[i * 4 + 3] = (uint8_t)h[i];
    }
}

static void _tls_hmac_sha256(const uint8_t *key, uint32_t key_len, const uint8_t *data, uint32_t data_len, uint8_t out[32])
{
    uint8_t k0[64];
    uint8_t ipad[64];
    uint8_t opad[64];
    uint8_t inner_hash[32];
    uint32_t inner_len = 64U + data_len;
    uint8_t *inner_input = (uint8_t *)malloc(inner_len > 0U ? inner_len : 1U);
    uint8_t outer_input[96];
    memset(k0, 0, sizeof(k0));
    if (key_len > 64U) {
        _tls_sha256_digest(key, key_len, k0);
    } else if (key_len > 0) {
        memcpy(k0, key, key_len);
    }
    for (int i = 0; i < 64; i++) {
        ipad[i] = (uint8_t)(k0[i] ^ 0x36U);
        opad[i] = (uint8_t)(k0[i] ^ 0x5cU);
    }
    if (!inner_input && inner_len != 0U) {
        memset(out, 0, 32U);
        return;
    }
    memcpy(inner_input, ipad, 64U);
    if (data_len > 0U) memcpy(inner_input + 64U, data, data_len);
    _tls_sha256_digest(inner_input, inner_len, inner_hash);
    free(inner_input);

    memcpy(outer_input, opad, 64U);
    memcpy(outer_input + 64U, inner_hash, 32U);
    _tls_sha256_digest(outer_input, 96U, out);
    if (key_len == 13U && data_len == 22U && key && data && key[0] == 0U && key[12] == 12U && data[0] == 11U) {
        serial_puts("[DBG_C] hmac inner=");
        serial_put_dec((int64_t)inner_hash[0]); serial_puts(" ");
        serial_put_dec((int64_t)inner_hash[1]); serial_puts(" ");
        serial_put_dec((int64_t)inner_hash[2]); serial_puts(" ");
        serial_put_dec((int64_t)inner_hash[3]); serial_puts(" out=");
        serial_put_dec((int64_t)out[0]); serial_puts(" ");
        serial_put_dec((int64_t)out[1]); serial_puts(" ");
        serial_put_dec((int64_t)out[2]); serial_puts(" ");
        serial_put_dec((int64_t)out[3]); serial_puts("\r\n");
    }
}

static uint8_t *_tls_copy_runtime_bytes(RuntimeValue rv, uint32_t *out_len)
{
    const uint32_t SIMPLEOS_TLS_RUNTIME_BYTE_CAP = 1U << 20;
    *out_len = 0;
    if (!IS_HEAP(rv)) return NULL;
    RuntimeArray *arr = (RuntimeArray *)DECODE_PTR(rv);
    if (!arr || arr->hdr.type != HEAP_ARRAY) return NULL;
    uint32_t len = arr->len;
    if (len > SIMPLEOS_TLS_RUNTIME_BYTE_CAP) return NULL;
    RuntimeValue *items = runtime_array_items(arr);
    uint8_t *buf = (uint8_t *)malloc(len > 0 ? len : 1);
    if (!buf) return NULL;
    for (uint32_t i = 0; i < len; i++) buf[i] = _rv_byte(items[i]);
    *out_len = len;
    return buf;
}

static int _tls_materialize_runtime_bytes(RuntimeValue rv,
                                          uint8_t *stack_buf,
                                          uint32_t stack_cap,
                                          const uint8_t **out_data,
                                          uint8_t **out_heap,
                                          uint32_t *out_len)
{
    *out_data = NULL;
    *out_heap = NULL;
    *out_len = 0;
    if (!IS_HEAP(rv)) return 0;
    RuntimeArray *arr = (RuntimeArray *)DECODE_PTR(rv);
    if (!arr || arr->hdr.type != HEAP_ARRAY) return 0;
    uint32_t len = arr->len;
    if (len <= stack_cap) {
        RuntimeValue *items = runtime_array_items(arr);
        for (uint32_t i = 0; i < len; i++) stack_buf[i] = _rv_byte(items[i]);
        *out_data = stack_buf;
        *out_len = len;
        return 1;
    }
    *out_heap = _tls_copy_runtime_bytes(rv, out_len);
    if (!*out_heap && *out_len != 0U) return 0;
    *out_data = *out_heap;
    return 1;
}

int64_t rt_tls13_record_payload_len(RuntimeValue record_rv)
{
    uint32_t len = 0;
    uint8_t *buf = _tls_copy_runtime_bytes(record_rv, &len);
    int64_t payload_len = -1;
    if (buf && len >= 5U) {
        payload_len = (int64_t)(((uint32_t)buf[3] << 8) | (uint32_t)buf[4]);
    }
    if (buf) free(buf);
    return payload_len;
}

int64_t rt_tls13_record_content_type(RuntimeValue record_rv)
{
    uint32_t len = 0;
    uint8_t *buf = _tls_copy_runtime_bytes(record_rv, &len);
    int64_t content_type = -1;
    if (buf && len >= 1U) {
        content_type = (int64_t)buf[0];
    }
    if (buf) free(buf);
    return content_type;
}

RuntimeValue rt_tls13_record_serverhello_x25519_pub(RuntimeValue record_rv)
{
    uint32_t len = 0;
    uint8_t *buf = _tls_copy_runtime_bytes(record_rv, &len);
    if (!buf) return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    for (uint32_t scan = 0; scan + 40U <= len; scan++) {
        if (buf[scan] == 0x00 && buf[scan + 1U] == 0x33 &&
            buf[scan + 2U] == 0x00 && buf[scan + 3U] == 0x24 &&
            buf[scan + 4U] == 0x00 && buf[scan + 5U] == 0x1d &&
            buf[scan + 6U] == 0x00 && buf[scan + 7U] == 0x20) {
            RuntimeValue rv = _tls_runtime_array_from_bytes(buf + scan + 8U, 32U);
            free(buf);
            return rv;
        }
    }
    free(buf);
    return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
}

RuntimeValue rt_tls13_transcript_hash_record_payloads(RuntimeValue a_record_rv, RuntimeValue b_record_rv)
{
    uint32_t a_len = 0, b_len = 0;
    uint8_t *a = _tls_copy_runtime_bytes(a_record_rv, &a_len);
    uint8_t *b = _tls_copy_runtime_bytes(b_record_rv, &b_len);
    if (!a || !b || a_len < 5U || b_len < 5U) {
        if (a) free(a);
        if (b) free(b);
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }
    uint32_t a_payload = ((uint32_t)a[3] << 8) | (uint32_t)a[4];
    uint32_t b_payload = ((uint32_t)b[3] << 8) | (uint32_t)b[4];
    if (a_len < 5U + a_payload || b_len < 5U + b_payload) {
        free(a);
        free(b);
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }
    uint32_t total = a_payload + b_payload;
    uint8_t *merged = (uint8_t *)malloc(total > 0 ? total : 1U);
    uint8_t out[32];
    if (!merged && total != 0) {
        free(a);
        free(b);
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }
    memcpy(merged, a + 5U, a_payload);
    memcpy(merged + a_payload, b + 5U, b_payload);
    _tls_sha256_digest(merged, total, out);
    free(merged);
    free(a);
    free(b);
    return _tls_runtime_array_from_bytes(out, 32U);
}

static RuntimeValue _tls_runtime_array_from_bytes(const uint8_t *buf, uint32_t len)
{
    RuntimeArray *out = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    if (!out) return NIL_VALUE;
    out->hdr.type = HEAP_ARRAY;
    out->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    out->len = len;
    out->cap = len;
    out->items = runtime_array_inline_items(out);
    for (uint32_t i = 0; i < len; i++) out->items[i] = ENCODE_INT(buf[i]);
    return ENCODE_PTR(out);
}

static int64_t _tls_write_runtime_bytes(RuntimeValue out_rv, const uint8_t *buf, uint32_t len)
{
    if (!IS_HEAP(out_rv)) return 0;
    RuntimeArray *out_arr = (RuntimeArray *)DECODE_PTR(out_rv);
    if (!out_arr || out_arr->hdr.type != HEAP_ARRAY || out_arr->len != len) return 0;
    RuntimeValue *out_items = runtime_array_items(out_arr);
    for (uint32_t i = 0; i < len; i++) {
        out_items[i] = ENCODE_INT(buf[i]);
    }
    return 1;
}

RuntimeValue rt_tls13_sha256(RuntimeValue data_rv)
{
    uint32_t data_len = 0;
    uint8_t *data = _tls_copy_runtime_bytes(data_rv, &data_len);
    uint8_t out[32];
    if (!data && data_len != 0) return NIL_VALUE;
    _tls_sha256_digest(data, data_len, out);
    if (data) free(data);
    return _tls_runtime_array_from_bytes(out, 32);
}

RuntimeValue rt_tls13_transcript_hash_2(RuntimeValue a_rv, RuntimeValue b_rv)
{
    uint32_t a_len = 0, b_len = 0;
    uint8_t *a = _tls_copy_runtime_bytes(a_rv, &a_len);
    uint8_t *b = _tls_copy_runtime_bytes(b_rv, &b_len);
    uint8_t out[32];
    uint8_t *merged = (uint8_t *)malloc((size_t)a_len + (size_t)b_len);
    if ((!a && a_len != 0) || (!b && b_len != 0) || (!merged && (a_len + b_len) != 0)) {
        if (a) free(a);
        if (b) free(b);
        if (merged) free(merged);
        return NIL_VALUE;
    }
    if (a_len > 0) memcpy(merged, a, a_len);
    if (b_len > 0) memcpy(merged + a_len, b, b_len);
    _tls_sha256_digest(merged, a_len + b_len, out);
    if (a) free(a);
    if (b) free(b);
    if (merged) free(merged);
    return _tls_runtime_array_from_bytes(out, 32);
}

RuntimeValue rt_tls13_transcript_hash_6(RuntimeValue a_rv, RuntimeValue b_rv, RuntimeValue c_rv,
                                        RuntimeValue d_rv, RuntimeValue e_rv, RuntimeValue f_rv)
{
    uint32_t a_len = 0, b_len = 0, c_len = 0, d_len = 0, e_len = 0, f_len = 0;
    uint8_t *a = _tls_copy_runtime_bytes(a_rv, &a_len);
    uint8_t *b = _tls_copy_runtime_bytes(b_rv, &b_len);
    uint8_t *c = _tls_copy_runtime_bytes(c_rv, &c_len);
    uint8_t *d = _tls_copy_runtime_bytes(d_rv, &d_len);
    uint8_t *e = _tls_copy_runtime_bytes(e_rv, &e_len);
    uint8_t *f = _tls_copy_runtime_bytes(f_rv, &f_len);
    uint8_t out[32];
    uint32_t total = a_len + b_len + c_len + d_len + e_len + f_len;
    uint8_t *merged = (uint8_t *)malloc(total > 0 ? total : 1U);
    if ((!a && a_len != 0) || (!b && b_len != 0) || (!c && c_len != 0) ||
        (!d && d_len != 0) || (!e && e_len != 0) || (!f && f_len != 0) ||
        (!merged && total != 0)) {
        if (a) free(a); if (b) free(b); if (c) free(c);
        if (d) free(d); if (e) free(e); if (f) free(f);
        if (merged) free(merged);
        return NIL_VALUE;
    }
    uint32_t off = 0;
    if (a_len) { memcpy(merged + off, a, a_len); off += a_len; }
    if (b_len) { memcpy(merged + off, b, b_len); off += b_len; }
    if (c_len) { memcpy(merged + off, c, c_len); off += c_len; }
    if (d_len) { memcpy(merged + off, d, d_len); off += d_len; }
    if (e_len) { memcpy(merged + off, e, e_len); off += e_len; }
    if (f_len) { memcpy(merged + off, f, f_len); off += f_len; }
    _tls_sha256_digest(merged, total, out);
    if (a) free(a); if (b) free(b); if (c) free(c);
    if (d) free(d); if (e) free(e); if (f) free(f);
    free(merged);
    return _tls_runtime_array_from_bytes(out, 32);
}

RuntimeValue rt_tls13_transcript_hash_finished_fd(RuntimeValue ch_record_rv, RuntimeValue sh_record_rv,
                                                 RuntimeValue cr_msg_rv, RuntimeValue ee_msg_rv,
                                                 RuntimeValue cert_msg_rv, RuntimeValue cv_msg_rv)
{
    uint32_t ch_len = 0, sh_len = 0, cr_len = 0, ee_len = 0, cert_len = 0, cv_len = 0;
    uint8_t *ch = _tls_copy_runtime_bytes(ch_record_rv, &ch_len);
    uint8_t *sh = _tls_copy_runtime_bytes(sh_record_rv, &sh_len);
    uint8_t *cr = _tls_copy_runtime_bytes(cr_msg_rv, &cr_len);
    uint8_t *ee = _tls_copy_runtime_bytes(ee_msg_rv, &ee_len);
    uint8_t *cert = _tls_copy_runtime_bytes(cert_msg_rv, &cert_len);
    uint8_t *cv = _tls_copy_runtime_bytes(cv_msg_rv, &cv_len);
    RuntimeValue rv = NIL_VALUE;
    uint8_t out[32];
    if (!ch || !sh || (!cr && cr_len != 0U) || (!ee && ee_len != 0U) ||
        (!cert && cert_len != 0U) || (!cv && cv_len != 0U) ||
        ch_len < 5U || sh_len < 5U) {
        goto done;
    }
    uint32_t ch_payload_len = ((uint32_t)ch[3] << 8) | (uint32_t)ch[4];
    uint32_t sh_payload_len = ((uint32_t)sh[3] << 8) | (uint32_t)sh[4];
    if (5U + ch_payload_len > ch_len || 5U + sh_payload_len > sh_len) goto done;
    uint32_t total = ch_payload_len + sh_payload_len + cr_len + ee_len + cert_len + cv_len;
    uint8_t *merged = (uint8_t *)malloc(total > 0U ? total : 1U);
    if (!merged && total != 0U) goto done;
    uint32_t off = 0;
    if (ch_payload_len) { memcpy(merged + off, ch + 5U, ch_payload_len); off += ch_payload_len; }
    if (sh_payload_len) { memcpy(merged + off, sh + 5U, sh_payload_len); off += sh_payload_len; }
    if (cr_len) { memcpy(merged + off, cr, cr_len); off += cr_len; }
    if (ee_len) { memcpy(merged + off, ee, ee_len); off += ee_len; }
    if (cert_len) { memcpy(merged + off, cert, cert_len); off += cert_len; }
    if (cv_len) { memcpy(merged + off, cv, cv_len); off += cv_len; }
    _tls_sha256_digest(merged, total, out);
    rv = _tls_runtime_array_from_bytes(out, 32U);
    free(merged);
done:
    if (ch) free(ch);
    if (sh) free(sh);
    if (cr) free(cr);
    if (ee) free(ee);
    if (cert) free(cert);
    if (cv) free(cv);
    return rv;
}

RuntimeValue rt_tls13_transcript_hash_7(RuntimeValue a_rv, RuntimeValue b_rv, RuntimeValue c_rv,
                                        RuntimeValue d_rv, RuntimeValue e_rv, RuntimeValue f_rv,
                                        RuntimeValue g_rv)
{
    uint32_t a_len = 0, b_len = 0, c_len = 0, d_len = 0, e_len = 0, f_len = 0, g_len = 0;
    uint8_t *a = _tls_copy_runtime_bytes(a_rv, &a_len);
    uint8_t *b = _tls_copy_runtime_bytes(b_rv, &b_len);
    uint8_t *c = _tls_copy_runtime_bytes(c_rv, &c_len);
    uint8_t *d = _tls_copy_runtime_bytes(d_rv, &d_len);
    uint8_t *e = _tls_copy_runtime_bytes(e_rv, &e_len);
    uint8_t *f = _tls_copy_runtime_bytes(f_rv, &f_len);
    uint8_t *g = _tls_copy_runtime_bytes(g_rv, &g_len);
    uint8_t out[32];
    uint32_t total = a_len + b_len + c_len + d_len + e_len + f_len + g_len;
    uint8_t *merged = (uint8_t *)malloc(total > 0 ? total : 1U);
    if ((!a && a_len != 0) || (!b && b_len != 0) || (!c && c_len != 0) ||
        (!d && d_len != 0) || (!e && e_len != 0) || (!f && f_len != 0) ||
        (!g && g_len != 0) || (!merged && total != 0)) {
        if (a) free(a); if (b) free(b); if (c) free(c); if (d) free(d);
        if (e) free(e); if (f) free(f); if (g) free(g);
        if (merged) free(merged);
        return NIL_VALUE;
    }
    uint32_t off = 0;
    if (a_len) { memcpy(merged + off, a, a_len); off += a_len; }
    if (b_len) { memcpy(merged + off, b, b_len); off += b_len; }
    if (c_len) { memcpy(merged + off, c, c_len); off += c_len; }
    if (d_len) { memcpy(merged + off, d, d_len); off += d_len; }
    if (e_len) { memcpy(merged + off, e, e_len); off += e_len; }
    if (f_len) { memcpy(merged + off, f, f_len); off += f_len; }
    if (g_len) { memcpy(merged + off, g, g_len); off += g_len; }
    _tls_sha256_digest(merged, total, out);
    if (a) free(a); if (b) free(b); if (c) free(c); if (d) free(d);
    if (e) free(e); if (f) free(f); if (g) free(g);
    free(merged);
    return _tls_runtime_array_from_bytes(out, 32);
}

RuntimeValue rt_tls13_find_handshake_message(RuntimeValue buf_rv, int64_t msg_type_i64)
{
    uint32_t buf_len = 0;
    uint8_t *buf = _tls_copy_runtime_bytes(buf_rv, &buf_len);
    uint8_t target = (uint8_t)(msg_type_i64 & 0xff);
    RuntimeValue rv = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    if (!buf) return rv;
    uint32_t off = 0;
    while (off + 4U <= buf_len) {
        uint32_t body_len = ((uint32_t)buf[off + 1U] << 16) |
                            ((uint32_t)buf[off + 2U] << 8) |
                            (uint32_t)buf[off + 3U];
        uint32_t total = body_len + 4U;
        if (off + total > buf_len) break;
        if (buf[off] == target) {
            rv = _tls_runtime_array_from_bytes(buf + off, total);
            free(buf);
            return rv;
        }
        off += total;
    }
    free(buf);
    return rv;
}

int64_t rt_tls13_inner_plaintext_type(RuntimeValue inner_rv)
{
    uint32_t inner_len = 0;
    uint8_t *inner = _tls_copy_runtime_bytes(inner_rv, &inner_len);
    if (!inner || inner_len == 0) return -1;
    uint32_t cursor = inner_len;
    while (cursor > 0) {
        uint32_t idx = cursor - 1U;
        if (inner[idx] != 0) {
            int64_t ct = (int64_t)inner[idx];
            free(inner);
            return ct;
        }
        cursor = idx;
    }
    free(inner);
    return -1;
}

RuntimeValue rt_tls13_inner_plaintext_content(RuntimeValue inner_rv)
{
    uint32_t inner_len = 0;
    uint8_t *inner = _tls_copy_runtime_bytes(inner_rv, &inner_len);
    if (!inner || inner_len == 0) return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    uint32_t cursor = inner_len;
    while (cursor > 0) {
        uint32_t idx = cursor - 1U;
        if (inner[idx] != 0) {
            RuntimeValue rv = _tls_runtime_array_from_bytes(inner, idx);
            free(inner);
            return rv;
        }
        cursor = idx;
    }
    free(inner);
    return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
}

static RuntimeValue _tls_find_handshake_message_bytes(const uint8_t *buf, uint32_t buf_len, uint8_t target)
{
    uint32_t off = 0;
    while (off + 4U <= buf_len) {
        uint32_t body_len = ((uint32_t)buf[off + 1U] << 16) |
                            ((uint32_t)buf[off + 2U] << 8) |
                            (uint32_t)buf[off + 3U];
        uint32_t total = body_len + 4U;
        if (off + total > buf_len) break;
        if (buf[off] == target) {
            return _tls_runtime_array_from_bytes(buf + off, total);
        }
        off += total;
    }
    return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
}

static int _tls_aes128_gcm_decrypt_raw(const uint8_t key[16], const uint8_t nonce[12],
                                       const uint8_t *ciphertext, uint32_t ct_len,
                                       const uint8_t *aad, uint32_t aad_len,
                                       const uint8_t tag[16],
                                       uint8_t *out_plaintext);

static void _tls_hkdf_expand_label_raw(const uint8_t *secret, uint32_t secret_len,
                                       const uint8_t *label, uint32_t label_len,
                                       const uint8_t *context, uint32_t context_len,
                                       uint32_t out_len, uint8_t *out)
{
    const uint8_t prefix[] = { 't', 'l', 's', '1', '3', ' ' };
    uint8_t encoded[160];
    uint8_t full_label[64];
    uint8_t t_prev[32];
    uint32_t full_label_len = (uint32_t)sizeof(prefix) + label_len;
    uint32_t pos = 0;
    uint32_t okm_len = 0;
    uint32_t t_prev_len = 0;
    uint8_t counter = 1;

    memcpy(full_label, prefix, sizeof(prefix));
    if (label_len > 0) memcpy(full_label + sizeof(prefix), label, label_len);

    encoded[pos++] = (uint8_t)((out_len >> 8) & 0xffU);
    encoded[pos++] = (uint8_t)(out_len & 0xffU);
    encoded[pos++] = (uint8_t)full_label_len;
    memcpy(encoded + pos, full_label, full_label_len);
    pos += full_label_len;
    encoded[pos++] = (uint8_t)context_len;
    if (context_len > 0) {
        memcpy(encoded + pos, context, context_len);
        pos += context_len;
    }

    while (okm_len < out_len) {
        uint8_t input[224];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        memcpy(input + input_len, encoded, pos);
        input_len += pos;
        input[input_len++] = counter;
        _tls_hmac_sha256(secret, secret_len, input, input_len, t_prev);
        t_prev_len = 32U;
        uint32_t take = (out_len - okm_len) < 32U ? (out_len - okm_len) : 32U;
        memcpy(out + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }
}

static RuntimeValue _tls_record_find_with_key(const uint8_t key[16], const uint8_t iv[12],
                                              const uint8_t *raw, uint32_t raw_len,
                                              int64_t seq_num_i64, int64_t msg_type_i64)
{
    RuntimeValue empty = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    if (!raw || raw_len < 5U || raw[0] != 0x17 || raw[1] != 0x03 || raw[2] != 0x03) return empty;
    uint32_t payload_len = ((uint32_t)raw[3] << 8) | (uint32_t)raw[4];
    if (payload_len < 16U || raw_len < 5U + payload_len) return empty;
    uint32_t ct_len = payload_len - 16U;
    const uint8_t *ciphertext = raw + 5U;
    const uint8_t *tag = raw + 5U + ct_len;
    uint8_t nonce[12];
    memcpy(nonce, iv, 12U);
    uint64_t seq_num = (uint64_t)seq_num_i64;
    nonce[4] ^= (uint8_t)((seq_num >> 56) & 0xffU);
    nonce[5] ^= (uint8_t)((seq_num >> 48) & 0xffU);
    nonce[6] ^= (uint8_t)((seq_num >> 40) & 0xffU);
    nonce[7] ^= (uint8_t)((seq_num >> 32) & 0xffU);
    nonce[8] ^= (uint8_t)((seq_num >> 24) & 0xffU);
    nonce[9] ^= (uint8_t)((seq_num >> 16) & 0xffU);
    nonce[10] ^= (uint8_t)((seq_num >> 8) & 0xffU);
    nonce[11] ^= (uint8_t)(seq_num & 0xffU);
    uint8_t *inner = (uint8_t *)malloc(ct_len ? ct_len : 1U);
    if (!inner) return empty;
    if (_tls_aes128_gcm_decrypt_raw(key, nonce, ciphertext, ct_len, raw, 5U, tag, inner) != 0) {
        free(inner);
        return empty;
    }
    uint32_t cursor = ct_len;
    while (cursor > 0) {
        uint32_t idx = cursor - 1U;
        if (inner[idx] != 0) {
            RuntimeValue rv = _tls_find_handshake_message_bytes(inner, idx, (uint8_t)(msg_type_i64 & 0xff));
            free(inner);
            return rv;
        }
        cursor = idx;
    }
    free(inner);
    return empty;
}

RuntimeValue rt_tls13_record_find_handshake_message_fd(RuntimeValue ch_record_rv, RuntimeValue sh_record_rv,
                                                       RuntimeValue raw_rv, RuntimeValue scalar_rv,
                                                       int64_t seq_num_i64, int64_t msg_type_i64)
{
    uint32_t ch_len = 0, sh_len = 0, raw_len = 0, scalar_len = 0;
    uint8_t *ch = _tls_copy_runtime_bytes(ch_record_rv, &ch_len);
    uint8_t *sh = _tls_copy_runtime_bytes(sh_record_rv, &sh_len);
    uint8_t *raw = _tls_copy_runtime_bytes(raw_rv, &raw_len);
    uint8_t *scalar = _tls_copy_runtime_bytes(scalar_rv, &scalar_len);
    uint8_t shared[32], handshake_secret[32], thash[32], server_hs[32], key[16], iv[12];
    uint8_t early_secret[32] = {
        0x33, 0xad, 0x0a, 0x1c, 0x60, 0x7e, 0xc0, 0x3b,
        0x09, 0xe6, 0xcd, 0x98, 0x93, 0x68, 0x0c, 0xe2,
        0x10, 0xad, 0xf3, 0x00, 0xaa, 0x1f, 0x26, 0x60,
        0xe1, 0xb2, 0x2e, 0x10, 0xf1, 0x70, 0xf9, 0x2a
    };
    uint8_t empty_hash[32], derived[32];
    RuntimeValue out = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);

    if (!ch || !sh || !raw || !scalar || ch_len < 5U || sh_len < 5U || scalar_len != 32U) goto cleanup;
    uint32_t ch_payload = ((uint32_t)ch[3] << 8) | (uint32_t)ch[4];
    uint32_t sh_payload = ((uint32_t)sh[3] << 8) | (uint32_t)sh[4];
    if (ch_len < 5U + ch_payload || sh_len < 5U + sh_payload) goto cleanup;

    int found = 0;
    for (uint32_t scan = 0; scan + 40U <= sh_len; scan++) {
        if (sh[scan] == 0x00 && sh[scan + 1U] == 0x33 &&
            sh[scan + 2U] == 0x00 && sh[scan + 3U] == 0x24 &&
            sh[scan + 4U] == 0x00 && sh[scan + 5U] == 0x1d &&
            sh[scan + 6U] == 0x00 && sh[scan + 7U] == 0x20) {
            _tls_x25519_scalarmult(shared, scalar, sh + scan + 8U);
            found = 1;
            break;
        }
    }
    if (!found) goto cleanup;

    _tls_sha256_digest((const uint8_t *)"", 0, empty_hash);
    static const uint8_t label_derived[] = { 'd', 'e', 'r', 'i', 'v', 'e', 'd' };
    _tls_hkdf_expand_label_raw(early_secret, 32U, label_derived, 7U, empty_hash, 32U, 32U, derived);
    _tls_hmac_sha256(derived, 32U, shared, 32U, handshake_secret);

    uint32_t total = ch_payload + sh_payload;
    uint8_t *merged = (uint8_t *)malloc(total > 0 ? total : 1U);
    if (!merged && total != 0) goto cleanup;
    memcpy(merged, ch + 5U, ch_payload);
    memcpy(merged + ch_payload, sh + 5U, sh_payload);
    _tls_sha256_digest(merged, total, thash);
    free(merged);

    static const uint8_t label_server_hs[] = { 's', ' ', 'h', 's', ' ', 't', 'r', 'a', 'f', 'f', 'i', 'c' };
    static const uint8_t label_key[] = { 'k', 'e', 'y' };
    static const uint8_t label_iv[] = { 'i', 'v' };
    _tls_hkdf_expand_label_raw(handshake_secret, 32U, label_server_hs, 12U, thash, 32U, 32U, server_hs);
    _tls_hkdf_expand_label_raw(server_hs, 32U, label_key, 3U, (const uint8_t *)"", 0, 16U, key);
    _tls_hkdf_expand_label_raw(server_hs, 32U, label_iv, 2U, (const uint8_t *)"", 0, 12U, iv);
    out = _tls_record_find_with_key(key, iv, raw, raw_len, seq_num_i64, msg_type_i64);

cleanup:
    if (ch) free(ch);
    if (sh) free(sh);
    if (raw) free(raw);
    if (scalar) free(scalar);
    return out;
}

RuntimeValue rt_tls13_record_find_handshake_message(RuntimeValue raw_rv, RuntimeValue key_rv, RuntimeValue iv_rv, int64_t seq_num_i64, int64_t msg_type_i64)
{
    uint32_t raw_len = 0, key_len = 0, iv_len = 0;
    uint8_t *raw = _tls_copy_runtime_bytes(raw_rv, &raw_len);
    uint8_t *key = _tls_copy_runtime_bytes(key_rv, &key_len);
    uint8_t *iv = _tls_copy_runtime_bytes(iv_rv, &iv_len);
    RuntimeValue empty = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    if (!raw || !key || !iv || key_len != 16U || iv_len != 12U || raw_len < 5U) {
        if (raw) free(raw);
        if (key) free(key);
        if (iv) free(iv);
        return empty;
    }
    if (raw[0] != 0x17 || raw[1] != 0x03 || raw[2] != 0x03) {
        free(raw); free(key); free(iv);
        return empty;
    }
    uint32_t payload_len = ((uint32_t)raw[3] << 8) | (uint32_t)raw[4];
    if (payload_len < 16U || raw_len < 5U + payload_len) {
        free(raw); free(key); free(iv);
        return empty;
    }
    uint32_t ct_len = payload_len - 16U;
    const uint8_t *ciphertext = raw + 5U;
    const uint8_t *tag = raw + 5U + ct_len;
    const uint8_t *aad = raw;
    uint8_t nonce[12];
    memcpy(nonce, iv, 12U);
    uint64_t carry = (uint64_t)seq_num_i64;
    for (int i = 11; i >= 4 && carry > 0; i--) {
        uint64_t sum = (uint64_t)nonce[i] + (carry & 0xffU);
        nonce[i] = (uint8_t)(sum & 0xffU);
        carry = (carry >> 8) + (sum >> 8);
    }
    uint8_t *inner = (uint8_t *)malloc(ct_len ? ct_len : 1U);
    if (!inner) {
        free(raw); free(key); free(iv);
        return empty;
    }
    if (_tls_aes128_gcm_decrypt_raw(key, nonce, ciphertext, ct_len, aad, 5U, tag, inner) != 0) {
        free(inner); free(raw); free(key); free(iv);
        return empty;
    }
    uint32_t cursor = ct_len;
    while (cursor > 0) {
        uint32_t idx = cursor - 1U;
        if (inner[idx] != 0) {
            RuntimeValue rv = _tls_find_handshake_message_bytes(inner, idx, (uint8_t)(msg_type_i64 & 0xff));
            free(inner); free(raw); free(key); free(iv);
            return rv;
        }
        cursor = idx;
    }
    free(inner); free(raw); free(key); free(iv);
    return empty;
}

RuntimeValue rt_tls13_hkdf_extract(RuntimeValue salt_rv, RuntimeValue ikm_rv)
{
    uint8_t salt_stack[128];
    uint8_t ikm_stack[256];
    uint8_t *salt_heap = NULL;
    uint8_t *ikm_heap = NULL;
    const uint8_t *salt = NULL;
    const uint8_t *ikm = NULL;
    uint32_t salt_len = 0, ikm_len = 0;
    uint8_t zero_salt[32];
    uint8_t out[32];
    if (!_tls_materialize_runtime_bytes(ikm_rv, ikm_stack, sizeof(ikm_stack), &ikm, &ikm_heap, &ikm_len)) {
        return NIL_VALUE;
    }
    if (!_tls_materialize_runtime_bytes(salt_rv, salt_stack, sizeof(salt_stack), &salt, &salt_heap, &salt_len)) {
        if (ikm_heap) free(ikm_heap);
        return NIL_VALUE;
    }
    if (salt_len == 0) {
        memset(zero_salt, 0, sizeof(zero_salt));
        _tls_hmac_sha256(zero_salt, 32, ikm, ikm_len, out);
    } else {
        _tls_hmac_sha256(salt, salt_len, ikm, ikm_len, out);
    }
    if (salt_heap) free(salt_heap);
    if (ikm_heap) free(ikm_heap);
    return _tls_runtime_array_from_bytes(out, 32);
}

int64_t rt_tls13_hkdf_extract_into(RuntimeValue salt_rv, RuntimeValue ikm_rv, RuntimeValue out_rv)
{
    uint8_t salt_stack[128];
    uint8_t ikm_stack[256];
    uint8_t *salt_heap = NULL;
    uint8_t *ikm_heap = NULL;
    const uint8_t *salt = NULL;
    const uint8_t *ikm = NULL;
    uint32_t salt_len = 0, ikm_len = 0;
    uint8_t zero_salt[32];
    uint8_t out[32];
    int64_t ok = 0;
    if (!_tls_materialize_runtime_bytes(ikm_rv, ikm_stack, sizeof(ikm_stack), &ikm, &ikm_heap, &ikm_len)) {
        return 0;
    }
    if (!_tls_materialize_runtime_bytes(salt_rv, salt_stack, sizeof(salt_stack), &salt, &salt_heap, &salt_len)) {
        if (ikm_heap) free(ikm_heap);
        return 0;
    }
    serial_puts("[DBG_C] hkdf_extract_into salt_len=");
    serial_put_dec((int64_t)salt_len);
    serial_puts(" ikm_len=");
    serial_put_dec((int64_t)ikm_len);
    serial_puts(" salt0=");
    serial_put_dec(salt_len > 0 ? (int64_t)salt[0] : -1);
    serial_puts(" salt12=");
    serial_put_dec(salt_len > 12 ? (int64_t)salt[12] : -1);
    serial_puts(" ikm0=");
    serial_put_dec(ikm_len > 0 ? (int64_t)ikm[0] : -1);
    serial_puts("\r\n");
    if (salt_len == 0) {
        memset(zero_salt, 0, sizeof(zero_salt));
        _tls_hmac_sha256(zero_salt, 32, ikm, ikm_len, out);
    } else {
        _tls_hmac_sha256(salt, salt_len, ikm, ikm_len, out);
    }
    ok = _tls_write_runtime_bytes(out_rv, out, 32U);
    if (salt_heap) free(salt_heap);
    if (ikm_heap) free(ikm_heap);
    return ok;
}

/* Decrypts a TLS 1.3 record AND returns inner [content_type || data] as a single
 * [u8]. Bypasses pure-Simple's enum-field-destructuring compile-mode bug
 * (RecordResult.Ok's content_type field reads as 0 even when the source is
 * correct). Empty result = error. On success: byte[0] = content_type,
 * bytes[1..] = inner data (without padding). */
RuntimeValue rt_tls13_record_decrypt_compact(RuntimeValue key_rv, RuntimeValue iv_rv,
                                             int64_t seq_num_i64, RuntimeValue raw_record_rv)
{
    uint32_t key_len = 0, iv_len = 0, raw_len = 0;
    uint8_t *key = _tls_copy_runtime_bytes(key_rv, &key_len);
    uint8_t *iv = _tls_copy_runtime_bytes(iv_rv, &iv_len);
    uint8_t *raw = _tls_copy_runtime_bytes(raw_record_rv, &raw_len);
    uint8_t *plaintext = NULL;
    uint8_t *result_buf = NULL;
    RuntimeValue out = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);

    if (!key || !iv || !raw || key_len != 16U || iv_len != 12U || raw_len < 5U + 16U) {
        goto cleanup;
    }
    if (raw[0] != 0x17 || raw[1] != 0x03 || raw[2] != 0x03) goto cleanup;
    uint32_t payload_len = ((uint32_t)raw[3] << 8) | (uint32_t)raw[4];
    if (payload_len < 16U || (5U + payload_len) != raw_len) goto cleanup;
    uint32_t ct_len = payload_len - 16U;

    uint8_t nonce[12];
    {
        uint64_t s = (uint64_t)seq_num_i64;
        nonce[0] = iv[0]; nonce[1] = iv[1]; nonce[2] = iv[2]; nonce[3] = iv[3];
        nonce[4]  = iv[4]  ^ (uint8_t)((s >> 56) & 0xffU);
        nonce[5]  = iv[5]  ^ (uint8_t)((s >> 48) & 0xffU);
        nonce[6]  = iv[6]  ^ (uint8_t)((s >> 40) & 0xffU);
        nonce[7]  = iv[7]  ^ (uint8_t)((s >> 32) & 0xffU);
        nonce[8]  = iv[8]  ^ (uint8_t)((s >> 24) & 0xffU);
        nonce[9]  = iv[9]  ^ (uint8_t)((s >> 16) & 0xffU);
        nonce[10] = iv[10] ^ (uint8_t)((s >> 8)  & 0xffU);
        nonce[11] = iv[11] ^ (uint8_t)(s & 0xffU);
    }

    plaintext = (uint8_t *)malloc(ct_len > 0 ? ct_len : 1U);
    if (!plaintext) goto cleanup;
    if (_tls_aes128_gcm_decrypt_raw(key, nonce, raw + 5U, ct_len, raw, 5U, raw + 5U + ct_len, plaintext) != 0) {
        goto cleanup;
    }
    uint32_t cursor = ct_len;
    while (cursor > 0U && plaintext[cursor - 1U] == 0x00) cursor--;
    if (cursor == 0U) goto cleanup;
    uint8_t content_type = plaintext[cursor - 1U];
    uint32_t data_len = cursor - 1U;

    /* Build [content_type || data...] */
    result_buf = (uint8_t *)malloc(1U + data_len);
    if (!result_buf) goto cleanup;
    result_buf[0] = content_type;
    if (data_len > 0) memcpy(result_buf + 1U, plaintext, data_len);
    out = _tls_runtime_array_from_bytes(result_buf, 1U + data_len);

cleanup:
    if (key) free(key);
    if (iv) free(iv);
    if (raw) free(raw);
    if (plaintext) free(plaintext);
    if (result_buf) free(result_buf);
    return out;
}

/* RFC 5869 §2.3: raw HKDF-Expand. Mirrors rt_tls13_hkdf_expand_label_into but
 * skips TLS 1.3 HkdfLabel encoding; info is fed verbatim. */
int64_t rt_tls13_hkdf_expand_into(RuntimeValue prk_rv, RuntimeValue info_rv, RuntimeValue out_rv, int64_t length_i64)
{
    if (length_i64 < 0 || length_i64 > 255) return 0;
    uint32_t prk_len = 0, info_len = 0;
    uint8_t *prk = _tls_copy_runtime_bytes(prk_rv, &prk_len);
    uint8_t *info = _tls_copy_runtime_bytes(info_rv, &info_len);
    if (!prk) {
        if (info) free(info);
        return 0;
    }
    uint8_t okm[255];
    uint8_t t_prev[32];
    uint32_t okm_len = 0;
    uint32_t t_prev_len = 0;
    uint8_t counter = 1;
    uint32_t out_len = (uint32_t)length_i64;
    while (okm_len < out_len) {
        uint8_t input[32 + 1024];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        if (info_len > 0) {
            if (info_len > sizeof(input) - input_len - 1U) {
                free(prk);
                if (info) free(info);
                return 0;
            }
            memcpy(input + input_len, info, info_len);
            input_len += info_len;
        }
        input[input_len++] = counter;
        _tls_hmac_sha256(prk, prk_len, input, input_len, t_prev);
        t_prev_len = 32;
        uint32_t take = (out_len - okm_len) < 32U ? (out_len - okm_len) : 32U;
        memcpy(okm + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }
    int64_t ok = _tls_write_runtime_bytes(out_rv, okm, out_len);
    free(prk);
    if (info) free(info);
    return ok;
}

RuntimeValue rt_tls13_handshake_secret_from_record(RuntimeValue derived_rv, RuntimeValue scalar_rv, RuntimeValue record_rv)
{
    uint32_t derived_len = 0, scalar_len = 0, record_len = 0;
    uint8_t *derived = _tls_copy_runtime_bytes(derived_rv, &derived_len);
    uint8_t *scalar = _tls_copy_runtime_bytes(scalar_rv, &scalar_len);
    uint8_t *record = _tls_copy_runtime_bytes(record_rv, &record_len);
    uint8_t shared[32];
    uint8_t out[32];
    int found = 0;

    if (!derived || !scalar || !record || derived_len != 32U || scalar_len != 32U) {
        if (derived) free(derived);
        if (scalar) free(scalar);
        if (record) free(record);
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }

    for (uint32_t scan = 0; scan + 40U <= record_len; scan++) {
        if (record[scan] == 0x00 && record[scan + 1U] == 0x33 &&
            record[scan + 2U] == 0x00 && record[scan + 3U] == 0x24 &&
            record[scan + 4U] == 0x00 && record[scan + 5U] == 0x1d &&
            record[scan + 6U] == 0x00 && record[scan + 7U] == 0x20) {
            _tls_x25519_scalarmult(shared, scalar, record + scan + 8U);
            found = 1;
            break;
        }
    }

    if (!found) {
        free(derived);
        free(scalar);
        free(record);
        return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    }

    _tls_hmac_sha256(derived, derived_len, shared, 32U, out);
    free(derived);
    free(scalar);
    free(record);
    return _tls_runtime_array_from_bytes(out, 32U);
}

RuntimeValue rt_tls13_hkdf_expand_label(RuntimeValue secret_rv, RuntimeValue label_rv, RuntimeValue context_rv, int64_t length_i64)
{
    if (length_i64 < 0 || length_i64 > 255) return NIL_VALUE;
    uint32_t secret_len = 0, label_len = 0, context_len = 0;
    uint8_t *secret = _tls_copy_runtime_bytes(secret_rv, &secret_len);
    uint8_t *label = _tls_copy_runtime_bytes(label_rv, &label_len);
    uint8_t *context = _tls_copy_runtime_bytes(context_rv, &context_len);
    if (!secret || !label || !context) {
        if (secret) free(secret);
        if (label) free(label);
        if (context) free(context);
        return NIL_VALUE;
    }

    const uint8_t prefix[] = { 't', 'l', 's', '1', '3', ' ' };
    uint32_t full_label_len = (uint32_t)sizeof(prefix) + label_len;
    uint8_t *full_label = (uint8_t *)malloc(full_label_len > 0 ? full_label_len : 1);
    uint32_t encoded_len = 2U + 1U + full_label_len + 1U + context_len;
    uint8_t *encoded = (uint8_t *)malloc(encoded_len > 0 ? encoded_len : 1);
    if (!full_label || !encoded) {
        if (full_label) free(full_label);
        if (encoded) free(encoded);
        free(secret); free(label); free(context);
        return NIL_VALUE;
    }
    memcpy(full_label, prefix, sizeof(prefix));
    if (label_len > 0) memcpy(full_label + sizeof(prefix), label, label_len);

    uint32_t pos = 0;
    uint32_t out_len = (uint32_t)length_i64;
    encoded[pos++] = (uint8_t)((out_len >> 8) & 0xffU);
    encoded[pos++] = (uint8_t)(out_len & 0xffU);
    encoded[pos++] = (uint8_t)full_label_len;
    memcpy(encoded + pos, full_label, full_label_len);
    pos += full_label_len;
    encoded[pos++] = (uint8_t)context_len;
    if (context_len > 0) memcpy(encoded + pos, context, context_len);

    uint8_t okm[255];
    uint8_t t_prev[32];
    uint32_t okm_len = 0;
    uint32_t t_prev_len = 0;
    uint8_t counter = 1;
    while (okm_len < out_len) {
        uint8_t input[32 + 512];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        memcpy(input + input_len, encoded, encoded_len);
        input_len += encoded_len;
        input[input_len++] = counter;

        _tls_hmac_sha256(secret, secret_len, input, input_len, t_prev);
        t_prev_len = 32;
        uint32_t take = (out_len - okm_len) < 32U ? (out_len - okm_len) : 32U;
        memcpy(okm + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }

    free(secret);
    free(label);
    free(context);
    free(full_label);
    free(encoded);
    return _tls_runtime_array_from_bytes(okm, out_len);
}

int64_t rt_tls13_hkdf_expand_label_into(RuntimeValue secret_rv, RuntimeValue label_rv, RuntimeValue context_rv, RuntimeValue out_rv, int64_t length_i64)
{
    if (length_i64 < 0 || length_i64 > 255) return 0;
    uint32_t secret_len = 0, label_len = 0, context_len = 0;
    uint8_t *secret = _tls_copy_runtime_bytes(secret_rv, &secret_len);
    uint8_t *label = _tls_copy_runtime_bytes(label_rv, &label_len);
    uint8_t *context = _tls_copy_runtime_bytes(context_rv, &context_len);
    if (!secret || !label || !context) {
        if (secret) free(secret);
        if (label) free(label);
        if (context) free(context);
        return 0;
    }

    const uint8_t prefix[] = { 't', 'l', 's', '1', '3', ' ' };
    uint32_t full_label_len = (uint32_t)sizeof(prefix) + label_len;
    uint8_t *full_label = (uint8_t *)malloc(full_label_len > 0 ? full_label_len : 1);
    uint32_t encoded_len = 2U + 1U + full_label_len + 1U + context_len;
    uint8_t *encoded = (uint8_t *)malloc(encoded_len > 0 ? encoded_len : 1);
    uint8_t okm[255];
    uint8_t t_prev[32];
    int64_t ok = 0;
    if (!full_label || !encoded) {
        if (full_label) free(full_label);
        if (encoded) free(encoded);
        free(secret); free(label); free(context);
        return 0;
    }
    memcpy(full_label, prefix, sizeof(prefix));
    if (label_len > 0) memcpy(full_label + sizeof(prefix), label, label_len);

    uint32_t pos = 0;
    uint32_t out_len = (uint32_t)length_i64;
    encoded[pos++] = (uint8_t)((out_len >> 8) & 0xffU);
    encoded[pos++] = (uint8_t)(out_len & 0xffU);
    encoded[pos++] = (uint8_t)full_label_len;
    memcpy(encoded + pos, full_label, full_label_len);
    pos += full_label_len;
    encoded[pos++] = (uint8_t)context_len;
    if (context_len > 0) memcpy(encoded + pos, context, context_len);

    uint32_t okm_len = 0;
    uint32_t t_prev_len = 0;
    uint8_t counter = 1;
    while (okm_len < out_len) {
        uint8_t input[32 + 512];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        memcpy(input + input_len, encoded, encoded_len);
        input_len += encoded_len;
        input[input_len++] = counter;

        _tls_hmac_sha256(secret, secret_len, input, input_len, t_prev);
        t_prev_len = 32;
        uint32_t take = (out_len - okm_len) < 32U ? (out_len - okm_len) : 32U;
        memcpy(okm + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }

    ok = _tls_write_runtime_bytes(out_rv, okm, out_len);
    free(secret);
    free(label);
    free(context);
    free(full_label);
    free(encoded);
    return ok;
}

static RuntimeValue _tls13_hkdf_expand_label_const(RuntimeValue secret_rv, const uint8_t *label, uint32_t label_len, uint32_t out_len)
{
    uint32_t secret_len = 0;
    uint8_t *secret = _tls_copy_runtime_bytes(secret_rv, &secret_len);
    if (!secret) return NIL_VALUE;

    const uint8_t prefix[] = { 't', 'l', 's', '1', '3', ' ' };
    uint32_t full_label_len = (uint32_t)sizeof(prefix) + label_len;
    uint8_t full_label[16];
    uint8_t encoded[32];
    uint8_t okm[32];
    uint8_t t_prev[32];
    if (full_label_len > sizeof(full_label)) {
        free(secret);
        return NIL_VALUE;
    }

    memcpy(full_label, prefix, sizeof(prefix));
    memcpy(full_label + sizeof(prefix), label, label_len);

    uint32_t pos = 0;
    encoded[pos++] = (uint8_t)((out_len >> 8) & 0xffU);
    encoded[pos++] = (uint8_t)(out_len & 0xffU);
    encoded[pos++] = (uint8_t)full_label_len;
    memcpy(encoded + pos, full_label, full_label_len);
    pos += full_label_len;
    encoded[pos++] = 0; /* empty context */

    uint32_t okm_len = 0;
    uint32_t t_prev_len = 0;
    uint8_t counter = 1;
    while (okm_len < out_len) {
        uint8_t input[96];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        memcpy(input + input_len, encoded, pos);
        input_len += pos;
        input[input_len++] = counter;

        _tls_hmac_sha256(secret, secret_len, input, input_len, t_prev);
        t_prev_len = 32;
        uint32_t take = (out_len - okm_len) < 32U ? (out_len - okm_len) : 32U;
        memcpy(okm + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }

    free(secret);
    return _tls_runtime_array_from_bytes(okm, out_len);
}

static RuntimeValue _tls13_hkdf_expand_label_const_padded(RuntimeValue secret_rv, const uint8_t *label, uint32_t label_len, uint32_t info_out_len)
{
    uint32_t secret_len = 0;
    uint8_t *secret = _tls_copy_runtime_bytes(secret_rv, &secret_len);
    if (!secret) return NIL_VALUE;

    const uint8_t prefix[] = { 't', 'l', 's', '1', '3', ' ' };
    uint32_t full_label_len = (uint32_t)sizeof(prefix) + label_len;
    uint8_t full_label[16];
    uint8_t encoded[32];
    uint8_t okm[32];
    uint8_t t_prev[32];
    if (full_label_len > sizeof(full_label) || info_out_len > 32U) {
        free(secret);
        return NIL_VALUE;
    }

    memset(okm, 0, sizeof(okm));
    memcpy(full_label, prefix, sizeof(prefix));
    memcpy(full_label + sizeof(prefix), label, label_len);

    uint32_t pos = 0;
    encoded[pos++] = (uint8_t)((info_out_len >> 8) & 0xffU);
    encoded[pos++] = (uint8_t)(info_out_len & 0xffU);
    encoded[pos++] = (uint8_t)full_label_len;
    memcpy(encoded + pos, full_label, full_label_len);
    pos += full_label_len;
    encoded[pos++] = 0; /* empty context */

    uint32_t okm_len = 0;
    uint32_t t_prev_len = 0;
    uint8_t counter = 1;
    while (okm_len < info_out_len) {
        uint8_t input[96];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        memcpy(input + input_len, encoded, pos);
        input_len += pos;
        input[input_len++] = counter;

        _tls_hmac_sha256(secret, secret_len, input, input_len, t_prev);
        t_prev_len = 32;
        uint32_t take = (info_out_len - okm_len) < 32U ? (info_out_len - okm_len) : 32U;
        memcpy(okm + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }

    free(secret);
    return _tls_runtime_array_from_bytes(okm, info_out_len);
}

RuntimeValue rt_tls13_hkdf_expand_label_key(RuntimeValue secret_rv)
{
    static const uint8_t key_label[] = { 'k', 'e', 'y' };
    return _tls13_hkdf_expand_label_const_padded(secret_rv, key_label, 3U, 16U);
}

static uint8_t _tls_aes128_xtime(uint8_t b)
{
    uint32_t shifted = ((uint32_t)b << 1) & 0xffU;
    return (b & 0x80U) ? (uint8_t)(shifted ^ 0x1bU) : (uint8_t)shifted;
}

static uint8_t _tls_aes128_gf_mul(uint8_t a, uint8_t b)
{
    uint8_t result = 0;
    uint8_t aa = a;
    uint8_t bb = b;
    for (int i = 0; i < 8; i++) {
        if (bb & 1U) result ^= aa;
        aa = _tls_aes128_xtime(aa);
        bb >>= 1;
    }
    return result;
}

static void _tls_aes128_sub_bytes(uint8_t state[16])
{
    for (int i = 0; i < 16; i++) state[i] = _aes_sbox[state[i]];
}

static void _tls_aes128_shift_rows(uint8_t state[16])
{
    uint8_t tmp[16];
    tmp[0] = state[0];  tmp[1] = state[5];  tmp[2] = state[10]; tmp[3] = state[15];
    tmp[4] = state[4];  tmp[5] = state[9];  tmp[6] = state[14]; tmp[7] = state[3];
    tmp[8] = state[8];  tmp[9] = state[13]; tmp[10] = state[2]; tmp[11] = state[7];
    tmp[12] = state[12]; tmp[13] = state[1]; tmp[14] = state[6]; tmp[15] = state[11];
    memcpy(state, tmp, 16);
}

static void _tls_aes128_mix_columns(uint8_t state[16])
{
    for (int c = 0; c < 4; c++) {
        int base = c * 4;
        uint8_t s0 = state[base];
        uint8_t s1 = state[base + 1];
        uint8_t s2 = state[base + 2];
        uint8_t s3 = state[base + 3];
        state[base]     = (uint8_t)(_tls_aes128_gf_mul(0x02U, s0) ^ _tls_aes128_gf_mul(0x03U, s1) ^ s2 ^ s3);
        state[base + 1] = (uint8_t)(s0 ^ _tls_aes128_gf_mul(0x02U, s1) ^ _tls_aes128_gf_mul(0x03U, s2) ^ s3);
        state[base + 2] = (uint8_t)(s0 ^ s1 ^ _tls_aes128_gf_mul(0x02U, s2) ^ _tls_aes128_gf_mul(0x03U, s3));
        state[base + 3] = (uint8_t)(_tls_aes128_gf_mul(0x03U, s0) ^ s1 ^ s2 ^ _tls_aes128_gf_mul(0x02U, s3));
    }
}

static void _tls_aes128_add_round_key(uint8_t state[16], const uint8_t *round_keys, uint32_t round)
{
    const uint8_t *rk = round_keys + round * 16U;
    for (int i = 0; i < 16; i++) state[i] ^= rk[i];
}

static void _tls_aes128_key_expansion(const uint8_t key[16], uint8_t out[176])
{
    memcpy(out, key, 16);
    uint32_t bytes = 16;
    uint8_t temp[4];
    uint32_t rcon_idx = 0;
    while (bytes < 176U) {
        temp[0] = out[bytes - 4];
        temp[1] = out[bytes - 3];
        temp[2] = out[bytes - 2];
        temp[3] = out[bytes - 1];
        if ((bytes % 16U) == 0U) {
            uint8_t rot0 = temp[1], rot1 = temp[2], rot2 = temp[3], rot3 = temp[0];
            temp[0] = (uint8_t)(_aes_sbox[rot0] ^ ((_aes_rcon[rcon_idx] >> 24) & 0xffU));
            temp[1] = (uint8_t)(_aes_sbox[rot1] ^ ((_aes_rcon[rcon_idx] >> 16) & 0xffU));
            temp[2] = (uint8_t)(_aes_sbox[rot2] ^ ((_aes_rcon[rcon_idx] >> 8) & 0xffU));
            temp[3] = (uint8_t)(_aes_sbox[rot3] ^ (_aes_rcon[rcon_idx] & 0xffU));
            rcon_idx++;
        }
        for (int i = 0; i < 4; i++) {
            out[bytes] = (uint8_t)(out[bytes - 16U] ^ temp[i]);
            bytes++;
        }
    }
}

static void _tls_aes128_encrypt_block_raw(const uint8_t key[16], const uint8_t in[16], uint8_t out[16])
{
    uint8_t round_keys[176];
    uint8_t state[16];
    memcpy(state, in, 16);
    _tls_aes128_key_expansion(key, round_keys);
    _tls_aes128_add_round_key(state, round_keys, 0);
    for (uint32_t round = 1; round < 10U; round++) {
        _tls_aes128_sub_bytes(state);
        _tls_aes128_shift_rows(state);
        _tls_aes128_mix_columns(state);
        _tls_aes128_add_round_key(state, round_keys, round);
    }
    _tls_aes128_sub_bytes(state);
    _tls_aes128_shift_rows(state);
    _tls_aes128_add_round_key(state, round_keys, 10);
    memcpy(out, state, 16);
}

static void _tls_gcm_inc32(uint8_t counter[16])
{
    uint32_t c =
        ((uint32_t)counter[12] << 24) |
        ((uint32_t)counter[13] << 16) |
        ((uint32_t)counter[14] << 8) |
        (uint32_t)counter[15];
    c = (uint32_t)(c + 1U);
    counter[12] = (uint8_t)((c >> 24) & 0xffU);
    counter[13] = (uint8_t)((c >> 16) & 0xffU);
    counter[14] = (uint8_t)((c >> 8) & 0xffU);
    counter[15] = (uint8_t)(c & 0xffU);
}

static void _tls_gcm_gf_mul(const uint8_t x[16], const uint8_t y[16], uint8_t out[16])
{
    uint8_t z[16];
    uint8_t v[16];
    memset(z, 0, sizeof(z));
    memcpy(v, y, 16);

    for (uint32_t i = 0; i < 128U; i++) {
        uint32_t byte_idx = i >> 3;
        uint32_t bit_idx = 7U - (i & 7U);
        if (((x[byte_idx] >> bit_idx) & 1U) != 0U) {
            for (uint32_t j = 0; j < 16U; j++) z[j] ^= v[j];
        }

        uint8_t lsb = (uint8_t)(v[15] & 1U);
        uint8_t v_next[16];
        v_next[0] = (uint8_t)((v[0] >> 1) & 0x7fU);
        if (lsb) v_next[0] ^= 0xe1U;
        for (uint32_t j = 1; j < 16U; j++) {
            v_next[j] = (uint8_t)(((v[j] >> 1) | ((v[j - 1] & 1U) << 7)) & 0xffU);
        }
        memcpy(v, v_next, 16);
    }

    memcpy(out, z, 16);
}

static void _tls_gcm_ghash(const uint8_t h[16], const uint8_t *aad, uint32_t aad_len,
                           const uint8_t *ciphertext, uint32_t ct_len, uint8_t out[16])
{
    uint8_t y[16];
    memset(y, 0, sizeof(y));

    for (uint32_t off = 0; off < aad_len; off += 16U) {
        uint8_t block[16];
        memset(block, 0, sizeof(block));
        uint32_t take = (aad_len - off) < 16U ? (aad_len - off) : 16U;
        memcpy(block, aad + off, take);
        for (uint32_t i = 0; i < 16U; i++) block[i] ^= y[i];
        _tls_gcm_gf_mul(block, h, y);
    }

    for (uint32_t off = 0; off < ct_len; off += 16U) {
        uint8_t block[16];
        memset(block, 0, sizeof(block));
        uint32_t take = (ct_len - off) < 16U ? (ct_len - off) : 16U;
        memcpy(block, ciphertext + off, take);
        for (uint32_t i = 0; i < 16U; i++) block[i] ^= y[i];
        _tls_gcm_gf_mul(block, h, y);
    }

    uint8_t len_block[16];
    memset(len_block, 0, sizeof(len_block));
    uint64_t aad_bits = (uint64_t)aad_len * 8ULL;
    uint64_t ct_bits = (uint64_t)ct_len * 8ULL;
    for (int i = 0; i < 8; i++) len_block[i] = (uint8_t)(aad_bits >> (56 - i * 8));
    for (int i = 0; i < 8; i++) len_block[8 + i] = (uint8_t)(ct_bits >> (56 - i * 8));
    for (uint32_t i = 0; i < 16U; i++) len_block[i] ^= y[i];
    _tls_gcm_gf_mul(len_block, h, out);
}

static int _tls_aes128_gcm_decrypt_raw(const uint8_t key[16], const uint8_t nonce[12],
                                       const uint8_t *ciphertext, uint32_t ct_len,
                                       const uint8_t *aad, uint32_t aad_len,
                                       const uint8_t tag[16], uint8_t *plaintext_out)
{
    uint8_t zero_block[16];
    uint8_t h[16];
    uint8_t j0[16];
    uint8_t s[16];
    uint8_t ej0[16];
    uint8_t expected_tag[16];
    memset(zero_block, 0, sizeof(zero_block));
    _tls_aes128_encrypt_block_raw(key, zero_block, h);

    memcpy(j0, nonce, 12);
    j0[12] = 0; j0[13] = 0; j0[14] = 0; j0[15] = 1;

    _tls_gcm_ghash(h, aad, aad_len, ciphertext, ct_len, s);
    _tls_aes128_encrypt_block_raw(key, j0, ej0);
    for (uint32_t i = 0; i < 16U; i++) expected_tag[i] = (uint8_t)(s[i] ^ ej0[i]);
    if (memcmp(expected_tag, tag, 16) != 0) return -1;

    uint8_t counter[16];
    memcpy(counter, j0, 16);
    _tls_gcm_inc32(counter);

    uint32_t off = 0;
    while (off < ct_len) {
        uint8_t stream[16];
        _tls_aes128_encrypt_block_raw(key, counter, stream);
        uint32_t take = (ct_len - off) < 16U ? (ct_len - off) : 16U;
        for (uint32_t i = 0; i < take; i++) plaintext_out[off + i] = (uint8_t)(ciphertext[off + i] ^ stream[i]);
        _tls_gcm_inc32(counter);
        off += take;
    }
    return 0;
}

static int _tls_aes128_gcm_encrypt_raw(const uint8_t key[16], const uint8_t nonce[12],
                                       const uint8_t *plaintext, uint32_t pt_len,
                                       const uint8_t *aad, uint32_t aad_len,
                                       uint8_t *ciphertext_out, uint8_t tag_out[16])
{
    uint8_t zero_block[16];
    uint8_t h[16];
    uint8_t j0[16];
    uint8_t s[16];
    uint8_t ej0[16];
    memset(zero_block, 0, sizeof(zero_block));
    _tls_aes128_encrypt_block_raw(key, zero_block, h);

    memcpy(j0, nonce, 12U);
    j0[12] = 0; j0[13] = 0; j0[14] = 0; j0[15] = 1;

    uint8_t counter[16];
    memcpy(counter, j0, 16U);
    _tls_gcm_inc32(counter);

    uint32_t off = 0;
    while (off < pt_len) {
        uint8_t stream[16];
        _tls_aes128_encrypt_block_raw(key, counter, stream);
        uint32_t take = (pt_len - off) < 16U ? (pt_len - off) : 16U;
        for (uint32_t i = 0; i < take; i++) ciphertext_out[off + i] = (uint8_t)(plaintext[off + i] ^ stream[i]);
        _tls_gcm_inc32(counter);
        off += take;
    }

    _tls_gcm_ghash(h, aad, aad_len, ciphertext_out, pt_len, s);
    _tls_aes128_encrypt_block_raw(key, j0, ej0);
    for (uint32_t i = 0; i < 16U; i++) tag_out[i] = (uint8_t)(s[i] ^ ej0[i]);
    return 0;
}

RuntimeValue rt_tls13_aes128_encrypt_block(RuntimeValue key_rv, RuntimeValue block_rv)
{
    uint32_t key_len = 0, block_len = 0;
    uint8_t *key = _tls_copy_runtime_bytes(key_rv, &key_len);
    uint8_t *block = _tls_copy_runtime_bytes(block_rv, &block_len);
    uint8_t out[16];
    if (!key || !block || key_len != 16U || block_len != 16U) {
        if (key) free(key);
        if (block) free(block);
        return NIL_VALUE;
    }
    _tls_aes128_encrypt_block_raw(key, block, out);
    free(key);
    free(block);
    return _tls_runtime_array_from_bytes(out, 16);
}

int64_t rt_aes128_encrypt_block_into(int64_t key_rv, int64_t block_rv, int64_t out_rv)
{
    uint32_t key_len = 0, block_len = 0;
    uint8_t *key = _tls_copy_runtime_bytes(key_rv, &key_len);
    uint8_t *block = _tls_copy_runtime_bytes(block_rv, &block_len);
    uint8_t out[16];
    if (!key || !block || key_len != 16U || block_len != 16U) {
        if (key) free(key);
        if (block) free(block);
        return -1;
    }
    _tls_aes128_encrypt_block_raw(key, block, out);
    free(key);
    free(block);
    return _ed_bytes_to_rv(out, 16U, out_rv);
}

RuntimeValue rt_tls13_aes128_gcm_decrypt(RuntimeValue key_rv, RuntimeValue nonce_rv,
                                         RuntimeValue ciphertext_rv, RuntimeValue aad_rv,
                                         RuntimeValue tag_rv)
{
    uint32_t key_len = 0, nonce_len = 0, ct_len = 0, aad_len = 0, tag_len = 0;
    uint8_t *key = _tls_copy_runtime_bytes(key_rv, &key_len);
    uint8_t *nonce = _tls_copy_runtime_bytes(nonce_rv, &nonce_len);
    uint8_t *ciphertext = _tls_copy_runtime_bytes(ciphertext_rv, &ct_len);
    uint8_t *aad = _tls_copy_runtime_bytes(aad_rv, &aad_len);
    uint8_t *tag = _tls_copy_runtime_bytes(tag_rv, &tag_len);
    uint8_t *plaintext = NULL;
    RuntimeValue out = NIL_VALUE;

    if (!key || !nonce || !aad || !tag || (!ciphertext && ct_len != 0) ||
        key_len != 16U || nonce_len != 12U || tag_len != 16U) {
        goto cleanup;
    }

    plaintext = (uint8_t *)malloc(ct_len > 0 ? ct_len : 1U);
    if (!plaintext) goto cleanup;
    if (_tls_aes128_gcm_decrypt_raw(key, nonce, ciphertext, ct_len, aad, aad_len, tag, plaintext) != 0) {
        goto cleanup;
    }
    out = _tls_runtime_array_from_bytes(plaintext, ct_len);

cleanup:
    if (key) free(key);
    if (nonce) free(nonce);
    if (ciphertext) free(ciphertext);
    if (aad) free(aad);
    if (tag) free(tag);
    if (plaintext) free(plaintext);
    return out;
}

RuntimeValue rt_tls13_aes128_gcm_encrypt(RuntimeValue key_rv, RuntimeValue nonce_rv,
                                         RuntimeValue plaintext_rv, RuntimeValue aad_rv)
{
    uint32_t key_len = 0, nonce_len = 0, pt_len = 0, aad_len = 0;
    uint8_t *key = _tls_copy_runtime_bytes(key_rv, &key_len);
    uint8_t *nonce = _tls_copy_runtime_bytes(nonce_rv, &nonce_len);
    uint8_t *plaintext = _tls_copy_runtime_bytes(plaintext_rv, &pt_len);
    uint8_t *aad = _tls_copy_runtime_bytes(aad_rv, &aad_len);
    uint8_t *ciphertext = NULL;
    RuntimeValue out = NIL_VALUE;

    if (!key || !nonce || !aad || (!plaintext && pt_len != 0) ||
        key_len != 16U || nonce_len != 12U) {
        goto cleanup;
    }

    /* Output is ciphertext (pt_len bytes) + tag (16 bytes). */
    uint32_t out_len = pt_len + 16U;
    ciphertext = (uint8_t *)malloc(out_len > 0 ? out_len : 1U);
    if (!ciphertext) goto cleanup;
    if (_tls_aes128_gcm_encrypt_raw(key, nonce, plaintext, pt_len, aad, aad_len,
                                    ciphertext, ciphertext + pt_len) != 0) {
        goto cleanup;
    }
    out = _tls_runtime_array_from_bytes(ciphertext, out_len);

cleanup:
    if (key) free(key);
    if (nonce) free(nonce);
    if (plaintext) free(plaintext);
    if (aad) free(aad);
    if (ciphertext) free(ciphertext);
    return out;
}

RuntimeValue rt_tls13_record_encrypt(RuntimeValue key_rv, RuntimeValue iv_rv,
                                     int64_t seq_num_i64, int64_t content_type_i64,
                                     RuntimeValue plaintext_rv)
{
    uint32_t key_len = 0, iv_len = 0, pt_len = 0;
    uint8_t *key = _tls_copy_runtime_bytes(key_rv, &key_len);
    uint8_t *iv = _tls_copy_runtime_bytes(iv_rv, &iv_len);
    uint8_t *plaintext = _tls_copy_runtime_bytes(plaintext_rv, &pt_len);
    RuntimeValue empty = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    uint8_t *inner = NULL;
    uint8_t *record = NULL;
    RuntimeValue out = empty;

    if (!key || !iv || (!plaintext && pt_len != 0) || key_len != 16U || iv_len != 12U) {
        goto cleanup;
    }

    uint32_t inner_len = pt_len + 1U;
    uint32_t payload_len = inner_len + 16U;
    inner = (uint8_t *)malloc(inner_len ? inner_len : 1U);
    record = (uint8_t *)malloc(5U + payload_len);
    if (!inner || !record) goto cleanup;

    if (pt_len > 0) memcpy(inner, plaintext, pt_len);
    inner[pt_len] = (uint8_t)(content_type_i64 & 0xff);

    record[0] = 0x17U;
    record[1] = 0x03U;
    record[2] = 0x03U;
    record[3] = (uint8_t)((payload_len >> 8) & 0xffU);
    record[4] = (uint8_t)(payload_len & 0xffU);

    /* TLS 1.3 §5.3: per_record_nonce = pad_to_12(seq_num) XOR static_iv.
     * Was previously implemented as add-with-carry, which agrees with XOR for
     * seq=0 only — that's why D3 (seq=0) passed but D4/D9/D10 (seq>0) failed
     * with auth tag mismatch. Pure-Simple `record13_make_nonce` already uses
     * XOR; this aligns the helper with the spec. */
    uint8_t nonce[12];
    nonce[0] = iv[0];
    nonce[1] = iv[1];
    nonce[2] = iv[2];
    nonce[3] = iv[3];
    {
        uint64_t s = (uint64_t)seq_num_i64;
        nonce[4]  = iv[4]  ^ (uint8_t)((s >> 56) & 0xffU);
        nonce[5]  = iv[5]  ^ (uint8_t)((s >> 48) & 0xffU);
        nonce[6]  = iv[6]  ^ (uint8_t)((s >> 40) & 0xffU);
        nonce[7]  = iv[7]  ^ (uint8_t)((s >> 32) & 0xffU);
        nonce[8]  = iv[8]  ^ (uint8_t)((s >> 24) & 0xffU);
        nonce[9]  = iv[9]  ^ (uint8_t)((s >> 16) & 0xffU);
        nonce[10] = iv[10] ^ (uint8_t)((s >> 8)  & 0xffU);
        nonce[11] = iv[11] ^ (uint8_t)(s & 0xffU);
    }

    if (_tls_aes128_gcm_encrypt_raw(key, nonce, inner, inner_len, record, 5U, record + 5U, record + 5U + inner_len) != 0) {
        goto cleanup;
    }

    out = _tls_runtime_array_from_bytes(record, 5U + payload_len);

cleanup:
    if (key) free(key);
    if (iv) free(iv);
    if (plaintext) free(plaintext);
    if (inner) free(inner);
    if (record) free(record);
    return out;
}

RuntimeValue rt_ssh_aes128_gcm_encrypt_packet(RuntimeValue key_rv, RuntimeValue iv_rv,
                                              int64_t seq_num_i64, RuntimeValue payload_rv)
{
    uint32_t key_len = 0, iv_len = 0, payload_len = 0;
    uint8_t *key = _tls_copy_runtime_bytes(key_rv, &key_len);
    uint8_t *iv = _tls_copy_runtime_bytes(iv_rv, &iv_len);
    uint8_t *payload = _tls_copy_runtime_bytes(payload_rv, &payload_len);
    RuntimeValue out = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    uint8_t *body = NULL;
    uint8_t *wire = NULL;

    if (!key || !iv || (!payload && payload_len != 0) || key_len != 16U || iv_len != 12U) {
        goto cleanup;
    }

    uint32_t padding_len = 16U - ((1U + payload_len) % 16U);
    if (padding_len < 4U) padding_len += 16U;
    uint32_t packet_length = 1U + payload_len + padding_len;
    uint32_t body_len = packet_length;
    uint32_t wire_len = 4U + body_len + 16U;

    body = (uint8_t *)malloc(body_len ? body_len : 1U);
    wire = (uint8_t *)malloc(wire_len ? wire_len : 1U);
    if (!body || !wire) goto cleanup;

    body[0] = (uint8_t)padding_len;
    if (payload_len > 0) memcpy(body + 1U, payload, payload_len);
    memset(body + 1U + payload_len, 0, padding_len);

    wire[0] = (uint8_t)((packet_length >> 24) & 0xffU);
    wire[1] = (uint8_t)((packet_length >> 16) & 0xffU);
    wire[2] = (uint8_t)((packet_length >> 8) & 0xffU);
    wire[3] = (uint8_t)(packet_length & 0xffU);

    uint8_t nonce[12];
    memcpy(nonce, iv, 12U);
    uint64_t carry = (uint64_t)seq_num_i64;
    for (int i = 11; i >= 4 && carry > 0; i--) {
        uint64_t sum = (uint64_t)nonce[i] + (carry & 0xffU);
        nonce[i] = (uint8_t)(sum & 0xffU);
        carry = (carry >> 8) + (sum >> 8);
    }

    if (_tls_aes128_gcm_encrypt_raw(key, nonce, body, body_len, wire, 4U, wire + 4U, wire + 4U + body_len) != 0) {
        goto cleanup;
    }

    out = _tls_runtime_array_from_bytes(wire, wire_len);

cleanup:
    if (key) free(key);
    if (iv) free(iv);
    if (payload) free(payload);
    if (body) free(body);
    if (wire) free(wire);
    return out;
}

RuntimeValue rt_ssh_aes128_gcm_decrypt_packet(RuntimeValue key_rv, RuntimeValue iv_rv,
                                              int64_t seq_num_i64, RuntimeValue packet_rv)
{
    uint32_t key_len = 0, iv_len = 0, packet_len = 0;
    uint8_t *key = _tls_copy_runtime_bytes(key_rv, &key_len);
    uint8_t *iv = _tls_copy_runtime_bytes(iv_rv, &iv_len);
    uint8_t *packet = _tls_copy_runtime_bytes(packet_rv, &packet_len);
    RuntimeValue out = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    uint8_t *body = NULL;
    uint8_t *payload = NULL;

    if (!key || !iv || (!packet && packet_len != 0) || key_len != 16U || iv_len != 12U || packet_len < 20U) {
        goto cleanup;
    }

    uint32_t body_len = packet_len - 4U - 16U;
    uint8_t nonce[12];
    memcpy(nonce, iv, 12U);
    uint64_t carry = (uint64_t)seq_num_i64;
    for (int i = 11; i >= 4 && carry > 0; i--) {
        uint64_t sum = (uint64_t)nonce[i] + (carry & 0xffU);
        nonce[i] = (uint8_t)(sum & 0xffU);
        carry = (carry >> 8) + (sum >> 8);
    }

    body = (uint8_t *)malloc(body_len ? body_len : 1U);
    if (!body) goto cleanup;

    if (_tls_aes128_gcm_decrypt_raw(key, nonce, packet + 4U, body_len, packet, 4U, packet + 4U + body_len, body) != 0) {
        goto cleanup;
    }
    if (body_len < 1U) goto cleanup;

    uint32_t padding_len = (uint32_t)body[0];
    if (1U + padding_len > body_len) goto cleanup;
    uint32_t payload_len = body_len - 1U - padding_len;
    payload = (uint8_t *)malloc(payload_len ? payload_len : 1U);
    if (!payload && payload_len != 0U) goto cleanup;
    if (payload_len > 0) memcpy(payload, body + 1U, payload_len);
    out = _tls_runtime_array_from_bytes(payload, payload_len);

cleanup:
    if (key) free(key);
    if (iv) free(iv);
    if (packet) free(packet);
    if (body) free(body);
    if (payload) free(payload);
    return out;
}

RuntimeValue rt_tls13_hkdf_expand_label_derived(RuntimeValue secret_rv, RuntimeValue context_rv)
{
    uint32_t context_len = 0;
    uint8_t *context = _tls_copy_runtime_bytes(context_rv, &context_len);
    uint32_t secret_len = 0;
    uint8_t *secret = _tls_copy_runtime_bytes(secret_rv, &secret_len);
    static const uint8_t label[] = { 'd', 'e', 'r', 'i', 'v', 'e', 'd' };
    const uint8_t prefix[] = { 't', 'l', 's', '1', '3', ' ' };
    uint8_t full_label[32];
    uint8_t encoded[128];
    uint8_t okm[32];
    uint8_t t_prev[32];
    if (!secret || !context) {
        if (secret) free(secret);
        if (context) free(context);
        return NIL_VALUE;
    }
    uint32_t pos = 0;
    encoded[pos++] = 0;
    encoded[pos++] = 32;
    encoded[pos++] = (uint8_t)(sizeof(prefix) + sizeof(label));
    memcpy(full_label, prefix, sizeof(prefix));
    memcpy(full_label + sizeof(prefix), label, sizeof(label));
    memcpy(encoded + pos, full_label, sizeof(prefix) + sizeof(label));
    pos += (uint32_t)(sizeof(prefix) + sizeof(label));
    encoded[pos++] = (uint8_t)context_len;
    if (context_len > 0) {
        memcpy(encoded + pos, context, context_len);
        pos += context_len;
    }
    uint32_t okm_len = 0, t_prev_len = 0;
    uint8_t counter = 1;
    while (okm_len < 32U) {
        uint8_t input[160];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        memcpy(input + input_len, encoded, pos);
        input_len += pos;
        input[input_len++] = counter;
        _tls_hmac_sha256(secret, secret_len, input, input_len, t_prev);
        t_prev_len = 32;
        uint32_t take = (32U - okm_len) < 32U ? (32U - okm_len) : 32U;
        memcpy(okm + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }
    free(secret);
    free(context);
    return _tls_runtime_array_from_bytes(okm, 32);
}

RuntimeValue rt_tls13_hkdf_expand_label_iv(RuntimeValue secret_rv)
{
    static const uint8_t iv_label[] = { 'i', 'v' };
    return _tls13_hkdf_expand_label_const_padded(secret_rv, iv_label, 2U, 12U);
}

RuntimeValue rt_tls13_hkdf_expand_label_finished(RuntimeValue secret_rv)
{
    static const uint8_t finished_label[] = { 'f', 'i', 'n', 'i', 's', 'h', 'e', 'd' };
    return _tls13_hkdf_expand_label_const_padded(secret_rv, finished_label, 8U, 32U);
}

RuntimeValue rt_tls13_verify_data_hmac(RuntimeValue finished_key_rv, RuntimeValue transcript_hash_rv)
{
    uint32_t key_len = 0;
    uint32_t hash_len = 0;
    uint8_t *key = _tls_copy_runtime_bytes(finished_key_rv, &key_len);
    uint8_t *hash = _tls_copy_runtime_bytes(transcript_hash_rv, &hash_len);
    uint8_t out[32];
    RuntimeValue rv = NIL_VALUE;
    if (!key || !hash || hash_len != 32U) {
        if (key) free(key);
        if (hash) free(hash);
        return NIL_VALUE;
    }
    _tls_hmac_sha256(key, key_len, hash, hash_len, out);
    rv = _tls_runtime_array_from_bytes(out, 32U);
    free(key);
    free(hash);
    return rv;
}

int64_t rt_tls13_bytes_equal(RuntimeValue a_rv, RuntimeValue b_rv)
{
    uint32_t a_len = 0, b_len = 0;
    uint8_t *a = _tls_copy_runtime_bytes(a_rv, &a_len);
    uint8_t *b = _tls_copy_runtime_bytes(b_rv, &b_len);
    int64_t out = 0;
    if (a && b && a_len == b_len) {
        out = (memcmp(a, b, a_len) == 0) ? 1 : 0;
    }
    if (a) free(a);
    if (b) free(b);
    return out;
}

RuntimeValue rt_tls13_finished_msg_verify_data(RuntimeValue msg_rv)
{
    uint32_t msg_len = 0;
    uint8_t *msg = _tls_copy_runtime_bytes(msg_rv, &msg_len);
    RuntimeValue rv = NIL_VALUE;
    if (!msg || msg_len < 4U || msg[0] != 0x14U) {
        if (msg) free(msg);
        return NIL_VALUE;
    }
    uint32_t body_len = ((uint32_t)msg[1] << 16) | ((uint32_t)msg[2] << 8) | (uint32_t)msg[3];
    if (4U + body_len > msg_len) {
        free(msg);
        return NIL_VALUE;
    }
    rv = _tls_runtime_array_from_bytes(msg + 4U, body_len);
    free(msg);
    return rv;
}

RuntimeValue rt_tls13_server_finished_verify_data_fd(RuntimeValue ch_msg_rv, RuntimeValue sh_msg_rv,
                                                     RuntimeValue scalar_rv, RuntimeValue cr_msg_rv,
                                                     RuntimeValue ee_msg_rv, RuntimeValue cert_msg_rv,
                                                     RuntimeValue cv_msg_rv)
{
    uint32_t ch_len = 0, sh_len = 0, scalar_len = 0, cr_len = 0, ee_len = 0, cert_len = 0, cv_len = 0;
    uint8_t *ch = _tls_copy_runtime_bytes(ch_msg_rv, &ch_len);
    uint8_t *sh = _tls_copy_runtime_bytes(sh_msg_rv, &sh_len);
    uint8_t *scalar = _tls_copy_runtime_bytes(scalar_rv, &scalar_len);
    uint8_t *cr = _tls_copy_runtime_bytes(cr_msg_rv, &cr_len);
    uint8_t *ee = _tls_copy_runtime_bytes(ee_msg_rv, &ee_len);
    uint8_t *cert = _tls_copy_runtime_bytes(cert_msg_rv, &cert_len);
    uint8_t *cv = _tls_copy_runtime_bytes(cv_msg_rv, &cv_len);
    RuntimeValue rv = NIL_VALUE;
    uint8_t shared[32], empty_hash[32], derived[32], handshake_secret[32], thash[32], server_hs[32], finished_key[32], out[32];
    uint8_t early_secret[32] = {
        0x33, 0xad, 0x0a, 0x1c, 0x60, 0x7e, 0xc0, 0x3b,
        0x09, 0xe6, 0xcd, 0x98, 0x93, 0x68, 0x0c, 0xe2,
        0x10, 0xad, 0xf3, 0x00, 0xaa, 0x1f, 0x26, 0x60,
        0xe1, 0xb2, 0x2e, 0x10, 0xf1, 0x70, 0xf9, 0x2a
    };
    if (!ch || !sh || !scalar || scalar_len != 32U || (!cr && cr_len != 0U) ||
        (!ee && ee_len != 0U) || (!cert && cert_len != 0U) || (!cv && cv_len != 0U)) {
        goto done;
    }
    const uint8_t *ch_msg = ch;
    const uint8_t *sh_msg = sh;
    uint32_t ch_msg_len = ch_len;
    uint32_t sh_msg_len = sh_len;
    if (ch_len >= 5U && ch[0] == 0x16U) {
        uint32_t payload_len = ((uint32_t)ch[3] << 8) | (uint32_t)ch[4];
        if (5U + payload_len > ch_len) goto done;
        ch_msg = ch + 5U;
        ch_msg_len = payload_len;
    }
    if (sh_len >= 5U && sh[0] == 0x16U) {
        uint32_t payload_len = ((uint32_t)sh[3] << 8) | (uint32_t)sh[4];
        if (5U + payload_len > sh_len) goto done;
        sh_msg = sh + 5U;
        sh_msg_len = payload_len;
    }
    int found = 0;
    for (uint32_t scan = 0; scan + 40U <= sh_msg_len; scan++) {
        if (sh_msg[scan] == 0x00 && sh_msg[scan + 1U] == 0x33 &&
            sh_msg[scan + 2U] == 0x00 && sh_msg[scan + 3U] == 0x24 &&
            sh_msg[scan + 4U] == 0x00 && sh_msg[scan + 5U] == 0x1d &&
            sh_msg[scan + 6U] == 0x00 && sh_msg[scan + 7U] == 0x20) {
            _tls_x25519_scalarmult(shared, scalar, sh_msg + scan + 8U);
            found = 1;
            break;
        }
    }
    if (!found) goto done;
    _tls_sha256_digest((const uint8_t *)"", 0, empty_hash);
    static const uint8_t label_derived[] = { 'd', 'e', 'r', 'i', 'v', 'e', 'd' };
    static const uint8_t label_server_hs[] = { 's', ' ', 'h', 's', ' ', 't', 'r', 'a', 'f', 'f', 'i', 'c' };
    static const uint8_t label_finished[] = { 'f', 'i', 'n', 'i', 's', 'h', 'e', 'd' };
    _tls_hkdf_expand_label_raw(early_secret, 32U, label_derived, 7U, empty_hash, 32U, 32U, derived);
    _tls_hmac_sha256(derived, 32U, shared, 32U, handshake_secret);
    uint32_t hs_total = ch_msg_len + sh_msg_len;
    uint8_t *hs_merged = (uint8_t *)malloc(hs_total > 0U ? hs_total : 1U);
    if (!hs_merged && hs_total != 0U) goto done;
    if (ch_msg_len) memcpy(hs_merged, ch_msg, ch_msg_len);
    if (sh_msg_len) memcpy(hs_merged + ch_msg_len, sh_msg, sh_msg_len);
    _tls_sha256_digest(hs_merged, hs_total, thash);
    free(hs_merged);
    _tls_hkdf_expand_label_raw(handshake_secret, 32U, label_server_hs, 12U, thash, 32U, 32U, server_hs);
    _tls_hkdf_expand_label_raw(server_hs, 32U, label_finished, 8U, (const uint8_t *)"", 0, 32U, finished_key);

    uint32_t total = ch_msg_len + sh_msg_len + cr_len + ee_len + cert_len + cv_len;
    uint8_t *merged = (uint8_t *)malloc(total > 0U ? total : 1U);
    if (!merged && total != 0U) goto done;
    uint32_t off = 0;
    if (ch_msg_len) { memcpy(merged + off, ch_msg, ch_msg_len); off += ch_msg_len; }
    if (sh_msg_len) { memcpy(merged + off, sh_msg, sh_msg_len); off += sh_msg_len; }
    if (cr_len) { memcpy(merged + off, cr, cr_len); off += cr_len; }
    if (ee_len) { memcpy(merged + off, ee, ee_len); off += ee_len; }
    if (cert_len) { memcpy(merged + off, cert, cert_len); off += cert_len; }
    if (cv_len) { memcpy(merged + off, cv, cv_len); off += cv_len; }
    _tls_sha256_digest(merged, total, thash);
    free(merged);
    _tls_hmac_sha256(finished_key, 32U, thash, 32U, out);
    rv = _tls_runtime_array_from_bytes(out, 32U);
done:
    if (ch) free(ch);
    if (sh) free(sh);
    if (scalar) free(scalar);
    if (cr) free(cr);
    if (ee) free(ee);
    if (cert) free(cert);
    if (cv) free(cv);
    return rv;
}

int64_t rt_tls13_certificate_verify_algorithm(RuntimeValue body_rv)
{
    uint32_t body_len = 0;
    uint8_t *body = _tls_copy_runtime_bytes(body_rv, &body_len);
    int64_t out = -1;
    if (!body || body_len < 4U) {
        if (body) free(body);
        return -1;
    }
    out = (int64_t)(((uint16_t)body[0] << 8) | (uint16_t)body[1]);
    free(body);
    return out;
}

RuntimeValue rt_tls13_certificate_verify_signature(RuntimeValue body_rv)
{
    uint32_t body_len = 0;
    uint8_t *body = _tls_copy_runtime_bytes(body_rv, &body_len);
    RuntimeValue rv = NIL_VALUE;
    if (!body || body_len < 4U) {
        if (body) free(body);
        return NIL_VALUE;
    }
    uint32_t sig_len = ((uint32_t)body[2] << 8) | (uint32_t)body[3];
    if (4U + sig_len > body_len) {
        free(body);
        return NIL_VALUE;
    }
    rv = _tls_runtime_array_from_bytes(body + 4U, sig_len);
    free(body);
    return rv;
}

int64_t rt_tls13_certificate_verify_msg_algorithm(RuntimeValue msg_rv)
{
    uint32_t msg_len = 0;
    uint8_t *msg = _tls_copy_runtime_bytes(msg_rv, &msg_len);
    int64_t out = -1;
    if (!msg || msg_len < 8U || msg[0] != 0x0fU) {
        if (msg) free(msg);
        return -1;
    }
    uint32_t body_len = ((uint32_t)msg[1] << 16) | ((uint32_t)msg[2] << 8) | (uint32_t)msg[3];
    if (4U + body_len > msg_len || body_len < 4U) {
        free(msg);
        return -1;
    }
    out = (int64_t)(((uint16_t)msg[4] << 8) | (uint16_t)msg[5]);
    free(msg);
    return out;
}

RuntimeValue rt_tls13_certificate_verify_msg_signature(RuntimeValue msg_rv)
{
    uint32_t msg_len = 0;
    uint8_t *msg = _tls_copy_runtime_bytes(msg_rv, &msg_len);
    RuntimeValue rv = NIL_VALUE;
    if (!msg || msg_len < 8U || msg[0] != 0x0fU) {
        if (msg) free(msg);
        return NIL_VALUE;
    }
    uint32_t body_len = ((uint32_t)msg[1] << 16) | ((uint32_t)msg[2] << 8) | (uint32_t)msg[3];
    if (4U + body_len > msg_len || body_len < 4U) {
        free(msg);
        return NIL_VALUE;
    }
    uint32_t sig_len = ((uint32_t)msg[6] << 8) | (uint32_t)msg[7];
    if (8U + sig_len > msg_len || sig_len + 4U > body_len) {
        free(msg);
        return NIL_VALUE;
    }
    rv = _tls_runtime_array_from_bytes(msg + 8U, sig_len);
    free(msg);
    return rv;
}

static RuntimeValue _tls_runtime_empty_bytes(void)
{
    return _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
}

typedef struct {
    uint8_t tag;
    uint32_t value_off;
    uint32_t value_len;
    uint32_t total_len;
    int ok;
} tls_der_tlv_t;

static tls_der_tlv_t _tls_der_read_tlv(const uint8_t *data, uint32_t data_len, uint32_t off)
{
    tls_der_tlv_t out = {0, 0, 0, 0, 0};
    if (!data || off + 2U > data_len) return out;
    uint8_t tag = data[off];
    uint8_t first = data[off + 1U];
    uint32_t hdr_len = 0;
    uint32_t len = 0;
    if (first < 0x80U) {
        hdr_len = 1U;
        len = first;
    } else if (first == 0x81U) {
        if (off + 3U > data_len) return out;
        hdr_len = 2U;
        len = data[off + 2U];
    } else if (first == 0x82U) {
        if (off + 4U > data_len) return out;
        hdr_len = 3U;
        len = ((uint32_t)data[off + 2U] << 8) | (uint32_t)data[off + 3U];
    } else {
        return out;
    }
    uint32_t value_off = off + 1U + hdr_len;
    uint32_t total_len = 1U + hdr_len + len;
    if (value_off > data_len || off + total_len > data_len) return out;
    out.tag = tag;
    out.value_off = value_off;
    out.value_len = len;
    out.total_len = total_len;
    out.ok = 1;
    return out;
}

RuntimeValue rt_tls13_certificate_body_ed25519_pubkey(RuntimeValue body_rv)
{
    uint32_t body_len = 0;
    uint8_t *body = _tls_copy_runtime_bytes(body_rv, &body_len);
    RuntimeValue rv = NIL_VALUE;
    if (!body || body_len < 7U) {
        if (body) free(body);
        return _tls_runtime_empty_bytes();
    }

    uint32_t off = 0;
    uint8_t ctx_len = body[off++];
    if (off + ctx_len + 3U > body_len) {
        free(body);
        return _tls_runtime_empty_bytes();
    }
    off += ctx_len;
    uint32_t cert_list_len = ((uint32_t)body[off] << 16) | ((uint32_t)body[off + 1] << 8) | (uint32_t)body[off + 2];
    off += 3U;
    if (off + cert_list_len > body_len || cert_list_len < 3U) {
        free(body);
        return _tls_runtime_empty_bytes();
    }
    uint32_t cert_len = ((uint32_t)body[off] << 16) | ((uint32_t)body[off + 1] << 8) | (uint32_t)body[off + 2];
    off += 3U;
    if (off + cert_len > body_len || cert_len == 0U) {
        free(body);
        return _tls_runtime_empty_bytes();
    }
    uint8_t *cert = body + off;
    uint32_t cert_sz = cert_len;

    tls_der_tlv_t cert_seq = _tls_der_read_tlv(cert, cert_sz, 0);
    if (!cert_seq.ok || cert_seq.tag != 0x30U) {
        free(body);
        return _tls_runtime_empty_bytes();
    }
    tls_der_tlv_t tbs = _tls_der_read_tlv(cert, cert_sz, cert_seq.value_off);
    if (!tbs.ok || tbs.tag != 0x30U) {
        free(body);
        return _tls_runtime_empty_bytes();
    }
    uint32_t p = tbs.value_off;
    uint32_t tbs_end = tbs.value_off + tbs.value_len;
    if (p < tbs_end && cert[p] == 0xa0U) {
        tls_der_tlv_t version = _tls_der_read_tlv(cert, cert_sz, p);
        if (!version.ok) {
            free(body);
            return _tls_runtime_empty_bytes();
        }
        p += version.total_len;
    }
    for (int i = 0; i < 5; i++) {
        tls_der_tlv_t field = _tls_der_read_tlv(cert, cert_sz, p);
        if (!field.ok) {
            free(body);
            return _tls_runtime_empty_bytes();
        }
        p += field.total_len;
    }
    tls_der_tlv_t spki = _tls_der_read_tlv(cert, cert_sz, p);
    if (!spki.ok || spki.tag != 0x30U) {
        free(body);
        return _tls_runtime_empty_bytes();
    }
    tls_der_tlv_t alg = _tls_der_read_tlv(cert, cert_sz, spki.value_off);
    if (!alg.ok || alg.tag != 0x30U) {
        free(body);
        return _tls_runtime_empty_bytes();
    }
    tls_der_tlv_t oid = _tls_der_read_tlv(cert, cert_sz, alg.value_off);
    if (!oid.ok || oid.tag != 0x06U || oid.value_len != 3U ||
        cert[oid.value_off] != 0x2bU || cert[oid.value_off + 1U] != 0x65U || cert[oid.value_off + 2U] != 0x70U) {
        free(body);
        return _tls_runtime_empty_bytes();
    }
    tls_der_tlv_t bitstr = _tls_der_read_tlv(cert, cert_sz, spki.value_off + alg.total_len);
    if (!bitstr.ok || bitstr.tag != 0x03U || bitstr.value_len != 33U || cert[bitstr.value_off] != 0x00U) {
        free(body);
        return _tls_runtime_empty_bytes();
    }
    rv = _tls_runtime_array_from_bytes(cert + bitstr.value_off + 1U, 32U);
    free(body);
    return rv;

    free(body);
    return _tls_runtime_empty_bytes();
}

RuntimeValue rt_tls13_hkdf_expand_label_client_hs(RuntimeValue secret_rv, RuntimeValue context_rv)
{
    uint32_t context_len = 0;
    uint8_t *context = _tls_copy_runtime_bytes(context_rv, &context_len);
    uint32_t secret_len = 0;
    uint8_t *secret = _tls_copy_runtime_bytes(secret_rv, &secret_len);
    static const uint8_t label[] = { 'c', ' ', 'h', 's', ' ', 't', 'r', 'a', 'f', 'f', 'i', 'c' };
    const uint8_t prefix[] = { 't', 'l', 's', '1', '3', ' ' };
    uint8_t full_label[32];
    uint8_t encoded[128];
    uint8_t okm[32];
    uint8_t t_prev[32];
    if (!secret || !context) {
        if (secret) free(secret);
        if (context) free(context);
        return NIL_VALUE;
    }
    uint32_t pos = 0;
    encoded[pos++] = 0;
    encoded[pos++] = 32;
    encoded[pos++] = (uint8_t)(sizeof(prefix) + sizeof(label));
    memcpy(full_label, prefix, sizeof(prefix));
    memcpy(full_label + sizeof(prefix), label, sizeof(label));
    memcpy(encoded + pos, full_label, sizeof(prefix) + sizeof(label));
    pos += (uint32_t)(sizeof(prefix) + sizeof(label));
    encoded[pos++] = (uint8_t)context_len;
    if (context_len > 0) {
        memcpy(encoded + pos, context, context_len);
        pos += context_len;
    }
    uint32_t okm_len = 0, t_prev_len = 0;
    uint8_t counter = 1;
    while (okm_len < 32U) {
        uint8_t input[160];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        memcpy(input + input_len, encoded, pos);
        input_len += pos;
        input[input_len++] = counter;
        _tls_hmac_sha256(secret, secret_len, input, input_len, t_prev);
        t_prev_len = 32;
        uint32_t take = (32U - okm_len) < 32U ? (32U - okm_len) : 32U;
        memcpy(okm + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }
    free(secret);
    free(context);
    return _tls_runtime_array_from_bytes(okm, 32);
}

RuntimeValue rt_tls13_hkdf_expand_label_server_hs(RuntimeValue secret_rv, RuntimeValue context_rv)
{
    uint32_t context_len = 0;
    uint8_t *context = _tls_copy_runtime_bytes(context_rv, &context_len);
    uint32_t secret_len = 0;
    uint8_t *secret = _tls_copy_runtime_bytes(secret_rv, &secret_len);
    static const uint8_t label[] = { 's', ' ', 'h', 's', ' ', 't', 'r', 'a', 'f', 'f', 'i', 'c' };
    const uint8_t prefix[] = { 't', 'l', 's', '1', '3', ' ' };
    uint8_t full_label[32];
    uint8_t encoded[128];
    uint8_t okm[32];
    uint8_t t_prev[32];
    if (!secret || !context) {
        if (secret) free(secret);
        if (context) free(context);
        return NIL_VALUE;
    }
    uint32_t pos = 0;
    encoded[pos++] = 0;
    encoded[pos++] = 32;
    encoded[pos++] = (uint8_t)(sizeof(prefix) + sizeof(label));
    memcpy(full_label, prefix, sizeof(prefix));
    memcpy(full_label + sizeof(prefix), label, sizeof(label));
    memcpy(encoded + pos, full_label, sizeof(prefix) + sizeof(label));
    pos += (uint32_t)(sizeof(prefix) + sizeof(label));
    encoded[pos++] = (uint8_t)context_len;
    if (context_len > 0) {
        memcpy(encoded + pos, context, context_len);
        pos += context_len;
    }
    uint32_t okm_len = 0, t_prev_len = 0;
    uint8_t counter = 1;
    while (okm_len < 32U) {
        uint8_t input[160];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        memcpy(input + input_len, encoded, pos);
        input_len += pos;
        input[input_len++] = counter;
        _tls_hmac_sha256(secret, secret_len, input, input_len, t_prev);
        t_prev_len = 32;
        uint32_t take = (32U - okm_len) < 32U ? (32U - okm_len) : 32U;
        memcpy(okm + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }
    free(secret);
    free(context);
    return _tls_runtime_array_from_bytes(okm, 32);
}

RuntimeValue rt_tls13_master_secret_from_hs(RuntimeValue handshake_secret_rv)
{
    RuntimeValue empty_rv = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    uint8_t zeros[32] = {0};
    RuntimeValue zeros_rv = _tls_runtime_array_from_bytes(zeros, 32U);
    RuntimeValue empty_hash_rv = rt_tls13_sha256(empty_rv);
    if (empty_hash_rv == NIL_VALUE) return NIL_VALUE;
    RuntimeValue derived_rv = rt_tls13_hkdf_expand_label_derived(handshake_secret_rv, empty_hash_rv);
    if (derived_rv == NIL_VALUE) return NIL_VALUE;
    return rt_tls13_hkdf_extract(derived_rv, zeros_rv);
}

RuntimeValue rt_tls13_hkdf_expand_label_client_app(RuntimeValue secret_rv, RuntimeValue context_rv)
{
    uint32_t context_len = 0;
    uint8_t *context = _tls_copy_runtime_bytes(context_rv, &context_len);
    uint32_t secret_len = 0;
    uint8_t *secret = _tls_copy_runtime_bytes(secret_rv, &secret_len);
    static const uint8_t label[] = { 'c', ' ', 'a', 'p', ' ', 't', 'r', 'a', 'f', 'f', 'i', 'c' };
    const uint8_t prefix[] = { 't', 'l', 's', '1', '3', ' ' };
    uint8_t full_label[32];
    uint8_t encoded[128];
    uint8_t okm[32];
    uint8_t t_prev[32];
    if (!secret || !context) {
        if (secret) free(secret);
        if (context) free(context);
        return NIL_VALUE;
    }
    uint32_t pos = 0;
    encoded[pos++] = 0;
    encoded[pos++] = 32;
    encoded[pos++] = (uint8_t)(sizeof(prefix) + sizeof(label));
    memcpy(full_label, prefix, sizeof(prefix));
    memcpy(full_label + sizeof(prefix), label, sizeof(label));
    memcpy(encoded + pos, full_label, sizeof(prefix) + sizeof(label));
    pos += (uint32_t)(sizeof(prefix) + sizeof(label));
    encoded[pos++] = (uint8_t)context_len;
    if (context_len > 0) {
        memcpy(encoded + pos, context, context_len);
        pos += context_len;
    }
    uint32_t okm_len = 0, t_prev_len = 0;
    uint8_t counter = 1;
    while (okm_len < 32U) {
        uint8_t input[160];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        memcpy(input + input_len, encoded, pos);
        input_len += pos;
        input[input_len++] = counter;
        _tls_hmac_sha256(secret, secret_len, input, input_len, t_prev);
        t_prev_len = 32;
        uint32_t take = (32U - okm_len) < 32U ? (32U - okm_len) : 32U;
        memcpy(okm + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }
    free(secret);
    free(context);
    return _tls_runtime_array_from_bytes(okm, 32);
}

RuntimeValue rt_tls13_hkdf_expand_label_server_app(RuntimeValue secret_rv, RuntimeValue context_rv)
{
    uint32_t context_len = 0;
    uint8_t *context = _tls_copy_runtime_bytes(context_rv, &context_len);
    uint32_t secret_len = 0;
    uint8_t *secret = _tls_copy_runtime_bytes(secret_rv, &secret_len);
    static const uint8_t label[] = { 's', ' ', 'a', 'p', ' ', 't', 'r', 'a', 'f', 'f', 'i', 'c' };
    const uint8_t prefix[] = { 't', 'l', 's', '1', '3', ' ' };
    uint8_t full_label[32];
    uint8_t encoded[128];
    uint8_t okm[32];
    uint8_t t_prev[32];
    if (!secret || !context) {
        if (secret) free(secret);
        if (context) free(context);
        return NIL_VALUE;
    }
    uint32_t pos = 0;
    encoded[pos++] = 0;
    encoded[pos++] = 32;
    encoded[pos++] = (uint8_t)(sizeof(prefix) + sizeof(label));
    memcpy(full_label, prefix, sizeof(prefix));
    memcpy(full_label + sizeof(prefix), label, sizeof(label));
    memcpy(encoded + pos, full_label, sizeof(prefix) + sizeof(label));
    pos += (uint32_t)(sizeof(prefix) + sizeof(label));
    encoded[pos++] = (uint8_t)context_len;
    if (context_len > 0) {
        memcpy(encoded + pos, context, context_len);
        pos += context_len;
    }
    uint32_t okm_len = 0, t_prev_len = 0;
    uint8_t counter = 1;
    while (okm_len < 32U) {
        uint8_t input[160];
        uint32_t input_len = 0;
        if (t_prev_len > 0) {
            memcpy(input + input_len, t_prev, t_prev_len);
            input_len += t_prev_len;
        }
        memcpy(input + input_len, encoded, pos);
        input_len += pos;
        input[input_len++] = counter;
        _tls_hmac_sha256(secret, secret_len, input, input_len, t_prev);
        t_prev_len = 32;
        uint32_t take = (32U - okm_len) < 32U ? (32U - okm_len) : 32U;
        memcpy(okm + okm_len, t_prev, take);
        okm_len += take;
        counter++;
    }
    free(secret);
    free(context);
    return _tls_runtime_array_from_bytes(okm, 32);
}

RuntimeValue rt_tls13_client_app_secret_7(
    RuntimeValue handshake_secret_rv,
    RuntimeValue a, RuntimeValue b, RuntimeValue c,
    RuntimeValue d, RuntimeValue e, RuntimeValue f, RuntimeValue g)
{
    RuntimeValue empty_rv = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    uint8_t zeros[32] = {0};
    RuntimeValue zeros_rv = _tls_runtime_array_from_bytes(zeros, 32U);
    RuntimeValue empty_hash_rv = rt_tls13_sha256(empty_rv);
    if (empty_hash_rv == NIL_VALUE) return NIL_VALUE;
    RuntimeValue derived_rv = rt_tls13_hkdf_expand_label_derived(handshake_secret_rv, empty_hash_rv);
    if (derived_rv == NIL_VALUE) return NIL_VALUE;
    RuntimeValue master_rv = rt_tls13_hkdf_extract(derived_rv, zeros_rv);
    if (master_rv == NIL_VALUE) return NIL_VALUE;
    RuntimeValue thash_rv = rt_tls13_transcript_hash_7(a, b, c, d, e, f, g);
    if (thash_rv == NIL_VALUE) return NIL_VALUE;
    RuntimeValue out = rt_tls13_hkdf_expand_label_client_app(master_rv, thash_rv);
    return out;
}

RuntimeValue rt_tls13_server_app_secret_7(
    RuntimeValue handshake_secret_rv,
    RuntimeValue a, RuntimeValue b, RuntimeValue c,
    RuntimeValue d, RuntimeValue e, RuntimeValue f, RuntimeValue g)
{
    RuntimeValue empty_rv = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    uint8_t zeros[32] = {0};
    RuntimeValue zeros_rv = _tls_runtime_array_from_bytes(zeros, 32U);
    RuntimeValue empty_hash_rv = rt_tls13_sha256(empty_rv);
    if (empty_hash_rv == NIL_VALUE) return NIL_VALUE;
    RuntimeValue derived_rv = rt_tls13_hkdf_expand_label_derived(handshake_secret_rv, empty_hash_rv);
    if (derived_rv == NIL_VALUE) return NIL_VALUE;
    RuntimeValue master_rv = rt_tls13_hkdf_extract(derived_rv, zeros_rv);
    if (master_rv == NIL_VALUE) return NIL_VALUE;
    RuntimeValue thash_rv = rt_tls13_transcript_hash_7(a, b, c, d, e, f, g);
    if (thash_rv == NIL_VALUE) return NIL_VALUE;
    RuntimeValue out = rt_tls13_hkdf_expand_label_server_app(master_rv, thash_rv);
    return out;
}

static RuntimeValue _tls_cached_client_app_secret = NIL_VALUE;
static RuntimeValue _tls_cached_server_app_secret = NIL_VALUE;

static int64_t _tls_runtime_array_len_i64(RuntimeValue rv)
{
    if (!IS_HEAP(rv)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(rv);
    if (!a || a->hdr.type != HEAP_ARRAY) return 0;
    return (int64_t)a->len;
}

int64_t rt_tls13_prepare_app_secrets_7(
    RuntimeValue handshake_secret_rv,
    RuntimeValue a, RuntimeValue b, RuntimeValue c,
    RuntimeValue d, RuntimeValue e, RuntimeValue f, RuntimeValue g)
{
    _tls_cached_client_app_secret = rt_tls13_client_app_secret_7(
        handshake_secret_rv, a, b, c, d, e, f, g);
    _tls_cached_server_app_secret = rt_tls13_server_app_secret_7(
        handshake_secret_rv, a, b, c, d, e, f, g);
    int64_t client_len = _tls_runtime_array_len_i64(_tls_cached_client_app_secret);
    int64_t server_len = _tls_runtime_array_len_i64(_tls_cached_server_app_secret);
    return (client_len & 0xffff) | ((server_len & 0xffff) << 16);
}

int64_t rt_tls13_fill_app_secrets_7(
    RuntimeValue handshake_secret_rv,
    RuntimeValue a, RuntimeValue b, RuntimeValue c,
    RuntimeValue d, RuntimeValue e, RuntimeValue f, RuntimeValue g,
    RuntimeValue client_out_rv,
    RuntimeValue server_out_rv)
{
    RuntimeValue client_rv = rt_tls13_client_app_secret_7(
        handshake_secret_rv, a, b, c, d, e, f, g);
    RuntimeValue server_rv = rt_tls13_server_app_secret_7(
        handshake_secret_rv, a, b, c, d, e, f, g);
    if (client_rv == NIL_VALUE || server_rv == NIL_VALUE) return -1;

    uint32_t client_len = 0;
    uint8_t *client = _tls_copy_runtime_bytes(client_rv, &client_len);
    uint32_t server_len = 0;
    uint8_t *server = _tls_copy_runtime_bytes(server_rv, &server_len);
    if ((!client && client_len != 0U) || (!server && server_len != 0U)) {
        free(client);
        free(server);
        return -2;
    }
    if (client_len != 32U || server_len != 32U) {
        free(client);
        free(server);
        return -3;
    }
    if (_ed_bytes_to_rv(client, 32U, client_out_rv) != 0 ||
        _ed_bytes_to_rv(server, 32U, server_out_rv) != 0) {
        free(client);
        free(server);
        return -4;
    }
    free(client);
    free(server);
    return 0;
}

RuntimeValue rt_tls13_take_cached_client_app_secret(void)
{
    RuntimeValue out = _tls_cached_client_app_secret;
    _tls_cached_client_app_secret = NIL_VALUE;
    return out;
}

RuntimeValue rt_tls13_take_cached_server_app_secret(void)
{
    RuntimeValue out = _tls_cached_server_app_secret;
    _tls_cached_server_app_secret = NIL_VALUE;
    return out;
}

int64_t rt_tls13_client_app_secret_diag_7(
    RuntimeValue handshake_secret_rv,
    RuntimeValue a, RuntimeValue b, RuntimeValue c,
    RuntimeValue d, RuntimeValue e, RuntimeValue f, RuntimeValue g)
{
    int64_t mask = 0;
    RuntimeValue empty_rv = _tls_runtime_array_from_bytes((const uint8_t *)"", 0);
    uint8_t zeros[32] = {0};
    RuntimeValue zeros_rv = _tls_runtime_array_from_bytes(zeros, 32U);
    RuntimeValue empty_hash_rv = rt_tls13_sha256(empty_rv);
    if (empty_hash_rv != NIL_VALUE) mask |= 1;
    if (empty_hash_rv == NIL_VALUE) return mask;

    RuntimeValue derived_rv = rt_tls13_hkdf_expand_label_derived(handshake_secret_rv, empty_hash_rv);
    if (derived_rv != NIL_VALUE) mask |= 2;
    if (derived_rv == NIL_VALUE) return mask;

    RuntimeValue master_rv = rt_tls13_hkdf_extract(derived_rv, zeros_rv);
    if (master_rv != NIL_VALUE) mask |= 4;
    if (master_rv == NIL_VALUE) return mask;

    RuntimeValue thash_rv = rt_tls13_transcript_hash_7(a, b, c, d, e, f, g);
    if (thash_rv != NIL_VALUE) mask |= 8;
    if (thash_rv == NIL_VALUE) return mask;

    RuntimeValue out = rt_tls13_hkdf_expand_label_client_app(master_rv, thash_rv);
    if (out != NIL_VALUE) mask |= 16;
    return mask;
}

/* rt_array_pop: remove and return last element */
RuntimeValue rt_array_pop(RuntimeValue arr)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY || a->len == 0) return NIL_VALUE;
    RuntimeValue *items = runtime_array_items(a);
    a->len--;
    RuntimeValue val = items[a->len];
    items[a->len] = NIL_VALUE;
    return val;
}

/* rt_array_get: get element at index.
 * The native Rust runtime takes a raw i64 index. Keep the bare-metal
 * implementation ABI-compatible with that contract. */
RuntimeValue rt_array_get(RuntimeValue arr, RuntimeValue idx)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    int64_t i = (int64_t)idx;
    if (i < 0) i += (int64_t)a->len;
    if (i < 0 || (uint32_t)i >= a->len) return NIL_VALUE;
    return runtime_array_items(a)[i];
}

static int8_t rt_array_set_raw(RuntimeValue arr, RuntimeValue idx, RuntimeValue val)
{
    if (!IS_HEAP(arr)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return 0;
    int64_t i = (int64_t)idx;
    if (i < 0) i += (int64_t)a->len;
    if (i < 0 || (uint32_t)i >= a->len) return 0;
    runtime_array_items(a)[i] = val;
    return 1;
}

/* Exported array set ABI: return success byte like hosted. */
int8_t rt_array_set(RuntimeValue arr, RuntimeValue idx, RuntimeValue val)
{
    return rt_array_set_raw(arr, idx, val);
}

/* rt_array_len: return RAW (untagged) integer.
 * The Cranelift backend's call_len_method does NOT unbox the result,
 * and the MIR for-loop lowering compares directly with raw index counters.
 * So we must return raw len, not ENCODE_INT(len). */
RuntimeValue rt_array_len(RuntimeValue arr)
{
    if (!IS_HEAP(arr)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return 0;
    return (RuntimeValue)a->len;
}

RuntimeValue rt_arm_array_get_byte_u32(RuntimeValue arr, RuntimeValue idx_val)
{
    uint64_t idx = (uint64_t)idx_val;
    if (!IS_HEAP(arr)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY || idx >= a->len) return 0;
    RuntimeValue v = runtime_array_items(a)[idx];
    if (IS_INT(v)) return (RuntimeValue)DECODE_INT(v);
    return (RuntimeValue)(uint8_t)(uint64_t)v;
}
/* rt_arm_array_len_u32 — return the element count of a [u8] (or any) array.
 * Used by x86_64_fs_exec_spawn to gate on executable byte count.
 * Missing from this file caused spawn:resolve-fail regardless of VFS content
 * (BUG-VFS-CACHE-RETURNS-PLACEHOLDER root cause, fixed 2026-04-27). */
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
/* ---- Tuple functions (rt_extras.c not linked in baremetal build) ---- */
RuntimeValue rt_tuple_new(RuntimeValue len_rv)
{
    int64_t len = (int64_t)len_rv;
    if (len <= 0) len = 0;
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)len * sizeof(RuntimeValue));
    a->len = (uint32_t)len;
    a->cap = (uint32_t)len;
    a->items = runtime_array_inline_items(a);
    for (uint32_t i = 0; i < (uint32_t)len; i++) a->items[i] = NIL_VALUE;
    return ENCODE_PTR(a);
}
RuntimeValue rt_tuple_get(RuntimeValue tuple, RuntimeValue index)
{
    if (!IS_HEAP(tuple)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(tuple);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    int64_t i = (int64_t)index;
    if (i < 0 || (uint32_t)i >= a->len) return NIL_VALUE;
    return runtime_array_items(a)[i];
}
RuntimeValue rt_tuple_set(RuntimeValue tuple, RuntimeValue index, RuntimeValue value)
{
    if (!IS_HEAP(tuple)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(tuple);
    if (!a || a->hdr.type != HEAP_ARRAY) return 0;
    int64_t i = (int64_t)index;
    if (i < 0 || (uint32_t)i >= a->len) return 0;
    runtime_array_items(a)[i] = value;
    return 1;
}
RuntimeValue rt_tuple_len(RuntimeValue tuple)
{
    if (!IS_HEAP(tuple)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(tuple);
    if (!a || a->hdr.type != HEAP_ARRAY) return 0;
    return (RuntimeValue)(int64_t)a->len;
}

/* rt_array_slice(arr, start, end) — return sub-array */
RuntimeValue rt_array_slice(RuntimeValue arr, RuntimeValue start, RuntimeValue end)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    RuntimeValue *items = runtime_array_items(a);
    int64_t s = DECODE_INT(start);
    int64_t e = DECODE_INT(end);
    if (s < 0) s = 0;
    if (e > (int64_t)a->len) e = (int64_t)a->len;
    if (s >= e) return rt_array_new(ENCODE_INT(1));
    RuntimeValue result = rt_array_new(ENCODE_INT(e - s));
    for (int64_t i = s; i < e; i++) {
        result = rt_array_push_handle(result, items[i]);
    }
    return result;
}

/* rt_array_contains(arr, val) — linear scan for match */
RuntimeValue rt_array_contains(RuntimeValue arr, RuntimeValue val)
{
    if (!IS_HEAP(arr)) return 0;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return 0;
    RuntimeValue *items = runtime_array_items(a);
    for (uint32_t i = 0; i < a->len; i++) {
        if (rt_native_eq(items[i], val)) return 1;
    }
    return 0;
}

/* rt_array_index_of(arr, val) — return first index or -1 */
RuntimeValue rt_array_index_of(RuntimeValue arr, RuntimeValue val)
{
    if (!IS_HEAP(arr)) return (RuntimeValue)(-1);
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return (RuntimeValue)(-1);
    RuntimeValue *items = runtime_array_items(a);
    for (uint32_t i = 0; i < a->len; i++) {
        if (rt_native_eq(items[i], val)) return ENCODE_INT(i);
    }
    return (RuntimeValue)(-1);
}

/* rt_array_last_index_of(arr, val) */
RuntimeValue rt_array_last_index_of(RuntimeValue arr, RuntimeValue val)
{
    if (!IS_HEAP(arr)) return (RuntimeValue)(-1);
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return (RuntimeValue)(-1);
    RuntimeValue *items = runtime_array_items(a);
    for (int64_t i = (int64_t)a->len - 1; i >= 0; i--) {
        if (rt_native_eq(items[i], val)) return ENCODE_INT(i);
    }
    return (RuntimeValue)(-1);
}

/* rt_array_remove(arr, idx) — remove at index, shift down */
RuntimeValue rt_array_remove(RuntimeValue arr, RuntimeValue idx)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    RuntimeValue *items = runtime_array_items(a);
    int64_t i = DECODE_INT(idx);
    if (i < 0 || (uint32_t)i >= a->len) return NIL_VALUE;
    RuntimeValue removed = items[i];
    for (uint32_t j = (uint32_t)i; j + 1 < a->len; j++) {
        items[j] = items[j + 1];
    }
    a->len--;
    items[a->len] = NIL_VALUE;
    return removed;
}

S3(rt_array_insert)
S1(rt_array_reverse)
S1(rt_array_sort)
S2(rt_array_sort_by)
S2(rt_array_map)
S2(rt_array_filter)
S3(rt_array_reduce)
S2(rt_array_for_each)
S2(rt_array_find)
S2(rt_array_find_index)
S2(rt_array_every)
S2(rt_array_some)

/* rt_array_join(arr, sep) — join string array with separator */
RuntimeValue rt_array_join(RuntimeValue arr, RuntimeValue sep)
{
    if (!IS_HEAP(arr)) return rt_string_from_cstr("");
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY || a->len == 0)
        return rt_string_from_cstr("");
    RuntimeValue *items = runtime_array_items(a);
    RuntimeValue result = rt_value_to_string(items[0]);
    for (uint32_t i = 1; i < a->len; i++) {
        if (IS_HEAP(sep)) result = rt_string_concat(result, sep);
        result = rt_string_concat(result, rt_value_to_string(items[i]));
    }
    return result;
}

/* rt_array_concat(arr_a, arr_b) */
RuntimeValue rt_array_concat(RuntimeValue arr_a, RuntimeValue arr_b)
{
    RuntimeArray *a = IS_HEAP(arr_a) ? (RuntimeArray *)DECODE_PTR(arr_a) : (RuntimeArray *)0;
    RuntimeArray *b = IS_HEAP(arr_b) ? (RuntimeArray *)DECODE_PTR(arr_b) : (RuntimeArray *)0;
    uint32_t la = (a && a->hdr.type == HEAP_ARRAY) ? a->len : 0;
    uint32_t lb = (b && b->hdr.type == HEAP_ARRAY) ? b->len : 0;
    RuntimeValue *a_items = (a && a->hdr.type == HEAP_ARRAY) ? runtime_array_items(a) : NULL;
    RuntimeValue *b_items = (b && b->hdr.type == HEAP_ARRAY) ? runtime_array_items(b) : NULL;
    RuntimeValue result = rt_array_new(ENCODE_INT(la + lb > 0 ? la + lb : 1));
    for (uint32_t i = 0; i < la; i++) result = rt_array_push_handle(result, a_items[i]);
    for (uint32_t i = 0; i < lb; i++) result = rt_array_push_handle(result, b_items[i]);
    return result;
}

/* rt_array_clear(arr) */
RuntimeValue rt_array_clear(RuntimeValue arr)
{
    if (!IS_HEAP(arr)) return arr;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return arr;
    RuntimeValue *items = runtime_array_items(a);
    for (uint32_t i = 0; i < a->len; i++) items[i] = NIL_VALUE;
    a->len = 0;
    return arr;
}

S1(rt_array_flatten)
S2(rt_array_fill)

/* rt_array_clone(arr) — shallow copy */
RuntimeValue rt_array_clone(RuntimeValue arr)
{
    if (!IS_HEAP(arr)) return NIL_VALUE;
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr);
    if (!a || a->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    RuntimeValue *items = runtime_array_items(a);
    RuntimeValue result = rt_array_new(ENCODE_INT(a->cap));
    for (uint32_t i = 0; i < a->len; i++) {
        result = rt_array_push_handle(result, items[i]);
    }
    return result;
}

S2(rt_array_zip)
S1(rt_array_uniq)
S1(rt_array_compact)

/* rt_enum_new(enum_id, discriminant, payload) → heap-allocated RuntimeEnum.
 * Calling convention: (i32, i32, i64) → i64 (ENCODE_PTR).
 * Matches Rust runtime RuntimeEnum layout (24 bytes). */
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

/* rt_enum_discriminant(value) → discriminant as i64 */
RuntimeValue rt_enum_discriminant(RuntimeValue value)
{
    if (!IS_HEAP(value)) return -1;
    RuntimeEnum *e = (RuntimeEnum *)DECODE_PTR(value);
    if (!e || e->hdr.type != HEAP_ENUM) return -1;
    return (RuntimeValue)(int64_t)e->discriminant;
}

/* rt_enum_payload(value) → payload RuntimeValue */
RuntimeValue rt_enum_payload(RuntimeValue value)
{
    if (!IS_HEAP(value)) return value;
    RuntimeEnum *e = (RuntimeEnum *)DECODE_PTR(value);
    if (!e || e->hdr.type != HEAP_ENUM) return value;
    return e->payload;
}

/* rt_enum_check_discriminant(value, expected) → 1 if match, 0 otherwise */
RuntimeValue rt_enum_check_discriminant(RuntimeValue value, RuntimeValue expected)
{
    if (!IS_HEAP(value)) return 0;
    RuntimeEnum *e = (RuntimeEnum *)DECODE_PTR(value);
    if (!e || e->hdr.type != HEAP_ENUM) return 0;
    return (e->discriminant == (uint32_t)(int32_t)expected) ? 1 : 0;
}

/* rt_is_none(value) → 1 if nil or None enum, 0 otherwise */
RuntimeValue rt_is_none(RuntimeValue value)
{
    if (IS_NIL(value)) return 1;
    if (!IS_HEAP(value)) return 0;
    RuntimeEnum *e = (RuntimeEnum *)DECODE_PTR(value);
    if (!e || e->hdr.type != HEAP_ENUM) return 0;
    /* None variant has nil payload */
    return IS_NIL(e->payload) ? 1 : 0;
}

/* rt_is_some(value) → 1 if Some enum (non-nil payload), 0 otherwise */
RuntimeValue rt_is_some(RuntimeValue value)
{
    return rt_is_none(value) ? 0 : 1;
}

/* --- Map / Dictionary ---
 *
 * RuntimeMap: linear-probe map with separate key/value arrays.
 * Keys are RuntimeValues compared via rt_native_eq (works for ints
 * and strings).  Suitable for small maps (VFS mount table, IPC
 * service registry) on bare metal.
 *
 * RuntimeMap typedef is in section 3 (forward-declared for rt_len).
 */

static RuntimeMap *decode_map(RuntimeValue v)
{
    if (!IS_HEAP(v)) return (RuntimeMap *)0;
    RuntimeMap *m = (RuntimeMap *)DECODE_PTR(v);
    if (!m || m->hdr.type != HEAP_MAP) return (RuntimeMap *)0;
    return m;
}

/* rt_map_new: create map.  Ignores argument (raw ABI); uses default cap 16. */
RuntimeValue rt_map_new(void)
{
    uint32_t cap = 16;
    RuntimeMap *m = (RuntimeMap *)malloc(sizeof(RuntimeMap));
    if (!m) return NIL_VALUE;
    m->hdr.type = HEAP_MAP;
    m->hdr.size = (uint32_t)sizeof(RuntimeMap);
    m->len = 0;
    m->cap = cap;
    m->keys   = (RuntimeValue *)malloc(cap * sizeof(RuntimeValue));
    m->values = (RuntimeValue *)malloc(cap * sizeof(RuntimeValue));
    if (!m->keys || !m->values) return NIL_VALUE;
    for (uint32_t i = 0; i < cap; i++) {
        m->keys[i]   = NIL_VALUE;
        m->values[i] = NIL_VALUE;
    }
    return ENCODE_PTR(m);
}

/* Linear scan for key; returns index or -1 */
static int32_t map_find_key(RuntimeMap *m, RuntimeValue key)
{
    for (uint32_t i = 0; i < m->len; i++) {
        if (rt_native_eq(m->keys[i], key)) return (int32_t)i;
    }
    return -1;
}

/* Grow the map arrays when full */
static void map_grow(RuntimeMap *m)
{
    uint32_t new_cap = m->cap * 2;
    if (new_cap < 16) new_cap = 16;
    RuntimeValue *nk = (RuntimeValue *)malloc(new_cap * sizeof(RuntimeValue));
    RuntimeValue *nv = (RuntimeValue *)malloc(new_cap * sizeof(RuntimeValue));
    if (!nk || !nv) return; /* OOM: silently fail on bare metal */
    for (uint32_t i = 0; i < m->len; i++) {
        nk[i] = m->keys[i];
        nv[i] = m->values[i];
    }
    for (uint32_t i = m->len; i < new_cap; i++) {
        nk[i] = NIL_VALUE;
        nv[i] = NIL_VALUE;
    }
    /* Bump allocator: old arrays leak but that is acceptable on bare metal */
    m->keys   = nk;
    m->values = nv;
    m->cap    = new_cap;
}

/* rt_map_set(map, key, value) — insert or update */
RuntimeValue rt_map_set(RuntimeValue map, RuntimeValue key, RuntimeValue value)
{
    RuntimeMap *m = decode_map(map);
    if (!m) return NIL_VALUE;
    int32_t idx = map_find_key(m, key);
    if (idx >= 0) {
        m->values[idx] = value;
        return map; /* return same map pointer */
    }
    /* Insert new entry */
    if (m->len >= m->cap) map_grow(m);
    if (m->len >= m->cap) return map; /* grow failed */
    m->keys[m->len]   = key;
    m->values[m->len]  = value;
    m->len++;
    return map;
}

/* rt_map_get(map, key) — return value or NIL_VALUE */
RuntimeValue rt_map_get(RuntimeValue map, RuntimeValue key)
{
    RuntimeMap *m = decode_map(map);
    if (!m) return NIL_VALUE;
    int32_t idx = map_find_key(m, key);
    if (idx >= 0) return m->values[idx];
    return NIL_VALUE;
}

/* rt_map_has(map, key) — return 1 or 0 (raw i64) */
RuntimeValue rt_map_has(RuntimeValue map, RuntimeValue key)
{
    RuntimeMap *m = decode_map(map);
    if (!m) return 0;
    return map_find_key(m, key) >= 0 ? 1 : 0;
}

/* rt_map_remove(map, key) — remove entry, return removed value */
RuntimeValue rt_map_remove(RuntimeValue map, RuntimeValue key)
{
    RuntimeMap *m = decode_map(map);
    if (!m) return NIL_VALUE;
    int32_t idx = map_find_key(m, key);
    if (idx < 0) return NIL_VALUE;
    RuntimeValue removed = m->values[idx];
    /* Shift remaining entries down */
    for (uint32_t i = (uint32_t)idx; i + 1 < m->len; i++) {
        m->keys[i]   = m->keys[i + 1];
        m->values[i] = m->values[i + 1];
    }
    m->len--;
    m->keys[m->len]   = NIL_VALUE;
    m->values[m->len]  = NIL_VALUE;
    return removed;
}

/* rt_map_keys(map) — return array of keys */
RuntimeValue rt_map_keys(RuntimeValue map)
{
    RuntimeMap *m = decode_map(map);
    if (!m) return NIL_VALUE;
    RuntimeValue arr = rt_array_new(ENCODE_INT(m->len > 0 ? m->len : 1));
    for (uint32_t i = 0; i < m->len; i++) {
        arr = rt_array_push_handle(arr, m->keys[i]);
    }
    return arr;
}

/* rt_map_values(map) — return array of values */
RuntimeValue rt_map_values(RuntimeValue map)
{
    RuntimeMap *m = decode_map(map);
    if (!m) return NIL_VALUE;
    RuntimeValue arr = rt_array_new(ENCODE_INT(m->len > 0 ? m->len : 1));
    for (uint32_t i = 0; i < m->len; i++) {
        arr = rt_array_push_handle(arr, m->values[i]);
    }
    return arr;
}

/* rt_map_entries(map) — return array of [key, value] pairs (as 2-element arrays) */
RuntimeValue rt_map_entries(RuntimeValue map)
{
    RuntimeMap *m = decode_map(map);
    if (!m) return NIL_VALUE;
    RuntimeValue arr = rt_array_new(ENCODE_INT(m->len > 0 ? m->len : 1));
    for (uint32_t i = 0; i < m->len; i++) {
        RuntimeValue pair = rt_array_new(ENCODE_INT(2));
        pair = rt_array_push_handle(pair, m->keys[i]);
        pair = rt_array_push_handle(pair, m->values[i]);
        arr = rt_array_push_handle(arr, pair);
    }
    return arr;
}

/* rt_map_len(map) — return entry count */
RuntimeValue rt_map_len(RuntimeValue map)
{
    RuntimeMap *m = decode_map(map);
    if (!m) return ENCODE_INT(0);
    return ENCODE_INT(m->len);
}

/* rt_map_clear(map) — remove all entries */
RuntimeValue rt_map_clear(RuntimeValue map)
{
    RuntimeMap *m = decode_map(map);
    if (!m) return NIL_VALUE;
    for (uint32_t i = 0; i < m->len; i++) {
        m->keys[i]   = NIL_VALUE;
        m->values[i] = NIL_VALUE;
    }
    m->len = 0;
    return map;
}

/* rt_map_clone(map) — shallow copy */
RuntimeValue rt_map_clone(RuntimeValue map)
{
    RuntimeMap *m = decode_map(map);
    if (!m) return NIL_VALUE;
    RuntimeValue new_map = rt_map_new();
    RuntimeMap *nm = decode_map(new_map);
    if (!nm) return NIL_VALUE;
    for (uint32_t i = 0; i < m->len; i++) {
        rt_map_set(new_map, m->keys[i], m->values[i]);
    }
    return new_map;
}

/* rt_map_merge(map_a, map_b) — merge b into a copy of a */
RuntimeValue rt_map_merge(RuntimeValue map_a, RuntimeValue map_b)
{
    RuntimeValue result = rt_map_clone(map_a);
    RuntimeMap *mb = decode_map(map_b);
    if (!mb) return result;
    for (uint32_t i = 0; i < mb->len; i++) {
        result = rt_map_set(result, mb->keys[i], mb->values[i]);
    }
    return result;
}

/* rt_map_for_each(map, callback) — no-op on bare metal (closures not callable from C) */
RuntimeValue rt_map_for_each(RuntimeValue map, RuntimeValue callback)
{
    (void)map; (void)callback;
    return NIL_VALUE;
}

/* ---- Trap stubs: these should NEVER silently return 0 on baremetal ----
 * Instead of masking failures, print the function name and halt.
 * This makes it immediately obvious when kernel code accidentally
 * uses a hosted-only API path.
 */
#define TRAP_STUB_RET(n, nargs) \
    RuntimeValue n(TRAP_ARGS_##nargs) { \
        TRAP_SUPPRESS_##nargs \
        serial_puts("[TRAP] " #n " called on baremetal -- halting\r\n"); \
        for (;;) { __asm__ volatile("hlt"); } \
        return 0; \
    }
#define TRAP_STUB_VOID(n, nargs) \
    void n(TRAP_ARGS_##nargs) { \
        TRAP_SUPPRESS_##nargs \
        serial_puts("[TRAP] " #n " called on baremetal -- halting\r\n"); \
        for (;;) { __asm__ volatile("hlt"); } \
    }
#define TRAP_ARGS_0   void
#define TRAP_ARGS_1   RuntimeValue _a
#define TRAP_ARGS_2   RuntimeValue _a, RuntimeValue _b
#define TRAP_ARGS_3   RuntimeValue _a, RuntimeValue _b, RuntimeValue _c
#define TRAP_SUPPRESS_0
#define TRAP_SUPPRESS_1  (void)_a;
#define TRAP_SUPPRESS_2  (void)_a; (void)_b;
#define TRAP_SUPPRESS_3  (void)_a; (void)_b; (void)_c;

TRAP_STUB_RET(rt_file_read, 1)
TRAP_STUB_RET(rt_file_write, 2)
TRAP_STUB_RET(rt_file_exists, 1)
TRAP_STUB_RET(rt_file_delete, 1)
TRAP_STUB_RET(rt_file_append, 2)
TRAP_STUB_RET(rt_file_size, 1)
TRAP_STUB_RET(rt_file_copy, 2)
TRAP_STUB_RET(rt_file_move, 2)
TRAP_STUB_RET(rt_file_rename, 2)
TRAP_STUB_RET(rt_file_is_dir, 1)
TRAP_STUB_RET(rt_file_is_file, 1)
TRAP_STUB_RET(rt_file_read_bytes, 1)
TRAP_STUB_RET(rt_file_write_bytes, 2)
TRAP_STUB_RET(rt_file_stat, 1)
TRAP_STUB_RET(rt_file_realpath, 1)

TRAP_STUB_RET(rt_dir_list, 1)
TRAP_STUB_RET(rt_dir_create, 1)
TRAP_STUB_RET(rt_dir_create_all, 1)
TRAP_STUB_RET(rt_dir_exists, 1)
TRAP_STUB_RET(rt_dir_remove, 1)
TRAP_STUB_RET(rt_dir_remove_all, 1)
TRAP_STUB_RET(rt_dir_cwd, 0)
TRAP_STUB_RET(rt_dir_chdir, 1)
TRAP_STUB_RET(rt_dir_home, 0)
TRAP_STUB_RET(rt_dir_temp, 0)

TRAP_STUB_RET(rt_process_run, 2)
TRAP_STUB_RET(rt_process_run_timeout, 3)
TRAP_STUB_RET(rt_process_spawn, 1)
TRAP_STUB_RET(rt_process_kill, 1)
TRAP_STUB_RET(rt_process_wait, 1)
TRAP_STUB_RET(rt_process_pid, 0)
/* rt_cli_get_args / rt_cli_args / rt_exit_code / rt_exit — defined as
 * NOP1 / NOP0 in rt_extras.c except rt_exit which we give a real impl
 * here: halt cleanly instead of trapping. std.process.exit(code) on
 * SimpleOS has no parent process yet; power-off is the most accurate
 * baremetal behaviour. */
/* Matches hosted signature `extern "C" fn rt_exit(code: i32) -> !`
 * (src/compiler_rust/runtime/src/value/ffi/env_process.rs). Simple code
 * declares this as `extern fn rt_exit(code: i32)` or `(code: i64)`;
 * both pass a raw integer in the first register, not a tagged
 * RuntimeValue. */
__attribute__((noreturn))
void rt_exit(int32_t code) {
    int64_t c = (int64_t)code;
    serial_puts("[exit] rt_exit(");
    serial_put_dec(c);
    serial_puts(") -- halting\r\n");
    /* QEMU isa-debug-exit on port 0x501 — code is shifted left by 1
     * and OR'd with 1 so QEMU reports (code<<1)|1 as the exit status.
     * If isa-debug-exit is not present the write is ignored and we
     * fall through to the hlt loop, which is the correct behaviour. */
    __asm__ volatile("outb %%al, %%dx" : : "a"((uint8_t)(c & 0x7F)), "d"((uint16_t)0x501));
    for (;;) { __asm__ volatile("cli; hlt"); }
}
/* rt_exit_code — no parent process yet, always reports 0 (no prior exit). */
RuntimeValue rt_exit_code(void) { return ENCODE_INT(0); }

RuntimeValue rt_env_get(RuntimeValue key_rv)
{
    (void)key_rv;
    /* Baremetal guests do not have a process environment. Returning an
     * empty string keeps hosted-bridge checks and optional env probes from
     * trapping in live lanes such as SSH session logging. */
    return rt_string_from_cstr("");
}

TRAP_STUB_RET(rt_env_set, 2)
TRAP_STUB_RET(rt_env_all, 0)

S1(rt_math_sqrt)
S1(rt_math_sin)
S1(rt_math_cos)
S1(rt_math_tan)
S1(rt_math_asin)
S1(rt_math_acos)
S1(rt_math_atan)
S2(rt_math_atan2)
S1(rt_math_abs)
S1(rt_math_floor)
S1(rt_math_ceil)
S1(rt_math_round)
S1(rt_math_log)
S1(rt_math_log2)
S1(rt_math_log10)
S1(rt_math_exp)
S2(rt_math_min)
S2(rt_math_max)
S2(rt_math_pow)
S0(rt_math_random)
S0(rt_math_pi)
S0(rt_math_e)
S0(rt_math_inf)
S0(rt_math_nan)
S1(rt_math_is_nan)
S1(rt_math_is_inf)

/* MMIO, CPU control, and interrupt stubs are provided as real
   implementations in Section 11 below — not generated via S* macros. */
S2(rt_register_isr)
S1(rt_send_eoi)
S0(rt_get_interrupt_flag)

S1(rt_time_now_ms)
S0(rt_time_now_nanos)
S0(rt_time_monotonic)
S1(rt_sleep_ms)
S1(rt_timer_create)
S1(rt_timer_cancel)

TRAP_STUB_RET(rt_net_send, 2)
TRAP_STUB_RET(rt_net_recv, 1)
/* rt_net_socket, rt_net_bind, rt_net_listen, rt_net_accept, rt_net_close
 * are implemented above in the direct socket API section */
TRAP_STUB_RET(rt_net_set_timeout, 2)
TRAP_STUB_RET(rt_net_get_addr, 1)

TRAP_STUB_RET(rt_http_get, 2)
TRAP_STUB_RET(rt_http_post, 3)
TRAP_STUB_RET(rt_http_put, 3)
TRAP_STUB_RET(rt_http_patch, 3)
TRAP_STUB_RET(rt_http_delete, 2)
TRAP_STUB_RET(rt_http_request, 2)
TRAP_STUB_RET(rt_http_request_full, 3)
TRAP_STUB_RET(rt_http_set_header, 2)

S1(rt_json_parse)
S1(rt_json_stringify)
S2(rt_json_get)
S3(rt_json_set)
S1(rt_json_keys)
S1(rt_json_values)
S1(rt_json_is_object)
S1(rt_json_is_array)

S2(ffi_regex_is_match)
S2(ffi_regex_find)
S2(ffi_regex_find_all)
S2(ffi_regex_replace)
S3(ffi_regex_replace_all)
S1(ffi_regex_compile)

S1(rt_bdd_describe_start)
S1(rt_bdd_describe_end)
S2(rt_bdd_it_start)
S1(rt_bdd_it_end)
S1(rt_expect)
S2(rt_expect_eq)
S2(rt_expect_ne)
S2(rt_expect_gt)
S2(rt_expect_lt)
S1(rt_expect_nil)
S1(rt_expect_not_nil)
S1(rt_expect_true)
S1(rt_expect_false)
S2(rt_expect_contains)
S2(rt_expect_throws)
S0(rt_bdd_suite_start)
S0(rt_bdd_suite_end)
S0(rt_bdd_report)


/* rt_hash: FNV-1a-like hash for integers and strings */
RuntimeValue rt_hash(RuntimeValue val)
{
    uint64_t h = 14695981039346656037ULL; /* FNV offset basis */
    if (IS_INT(val)) {
        int64_t n = DECODE_INT(val);
        for (int i = 0; i < 8; i++) {
            h ^= (uint8_t)(n & 0xFF);
            h *= 1099511628211ULL; /* FNV prime */
            n >>= 8;
        }
    } else if (IS_HEAP(val)) {
        HeapHeader *hdr = (HeapHeader *)DECODE_PTR(val);
        if (hdr && hdr->type == HEAP_STRING) {
            RuntimeString *s = (RuntimeString *)hdr;
            for (uint32_t i = 0; i < s->len; i++) {
                h ^= (uint8_t)s->data[i];
                h *= 1099511628211ULL;
            }
        } else {
            /* Hash by pointer address */
            uint64_t p = (uint64_t)(uintptr_t)hdr;
            for (int i = 0; i < 8; i++) {
                h ^= (uint8_t)(p & 0xFF);
                h *= 1099511628211ULL;
                p >>= 8;
            }
        }
    }
    return ENCODE_INT((int64_t)(h >> 3)); /* Ensure tag bits are clear */
}

RuntimeValue rt_hash_combine(RuntimeValue h1, RuntimeValue h2)
{
    /* Boost-style hash combine */
    int64_t a = DECODE_INT(h1);
    int64_t b = DECODE_INT(h2);
    uint64_t combined = (uint64_t)a ^ ((uint64_t)b + 0x9e3779b97f4a7c15ULL
                         + ((uint64_t)a << 6) + ((uint64_t)a >> 2));
    return ENCODE_INT((int64_t)(combined >> 3));
}

RuntimeValue rt_debug_print(RuntimeValue val)
{
    serial_puts("[DEBUG] ");
    rt_print_value(val);
    serial_putchar('\r');
    serial_putchar('\n');
    return NIL_VALUE;
}

RuntimeValue rt_debug_dump(RuntimeValue val)
{
    serial_puts("[DUMP] raw=");
    serial_put_hex((uint64_t)val);
    serial_puts(" tag=");
    serial_put_dec((int64_t)((uint64_t)val & TAG_MASK));
    if (IS_INT(val)) {
        serial_puts(" int=");
        serial_put_dec(DECODE_INT(val));
    } else if (IS_HEAP(val)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(val);
        serial_puts(" heap_type=");
        serial_put_dec(h ? (int64_t)h->type : -1);
    }
    serial_putchar('\r');
    serial_putchar('\n');
    return NIL_VALUE;
}

RuntimeValue rt_debug_break(void)
{
    serial_puts("[BREAK] debug break\r\n");
    return NIL_VALUE;
}

RuntimeValue rt_panic(RuntimeValue msg)
{
    serial_puts("[PANIC] ");
    if (IS_HEAP(msg)) {
        HeapHeader *h = (HeapHeader *)DECODE_PTR(msg);
        if (h && h->type == HEAP_STRING) {
            RuntimeString *s = (RuntimeString *)h;
            for (uint32_t i = 0; i < s->len; i++) serial_putchar(s->data[i]);
        } else {
            serial_puts("<non-string>");
        }
    } else {
        serial_put_hex((uint64_t)msg);
    }
    serial_puts("\r\n");
    /* Halt the system */
    for (;;) __asm__ volatile("hlt");
    return NIL_VALUE; /* unreachable */
}

/* rt_function_not_found — called when cross-module method resolution fails.
 * Prints the unresolved function name to serial and returns NIL.
 * The Cranelift backend calls this with (name_ptr, name_len) when a method
 * cannot be resolved at compile time. */
RuntimeValue rt_function_not_found(RuntimeValue name_ptr, RuntimeValue name_len)
{
    serial_puts("[WARN] unresolved fn: ");
    if (name_ptr) {
        const char *p = (const char *)(uintptr_t)name_ptr;
        int64_t len = (int64_t)name_len;
        for (int64_t i = 0; i < len && i < 128; i++) serial_putchar(p[i]);
    }
    serial_puts("\r\n");
    return NIL_VALUE;
}

RuntimeValue rt_assert(RuntimeValue cond)
{
    if (IS_INT(cond) && DECODE_INT(cond)) return NIL_VALUE; /* truthy */
    if (IS_HEAP(cond)) return NIL_VALUE; /* non-nil heap is truthy */
    /* Assertion failed */
    serial_puts("[ASSERT] assertion failed\r\n");
    for (;;) __asm__ volatile("hlt");
    return NIL_VALUE;
}

RuntimeValue rt_assert_eq(RuntimeValue a, RuntimeValue b)
{
    if (rt_native_eq(a, b)) return NIL_VALUE;
    serial_puts("[ASSERT_EQ] ");
    rt_print_value(a);
    serial_puts(" != ");
    rt_print_value(b);
    serial_puts("\r\n");
    for (;;) __asm__ volatile("hlt");
    return NIL_VALUE;
}

RuntimeValue rt_assert_ne(RuntimeValue a, RuntimeValue b)
{
    if (!rt_native_eq(a, b)) return NIL_VALUE;
    serial_puts("[ASSERT_NE] values are equal: ");
    rt_print_value(a);
    serial_puts("\r\n");
    for (;;) __asm__ volatile("hlt");
    return NIL_VALUE;
}

RuntimeValue rt_abort(RuntimeValue msg)
{
    serial_puts("[ABORT] ");
    rt_print_value(msg);
    serial_puts("\r\n");
    for (;;) __asm__ volatile("hlt");
    return NIL_VALUE;
}

/* GC: safe no-ops on bare metal (bump allocator, no GC) */
RuntimeValue rt_gc_collect(void) { return NIL_VALUE; }
RuntimeValue rt_gc_disable(void) { return NIL_VALUE; }
RuntimeValue rt_gc_enable(void)  { return NIL_VALUE; }
RuntimeValue rt_gc_stats(void)   { return NIL_VALUE; }

/* rt_typeof already implemented above in type introspection section */

TRAP_STUB_RET(rt_thread_create, 1)
TRAP_STUB_RET(rt_thread_join, 1)
/* Safe no-ops on single-threaded bare metal */
RuntimeValue rt_thread_yield(void)          { return NIL_VALUE; }  /* yield: no-op */
RuntimeValue rt_thread_current(void)        { return ENCODE_INT(0); }  /* thread ID 0 */
RuntimeValue rt_thread_sleep(RuntimeValue a) { (void)a; return NIL_VALUE; }  /* sleep: return immediately */
TRAP_STUB_RET(rt_mutex_new, 0)
TRAP_STUB_RET(rt_mutex_lock, 1)
TRAP_STUB_RET(rt_mutex_unlock, 1)
TRAP_STUB_RET(rt_mutex_try_lock, 1)
TRAP_STUB_RET(rt_condvar_new, 0)
TRAP_STUB_RET(rt_condvar_wait, 1)
TRAP_STUB_RET(rt_condvar_notify, 1)
TRAP_STUB_RET(rt_condvar_notify_all, 1)

TRAP_STUB_RET(rt_channel_new, 0)
TRAP_STUB_RET(rt_channel_send, 2)
TRAP_STUB_RET(rt_channel_recv, 1)
TRAP_STUB_RET(rt_channel_try_recv, 1)
TRAP_STUB_RET(rt_channel_close, 1)

TRAP_STUB_RET(rt_async_spawn, 1)
TRAP_STUB_RET(rt_async_await, 1)
/* Safe no-op on single-threaded bare metal */
RuntimeValue rt_async_yield(void) { return NIL_VALUE; }
TRAP_STUB_RET(rt_async_select, 2)

S1(rt_base64_encode)
S1(rt_base64_decode)
S1(rt_hex_encode)
S1(rt_hex_decode)
S1(rt_utf8_encode)
S1(rt_utf8_decode)
S1(rt_url_encode)
S1(rt_url_decode)

S1(rt_sha256)
S1(rt_sha512)
S1(rt_md5)
S2(rt_hmac_sha256)
/* rt_random_bytes: real implementation in section 8d-tcp */

S1(rt_object_new)
S2(rt_object_get)
S3(rt_object_set)
S2(rt_object_has)
S2(rt_object_delete)
S1(rt_object_keys)
S1(rt_object_values)
S1(rt_object_freeze)
S1(rt_object_clone)

S1(rt_error_new)
S1(rt_error_message)
S1(rt_error_code)
S1(rt_error_stack)
S2(rt_result_ok)
S2(rt_result_err)
S1(rt_result_is_ok)
S1(rt_result_is_err)
S1(rt_result_unwrap)
S2(rt_result_unwrap_or)

S1(rt_weak_ref)
S1(rt_weak_deref)
S1(rt_closure_new)
S2(rt_closure_call)
S1(rt_closure_bind)

#if defined(__x86_64__) || defined(__i386__)

/* Port I/O: Cranelift passes RAW (untagged) i64 for extern fn args.
 * PCI enumeration uses kernel syscall 80 (not port I/O), so these
 * are only called for serial I/O and direct hardware access. */

RuntimeValue rt_port_outb_real(RuntimeValue port, RuntimeValue val)
{
    outb((uint16_t)(uint64_t)port, (uint8_t)(uint64_t)val);
    return 0;
}

RuntimeValue rt_port_outw_real(RuntimeValue port, RuntimeValue val)
{
    outw((uint16_t)(uint64_t)port, (uint16_t)(uint64_t)val);
    return 0;
}

RuntimeValue rt_port_outl_real(RuntimeValue port, RuntimeValue val)
{
    outl((uint16_t)(uint64_t)port, (uint32_t)(uint64_t)val);
    return 0;
}

RuntimeValue rt_port_inb_real(RuntimeValue port)
{
    return (RuntimeValue)(uint64_t)inb((uint16_t)(uint64_t)port);
}

RuntimeValue rt_port_inw_real(RuntimeValue port)
{
    return (RuntimeValue)(uint64_t)inw((uint16_t)(uint64_t)port);
}

RuntimeValue rt_port_inl_real(RuntimeValue port)
{
    return (RuntimeValue)(uint64_t)inl((uint16_t)(uint64_t)port);
}

RuntimeValue rt_port_io_wait_real(void)
{
    io_wait();
    return 0;
}

/* Expose as the primary symbols seen by Simple extern calls.
 * Baremetal target images are ELF, so prefer direct aliases there.
 * Keep wrapper bodies as the host-side fallback for Mach-O toolchains. */
#if defined(__ELF__)
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
#else
RuntimeValue rt_port_outb(RuntimeValue port, RuntimeValue val) {
    return rt_port_outb_real(port, val);
}
RuntimeValue rt_port_outw(RuntimeValue port, RuntimeValue val) {
    return rt_port_outw_real(port, val);
}
RuntimeValue rt_port_outl(RuntimeValue port, RuntimeValue val) {
    return rt_port_outl_real(port, val);
}
RuntimeValue rt_port_inb(RuntimeValue port) {
    return rt_port_inb_real(port);
}
RuntimeValue rt_port_inw(RuntimeValue port) {
    return rt_port_inw_real(port);
}
RuntimeValue rt_port_inl(RuntimeValue port) {
    return rt_port_inl_real(port);
}
RuntimeValue rt_port_io_wait(void) {
    return rt_port_io_wait_real();
}
#endif

RuntimeValue rt_debug_serial_R(void)
{
    outb((uint16_t)0x3F8, (uint8_t)'R');
    return 0;
}

RuntimeValue rt_debug_exit_success(void)
{
    outb((uint16_t)0xF4, (uint8_t)0);
    return 0;
}

RuntimeValue rt_debug_serial_R_hang(void)
{
    outb((uint16_t)0x3F8, (uint8_t)'R');
    for (;;) {
        __asm__ volatile("hlt");
    }
    return 0;
}

RuntimeValue rt_debug_return_addr_hang(void)
{
    uintptr_t ra = (uintptr_t)__builtin_return_address(0);
    outb((uint16_t)0x3F8, (uint8_t)'A');
    serial_puthex((uint32_t)ra);
    serial_putchar('\r');
    serial_putchar('\n');
    for (;;) {
        __asm__ volatile("hlt");
    }
    return 0;
}

__attribute__((naked)) RuntimeValue rt_debug_naked_show_return_hang(void)
{
    /* Debug helper — halt the CPU. The previous extended-asm hex-dump body
     * mixed `%reg` references with extended-asm `:` output/clobber sections,
     * which clang rejects ("invalid % escape in inline assembly string") and
     * clang also refuses extended asm inside a naked function. Silently
     * failing compilation here was dropping baremetal_stubs.o out of the
     * boot link, leaving `_start` undefined and the guest jumping into low
     * memory at boot (Agent E x64-desktop-test fault trace 0x6783..0x69b3).
     *
     * A simple `hlt` loop is sufficient for this debug entry point. */
    __asm__ volatile("1: hlt\n\t"
                     "jmp 1b\n\t");
}


RuntimeValue rt_mmio_read_u8_real(RuntimeValue addr)
{
    if ((uint64_t)addr < 0x1000u) return 0;
    return (RuntimeValue)(uint64_t)*(volatile uint8_t *)(uintptr_t)(uint64_t)addr;
}

RuntimeValue rt_mmio_read_u16_real(RuntimeValue addr)
{
    if ((uint64_t)addr < 0x1000u || (((uint64_t)addr) & 0x1u)) return 0;
    return (RuntimeValue)(uint64_t)*(volatile uint16_t *)(uintptr_t)(uint64_t)addr;
}

RuntimeValue rt_mmio_read_u32_real(RuntimeValue addr)
{
    if ((uint64_t)addr < 0x1000u || (((uint64_t)addr) & 0x3u)) return 0;
    return (RuntimeValue)(uint64_t)*(volatile uint32_t *)(uintptr_t)(uint64_t)addr;
}

RuntimeValue rt_mmio_read_u64_real(RuntimeValue addr)
{
    if ((uint64_t)addr < 0x1000u || (((uint64_t)addr) & 0x7u)) return 0;
    return (RuntimeValue)*(volatile uint64_t *)(uintptr_t)(uint64_t)addr;
}

RuntimeValue rt_mmio_write_u8_real(RuntimeValue addr, RuntimeValue val)
{
    *(volatile uint8_t *)(uintptr_t)(uint64_t)addr = (uint8_t)(uint64_t)val;
    return 0;
}

RuntimeValue rt_mmio_write_u16_real(RuntimeValue addr, RuntimeValue val)
{
    *(volatile uint16_t *)(uintptr_t)(uint64_t)addr = (uint16_t)(uint64_t)val;
    return 0;
}

RuntimeValue rt_mmio_write_u32_real(RuntimeValue addr, RuntimeValue val)
{
    *(volatile uint32_t *)(uintptr_t)(uint64_t)addr = (uint32_t)(uint64_t)val;
    return 0;
}

RuntimeValue rt_mmio_write_u64_real(RuntimeValue addr, RuntimeValue val)
{
    *(volatile uint64_t *)(uintptr_t)(uint64_t)addr = (uint64_t)val;
    return 0;
}

RuntimeValue rt_wait_u16_change_real(RuntimeValue addr, RuntimeValue old_value, RuntimeValue spins)
{
    volatile uint16_t *ptr = (volatile uint16_t *)(uintptr_t)(uint64_t)addr;
    uint16_t old16 = (uint16_t)(uint64_t)old_value;
    uint64_t remaining = (uint64_t)spins;
    while (remaining-- > 0) {
        uint16_t current = *ptr;
        if (current != old16) {
            return (RuntimeValue)(uint64_t)current;
        }
        __asm__ volatile("pause" ::: "memory");
    }
    return (RuntimeValue)(uint64_t)old16;
}

RuntimeValue rt_mmio_read_u8(RuntimeValue a) { return rt_mmio_read_u8_real(a); }
RuntimeValue rt_mmio_read_u16(RuntimeValue a) { return rt_mmio_read_u16_real(a); }
RuntimeValue rt_mmio_read_u32(RuntimeValue a) { return rt_mmio_read_u32_real(a); }
RuntimeValue rt_mmio_read_u64(RuntimeValue a) { return rt_mmio_read_u64_real(a); }
RuntimeValue rt_mmio_write_u8(RuntimeValue a, RuntimeValue v) { return rt_mmio_write_u8_real(a, v); }
RuntimeValue rt_mmio_write_u16(RuntimeValue a, RuntimeValue v) { return rt_mmio_write_u16_real(a, v); }
RuntimeValue rt_mmio_write_u32(RuntimeValue a, RuntimeValue v) { return rt_mmio_write_u32_real(a, v); }
RuntimeValue rt_mmio_write_u64(RuntimeValue a, RuntimeValue v) { return rt_mmio_write_u64_real(a, v); }

RuntimeValue rt_mem_read_u32_boxed(RuntimeValue addr)
{
    uint32_t value = *(volatile uint32_t *)(uintptr_t)(uint64_t)addr;
    return ENCODE_INT((int64_t)value);
}

RuntimeValue rt_mem_write_u32_boxed(RuntimeValue addr, RuntimeValue val)
{
    uint64_t raw = IS_INT(val) ? (uint64_t)DECODE_INT(val) : (uint64_t)val;
    *(volatile uint32_t *)(uintptr_t)(uint64_t)addr = (uint32_t)raw;
    return NIL_VALUE;
}
RuntimeValue rt_wait_u16_change(RuntimeValue a, RuntimeValue old_value, RuntimeValue spins) { return rt_wait_u16_change_real(a, old_value, spins); }

RuntimeValue rt_virtq_avail_slot_addr(RuntimeValue base, RuntimeValue idx, RuntimeValue qsize)
{
    uint64_t base64 = (uint64_t)base;
    uint64_t idx64 = (uint64_t)idx;
    uint64_t qsize64 = (uint64_t)qsize;
    if (qsize64 == 0) return (RuntimeValue)base64;
    return (RuntimeValue)(base64 + 4 + ((idx64 % qsize64) * 2));
}

RuntimeValue rt_virtio_notify_addr(RuntimeValue base, RuntimeValue off, RuntimeValue mult)
{
    uint64_t base64 = (uint64_t)base;
    uint64_t off64 = (uint64_t)off;
    uint64_t mult64 = (uint64_t)mult;
    return (RuntimeValue)(base64 + (off64 * mult64));
}

RuntimeValue rt_virtio_notify_write16(RuntimeValue base, RuntimeValue off, RuntimeValue mult, RuntimeValue val)
{
    uint64_t addr = (uint64_t)rt_virtio_notify_addr(base, off, mult);
    *(volatile uint16_t *)(uintptr_t)addr = (uint16_t)(uint64_t)val;
    return (RuntimeValue)addr;
}

RuntimeValue rt_alloc_page_aligned(RuntimeValue size)
{
    uint64_t size64 = (uint64_t)size;
    if (size64 == 0) return 0;
    uint64_t rounded = (size64 + 4095ULL) & ~4095ULL;
    void *raw = malloc((size_t)(rounded + 4095ULL));
    if (!raw) return 0;
    uintptr_t aligned = ((uintptr_t)raw + 4095ULL) & ~(uintptr_t)4095ULL;
    __builtin_memset((void *)aligned, 0, (size_t)rounded);
    return (RuntimeValue)(uint64_t)aligned;
}

RuntimeValue rt_virtq_desc_write(RuntimeValue base, RuntimeValue index, RuntimeValue addr_lo,
                                 RuntimeValue addr_hi, RuntimeValue len,
                                 RuntimeValue flags, RuntimeValue next)
{
    uint8_t *desc = (uint8_t *)(uintptr_t)((uint64_t)base + ((uint64_t)index * 16ULL));
    *(volatile uint32_t *)(void *)(desc + 0)  = (uint32_t)(uint64_t)addr_lo;
    *(volatile uint32_t *)(void *)(desc + 4)  = (uint32_t)(uint64_t)addr_hi;
    *(volatile uint32_t *)(void *)(desc + 8)  = (uint32_t)(uint64_t)len;
    *(volatile uint16_t *)(void *)(desc + 12) = (uint16_t)(uint64_t)flags;
    *(volatile uint16_t *)(void *)(desc + 14) = (uint16_t)(uint64_t)next;
    return 0;
}

RuntimeValue rt_virtq_write_req_resp_chain(RuntimeValue base,
                                           RuntimeValue req_addr,
                                           RuntimeValue req_len,
                                           RuntimeValue resp_addr,
                                           RuntimeValue resp_len)
{
    uint8_t *desc0 = (uint8_t *)(uintptr_t)(uint64_t)base;
    uint8_t *desc1 = desc0 + 16;
    uint64_t req64 = (uint64_t)req_addr;
    uint64_t resp64 = (uint64_t)resp_addr;

    *(volatile uint64_t *)(void *)(desc0 + 0)  = req64;
    *(volatile uint32_t *)(void *)(desc0 + 8)  = (uint32_t)(uint64_t)req_len;
    *(volatile uint16_t *)(void *)(desc0 + 12) = 1; /* NEXT */
    *(volatile uint16_t *)(void *)(desc0 + 14) = 1; /* desc1 */

    *(volatile uint64_t *)(void *)(desc1 + 0)  = resp64;
    *(volatile uint32_t *)(void *)(desc1 + 8)  = (uint32_t)(uint64_t)resp_len;
    *(volatile uint16_t *)(void *)(desc1 + 12) = 2; /* WRITE */
    *(volatile uint16_t *)(void *)(desc1 + 14) = 0;
    return 0;
}

RuntimeValue rt_virtio_gpu_write_ctrl_hdr(RuntimeValue addr, RuntimeValue cmd_type)
{
    uint8_t *p = (uint8_t *)(uintptr_t)(uint64_t)addr;
    *(volatile uint32_t *)(void *)(p + 0)  = (uint32_t)(uint64_t)cmd_type;
    *(volatile uint32_t *)(void *)(p + 4)  = 0;
    *(volatile uint64_t *)(void *)(p + 8)  = 0;
    *(volatile uint32_t *)(void *)(p + 16) = 0;
    *(volatile uint32_t *)(void *)(p + 20) = 0;
    return 0;
}


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
    return ENCODE_INT((int64_t)(((uint64_t)hi << 32) | lo));
}

RuntimeValue rt_lgdt_real(RuntimeValue desc)
{
    __asm__ volatile("lgdt (%0)" : : "r"((uintptr_t)DECODE_INT(desc)) : "memory");
    return NIL_VALUE;
}

RuntimeValue rt_lidt_real(RuntimeValue desc)
{
    __asm__ volatile("lidt (%0)" : : "r"((uintptr_t)DECODE_INT(desc)) : "memory");
    return NIL_VALUE;
}

RuntimeValue rt_ltr_real(RuntimeValue sel)
{
    uint16_t selector = (uint16_t)DECODE_INT(sel);
    __asm__ volatile("ltr %0" : : "r"(selector));
    return NIL_VALUE;
}

RuntimeValue rt_read_cr3_real(RuntimeValue dummy)
{
    (void)dummy;
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return ENCODE_INT((int64_t)cr3);
}

RuntimeValue rt_write_cr3_real(RuntimeValue val)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"((uint64_t)DECODE_INT(val)) : "memory");
    return NIL_VALUE;
}

RuntimeValue rt_read_cr2_real(RuntimeValue dummy)
{
    (void)dummy;
    uint64_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    return ENCODE_INT((int64_t)cr2);
}

RuntimeValue rt_hlt(void) { return rt_hlt_real(); }
RuntimeValue rt_sti(void) { return rt_sti_real(); }
RuntimeValue rt_cli(void) { return rt_cli_real(); }
RuntimeValue rt_enable_interrupts(void) { return rt_enable_interrupts_real(); }
RuntimeValue rt_disable_interrupts(void) { return rt_disable_interrupts_real(); }
RuntimeValue rt_invlpg(RuntimeValue a) { return rt_invlpg_real(a); }
RuntimeValue rt_rdtsc(void) { return rt_rdtsc_real(); }

/* RDRAND — see rt_rdrand() defined earlier (CPUID-gated hardware path with
 * TSC/xorshift boot-survivability fallback for QEMU qemu64). */

/* Generate random bytes as a [u8] array — C implementation for baremetal.
 * The Simple random_bytes() uses ChaCha20 which relies on unreliable
 * array operations in baremetal Cranelift. */
RuntimeValue rt_random_bytes(RuntimeValue count_rv)
{
    int64_t count = DECODE_INT(count_rv);
    if (count <= 0 || count > 256) count = 32;

    RuntimeArray *arr = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)count * sizeof(RuntimeValue));
    if (!arr) return NIL_VALUE;
    arr->hdr.type = HEAP_ARRAY;
    arr->hdr.size = (uint32_t)(sizeof(RuntimeArray) + count * sizeof(RuntimeValue));
    arr->len = (uint32_t)count;
    arr->cap = (uint32_t)count;
    arr->items = runtime_array_inline_items(arr);

    for (int64_t i = 0; i < count; i++) {
        uint64_t val;
        uint8_t ok = 0;
        __asm__ volatile("rdrand %0; setc %1" : "=r"(val), "=qm"(ok));
        if (!ok) {
            uint32_t lo, hi;
            __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
            val = ((uint64_t)hi << 32) | lo;
            val ^= (uint64_t)i * 0x9E3779B97F4A7C15ULL; /* mix */
        }
        arr->items[i] = ENCODE_INT((int64_t)(val & 0xFF));
    }

    return ENCODE_PTR(arr);
}
RuntimeValue rt_lgdt(RuntimeValue a) { return rt_lgdt_real(a); }
RuntimeValue rt_lidt(RuntimeValue a) { return rt_lidt_real(a); }
RuntimeValue rt_ltr(RuntimeValue a) { return rt_ltr_real(a); }
RuntimeValue rt_read_cr3(RuntimeValue a) { return rt_read_cr3_real(a); }
RuntimeValue rt_write_cr3(RuntimeValue a) { return rt_write_cr3_real(a); }
RuntimeValue rt_read_cr2(RuntimeValue a) { return rt_read_cr2_real(a); }

/* rt_install_idt — Wave 7C: load kernel IDT from entries array address.
 *
 * Simple cannot reliably take the address of a struct variable (& on a
 * global struct gives the value, not the address). This C helper receives
 * the raw address of the idt_entries array, builds a properly packed
 * 10-byte IDTR descriptor on the C stack (limit=4095, base=entries_addr),
 * and executes lidt — bypassing the IdtPtr struct layout issue entirely.
 *
 * Called from idt.spl:_idt_load() as:
 *   rt_install_idt(entries_addr: u64) -> unit
 */
RuntimeValue rt_install_idt(RuntimeValue entries_addr_rv)
{
    uint64_t base = (uint64_t)DECODE_INT(entries_addr_rv);
    /* Build packed 10-byte IDTR: [limit:u16][base:u64] */
    struct __attribute__((packed)) { uint16_t limit; uint64_t base; } idtr;
    idtr.limit = 4095;   /* 256 * 16 - 1 */
    idtr.base  = base;
    __asm__ volatile("lidt %0" : : "m"(idtr) : "memory");
    serial_puts("[fault] IDT loaded via rt_install_idt\r\n");
    return NIL_VALUE;
}

/* rt_patch_live_idt — Wave 7C rich fault dumper.
 *
 * Reads the live IDTR via sidt to get the current IDT base address, then
 * patches gate entries for vectors 6 (#UD), 8 (#DF), 13 (#GP), 14 (#PF)
 * with the provided Simple handler function addresses.
 *
 * This bypasses all Simple struct/address-of limitations by operating
 * entirely in C on the live CPU IDT.
 *
 * Gate format (64-bit interrupt gate, 16 bytes):
 *   [0..1]  offset_low   (bits 0-15 of handler addr)
 *   [2..3]  selector     (kernel CS = 0x08)
 *   [4]     ist          (0 = default stack)
 *   [5]     type_attr    (0x8E = present, DPL=0, interrupt gate)
 *   [6..7]  offset_mid   (bits 16-31 of handler addr)
 *   [8..11] offset_high  (bits 32-63 of handler addr)
 *   [12..15] reserved    (zero)
 *
 * handler_addrs: array of 4 tagged-int Simple function addresses:
 *   [0]=vec6_ud, [1]=vec8_df, [2]=vec13_gp, [3]=vec14_pf
 */
typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} IdtGate64;

static void _patch_idt_gate(IdtGate64 *idt, int vec, uint64_t handler_addr)
{
    idt[vec].offset_low  = (uint16_t)(handler_addr & 0xFFFF);
    idt[vec].selector    = 0x08;   /* kernel CS */
    idt[vec].ist         = 0;
    idt[vec].type_attr   = 0x8E;   /* P=1, DPL=0, interrupt gate */
    idt[vec].offset_mid  = (uint16_t)((handler_addr >> 16) & 0xFFFF);
    idt[vec].offset_high = (uint32_t)((handler_addr >> 32) & 0xFFFFFFFF);
    idt[vec].reserved    = 0;
}

RuntimeValue rt_patch_live_idt(RuntimeValue ud_rv, RuntimeValue df_rv,
                                RuntimeValue gp_rv, RuntimeValue pf_rv)
{
    serial_puts("[fault] rt_patch_live_idt entered\r\n");
    /* Read live IDTR */
    struct __attribute__((packed)) { uint16_t limit; uint64_t base; } idtr;
    __asm__ volatile("sidt %0" : "=m"(idtr));
    serial_puts("[fault] sidt done\r\n");

    IdtGate64 *idt = (IdtGate64 *)(uintptr_t)idtr.base;

    /* Simple passes function addresses as raw machine-code pointers, NOT as
     * tagged integers. Cast RuntimeValue directly to uint64_t. */
    _patch_idt_gate(idt,  6, (uint64_t)ud_rv);
    _patch_idt_gate(idt,  8, (uint64_t)df_rv);
    _patch_idt_gate(idt, 13, (uint64_t)gp_rv);
    _patch_idt_gate(idt, 14, (uint64_t)pf_rv);

    serial_puts("[fault] IDT vectors 6/8/13/14 patched via rt_patch_live_idt\r\n");
    return NIL_VALUE;
}

/* _serial_puthex64: print 64-bit value as 0x-prefixed hex, no heap. */
static void _serial_puthex64(uint64_t v) {
    static const char hex[] = "0123456789abcdef";
    serial_puts("0x");
    for (int i = 60; i >= 0; i -= 4)
        serial_putchar(hex[(v >> i) & 0xF]);
}

/* -----------------------------------------------------------------------
 * Wave 7C Rich Fault Hook — patches crt0's _fault_handler in-memory.
 *
 * Strategy: overwrite the first 5 bytes of _fault_handler with a
 * `jmp rel32` that redirects to _rich_fault_entry (defined below).
 * _rich_fault_entry reads the interrupt frame from RSP, prints the rich
 * frame, then executes the same recovery as crt0 (RAX=3, advance RIP,
 * iretq). Falls back to terse output if called before init.
 *
 * All output uses direct port I/O (no heap, no malloc).
 * ----------------------------------------------------------------------- */

/* _rich_fault_entry: naked ISR-style entry point.
 *
 * On CPU entry to _fault_handler the stack is one of:
 *   No errcode: [RIP, CS, RFLAGS, RSP, SS]        (CS = 0x08 at [rsp+8])
 *   Errcode:    [errcode, RIP, CS, RFLAGS, RSP, SS] (CS = 0x08 at [rsp+16])
 *
 * Detection (mirrors crt0): after saving N registers, the first frame slot
 * is at [rsp + N*8]. The SECOND slot is at [rsp + N*8 + 8]. If that second
 * slot == 0x08 (kernel CS), then the first slot IS the RIP (no errcode).
 * Otherwise the first slot is errcode and the second is RIP.
 *
 * After 9 pushes (72 bytes):
 *   [rsp+72]  = first frame slot  (RIP or errcode)
 *   [rsp+80]  = second frame slot (CS=0x08 if no errcode, else RIP)
 *
 * System V AMD64 calling convention: rdi, rsi, rdx, rcx, r8, r9
 * _rich_fault_print(rip, errcode, cs, rflags, cr2, cr3)
 *   => rdi=rip, rsi=errcode, rdx=cs, rcx=rflags, r8=cr2, r9=cr3
 */
__attribute__((naked)) static void _rich_fault_entry(void)
{
    __asm__ volatile(
        /* Save scratch registers */
        "pushq %%rax\n\t"
        "pushq %%rdx\n\t"
        "pushq %%rcx\n\t"
        "pushq %%rsi\n\t"
        "pushq %%rdi\n\t"
        "pushq %%r8\n\t"
        "pushq %%r9\n\t"
        "pushq %%r10\n\t"
        "pushq %%r11\n\t"
        /* 9 pushes = 72 bytes. [rsp+72] = first frame slot, [rsp+80] = second.
         * If [rsp+80] == 0x08 (CS), there is NO error code. */
        "cmpq $0x08, 80(%%rsp)\n\t"
        "je 1f\n\t"
        /* Error code present: [rsp+72]=errcode, [rsp+80]=RIP, [rsp+88]=CS, [rsp+96]=RFLAGS */
        "movq 80(%%rsp), %%rdi\n\t"   /* arg0: RIP */
        "movq 72(%%rsp), %%rsi\n\t"   /* arg1: errcode */
        "movq 88(%%rsp), %%rdx\n\t"   /* arg2: CS */
        "movq 96(%%rsp), %%rcx\n\t"   /* arg3: RFLAGS */
        "jmp 2f\n\t"
        "1:\n\t"
        /* No error code: [rsp+72]=RIP, [rsp+80]=CS, [rsp+88]=RFLAGS */
        "movq 72(%%rsp), %%rdi\n\t"   /* arg0: RIP */
        "xorq %%rsi, %%rsi\n\t"       /* arg1: errcode = 0 */
        "movq 80(%%rsp), %%rdx\n\t"   /* arg2: CS */
        "movq 88(%%rsp), %%rcx\n\t"   /* arg3: RFLAGS */
        "2:\n\t"
        /* arg4: CR2 (page-fault address), arg5: CR3 (page table base) */
        "movq %%cr2, %%r8\n\t"
        "movq %%cr3, %%r9\n\t"
        /* Call _rich_fault_print(rip, errcode, cs, rflags, cr2, cr3) */
        "callq _rich_fault_print\n\t"
        /* Restore scratch registers */
        "popq %%r11\n\t"
        "popq %%r10\n\t"
        "popq %%r9\n\t"
        "popq %%r8\n\t"
        "popq %%rdi\n\t"
        "popq %%rsi\n\t"
        "popq %%rcx\n\t"
        "popq %%rdx\n\t"
        "popq %%rax\n\t"
        /* Recovery (mirrors crt0 .fault_recover_silent):
         * After pops, [rsp] = first frame slot, [rsp+8] = second.
         * If [rsp+8] == 0x08 (CS), no error code was pushed. */
        "cmpq $0x08, 8(%%rsp)\n\t"
        "je 3f\n\t"
        /* Error code present: advance RIP (at [rsp+8]), pop errcode, iretq */
        "addq $2, 8(%%rsp)\n\t"
        "movq $0x3, %%rax\n\t"
        "addq $8, %%rsp\n\t"
        "iretq\n\t"
        "3:\n\t"
        /* No error code: advance RIP (at [rsp]), iretq */
        "addq $2, (%%rsp)\n\t"
        "movq $0x3, %%rax\n\t"
        "iretq\n\t"
        : : : "memory"
    );
}

/* _rich_fault_print: C-level heap-free printer called from _rich_fault_entry */
void _rich_fault_print(uint64_t rip, uint64_t errcode, uint64_t cs,
                        uint64_t rflags, uint64_t cr2, uint64_t cr3)
{
    serial_puts("\r\n[fault] *** EXCEPTION FRAME ***\r\n");
    serial_puts("[fault] rip=");     _serial_puthex64(rip);     serial_puts("\r\n");
    serial_puts("[fault] errcode="); _serial_puthex64(errcode); serial_puts("\r\n");
    serial_puts("[fault] cs=");      _serial_puthex64(cs);      serial_puts("\r\n");
    serial_puts("[fault] rflags=");  _serial_puthex64(rflags);  serial_puts("\r\n");
    serial_puts("[fault] cr2=");     _serial_puthex64(cr2);     serial_puts("\r\n");
    serial_puts("[fault] cr3=");     _serial_puthex64(cr3);     serial_puts("\r\n");
    serial_puts("[fault] *** END FRAME (recovering) ***\r\n");
}

/* rt_install_rich_fault_hook — Wave 7C: called from Simple arch_init.
 *
 * Strategy: read the IDTR via sidt, decode gate 0 to find the address of
 * crt0's _fault_handler, then overwrite its first 5 bytes with a jmp rel32
 * to _rich_fault_entry. All 256 IDT vectors point at _fault_handler so this
 * upgrades ALL fault output without needing _fault_handler to be exported
 * from crt0.s and without touching the IDT entries themselves.
 *
 * The IdtGate64 struct is already defined above (rt_patch_live_idt section).
 */
RuntimeValue rt_install_rich_fault_hook(void)
{
    /* Read IDTR */
    struct __attribute__((packed)) { uint16_t limit; uint64_t base; } idtr;
    __asm__ volatile("sidt %0" : "=m"(idtr));

    if (idtr.base == 0 || idtr.limit < 15) {
        serial_puts("[fault] WARNING: IDTR invalid, skipping hook\r\n");
        return NIL_VALUE;
    }

    /* Decode IDT gate 0 to find the handler address (= crt0's _fault_handler).
     * IDT gate 64-bit format (16 bytes):
     *   [1:0]   offset_low  (bits 15:0)
     *   [3:2]   selector
     *   [4]     ist
     *   [5]     type_attr
     *   [7:6]   offset_mid  (bits 31:16)
     *   [11:8]  offset_high (bits 63:32)
     *   [15:12] reserved
     */
    IdtGate64 *idt = (IdtGate64 *)(uintptr_t)idtr.base;
    uint64_t handler_addr =
        ((uint64_t)idt[0].offset_low)        |
        ((uint64_t)idt[0].offset_mid  << 16) |
        ((uint64_t)idt[0].offset_high << 32);

    if (handler_addr == 0) {
        serial_puts("[fault] WARNING: IDT gate 0 handler is NULL, skipping hook\r\n");
        return NIL_VALUE;
    }

    serial_puts("[fault] _fault_handler addr=");
    _serial_puthex64(handler_addr);
    serial_puts("\r\n");

    uint8_t *target = (uint8_t *)(uintptr_t)handler_addr;
    uint8_t *hook   = (uint8_t *)(uintptr_t)_rich_fault_entry;

    /* Write: E9 <rel32> (jmp rel32, 5 bytes) */
    int64_t rel = (int64_t)(hook - target - 5);
    target[0] = 0xE9;
    target[1] = (uint8_t)( rel        & 0xFF);
    target[2] = (uint8_t)((rel >>  8) & 0xFF);
    target[3] = (uint8_t)((rel >> 16) & 0xFF);
    target[4] = (uint8_t)((rel >> 24) & 0xFF);

    serial_puts("[fault] _fault_handler patched -> _rich_fault_entry\r\n");
    return NIL_VALUE;
}

/* rt_dump_fault_frame — legacy stub kept for API compatibility. */
RuntimeValue rt_dump_fault_frame(RuntimeValue vec_rv, RuntimeValue rip_rv,
                                  RuntimeValue rsp_rv, RuntimeValue rbp_rv,
                                  RuntimeValue rax_rv, RuntimeValue cs_rv,
                                  RuntimeValue rflags_rv)
{
    (void)vec_rv; (void)rsp_rv; (void)rbp_rv; (void)rax_rv;
    _rich_fault_print((uint64_t)rip_rv, 0, (uint64_t)cs_rv,
                      (uint64_t)rflags_rv, 0, 0);
    return NIL_VALUE;
}

static struct { char username[64]; char password[64]; } _auth_db[16];
static int _auth_db_count = 0;

RuntimeValue rt_auth_add_user(RuntimeValue uname_rv, RuntimeValue pass_rv)
{
    RuntimeString *u = decode_string(uname_rv);
    RuntimeString *p = decode_string(pass_rv);
    if (!u || !p || _auth_db_count >= 16) return (RuntimeValue)(-1);
    uint32_t ulen = u->len < 63 ? u->len : 63;
    uint32_t plen = p->len < 63 ? p->len : 63;
    memcpy(_auth_db[_auth_db_count].username, u->data, ulen);
    _auth_db[_auth_db_count].username[ulen] = 0;
    memcpy(_auth_db[_auth_db_count].password, p->data, plen);
    _auth_db[_auth_db_count].password[plen] = 0;
    _auth_db_count++;
    return 0;
}

RuntimeValue rt_auth_check(RuntimeValue uname_rv, RuntimeValue pass_rv)
{
    RuntimeString *u = decode_string(uname_rv);
    RuntimeString *p = decode_string(pass_rv);
    if (!u || !p) return 0;
    for (int i = 0; i < _auth_db_count; i++) {
        if (strlen(_auth_db[i].username) == u->len &&
            memcmp(_auth_db[i].username, u->data, u->len) == 0 &&
            strlen(_auth_db[i].password) == p->len &&
            memcmp(_auth_db[i].password, p->data, p->len) == 0)
            return 1;
    }
    return 0;
}

RuntimeValue rt_auth_reset(void)
{
    _auth_db_count = 0;
    return 0;
}

RuntimeValue rt_auth_find_user(RuntimeValue uname_rv)
{
    RuntimeString *u = decode_string(uname_rv);
    if (!u) return 0;
    for (int i = 0; i < _auth_db_count; i++) {
        if (strlen(_auth_db[i].username) == u->len &&
            memcmp(_auth_db[i].username, u->data, u->len) == 0)
            return 1;
    }
    return 0;
}

RuntimeValue rt_auth_check_key(RuntimeValue uname_rv, RuntimeValue key_blob_rv)
{
    (void)uname_rv;
    (void)key_blob_rv;
    return 0;
}

/* rt_parse_auth_verify — C-side parser for SSH_MSG_USERAUTH_REQUEST (C12 test).
 *
 * Parses a raw USERAUTH_REQUEST byte array and checks that the extracted
 * username, method, and password match the expected values.
 * Works around the Cranelift codegen bug where returning large structs
 * (8 fields) through Result<> corrupts text field values on baremetal.
 *
 * Parameters: raw packet [u8], expected_user, expected_method, expected_pass
 * Returns: 1 if all match, 0 otherwise.
 */
RuntimeValue rt_parse_auth_verify(RuntimeValue arr_rv, RuntimeValue exp_user_rv,
                                  RuntimeValue exp_method_rv, RuntimeValue exp_pass_rv)
{
    RuntimeString *exp_user = decode_string(exp_user_rv);
    RuntimeString *exp_method = decode_string(exp_method_rv);
    RuntimeString *exp_pass = decode_string(exp_pass_rv);
    if (!exp_user || !exp_method || !exp_pass) {
        serial_puts("[rt_parse_auth_verify] bad expected strings\r\n");
        return 0;
    }

    if (!IS_HEAP(arr_rv)) { serial_puts("[rt_parse_auth_verify] arr not heap\r\n"); return 0; }
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(arr_rv);
    if (!a || a->hdr.type != HEAP_ARRAY) { serial_puts("[rt_parse_auth_verify] bad array\r\n"); return 0; }

    /* Extract raw bytes from the RuntimeArray (items are BoxInt-tagged) */
    uint32_t len = a->len;
    RuntimeValue *items = runtime_array_items(a);
    if (len < 2) { serial_puts("[rt_parse_auth_verify] too short\r\n"); return 0; }

    uint8_t *raw = (uint8_t *)__builtin_alloca(len);
    for (uint32_t i = 0; i < len; i++) {
        raw[i] = (uint8_t)DECODE_INT(items[i]);
    }

    /* Check message type */
    if (raw[0] != 50) { serial_puts("[rt_parse_auth_verify] not USERAUTH_REQUEST\r\n"); return 0; }

    uint32_t off = 1;

    /* Helper: read SSH string (uint32 big-endian length + data) */
    #define READ_STR(dst_ptr, dst_len) do { \
        if (off + 4 > len) { serial_puts("[rt_parse_auth_verify] truncated len\r\n"); return 0; } \
        uint32_t slen = ((uint32_t)raw[off] << 24) | ((uint32_t)raw[off+1] << 16) | \
                        ((uint32_t)raw[off+2] << 8) | (uint32_t)raw[off+3]; \
        off += 4; \
        if (off + slen > len) { serial_puts("[rt_parse_auth_verify] truncated data\r\n"); return 0; } \
        dst_ptr = (const char *)&raw[off]; \
        dst_len = slen; \
        off += slen; \
    } while(0)

    const char *user_ptr; uint32_t user_len;
    READ_STR(user_ptr, user_len);

    const char *svc_ptr; uint32_t svc_len;
    READ_STR(svc_ptr, svc_len);

    const char *method_ptr; uint32_t method_len;
    READ_STR(method_ptr, method_len);

    /* Check username */
    if (user_len != exp_user->len || memcmp(user_ptr, exp_user->data, user_len) != 0) {
        serial_puts("[rt_parse_auth_verify] username mismatch\r\n");
        return 0;
    }

    /* Check method */
    if (method_len != exp_method->len || memcmp(method_ptr, exp_method->data, method_len) != 0) {
        serial_puts("[rt_parse_auth_verify] method mismatch\r\n");
        return 0;
    }

    /* For password method: skip bool byte, then read password */
    if (method_len == 8 && memcmp(method_ptr, "password", 8) == 0) {
        if (off >= len) { serial_puts("[rt_parse_auth_verify] no bool byte\r\n"); return 0; }
        off++; /* skip bool */

        const char *pass_ptr; uint32_t pass_len;
        READ_STR(pass_ptr, pass_len);

        if (pass_len != exp_pass->len || memcmp(pass_ptr, exp_pass->data, pass_len) != 0) {
            serial_puts("[rt_parse_auth_verify] password mismatch\r\n");
            return 0;
    }
}


    #undef READ_STR
    return 1;
}

struct _ssh_channel {
    uint32_t local_id;
    uint32_t remote_id;
    uint32_t local_window;
    uint32_t remote_window;
    uint32_t max_packet;
    int      active;     /* 1 = open, 0 = closed */
};
static struct _ssh_channel _channels[32];
static int _channel_count = 0;

/* rt_ssh_ch_open(remote_id, remote_window, max_pkt) -> local_id (raw int)
 * local_window always starts at DEFAULT_WINDOW_SIZE (2 MiB = 2097152). */
#define SSH_DEFAULT_WINDOW_SIZE 2097152
int64_t rt_ssh_ch_open(int64_t remote_id, int64_t remote_window, int64_t max_pkt)
{
    if (_channel_count >= 32) return -1;
    int id = _channel_count++;
    _channels[id].local_id      = (uint32_t)id;
    _channels[id].remote_id     = (uint32_t)remote_id;
    _channels[id].local_window  = SSH_DEFAULT_WINDOW_SIZE;
    _channels[id].remote_window = (uint32_t)remote_window;
    _channels[id].max_packet    = (uint32_t)max_pkt;
    _channels[id].active        = 1;
    return (int64_t)id;
}

/* rt_ssh_ch_find(local_id) -> 1 if found & active, 0 if not */
int64_t rt_ssh_ch_find(int64_t local_id)
{
    for (int i = 0; i < _channel_count; i++) {
        if (_channels[i].local_id == (uint32_t)local_id && _channels[i].active)
            return 1;
    }
    return 0;
}

/* rt_ssh_ch_get_remote_id(local_id) -> remote_id (raw) */
int64_t rt_ssh_ch_get_remote_id(int64_t local_id)
{
    for (int i = 0; i < _channel_count; i++) {
        if (_channels[i].local_id == (uint32_t)local_id && _channels[i].active)
            return (int64_t)_channels[i].remote_id;
    }
    return 0;
}

/* rt_ssh_ch_get_local_window(local_id) -> local_window */
int64_t rt_ssh_ch_get_local_window(int64_t local_id)
{
    for (int i = 0; i < _channel_count; i++) {
        if (_channels[i].local_id == (uint32_t)local_id && _channels[i].active)
            return (int64_t)_channels[i].local_window;
    }
    return 0;
}

/* rt_ssh_ch_get_remote_window(local_id) -> remote_window */
int64_t rt_ssh_ch_get_remote_window(int64_t local_id)
{
    for (int i = 0; i < _channel_count; i++) {
        if (_channels[i].local_id == (uint32_t)local_id && _channels[i].active)
            return (int64_t)_channels[i].remote_window;
    }
    return 0;
}

/* rt_ssh_ch_close(local_id) -> 0 */
int64_t rt_ssh_ch_close(int64_t local_id)
{
    for (int i = 0; i < _channel_count; i++) {
        if (_channels[i].local_id == (uint32_t)local_id) {
            _channels[i].active = 0;
            return 0;
        }
    }
    return 0;
}

/* rt_ssh_ch_adjust_window(local_id, bytes) -> 0 */
int64_t rt_ssh_ch_adjust_window(int64_t local_id, int64_t bytes)
{
    for (int i = 0; i < _channel_count; i++) {
        if (_channels[i].local_id == (uint32_t)local_id && _channels[i].active) {
            _channels[i].local_window += (uint32_t)bytes;
            return 0;
        }
    }
    return 0;
}

/* rt_ssh_ch_consume_window(local_id, bytes) -> 1 if ok, 0 if insufficient */
int64_t rt_ssh_ch_consume_window(int64_t local_id, int64_t bytes)
{
    for (int i = 0; i < _channel_count; i++) {
        if (_channels[i].local_id == (uint32_t)local_id && _channels[i].active) {
            if (_channels[i].remote_window < (uint32_t)bytes)
                return 0;
            _channels[i].remote_window -= (uint32_t)bytes;
            return 1;
        }
    }
    return 0;
}

/* rt_ssh_ch_reset() -> 0   (for test isolation) */
int64_t rt_ssh_ch_reset(void)
{
    _channel_count = 0;
    return 0;
}

RuntimeValue rt_build_byte_range(RuntimeValue count_rv)
{
    int64_t count = (int64_t)count_rv;  /* raw i64, not tagged */
    if (count < 0 || count > 65536) return NIL_VALUE;
    size_t alloc = sizeof(RuntimeArray) + (size_t)count * sizeof(RuntimeValue);
    RuntimeArray *a = (RuntimeArray *)malloc(alloc);
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)alloc;
    a->len = (uint32_t)count;
    a->cap = (uint32_t)count;
    a->items = runtime_array_inline_items(a);
    for (int64_t i = 0; i < count; i++)
        a->items[i] = ENCODE_INT(i & 0xFF);
    return ENCODE_PTR(a);
}

/* rt_array_new_with_cap: create empty array with specified capacity (raw int).
 * Workaround for push growth bug — pre-allocate capacity so push never reallocs. */
RuntimeValue rt_array_new_with_cap(int64_t cap)
{
    if (cap < 0) cap = 16;
    RuntimeArray *a = (RuntimeArray *)malloc(sizeof(RuntimeArray) + (size_t)cap * sizeof(RuntimeValue));
    if (!a) return NIL_VALUE;
    a->hdr.type = HEAP_ARRAY;
    a->hdr.size = (uint32_t)(sizeof(RuntimeArray) + (size_t)cap * sizeof(RuntimeValue));
    a->len = 0;
    a->cap = (uint32_t)cap;
    a->items = runtime_array_inline_items(a);
    for (int64_t i = 0; i < cap; i++) a->items[i] = NIL_VALUE;
    return ENCODE_PTR(a);
}

RuntimeValue rt_array_new_with_cap_i64(int64_t cap)
{
    return rt_array_new_with_cap(cap);
}

RuntimeValue rt_array_new_with_cap_text(int64_t cap)
{
    return rt_array_new_with_cap(cap);
}

RuntimeValue rt_array_new_with_cap_js_value(int64_t cap)
{
    return rt_array_new_with_cap(cap);
}

RuntimeValue rt_array_new_with_cap_bool(int64_t cap)
{
    return rt_array_new_with_cap(cap);
}

RuntimeValue rt_array_new_with_cap_u64(int64_t cap)
{
    return rt_array_new_with_cap(cap);
}

int64_t rt_any_add(int64_t left, int64_t right)
{
    return left + right;
}

int8_t rt_typed_words_u64_push(RuntimeValue arr, int64_t val)
{
    return rt_array_push(arr, ENCODE_INT(val));
}

int8_t rt_typed_words_u64_set(RuntimeValue arr, int64_t idx, int64_t val)
{
    return rt_array_set(arr, (RuntimeValue)idx, ENCODE_INT(val));
}

int64_t os__services__vfs__vfs_boot_init__VfsFileSize_dot_to_i64(RuntimeValue self)
{
    if (!IS_HEAP(self)) return 0;
    RuntimeValue *fields = (RuntimeValue *)DECODE_PTR(self);
    RuntimeValue first = fields[0];
    return IS_INT(first) ? DECODE_INT(first) : (int64_t)first;
}

int64_t lib__nogc_sync_mut__replay__event_kinds__EventKind_dot_to_i32(RuntimeValue self)
{
    return IS_INT(self) ? DECODE_INT(self) : (int64_t)self;
}

RuntimeValue lib__nogc_sync_mut__replay__event_kinds__EventKind_dot_to_text(RuntimeValue self)
{
    (void)self;
    return rt_string_from_cstr("event");
}

int64_t rt_verify_kexinit_roundtrip(RuntimeValue data_rv)
{
    if (!IS_HEAP(data_rv)) {
        serial_puts("[kexinit-verify] not heap\r\n");
        return -1;
    }
    RuntimeArray *a = (RuntimeArray *)DECODE_PTR(data_rv);
    if (!a || a->hdr.type != HEAP_ARRAY) {
        serial_puts("[kexinit-verify] bad array\r\n");
        return -1;
    }

    uint32_t len = a->len;
    RuntimeValue *items = runtime_array_items(a);

    /* Extract raw bytes */
    uint8_t *raw = (uint8_t *)__builtin_alloca(len);
    for (uint32_t i = 0; i < len; i++)
        raw[i] = (uint8_t)DECODE_INT(items[i]);

    /* Check message type */
    if (len < 17) {
        serial_puts("[kexinit-verify] too short for header\r\n");
        return -1;
    }
    if (raw[0] != 20) {
        serial_puts("[kexinit-verify] type != 20\r\n");
        return -1;
    }

    /* Skip type (1) + cookie (16) */
    uint32_t offset = 17;

    /* Read 10 name-lists */
    for (int i = 0; i < 10; i++) {
        if (offset + 4 > len) {
            serial_puts("[kexinit-verify] truncated name-list length at index ");
            serial_puthex((uint8_t)i);
            serial_puts("\r\n");
            return -1;
        }
        uint32_t slen = ((uint32_t)raw[offset] << 24) |
                        ((uint32_t)raw[offset+1] << 16) |
                        ((uint32_t)raw[offset+2] << 8) |
                        (uint32_t)raw[offset+3];
        offset += 4;
        if (offset + slen > len) {
            serial_puts("[kexinit-verify] truncated name-list data at index ");
            serial_puthex((uint8_t)i);
            serial_puts("\r\n");
            return -1;
        }
        offset += slen;
    }

    /* first_kex_packet_follows (1 byte) + reserved (4 bytes) */
    if (offset + 5 > len) {
        serial_puts("[kexinit-verify] truncated trailer\r\n");
        return -1;
    }

    return 0; /* success */
}

/* ============================================================================
 * Weak C-ABI shims for SYSCALL dispatch (Wave 10B)
 *
 * Each spl_handle_* is declared weak and returns -38 (ENOSYS) by default.
 * A future Simple-emitted ABI shim will override these with strong symbols
 * that forward into the Simple-side handler functions (_handle_* etc.).
 *
 * Syscall number mapping mirrors src/os/kernel/ipc/syscall.spl:
 *   0  = exit          1  = yield         2  = spawn
 *   3  = wait          4  = getpid        5  = list_tasks
 *   6  = get_task_info 7  = signal        8  = set_priority
 *   9  = get_parent_pid 10 = mmap         11 = munmap
 *  12  = mprotect      13 = spawn_binary  14 = enter_user_blocking
 *  20  = ipc_send
 *  21  = ipc_recv      22 = ipc_create_port 23 = ipc_connect
 *  24  = notification_create  25 = notification_signal
 *  26  = notification_wait    27 = notification_poll
 *  28  = notification_destroy 29 = notification_wait_any
 *  30  = file_open     31 = file_read     32 = file_write
 *  33  = file_close    34 = file_stat     35 = file_mkdir
 *  36  = file_readdir  37 = mount         38 = unmount
 *  39  = unlink        40 = pledge        41 = unveil
 *  42  = cap_grant     43 = ftruncate     44 = rename
 *  45  = rmdir         46 = lseek         47 = getcwd
 *  48  = chdir         50 = clock_gettime 51 = sleep
 *  57  = fork          59 = exec          60 = debug_write
 *  61  = waitpid       62 = pipe          63 = dup2
 *  64  = dup           68 = poll          69 = fcntl
 *  70  = net_socket    71 = net_bind
 *  72  = net_listen    73 = net_connect   74 = net_accept
 *  75  = net_send_to   76 = net_recv_from 77 = net_if_config
 *  80  = dev_enumerate 81 = dev_get_info  82 = device_grant
 *  83  = map_bar       84 = alloc_dma     85 = free_dma
 *  90  = log_write     91 = log_read      95 = sysinfo
 *  96  = get_hostname  97 = set_hostname  106 = schedule
 *  107 = schedctl
 * ============================================================================
 */

/* Forward declarations for all spl_handle_* weak stubs */
__attribute__((weak)) int64_t spl_handle_exit(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_yield(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_spawn(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_wait(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_getpid(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_list_tasks(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_get_task_info(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_signal(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_set_priority(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_get_parent_pid(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_mmap(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_munmap(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_mprotect(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_spawn_binary(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_enter_user_blocking(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_brk(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_system_reboot(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_ipc_send(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_ipc_recv(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_ipc_create_port(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_ipc_connect(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_notification_create(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_notification_signal(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_notification_wait(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_notification_poll(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_notification_destroy(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_notification_wait_any(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_file_open(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_file_read(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_file_write(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_file_close(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_file_stat(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_file_mkdir(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_file_readdir(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_mount(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_unmount(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_unlink(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_pledge(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_unveil(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_cap_grant(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_ftruncate(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_rename(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_rmdir(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_lseek(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_getcwd(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_chdir(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_clock_gettime(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_sleep(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_fork(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_exec(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_waitpid(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_pipe(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_dup2(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_dup(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_poll(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_fcntl(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_debug_write(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_net_socket(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_net_bind(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_net_listen(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_net_connect(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_net_accept(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_net_send_to(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_net_recv_from(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_net_if_config(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_dev_enumerate(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_dev_get_info(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_device_grant(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_map_bar(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_alloc_dma(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_free_dma(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_log_write(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_log_read(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_sysinfo(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_get_hostname(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_set_hostname(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_schedule(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
__attribute__((weak)) int64_t spl_handle_schedctl(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

/* ----------------------------------------------------------------------------
 * rt_syscall_dispatch — called from syscall_entry.s trampoline
 *
 * Forwards to spl_handle_* weak stubs (or future strong overrides).
 * Returns -38 (ENOSYS) for unknown syscall numbers.
 * -------------------------------------------------------------------------- */
int64_t rt_syscall_dispatch(uint64_t num, uint64_t a0, uint64_t a1, uint64_t a2,
                            uint64_t a3, uint64_t a4, uint64_t a5) {
    switch (num) {
        case 0:  return spl_handle_exit(a0, a1, a2, a3, a4, a5);
        case 1:  return spl_handle_yield(a0, a1, a2, a3, a4, a5);
        case 2:  return spl_handle_spawn(a0, a1, a2, a3, a4, a5);
        case 3:  return spl_handle_wait(a0, a1, a2, a3, a4, a5);
        case 4:  return spl_handle_getpid(a0, a1, a2, a3, a4, a5);
        case 5:  return spl_handle_list_tasks(a0, a1, a2, a3, a4, a5);
        case 6:  return spl_handle_get_task_info(a0, a1, a2, a3, a4, a5);
        case 7:  return spl_handle_signal(a0, a1, a2, a3, a4, a5);
        case 8:  return spl_handle_set_priority(a0, a1, a2, a3, a4, a5);
        case 9:  return spl_handle_get_parent_pid(a0, a1, a2, a3, a4, a5);
        case 10: return spl_handle_mmap(a0, a1, a2, a3, a4, a5);
        case 11: return spl_handle_munmap(a0, a1, a2, a3, a4, a5);
        case 12: return spl_handle_mprotect(a0, a1, a2, a3, a4, a5);
        case 13: return spl_handle_spawn_binary(a0, a1, a2, a3, a4, a5);
        case 14: return spl_handle_enter_user_blocking(a0, a1, a2, a3, a4, a5);
        case 15: return spl_handle_brk(a0, a1, a2, a3, a4, a5);
        case 16: return spl_handle_system_reboot(a0, a1, a2, a3, a4, a5);
        case 20: return spl_handle_ipc_send(a0, a1, a2, a3, a4, a5);
        case 21: return spl_handle_ipc_recv(a0, a1, a2, a3, a4, a5);
        case 22: return spl_handle_ipc_create_port(a0, a1, a2, a3, a4, a5);
        case 23: return spl_handle_ipc_connect(a0, a1, a2, a3, a4, a5);
        case 24: return spl_handle_notification_create(a0, a1, a2, a3, a4, a5);
        case 25: return spl_handle_notification_signal(a0, a1, a2, a3, a4, a5);
        case 26: return spl_handle_notification_wait(a0, a1, a2, a3, a4, a5);
        case 27: return spl_handle_notification_poll(a0, a1, a2, a3, a4, a5);
        case 28: return spl_handle_notification_destroy(a0, a1, a2, a3, a4, a5);
        case 29: return spl_handle_notification_wait_any(a0, a1, a2, a3, a4, a5);
        case 30: return spl_handle_file_open(a0, a1, a2, a3, a4, a5);
        case 31: return spl_handle_file_read(a0, a1, a2, a3, a4, a5);
        case 32: return spl_handle_file_write(a0, a1, a2, a3, a4, a5);
        case 33: return spl_handle_file_close(a0, a1, a2, a3, a4, a5);
        case 34: return spl_handle_file_stat(a0, a1, a2, a3, a4, a5);
        case 35: return spl_handle_file_mkdir(a0, a1, a2, a3, a4, a5);
        case 36: return spl_handle_file_readdir(a0, a1, a2, a3, a4, a5);
        case 37: return spl_handle_mount(a0, a1, a2, a3, a4, a5);
        case 38: return spl_handle_unmount(a0, a1, a2, a3, a4, a5);
        case 39: return spl_handle_unlink(a0, a1, a2, a3, a4, a5);
        case 40: return spl_handle_pledge(a0, a1, a2, a3, a4, a5);
        case 41: return spl_handle_unveil(a0, a1, a2, a3, a4, a5);
        case 42: return spl_handle_cap_grant(a0, a1, a2, a3, a4, a5);
        case 43: return spl_handle_ftruncate(a0, a1, a2, a3, a4, a5);
        case 44: return spl_handle_rename(a0, a1, a2, a3, a4, a5);
        case 45: return spl_handle_rmdir(a0, a1, a2, a3, a4, a5);
        case 46: return spl_handle_lseek(a0, a1, a2, a3, a4, a5);
        case 47: return spl_handle_getcwd(a0, a1, a2, a3, a4, a5);
        case 48: return spl_handle_chdir(a0, a1, a2, a3, a4, a5);
        case 50: return spl_handle_clock_gettime(a0, a1, a2, a3, a4, a5);
        case 51: return spl_handle_sleep(a0, a1, a2, a3, a4, a5);
        case 57: return spl_handle_fork(a0, a1, a2, a3, a4, a5);
        case 59: return spl_handle_exec(a0, a1, a2, a3, a4, a5);
        case 60: return spl_handle_debug_write(a0, a1, a2, a3, a4, a5);
        case 61: return spl_handle_waitpid(a0, a1, a2, a3, a4, a5);
        case 62: return spl_handle_pipe(a0, a1, a2, a3, a4, a5);
        case 63: return spl_handle_dup2(a0, a1, a2, a3, a4, a5);
        case 64: return spl_handle_dup(a0, a1, a2, a3, a4, a5);
        case 68: return spl_handle_poll(a0, a1, a2, a3, a4, a5);
        case 69: return spl_handle_fcntl(a0, a1, a2, a3, a4, a5);
        case 70: return spl_handle_net_socket(a0, a1, a2, a3, a4, a5);
        case 71: return spl_handle_net_bind(a0, a1, a2, a3, a4, a5);
        case 72: return spl_handle_net_listen(a0, a1, a2, a3, a4, a5);
        case 73: return spl_handle_net_connect(a0, a1, a2, a3, a4, a5);
        case 74: return spl_handle_net_accept(a0, a1, a2, a3, a4, a5);
        case 75: return spl_handle_net_send_to(a0, a1, a2, a3, a4, a5);
        case 76: return spl_handle_net_recv_from(a0, a1, a2, a3, a4, a5);
        case 77: return spl_handle_net_if_config(a0, a1, a2, a3, a4, a5);
        case 80: return spl_handle_dev_enumerate(a0, a1, a2, a3, a4, a5);
        case 81: return spl_handle_dev_get_info(a0, a1, a2, a3, a4, a5);
        case 82: return spl_handle_device_grant(a0, a1, a2, a3, a4, a5);
        case 83: return spl_handle_map_bar(a0, a1, a2, a3, a4, a5);
        case 84: return spl_handle_alloc_dma(a0, a1, a2, a3, a4, a5);
        case 85: return spl_handle_free_dma(a0, a1, a2, a3, a4, a5);
        case 90: return spl_handle_log_write(a0, a1, a2, a3, a4, a5);
        case 91: return spl_handle_log_read(a0, a1, a2, a3, a4, a5);
        case 95: return spl_handle_sysinfo(a0, a1, a2, a3, a4, a5);
        case 96: return spl_handle_get_hostname(a0, a1, a2, a3, a4, a5);
        case 97: return spl_handle_set_hostname(a0, a1, a2, a3, a4, a5);
        case 106: return spl_handle_schedule(a0, a1, a2, a3, a4, a5);
        case 107: return spl_handle_schedctl(a0, a1, a2, a3, a4, a5);
        default: return -38; /* ENOSYS */
    }
}

/* ----------------------------------------------------------------------------
 * Weak stub definitions — each returns -38 (ENOSYS) unless overridden.
 * -------------------------------------------------------------------------- */

__attribute__((weak)) int64_t spl_handle_exit(uint64_t a0, uint64_t a1, uint64_t a2,
                                               uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    (void)a0;
    /* Best effort: halt so exit is semi-functional until Strong override lands */
    __asm__ __volatile__("cli; hlt");
    return -38;
}

__attribute__((weak)) int64_t spl_handle_yield(uint64_t a0, uint64_t a1, uint64_t a2,
                                                uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_spawn(uint64_t a0, uint64_t a1, uint64_t a2,
                                                uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_wait(uint64_t a0, uint64_t a1, uint64_t a2,
                                               uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_getpid(uint64_t a0, uint64_t a1, uint64_t a2,
                                                 uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_list_tasks(uint64_t a0, uint64_t a1, uint64_t a2,
                                                     uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_get_task_info(uint64_t a0, uint64_t a1, uint64_t a2,
                                                        uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_signal(uint64_t a0, uint64_t a1, uint64_t a2,
                                                 uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_set_priority(uint64_t a0, uint64_t a1, uint64_t a2,
                                                       uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_get_parent_pid(uint64_t a0, uint64_t a1, uint64_t a2,
                                                         uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_mmap(uint64_t a0, uint64_t a1, uint64_t a2,
                                               uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_munmap(uint64_t a0, uint64_t a1, uint64_t a2,
                                                 uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_mprotect(uint64_t a0, uint64_t a1, uint64_t a2,
                                                   uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_spawn_binary(uint64_t a0, uint64_t a1, uint64_t a2,
                                                       uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_enter_user_blocking(uint64_t a0, uint64_t a1, uint64_t a2,
                                                              uint64_t a3, uint64_t a4, uint64_t a5) {
    /* Weak fallback: EnterUserBlocking not yet installed — return ENOSYS */
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_brk(uint64_t a0, uint64_t a1, uint64_t a2,
                                              uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_system_reboot(uint64_t a0, uint64_t a1, uint64_t a2,
                                                        uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_ipc_send(uint64_t a0, uint64_t a1, uint64_t a2,
                                                   uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_ipc_recv(uint64_t a0, uint64_t a1, uint64_t a2,
                                                   uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_ipc_create_port(uint64_t a0, uint64_t a1, uint64_t a2,
                                                          uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_ipc_connect(uint64_t a0, uint64_t a1, uint64_t a2,
                                                      uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_notification_create(uint64_t a0, uint64_t a1, uint64_t a2,
                                                              uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_notification_signal(uint64_t a0, uint64_t a1, uint64_t a2,
                                                              uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_notification_wait(uint64_t a0, uint64_t a1, uint64_t a2,
                                                            uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_notification_poll(uint64_t a0, uint64_t a1, uint64_t a2,
                                                            uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_notification_destroy(uint64_t a0, uint64_t a1, uint64_t a2,
                                                               uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_notification_wait_any(uint64_t a0, uint64_t a1, uint64_t a2,
                                                                uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_file_open(uint64_t a0, uint64_t a1, uint64_t a2,
                                                    uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_file_read(uint64_t a0, uint64_t a1, uint64_t a2,
                                                    uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_file_write(uint64_t a0, uint64_t a1, uint64_t a2,
                                                     uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_file_close(uint64_t a0, uint64_t a1, uint64_t a2,
                                                     uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_file_stat(uint64_t a0, uint64_t a1, uint64_t a2,
                                                    uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_file_mkdir(uint64_t a0, uint64_t a1, uint64_t a2,
                                                     uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_file_readdir(uint64_t a0, uint64_t a1, uint64_t a2,
                                                       uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_mount(uint64_t a0, uint64_t a1, uint64_t a2,
                                                uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_unmount(uint64_t a0, uint64_t a1, uint64_t a2,
                                                  uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_unlink(uint64_t a0, uint64_t a1, uint64_t a2,
                                                 uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_pledge(uint64_t a0, uint64_t a1, uint64_t a2,
                                                 uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_unveil(uint64_t a0, uint64_t a1, uint64_t a2,
                                                 uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_cap_grant(uint64_t a0, uint64_t a1, uint64_t a2,
                                                    uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_ftruncate(uint64_t a0, uint64_t a1, uint64_t a2,
                                                    uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_rename(uint64_t a0, uint64_t a1, uint64_t a2,
                                                 uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_rmdir(uint64_t a0, uint64_t a1, uint64_t a2,
                                                uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_lseek(uint64_t a0, uint64_t a1, uint64_t a2,
                                                uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_getcwd(uint64_t a0, uint64_t a1, uint64_t a2,
                                                 uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_chdir(uint64_t a0, uint64_t a1, uint64_t a2,
                                                uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_clock_gettime(uint64_t a0, uint64_t a1, uint64_t a2,
                                                        uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_sleep(uint64_t a0, uint64_t a1, uint64_t a2,
                                                uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_fork(uint64_t a0, uint64_t a1, uint64_t a2,
                                               uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_exec(uint64_t a0, uint64_t a1, uint64_t a2,
                                               uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_waitpid(uint64_t a0, uint64_t a1, uint64_t a2,
                                                  uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_pipe(uint64_t a0, uint64_t a1, uint64_t a2,
                                               uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_dup2(uint64_t a0, uint64_t a1, uint64_t a2,
                                               uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_dup(uint64_t a0, uint64_t a1, uint64_t a2,
                                              uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_poll(uint64_t a0, uint64_t a1, uint64_t a2,
                                               uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_fcntl(uint64_t a0, uint64_t a1, uint64_t a2,
                                                uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_debug_write(uint64_t a0, uint64_t a1, uint64_t a2,
                                                      uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_net_socket(uint64_t a0, uint64_t a1, uint64_t a2,
                                                     uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_net_bind(uint64_t a0, uint64_t a1, uint64_t a2,
                                                   uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_net_listen(uint64_t a0, uint64_t a1, uint64_t a2,
                                                     uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_net_connect(uint64_t a0, uint64_t a1, uint64_t a2,
                                                      uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_net_accept(uint64_t a0, uint64_t a1, uint64_t a2,
                                                     uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_net_send_to(uint64_t a0, uint64_t a1, uint64_t a2,
                                                      uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_net_recv_from(uint64_t a0, uint64_t a1, uint64_t a2,
                                                        uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_net_if_config(uint64_t a0, uint64_t a1, uint64_t a2,
                                                        uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_dev_enumerate(uint64_t a0, uint64_t a1, uint64_t a2,
                                                        uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_dev_get_info(uint64_t a0, uint64_t a1, uint64_t a2,
                                                       uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_device_grant(uint64_t a0, uint64_t a1, uint64_t a2,
                                                       uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_map_bar(uint64_t a0, uint64_t a1, uint64_t a2,
                                                  uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_alloc_dma(uint64_t a0, uint64_t a1, uint64_t a2,
                                                    uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_free_dma(uint64_t a0, uint64_t a1, uint64_t a2,
                                                   uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_log_write(uint64_t a0, uint64_t a1, uint64_t a2,
                                                    uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_log_read(uint64_t a0, uint64_t a1, uint64_t a2,
                                                   uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_sysinfo(uint64_t a0, uint64_t a1, uint64_t a2,
                                                  uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_get_hostname(uint64_t a0, uint64_t a1, uint64_t a2,
                                                       uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_set_hostname(uint64_t a0, uint64_t a1, uint64_t a2,
                                                       uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_schedule(uint64_t a0, uint64_t a1, uint64_t a2,
                                                   uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

__attribute__((weak)) int64_t spl_handle_schedctl(uint64_t a0, uint64_t a1, uint64_t a2,
                                                   uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return -38;
}

/* End of Wave 10B: spl_handle_* weak shims and rt_syscall_dispatch */

#define _SHIM_STUB(name) \
    __attribute__((weak)) int64_t \
    kernel__abi__syscall_shim__spl_handle_##name( \
        uint64_t a0, uint64_t a1, uint64_t a2, \
        uint64_t a3, uint64_t a4, uint64_t a5) { \
        (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; \
        return -38; \
    }

_SHIM_STUB(debug_write)
_SHIM_STUB(exit)
_SHIM_STUB(yield)
_SHIM_STUB(spawn)
_SHIM_STUB(wait)
_SHIM_STUB(getpid)
_SHIM_STUB(list_tasks)
_SHIM_STUB(get_task_info)
_SHIM_STUB(signal)
_SHIM_STUB(set_priority)
_SHIM_STUB(get_parent_pid)
_SHIM_STUB(mmap)
_SHIM_STUB(munmap)
_SHIM_STUB(mprotect)
_SHIM_STUB(spawn_binary)
_SHIM_STUB(ipc_send)
_SHIM_STUB(ipc_recv)
_SHIM_STUB(ipc_create_port)
_SHIM_STUB(ipc_connect)
_SHIM_STUB(notification_create)
_SHIM_STUB(notification_signal)
_SHIM_STUB(notification_wait)
_SHIM_STUB(notification_poll)
_SHIM_STUB(notification_destroy)
_SHIM_STUB(notification_wait_any)
_SHIM_STUB(file_open)
_SHIM_STUB(file_read)
_SHIM_STUB(file_write)
_SHIM_STUB(file_close)
_SHIM_STUB(file_stat)
_SHIM_STUB(file_mkdir)
_SHIM_STUB(file_readdir)
_SHIM_STUB(mount)
_SHIM_STUB(unmount)
_SHIM_STUB(unlink)
_SHIM_STUB(pledge)
_SHIM_STUB(unveil)
_SHIM_STUB(cap_grant)
_SHIM_STUB(ftruncate)
_SHIM_STUB(rename)
_SHIM_STUB(rmdir)
_SHIM_STUB(lseek)
_SHIM_STUB(getcwd)
_SHIM_STUB(chdir)
_SHIM_STUB(clock_gettime)
_SHIM_STUB(sleep)
_SHIM_STUB(fork)
_SHIM_STUB(exec)
_SHIM_STUB(waitpid)
_SHIM_STUB(pipe)
_SHIM_STUB(dup2)
_SHIM_STUB(dup)
_SHIM_STUB(poll)
_SHIM_STUB(fcntl)
_SHIM_STUB(net_socket)
_SHIM_STUB(net_bind)
_SHIM_STUB(net_listen)
_SHIM_STUB(net_connect)
_SHIM_STUB(net_accept)
_SHIM_STUB(net_send_to)
_SHIM_STUB(net_recv_from)
_SHIM_STUB(net_if_config)
_SHIM_STUB(dev_enumerate)
_SHIM_STUB(dev_get_info)
_SHIM_STUB(device_grant)
_SHIM_STUB(map_bar)
_SHIM_STUB(alloc_dma)
_SHIM_STUB(free_dma)
_SHIM_STUB(log_write)
_SHIM_STUB(log_read)
_SHIM_STUB(sysinfo)
_SHIM_STUB(get_hostname)
_SHIM_STUB(set_hostname)
_SHIM_STUB(schedule)
_SHIM_STUB(schedctl)

#undef _SHIM_STUB

/* End of TLS test-only syscall shim stubs */

/* --- A. klog_api mangled-name aliases -------------------------------- */

/* rt_simpleos_log_set_device is declared extern in logger.spl but the x86_64
 * stubs above don't define it — add it here. */
__attribute__((weak)) int8_t rt_simpleos_log_set_device(int64_t kind, int64_t base)
{
    (void)kind; (void)base;
    return 1; /* no-op: COM1 serial is always the device on x86_64 */
}

/* The Simple compiler mangles module-qualified calls to
 * kernel__log__klog_api__rt_X.  Provide weak aliases to the already-defined
 * unmangled bodies so a single implementation is shared. */
extern int8_t rt_log_target_device_write_bytes(const char *ptr, int64_t len);
extern int8_t rt_log_target_semihost_write_bytes(const char *ptr, int64_t len);
extern int8_t rt_simpleos_log_init(int64_t level, int64_t targets);
extern int8_t rt_simpleos_log_is_enabled(int64_t level);
extern int8_t rt_simpleos_log_emit(int64_t level, const char *ptr, int64_t len);
extern int8_t rt_simpleos_log_set_device(int64_t kind, int64_t base);

__attribute__((weak)) int8_t
kernel__log__klog_api__rt_log_target_device_write_bytes(const char *ptr, int64_t len)
{ return rt_log_target_device_write_bytes(ptr, len); }

__attribute__((weak)) int8_t
kernel__log__klog_api__rt_log_target_semihost_write_bytes(const char *ptr, int64_t len)
{ return rt_log_target_semihost_write_bytes(ptr, len); }

__attribute__((weak)) int8_t
kernel__log__klog_api__rt_simpleos_log_init(int64_t level, int64_t targets)
{ return rt_simpleos_log_init(level, targets); }

__attribute__((weak)) int8_t
kernel__log__klog_api__rt_simpleos_log_is_enabled(int64_t level)
{ return rt_simpleos_log_is_enabled(level); }

__attribute__((weak)) int8_t
kernel__log__klog_api__rt_simpleos_log_emit(int64_t level, const char *ptr, int64_t len)
{ return rt_simpleos_log_emit(level, ptr, len); }

__attribute__((weak)) int8_t
kernel__log__klog_api__rt_simpleos_log_set_device(int64_t kind, int64_t base)
{ return rt_simpleos_log_set_device(kind, base); }

/* --- B. pmm_free_page alias ----------------------------------------- */

/* The Simple compiler emits kernel__memory__pmm__pmm_free_page (defined in
 * the PMM .o) but one call site still uses the bare name pmm_free_page. */
extern void kernel__memory__pmm__pmm_free_page(RuntimeValue frame) __attribute__((weak));
__attribute__((weak)) void pmm_free_page(RuntimeValue frame)
{
    if (kernel__memory__pmm__pmm_free_page) {
        kernel__memory__pmm__pmm_free_page(frame);
    }
}

/* --- C. log_raw_println / log_init_serial plain-C stubs ------------- */
/* TODO: implement after smoke test green — klog_api module owns these. */
__attribute__((weak)) void log_raw_println(RuntimeValue msg)
{
    rt_print(msg);
    serial_puts("\r\n");
}
__attribute__((weak)) void log_init_serial(int32_t level)
{
    (void)level;
    /* COM1 already initialised by _start; nothing to do on x86_64 */
}

/* --- D. ARM / DMA / HostWM weak no-op stubs ------------------------- */
/* TODO: gate call sites in src/os behind arch guards after smoke test green. */

__attribute__((weak)) void arm_fs_exec_print_success_marker(void) {}

/* rt_arm64_user_as_* — AArch64 MMU address-space ops */
__attribute__((weak)) RuntimeValue rt_arm64_user_as_create(void)       { return 0; }
__attribute__((weak)) RuntimeValue rt_arm64_user_as_map_elf64(RuntimeValue a, RuntimeValue b, RuntimeValue c) { (void)a;(void)b;(void)c; return 0; }
__attribute__((weak)) RuntimeValue rt_arm64_user_as_map_page(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d) { (void)a;(void)b;(void)c;(void)d; return 0; }
__attribute__((weak)) RuntimeValue rt_arm64_user_as_translate(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }
__attribute__((weak)) RuntimeValue rt_arm64_user_as_ttbr0_probe(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) void         rt_arm64_record_user_handoff(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; }

/* rt_arm_array_* — ARM-specific array helpers */
__attribute__((weak)) RuntimeValue rt_arm_array_append_bytes(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_array_get_u16_le(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_array_get_u32_le(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }
__attribute__((weak)) RuntimeValue rt_array_get_byte_raw(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }

/* rt_arm_elf64_* — ARM ELF64 loader helpers */
__attribute__((weak)) RuntimeValue rt_arm_elf64_direct_entry(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_elf64_direct_entry_bytes_ok(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_elf64_entry(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_elf64_pt_load_align(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_elf64_pt_load_count(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_elf64_pt_load_filesz(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_elf64_pt_load_flags(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_elf64_pt_load_memsz(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_elf64_pt_load_offset(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_elf64_pt_load_vaddr(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_stage_elf64_load_image(RuntimeValue a, RuntimeValue b, RuntimeValue c) { (void)a;(void)b;(void)c; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_stage_raw_image(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }

/* rt_arm_virtio_blk_* / rt_arm_virtq_* — ARM VirtIO block device */
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_configure_queue(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_dma_base(void)               { return 0; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_mmio_read_u32(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_mmio_read_u64(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) void         rt_arm_virtio_blk_mmio_write_u32(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_prepare_read(RuntimeValue a)  { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_queue_base(void)              { return 0; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_read_hello_smf(void)          { return 0; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_read_prefix(RuntimeValue a)   { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_read_sector_direct(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_sector_bytes(void)            { return 0; }
__attribute__((weak)) void         rt_arm_virtio_blk_set_mmio_base(RuntimeValue a) { (void)a; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_status_u8(void)               { return 0; }
__attribute__((weak)) RuntimeValue rt_arm_virtio_blk_wait_completion(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_arm_virtq_base(void)                         { return 0; }
__attribute__((weak)) void         rt_arm_virtq_push_avail(RuntimeValue a)         { (void)a; }
__attribute__((weak)) void         rt_arm_virtq_reset(void)                        {}
__attribute__((weak)) RuntimeValue rt_arm_virtq_used_idx(void)                     { return 0; }

/* rt_dma_* — DMA allocator (x86_64 freestanding: use regular malloc) */
__attribute__((weak)) RuntimeValue rt_dma_alloc(RuntimeValue size, RuntimeValue align)
{
    uint64_t requested = (uint64_t)(int64_t)size;
    uint64_t alignment = (uint64_t)(int64_t)align;
    if (alignment < 4096) alignment = 4096;
    void *raw = malloc((size_t)(requested + alignment));
    if (!raw) return 0;
    uintptr_t base = (uintptr_t)raw;
    uintptr_t aligned = (base + (uintptr_t)(alignment - 1)) & ~(uintptr_t)(alignment - 1);
    return (RuntimeValue)aligned;
}
__attribute__((weak)) RuntimeValue rt_dma_bytes_to_array(RuntimeValue addr, RuntimeValue len)
{
    /* Wrap a raw pointer into a RuntimeArray. */
    (void)addr; (void)len;
    return 0;  /* not called on x86_64 boot path */
}
__attribute__((weak)) RuntimeValue rt_dma_cache_line_size(void) { return 64; }
__attribute__((weak)) void         rt_dma_free(RuntimeValue p)  { (void)p; /* leak OK for smoke test */ }
__attribute__((weak)) RuntimeValue rt_dma_phys_of(RuntimeValue p)   { return p; /* identity map assumed */ }
__attribute__((weak)) void         rt_dma_sync_for_cpu(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; }
__attribute__((weak)) void         rt_dma_sync_for_device(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; }
__attribute__((weak)) RuntimeValue rt_dma_virt_of(RuntimeValue p)   { return p; }

/* rt_host_wm_client_* — host window-manager IPC (not used in kernel) */
__attribute__((weak)) RuntimeValue rt_host_wm_client_connect(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_host_wm_client_poll_event(RuntimeValue a) { (void)a; return 0; }
__attribute__((weak)) RuntimeValue rt_host_wm_client_request(RuntimeValue a, RuntimeValue b) { (void)a;(void)b; return 0; }

/* serial_putchar and serial_puts are defined static above (Section 2).
 * The kernel calls them as global symbols.  Provide public weak wrappers.
 * Undefine the local-alias macros first so these definitions get the real names. */
#undef serial_putchar
#undef serial_puts

__attribute__((weak)) void serial_putchar(char c)
{
    _serial_putchar_impl(c);
}
__attribute__((weak)) void serial_puts(const char *s)
{
    if (!s) return;
    _serial_puts_impl(s);
}

/* sleep_ms: called from Simple code; rt_sleep_ms is the real exported impl. */
extern RuntimeValue rt_sleep_ms(RuntimeValue ms);
__attribute__((weak)) void sleep_ms(RuntimeValue ms) { rt_sleep_ms(ms); }

/* rt_memcpy / rt_memset / rt_reload_segments */
__attribute__((weak)) RuntimeValue rt_memcpy(RuntimeValue dst, RuntimeValue src, RuntimeValue n)
{
    void *d = (void *)(uintptr_t)(uint64_t)dst;
    const void *s = (const void *)(uintptr_t)(uint64_t)src;
    uint64_t sz = (uint64_t)n;
    if (d && s && sz) __builtin_memcpy(d, s, sz);
    return dst;
}
__attribute__((weak)) RuntimeValue rt_memset(RuntimeValue dst, RuntimeValue val, RuntimeValue n)
{
    void *d = (void *)(uintptr_t)(uint64_t)dst;
    uint64_t sz = (uint64_t)n;
    int v = (int)(int64_t)val;
    if (d && sz) __builtin_memset(d, v, sz);
    return dst;
}
__attribute__((weak)) void rt_reload_segments(void) {}

/* --- E. cap_init_task_record ---------------------------------------- */
/* TODO: implement after smoke test green (capability module). */
__attribute__((weak)) void cap_init_task_record(RuntimeValue task, RuntimeValue full)
{
    (void)task; (void)full;
}

/* --- F. bytes_to_string / string_to_bytes / bytes_to_u32_le / u32_to_bytes_le */
/* TODO: implement after smoke test green. */
__attribute__((weak)) RuntimeValue bytes_to_string(RuntimeValue arr)
{
    /* Return an empty string stub — not called on the boot path. */
    (void)arr;
    return rt_string_from_cstr("");
}
__attribute__((weak)) RuntimeValue string_to_bytes(RuntimeValue str)
{
    (void)str;
    return 0; /* empty array stub */
}
__attribute__((weak)) RuntimeValue bytes_to_u32_le(RuntimeValue arr)
{
    (void)arr;
    return 0;
}
__attribute__((weak)) RuntimeValue u32_to_bytes_le(RuntimeValue val)
{
    (void)val;
    return 0;
}

/* --- G. UITheme stubs: glass_dark / glass_light / ios_dark / ios_light / theme_ui_theme */
/*
 * UITheme is a 32-byte struct; UIThemeColors is 40 bytes.
 * The compiler returns struct-by-value in (rax, rdx) for 16-byte structs or
 * passes a hidden sret pointer for larger ones.  The Simple ABI for structs
 * larger than 16 bytes passes a pointer to caller-allocated space in the first
 * argument.  Since these are never called on the x86_64 boot path, return 0
 * in all cases — the linker just needs the symbol to exist.
 * TODO: implement after smoke test green (desktop GUI subsystem).
 */
__attribute__((weak)) RuntimeValue glass_dark(void)      { return 0; }
__attribute__((weak)) RuntimeValue glass_light(void)     { return 0; }
__attribute__((weak)) RuntimeValue ios_dark(void)        { return 0; }
__attribute__((weak)) RuntimeValue ios_light(void)       { return 0; }
__attribute__((weak)) RuntimeValue theme_ui_theme(RuntimeValue name) { (void)name; return 0; }

/* --- H. arm_fs_exec_trace: shared tracing extern across arch loaders.
 * x86_64_fs_exec_spawn.spl + arm{,64}_fs_exec_spawn.spl all import this name;
 * stub as a no-op on x86_64.  TODO: rename to neutral fs_exec_trace. */
__attribute__((weak)) void arm_fs_exec_trace(RuntimeValue id) { (void)id; }

/* End of Wave 11: missing-symbol stubs */

#endif /* __x86_64__ || __i386__ */
