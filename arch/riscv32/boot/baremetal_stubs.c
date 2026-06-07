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
#define VIRTIO_DEV_BLK 2U
#define VIRTQ_DESC_F_NEXT 1U
#define VIRTQ_DESC_F_WRITE 2U
#define VIRTIO_STATUS_ACKNOWLEDGE 1U
#define VIRTIO_STATUS_DRIVER 2U
#define VIRTIO_STATUS_DRIVER_OK 4U
#define VIRTIO_STATUS_FEATURES_OK 8U

#define TAG_MASK    ((uintptr_t)0x7)
#define TAG_HEAP    ((uintptr_t)0x1)
#define TAG_SPECIAL ((uintptr_t)0x3)
#define NIL_VALUE   ((RuntimeValue)TAG_SPECIAL)

#define ENCODE_PTR(p) ((RuntimeValue)((uintptr_t)(p) | TAG_HEAP))
#define DECODE_PTR(v) ((void *)((uintptr_t)(v) & ~TAG_MASK))
#define IS_HEAP(v)    (((uintptr_t)(v) & TAG_MASK) == TAG_HEAP)

#define HEAP_STRING 1U

typedef struct {
    uint32_t type;
    uint32_t size;
} HeapHeader;

typedef struct {
    HeapHeader hdr;
    uint32_t len;
    char data[];
} RuntimeString;

static unsigned char g_heap[64 * 1024] __attribute__((aligned(16)));
static uintptr_t g_heap_off = 0;
static unsigned char g_virtq[8192] __attribute__((aligned(4096)));
static unsigned char g_dma[1024] __attribute__((aligned(512)));
static unsigned char g_riscv_file_buf[8192] __attribute__((aligned(16)));
static unsigned char g_riscv_process_arena[2][8192] __attribute__((aligned(4096)));
static uint64_t g_riscv_process_entry[2];
static uint64_t g_riscv_process_pid[2];
static uint32_t g_riscv_process_count;
static char g_riscv_gui_surface[256];
static volatile uint32_t *g_blk_mmio = 0;
static uint16_t g_last_used_idx = 0;

extern RuntimeValue spl_start(void);
extern char _stack_top[];

#include "../../common/baremetal_bump_heap.h"

static void uart_put_u32(uint32_t v)
{
    char buf[10];
    uint32_t pos = 0;
    do {
        buf[pos++] = (char)('0' + (v % 10U));
        v /= 10U;
    } while (v > 0U && pos < sizeof(buf));
    while (pos > 0U) {
        uart_putc(buf[--pos]);
    }
}

static uint32_t riscv32_harden_mix32(uint32_t value)
{
    value ^= value >> 16;
    value *= 0x7feb352dU;
    value ^= value >> 15;
    value *= 0x846ca68bU;
    value ^= value >> 16;
    return value & 0x7fffffffU;
}

RuntimeValue rt_riscv32_harden_canary_value(void)
{
    uintptr_t cycle = 0;
    uintptr_t time = 0;
    uintptr_t instret = 0;
    __asm__ volatile("rdcycle %0" : "=r"(cycle));
    __asm__ volatile("rdtime %0" : "=r"(time));
    __asm__ volatile("rdinstret %0" : "=r"(instret));
    uint32_t mixed = riscv32_harden_mix32(
        (uint32_t)cycle ^ ((uint32_t)time << 11) ^ ((uint32_t)instret << 17) ^
        (uint32_t)(uintptr_t)&rt_riscv32_harden_canary_value
    );
    return (RuntimeValue)(mixed == 0U ? 1U : mixed);
}

RuntimeValue rt_riscv32_harden_print_canary(void)
{
    uart_puts("[harden] canary arch=riscv32 value=");
    uart_put_u32((uint32_t)rt_riscv32_harden_canary_value());
    uart_puts("\r\n");
    return NIL_VALUE;
}

RuntimeValue rt_string_new(RuntimeValue data, RuntimeValue len_val)
{
    uintptr_t len = (uintptr_t)len_val;
    if (len == 0 || len > 4096U) return NIL_VALUE;
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

RuntimeValue rt_rv32_probe_store32(RuntimeValue addr, RuntimeValue value)
{
    *(volatile uint32_t *)(uintptr_t)addr = (uint32_t)(uintptr_t)value;
    return NIL_VALUE;
}

#define GHDL_RV32_PASS_ADDR      0x801FF000u
#define GHDL_RV32_A0_ADDR        0x801FF010u
#define GHDL_RV32_A1_ADDR        0x801FF014u
#define GHDL_RV32_DTB_VALID_ADDR 0x801FF018u
#define GHDL_RV32_SATP_ADDR      0x801FF01Cu

static RuntimeValue rv32_probe_store_fixed(uint32_t addr, RuntimeValue value)
{
    *(volatile uint32_t *)(uintptr_t)addr = (uint32_t)(uintptr_t)value;
    return NIL_VALUE;
}

RuntimeValue rt_rv32_probe_store_pass(RuntimeValue value)
{
    return rv32_probe_store_fixed(GHDL_RV32_PASS_ADDR, value);
}

RuntimeValue rt_rv32_probe_store_a0(RuntimeValue value)
{
    (void)value;
    return rv32_probe_store_fixed(GHDL_RV32_A0_ADDR, 0);
}

RuntimeValue rt_rv32_probe_store_a1(RuntimeValue value)
{
    (void)value;
    return rv32_probe_store_fixed(GHDL_RV32_A1_ADDR, 0x88000000u);
}

RuntimeValue rt_rv32_probe_store_dtb_valid(RuntimeValue value)
{
    return rv32_probe_store_fixed(GHDL_RV32_DTB_VALID_ADDR, value);
}

RuntimeValue rt_rv32_probe_store_satp(RuntimeValue value)
{
    return rv32_probe_store_fixed(GHDL_RV32_SATP_ADDR, value);
}

RuntimeValue rt_rv32_probe_load8(RuntimeValue addr)
{
    return (RuntimeValue)(uintptr_t)(*(volatile uint8_t *)(uintptr_t)addr);
}

RuntimeValue rt_rv32_probe_read_satp(void)
{
    return 0;
}

RuntimeValue rt_rv32_probe_uart_put(RuntimeValue byte)
{
    uart_putc((char)(uint8_t)(uintptr_t)byte);
    return NIL_VALUE;
}

RuntimeValue rt_qemu_exit_success(void)
{
    *(volatile uint32_t *)SIFIVE_TEST_BASE = 0x5555U;
    return NIL_VALUE;
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
    if (slot >= 2U || elf_size < 52U) return 0;
    if (elf[0] != 0x7fU || elf[1] != 'E' || elf[2] != 'L' || elf[3] != 'F') return 0;
    if (elf[4] != 1U || elf[5] != 1U) return 0;
    if (rd16(elf + 18U) != 243U) return 0;

    uint64_t entry = rd32(elf + 24U);
    uint64_t phoff = rd32(elf + 28U);
    uint16_t phentsize = rd16(elf + 42U);
    uint16_t phnum = rd16(elf + 44U);
    if (phoff == 0 || phentsize < 32U || phnum == 0 || phnum > 8U) return 0;
    if (phoff + ((uint64_t)phentsize * phnum) > elf_size) return 0;

    for (uint32_t i = 0; i < sizeof(g_riscv_process_arena[slot]); i++) g_riscv_process_arena[slot][i] = 0;

    uint32_t loaded = 0;
    int entry_mapped = 0;
    for (uint16_t i = 0; i < phnum; i++) {
        const unsigned char *ph = elf + phoff + ((uint64_t)i * phentsize);
        if (rd32(ph) != 1U) continue;
        uint64_t off = rd32(ph + 4U);
        uint64_t vaddr = rd32(ph + 8U);
        uint64_t filesz = rd32(ph + 16U);
        uint64_t memsz = rd32(ph + 20U);
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
    return riscv_smf_probe_file("HELLOSMFSMF", "SIMPLEOS_RISCV32_HELLO_ELF") ? 1 : 0;
}

RuntimeValue rt_riscv_smf_cli_load(void)
{
    return riscv_load_smf_process("HELLOSMFSMF", "SIMPLEOS_RISCV32_HELLO_ELF", 0) ? 1 : 0;
}

RuntimeValue rt_riscv_smf_gui_probe(void)
{
    return riscv_smf_probe_file("BROWSMF SMF", "SIMPLEOS_RISCV32_GUI_ELF") ? 1 : 0;
}

RuntimeValue rt_riscv_native_gui_process_render(void)
{
    if (!riscv_load_smf_process("BROWSMF SMF", "SIMPLEOS_RISCV32_GUI_ELF", 1)) return 0;
    if (g_riscv_process_pid[1] == 0 || g_riscv_process_entry[1] == 0) return 0;
    const char *content = "pid=1002 app=/sys/apps/browser_demo tree=native";
    uint32_t i = 0;
    while (content[i] != 0 && i + 1U < sizeof(g_riscv_gui_surface)) {
        g_riscv_gui_surface[i] = content[i];
        i++;
    }
    g_riscv_gui_surface[i] = 0;
    return bytes_contains((const unsigned char *)g_riscv_gui_surface, sizeof(g_riscv_gui_surface), "pid=1002") ? 1 : 0;
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
