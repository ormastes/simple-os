/*
 * Glass Rendering Primitives for SimpleOS
 *
 * DEPRECATED 2026-04-14 (D2 Phase 3): the in-tree compositor facade
 * `src/os/compositor/glass_effects.spl` has been deleted, so the only
 * remaining in-tree callers of the `rt_gui_blend_fill / box_blur /
 * gradient_v / read_pixel / shadow / shadow_fill / gradient_h /
 * begin_frame / present` symbols implemented below are:
 *   - `src/os/compositor/display_backend.spl` (FbCompositorBackend's
 *     CompositorGlassCapable impl) — kept until D2 Phase 2 reimplements
 *     blend/blur natively against FramebufferDriver.
 *   - The standalone arch-layer entry points `wm_entry.spl`,
 *     `desktop_entry.spl`, `gpu_render_test_entry.spl` — out of D2 scope.
 * Once D2 Phase 2 lands and FbCompositorBackend is verified on the native
 * impl, this translation unit becomes safe to delete (next cycle). Kept
 * for one cycle so out-of-tree callers (if any) do not break.
 *
 * NOTE: arm64 and riscv64 reach this same source via symlink — the
 * deprecation note above applies to all three baremetal arches.
 *
 * Portable reference impl, per doc/06_spec/runtime/rt_gui_glass_contract.md
 * (ARGB8888 straight-alpha, exclusive right/bottom, 5-pass separable box blur).
 *
 * SHARED ACROSS ARCHES: this translation unit is the canonical source for the
 * `rt_gui_*` glass symbols. Both arm64 and riscv64 reference it via symlink
 * (`arch/arm64/boot/glass_render.c` -> `../../x86_64/boot/glass_render.c`,
 *  `arch/riscv64/boot/glass_render.c` -> `../../x86_64/boot/glass_render.c`).
 * Keep this file free of x86-specific intrinsics or inline asm — it is
 * compiled under arm64 and riscv64 baremetal toolchains unmodified.
 * Decision: Approach A (shared source via symlink), matching the existing
 * arm64 precedent; portable C (stdint.h + stddef.h only), no arch-specific
 * code paths, so a single implementation serves all three arches.
 *
 * Alpha blending, box blur, gradients, and shadows for glassmorphism UI.
 *
 * ACCELERATION STRATEGY:
 *   All effects operate on a CPU-side shadow buffer (g_shadow_buf) in normal
 *   RAM -- NOT on MMIO framebuffer directly. This avoids the massive penalty
 *   of volatile MMIO reads (each one traps into QEMU host), which makes blur
 *   (millions of reads) practical.
 *
 *   Rendering flow:
 *     1. rt_gui_begin_frame() — copies MMIO framebuffer → shadow buffer
 *        (or just clears if starting fresh)
 *     2. All rt_gui_* effects operate on shadow buffer (fast RAM access)
 *     3. rt_gui_present() — bulk-copies shadow buffer → MMIO framebuffer
 *        (single memcpy, or dirty-rect transfer for partial updates)
 *
 *   For VirtIO-GPU: shadow buffer IS the DMA backing memory, so
 *   present() just calls TRANSFER_TO_HOST_2D + RESOURCE_FLUSH.
 *
 * Packing convention (same as rt_gui_fill4):
 *   xy = (x << 32) | y
 *   wh = (w << 32) | h
 */

#include <stdint.h>
#include <stddef.h>

typedef int64_t RuntimeValue;

/* Globals from baremetal_stubs.c */
extern uint64_t g_fb_addr;
extern uint64_t g_fb_w;
extern void *malloc(size_t);

/* Screen dimensions */
#define SCREEN_W_MAX 1024
#define SCREEN_H 768

/* ===================================================================
 * Shadow buffer — CPU-side framebuffer for fast read/write
 *
 * The MMIO framebuffer (g_fb_addr) is mapped to device memory. Each
 * read/write is a volatile operation that traps into QEMU. The shadow
 * buffer lives in normal RAM where reads are ~100x faster.
 *
 * Dirty tracking: g_dirty_* records the bounding box of all writes
 * since last present(), enabling partial MMIO transfer.
 * =================================================================== */

static uint32_t *g_shadow_buf = 0;
static uint32_t  g_shadow_w = 0;
static uint32_t  g_shadow_h = 0;
static int       g_shadow_ready = 0;

/* Dirty region tracking for partial present */
static uint32_t g_dirty_x1 = 0xFFFFFFFF;
static uint32_t g_dirty_y1 = 0xFFFFFFFF;
static uint32_t g_dirty_x2 = 0;
static uint32_t g_dirty_y2 = 0;

/* VirtIO-GPU acceleration flag — set by Simple code when GPU detected.
 * When enabled, present() uses the shadow buffer as DMA backing memory
 * and skips the MMIO memcpy (QEMU reads from DMA directly). */
static int g_use_virtio_gpu = 0;
static uint64_t g_virtio_gpu_resource_id = 0;

/* Runtime GPU-accel feature flag — separate knob from g_use_virtio_gpu.
 * Set by Simple-side boot probe (src/os/drivers/gpu/gpu_accel.spl) ONLY
 * after VirtIO-GPU is fully detected and its controlq is ready. Default
 * OFF so the CPU fallback keeps working; new GPU code paths fire only
 * when this flag is ON. The two flags are intentionally distinct:
 *   - g_use_virtio_gpu: shadow buffer is backed by GPU DMA (layout hint)
 *   - g_gpu_accel_enabled: C side is allowed to issue virtio-gpu commands
 */
static int g_gpu_accel_enabled = 0;

/* Called from Simple code to enable GPU-accelerated present */
RuntimeValue rt_gui_set_gpu_mode(RuntimeValue enabled, RuntimeValue resource_id,
                                  RuntimeValue unused1, RuntimeValue unused2)
{
    (void)unused1; (void)unused2;
    g_use_virtio_gpu = (int)(uint64_t)enabled;
    g_virtio_gpu_resource_id = (uint64_t)resource_id;
    return 0;
}

/* Runtime feature flag setter — called from Simple after VirtIO-GPU probe. */
RuntimeValue rt_gui_set_gpu_accel_enabled(RuntimeValue enabled,
                                           RuntimeValue unused1,
                                           RuntimeValue unused2,
                                           RuntimeValue unused3)
{
    (void)unused1; (void)unused2; (void)unused3;
    g_gpu_accel_enabled = (int)(uint64_t)enabled;
    return 0;
}

/* C-callable query — returns non-zero when GPU-accel feature flag is ON. */
int glass_render_gpu_accel_enabled(void)
{
    return g_gpu_accel_enabled;
}

/* --------------------------------------------------------------------
 * VirtIO-GPU transfer/flush path
 *
 * This is the foundation call every higher-level GPU offload (blit,
 * blur compute, pixel shaders, bezier raster) ultimately depends on:
 * once a resource is updated on the host, the guest must issue
 * TRANSFER_TO_HOST_2D + RESOURCE_FLUSH for the region to become
 * visible. Everything else (shader pipelines, command buffers) is
 * built on top of this cycle.
 *
 * STATUS: The virtio-gpu controlq is owned by the Simple-side driver
 * (src/os/drivers/virtio/virtio_gpu.spl). From C we do NOT touch the
 * virtqueue directly — that would duplicate state the Simple driver
 * is managing. Instead, when the GPU-accel flag is ON, we record the
 * dirty rect as "pending flush" and return. The Simple compositor's
 * present loop (src/os/compositor/engine2d_display.spl) picks up the
 * pending rect on its next tick and issues the transfer/flush through
 * the existing Simple driver.
 *
 * This is a stub in the sense that it does not issue the virtio
 * command from C directly — but it is a REAL handoff: when the flag
 * is ON, the CPU MMIO memcpy is suppressed and the Simple side takes
 * over. When the flag is OFF, behavior is unchanged (MMIO fallback).
 * ------------------------------------------------------------------ */

static uint32_t g_pending_flush_x1 = 0;
static uint32_t g_pending_flush_y1 = 0;
static uint32_t g_pending_flush_x2 = 0;
static uint32_t g_pending_flush_y2 = 0;
static int      g_pending_flush_valid = 0;

/* C helper — records the rect the Simple side should transfer+flush.
 * Returns 1 if the rect was recorded (flag ON), 0 if caller should
 * fall through to MMIO copy. */
static int virtio_gpu_queue_transfer_flush(uint32_t x1, uint32_t y1,
                                            uint32_t x2, uint32_t y2)
{
    if (!g_gpu_accel_enabled) return 0;
    if (g_virtio_gpu_resource_id == 0) return 0;
    g_pending_flush_x1 = x1;
    g_pending_flush_y1 = y1;
    g_pending_flush_x2 = x2;
    g_pending_flush_y2 = y2;
    g_pending_flush_valid = 1;
    return 1;
}

/* Simple-side driver polls this after each compositor tick.
 * Returns packed (x1<<48)|(y1<<32)|(x2<<16)|y2 if a flush is pending,
 * or 0 if nothing pending. Clears the pending state on read. */
RuntimeValue rt_gui_take_pending_flush(RuntimeValue unused1, RuntimeValue unused2,
                                        RuntimeValue unused3, RuntimeValue unused4)
{
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    if (!g_pending_flush_valid) return 0;
    uint64_t packed =
        ((uint64_t)(g_pending_flush_x1 & 0xFFFFu) << 48) |
        ((uint64_t)(g_pending_flush_y1 & 0xFFFFu) << 32) |
        ((uint64_t)(g_pending_flush_x2 & 0xFFFFu) << 16) |
        ((uint64_t)(g_pending_flush_y2 & 0xFFFFu));
    g_pending_flush_valid = 0;
    return (RuntimeValue)packed;
}

static void dirty_mark(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    if (x < g_dirty_x1) g_dirty_x1 = x;
    if (y < g_dirty_y1) g_dirty_y1 = y;
    uint32_t x2 = x + w;
    uint32_t y2 = y + h;
    if (x2 > g_dirty_x2) g_dirty_x2 = x2;
    if (y2 > g_dirty_y2) g_dirty_y2 = y2;
}

static void dirty_reset(void)
{
    g_dirty_x1 = 0xFFFFFFFF;
    g_dirty_y1 = 0xFFFFFFFF;
    g_dirty_x2 = 0;
    g_dirty_y2 = 0;
}

/* ===================================================================
 * Pixel helpers — operate on shadow buffer (fast) or MMIO (fallback)
 * =================================================================== */

static inline uint32_t fb_read(uint32_t x, uint32_t y)
{
    if (g_shadow_ready && x < g_shadow_w && y < g_shadow_h)
        return g_shadow_buf[y * g_shadow_w + x];
    /* Fallback to MMIO (slow) */
    uint64_t off = ((uint64_t)y * g_fb_w + x) * 4;
    return *(volatile uint32_t *)(uintptr_t)(g_fb_addr + off);
}

static inline void fb_write(uint32_t x, uint32_t y, uint32_t color)
{
    if (g_shadow_ready && x < g_shadow_w && y < g_shadow_h) {
        g_shadow_buf[y * g_shadow_w + x] = color;
        return;
    }
    /* Fallback to MMIO (slow) */
    uint64_t off = ((uint64_t)y * g_fb_w + x) * 4;
    *(volatile uint32_t *)(uintptr_t)(g_fb_addr + off) = color;
}

/* ===================================================================
 * Frame lifecycle — begin_frame / present
 * =================================================================== */

/* rt_gui_begin_frame(width, height, _, _)
 * Allocates shadow buffer (once) and marks frame start.
 * Call before any rendering. */
RuntimeValue rt_gui_begin_frame(RuntimeValue w_rv, RuntimeValue h_rv,
                                 RuntimeValue unused1, RuntimeValue unused2)
{
    (void)unused1; (void)unused2;
    uint32_t w = (uint32_t)(uint64_t)w_rv;
    uint32_t h = (uint32_t)(uint64_t)h_rv;
    if (w > SCREEN_W_MAX) w = SCREEN_W_MAX;
    if (h > SCREEN_H) h = SCREEN_H;

    /* Allocate shadow buffer once */
    if (!g_shadow_buf || g_shadow_w != w || g_shadow_h != h) {
        g_shadow_buf = (uint32_t *)malloc((size_t)w * h * 4);
        g_shadow_w = w;
        g_shadow_h = h;
    }
    if (!g_shadow_buf) return 0;

    g_shadow_ready = 1;
    dirty_reset();
    return 0;
}

/* rt_gui_present(_, _, _, _)
 * Copies shadow buffer to MMIO framebuffer.
 * Uses dirty rect tracking: only copies changed region.
 *
 * GPU path: when glass_render_gpu_accel_enabled() is true, the dirty
 * rect is handed off via virtio_gpu_queue_transfer_flush() and the
 * Simple-side driver (src/os/drivers/virtio/virtio_gpu.spl) issues
 * TRANSFER_TO_HOST_2D + RESOURCE_FLUSH through the controlq. The
 * CPU MMIO memcpy is suppressed on that path. */
RuntimeValue rt_gui_present(RuntimeValue unused1, RuntimeValue unused2,
                             RuntimeValue unused3, RuntimeValue unused4)
{
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    if (!g_shadow_ready || !g_shadow_buf) return 0;

    /* Determine transfer region */
    uint32_t x1 = 0, y1 = 0, x2 = g_shadow_w, y2 = g_shadow_h;
    if (g_dirty_x1 < g_dirty_x2 && g_dirty_y1 < g_dirty_y2) {
        /* Use dirty rect (clamped) */
        x1 = g_dirty_x1;
        y1 = g_dirty_y1;
        x2 = g_dirty_x2 < g_shadow_w ? g_dirty_x2 : g_shadow_w;
        y2 = g_dirty_y2 < g_shadow_h ? g_dirty_y2 : g_shadow_h;
    }

    /* GPU fast path: shadow buffer IS the DMA backing.
     * Record the dirty rect for the Simple-side virtio-gpu driver to
     * pick up on next poll of rt_gui_take_pending_flush(). */
    if (g_use_virtio_gpu || g_gpu_accel_enabled) {
        if (virtio_gpu_queue_transfer_flush(x1, y1, x2, y2)) {
            dirty_reset();
            return 0;
        }
        /* Flag ON but resource_id==0: fall through to MMIO fallback. */
        if (g_use_virtio_gpu) {
            dirty_reset();
            return 0;
        }
    }

    /* Bulk copy dirty region to MMIO framebuffer (row by row) */
    for (uint32_t row = y1; row < y2; row++) {
        uint64_t mmio_row = g_fb_addr + ((uint64_t)row * g_fb_w + x1) * 4;
        uint32_t *src_row = &g_shadow_buf[row * g_shadow_w + x1];
        uint32_t cols = x2 - x1;
        for (uint32_t col = 0; col < cols; col++) {
            *(volatile uint32_t *)(uintptr_t)(mmio_row + col * 4) = src_row[col];
        }
    }

    dirty_reset();
    return 0;
}

/* Alpha blend: dst over src with alpha [0..255]
 * result = (src * alpha + dst * (255 - alpha)) / 255
 * Uses (x + 128) >> 8 approximation for speed */
static inline uint32_t alpha_blend(uint32_t dst, uint32_t src, uint8_t alpha)
{
    if (alpha == 255) return src;
    if (alpha == 0) return dst;

    uint32_t inv = 255 - alpha;

    uint32_t sr = (src >> 16) & 0xFF;
    uint32_t sg = (src >> 8) & 0xFF;
    uint32_t sb = src & 0xFF;

    uint32_t dr = (dst >> 16) & 0xFF;
    uint32_t dg = (dst >> 8) & 0xFF;
    uint32_t db = dst & 0xFF;

    uint32_t rr = (sr * alpha + dr * inv + 128) >> 8;
    uint32_t rg = (sg * alpha + dg * inv + 128) >> 8;
    uint32_t rb = (sb * alpha + db * inv + 128) >> 8;

    return 0xFF000000u | (rr << 16) | (rg << 8) | rb;
}

/* Linear interpolation between two colors (signed to fix gradient banding) */
static inline uint32_t lerp_color(uint32_t c1, uint32_t c2, uint32_t t, uint32_t max)
{
    if (max == 0) return c1;
    uint32_t r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
    uint32_t r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
    /* Signed math prevents unsigned underflow when c2 channel < c1 channel */
    int32_t r = (int32_t)r1 + ((int32_t)r2 - (int32_t)r1) * (int32_t)t / (int32_t)max;
    int32_t g = (int32_t)g1 + ((int32_t)g2 - (int32_t)g1) * (int32_t)t / (int32_t)max;
    int32_t b = (int32_t)b1 + ((int32_t)b2 - (int32_t)b1) * (int32_t)t / (int32_t)max;
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
    return 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

/* ===================================================================
 * 1. Alpha-blended rectangle fill
 *    rt_gui_blend_fill(xy, wh, color, alpha)
 *
 *    TODO(P3/gpu): offload to GPU blit with per-pixel alpha.
 *      Depends on: virtio-gpu RESOURCE_CREATE_2D + TRANSFER_TO_HOST_2D
 *        (stub path wired via rt_gui_present; see glass_render.c:~155)
 *        plus a 2D blit command — virtio-gpu spec doesn't expose one,
 *        so this needs virgl/venus renderer negotiation (CAPSET_VIRGL).
 *      Expected speedup: ~5-10x for large alpha rects.
 * =================================================================== */

RuntimeValue rt_gui_blend_fill(RuntimeValue xy, RuntimeValue wh,
                                RuntimeValue color_rv, RuntimeValue alpha_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t src = (uint32_t)(uint64_t)color_rv;
    uint8_t  alpha = (uint8_t)(uint64_t)alpha_rv;

    /* Clamp to screen */
    if (x >= g_fb_w || y >= SCREEN_H) return 0;
    if (x + w > g_fb_w) w = (uint32_t)g_fb_w - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;

    dirty_mark(x, y, w, h);

    for (uint32_t row = 0; row < h; row++) {
        for (uint32_t col = 0; col < w; col++) {
            uint32_t dst = fb_read(x + col, y + row);
            fb_write(x + col, y + row, alpha_blend(dst, src, alpha));
        }
    }
    return 0;
}

/* ===================================================================
 * 24. Noise texture blend — adds ordered dither noise to glass surfaces
 *     rt_gui_noise_blend(xy, wh, intensity, unused)
 *     Adds a subtle noise pattern (like macOS frosted glass grain)
 * =================================================================== */
RuntimeValue rt_gui_noise_blend(RuntimeValue xy, RuntimeValue wh,
                                 RuntimeValue intensity_rv, RuntimeValue unused)
{
    (void)unused;
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint8_t intensity = (uint8_t)(uint64_t)intensity_rv;

    if (x >= g_fb_w || y >= SCREEN_H || w == 0 || h == 0) return 0;

    dirty_mark(x, y, w, h);

    /* Pseudo-random noise using hash function */
    for (uint32_t row = 0; row < h; row++) {
        uint32_t py = y + row;
        if (py >= SCREEN_H) break;
        for (uint32_t col = 0; col < w; col++) {
            uint32_t px = x + col;
            if (px >= g_fb_w) break;

            /* Simple hash for pseudo-random noise */
            uint32_t hash = (px * 2654435761u) ^ (py * 2246822519u);
            hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
            hash = ((hash >> 16) ^ hash);

            /* Only apply to ~30% of pixels for subtlety */
            if ((hash & 7) < 3) {
                uint32_t dst = fb_read(px, py);
                uint8_t noise_val = (hash >> 8) & 1; /* 0 or 1 = darken or lighten */
                uint32_t noise_color = noise_val ? 0x00FFFFFF : 0x00000000;
                fb_write(px, py, alpha_blend(dst, noise_color, intensity));
            }
        }
    }
    return 0;
}

/* ===================================================================
 * 2. Single pixel alpha blend
 *    rt_gui_blend_pixel(pack(x,y), pack(color,alpha), _, _)
 * =================================================================== */

RuntimeValue rt_gui_blend_pixel(RuntimeValue x_y, RuntimeValue color_alpha,
                                 RuntimeValue unused1, RuntimeValue unused2)
{
    (void)unused1; (void)unused2;
    uint32_t x = (uint32_t)((uint64_t)x_y >> 32);
    uint32_t y = (uint32_t)((uint64_t)x_y & 0xFFFFFFFF);
    uint32_t src = (uint32_t)((uint64_t)color_alpha >> 32);
    uint8_t  alpha = (uint8_t)((uint64_t)color_alpha & 0xFF);

    if (x >= g_fb_w || y >= SCREEN_H) return 0;
    uint32_t dst = fb_read(x, y);
    fb_write(x, y, alpha_blend(dst, src, alpha));
    return 0;
}

/* ===================================================================
 * 3. Box blur (3-pass approximation of Gaussian)
 *    rt_gui_box_blur(xy, wh, radius, _)
 *
 *    TODO(P3/gpu): offload to GPU compute shader.
 *      Depends on: virgl CAPSET_VIRGL renderer negotiation (host-side
 *        support from QEMU --display gtk,gl=on or similar) + a
 *        GLSL/SPIR-V compute program shipped with the guest + a
 *        command-buffer builder that doesn't exist yet. Foundation
 *        TRANSFER/FLUSH path already wired (glass_render.c:~155).
 *      Expected speedup: 10-50x for large blur passes.
 * =================================================================== */

/* Scratch buffer for blur passes — static to avoid stack overflow.
 * Max dimension = 2048 pixels. */
static uint32_t blur_row_r[2048];
static uint32_t blur_row_g[2048];
static uint32_t blur_row_b[2048];

static void box_blur_h(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t r)
{
    if (r == 0 || w == 0) return;
    uint32_t d = 2 * r + 1;

    for (uint32_t row = 0; row < h; row++) {
        uint32_t py = y0 + row;
        if (py >= SCREEN_H) break;

        /* Build initial accumulator for first pixel */
        uint32_t acc_r = 0, acc_g = 0, acc_b = 0;
        for (uint32_t i = 0; i < d && i < w; i++) {
            uint32_t px = x0 + (i < r ? 0 : i - r);
            if (px >= g_fb_w) px = (uint32_t)g_fb_w - 1;
            uint32_t c = fb_read(px, py);
            acc_r += (c >> 16) & 0xFF;
            acc_g += (c >> 8) & 0xFF;
            acc_b += c & 0xFF;
        }

        /* Slide window across row */
        for (uint32_t col = 0; col < w; col++) {
            blur_row_r[col] = acc_r / d;
            blur_row_g[col] = acc_g / d;
            blur_row_b[col] = acc_b / d;

            /* Remove left pixel, add right pixel */
            int32_t left = (int32_t)(x0 + col) - (int32_t)r;
            int32_t right = (int32_t)(x0 + col) + (int32_t)r + 1;
            if (left < (int32_t)x0) left = (int32_t)x0;
            if (right >= (int32_t)g_fb_w) right = (int32_t)g_fb_w - 1;
            if (right >= (int32_t)(x0 + w)) right = (int32_t)(x0 + w - 1);

            uint32_t cl = fb_read((uint32_t)left, py);
            uint32_t cr = fb_read((uint32_t)right, py);
            acc_r += ((cr >> 16) & 0xFF) - ((cl >> 16) & 0xFF);
            acc_g += ((cr >> 8) & 0xFF) - ((cl >> 8) & 0xFF);
            acc_b += (cr & 0xFF) - (cl & 0xFF);
        }

        /* Write blurred row back */
        for (uint32_t col = 0; col < w; col++) {
            uint32_t px = x0 + col;
            if (px >= g_fb_w) break;
            fb_write(px, py,
                0xFF000000u | (blur_row_r[col] << 16) | (blur_row_g[col] << 8) | blur_row_b[col]);
        }
    }
}

static void box_blur_v(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t r)
{
    if (r == 0 || h == 0) return;
    uint32_t d = 2 * r + 1;

    for (uint32_t col = 0; col < w; col++) {
        uint32_t px = x0 + col;
        if (px >= g_fb_w) break;

        uint32_t acc_r = 0, acc_g = 0, acc_b = 0;
        for (uint32_t i = 0; i < d && i < h; i++) {
            uint32_t py = y0 + (i < r ? 0 : i - r);
            if (py >= SCREEN_H) py = SCREEN_H - 1;
            uint32_t c = fb_read(px, py);
            acc_r += (c >> 16) & 0xFF;
            acc_g += (c >> 8) & 0xFF;
            acc_b += c & 0xFF;
        }

        /* Use blur_row arrays as column buffer (max 768 < 1024) */
        for (uint32_t row = 0; row < h; row++) {
            blur_row_r[row] = acc_r / d;
            blur_row_g[row] = acc_g / d;
            blur_row_b[row] = acc_b / d;

            int32_t top = (int32_t)(y0 + row) - (int32_t)r;
            int32_t bot = (int32_t)(y0 + row) + (int32_t)r + 1;
            if (top < (int32_t)y0) top = (int32_t)y0;
            if (bot >= (int32_t)SCREEN_H) bot = SCREEN_H - 1;
            if (bot >= (int32_t)(y0 + h)) bot = (int32_t)(y0 + h - 1);

            uint32_t ct = fb_read(px, (uint32_t)top);
            uint32_t cb = fb_read(px, (uint32_t)bot);
            acc_r += ((cb >> 16) & 0xFF) - ((ct >> 16) & 0xFF);
            acc_g += ((cb >> 8) & 0xFF) - ((ct >> 8) & 0xFF);
            acc_b += (cb & 0xFF) - (ct & 0xFF);
        }

        for (uint32_t row = 0; row < h; row++) {
            uint32_t py = y0 + row;
            if (py >= SCREEN_H) break;
            fb_write(px, py,
                0xFF000000u | (blur_row_r[row] << 16) | (blur_row_g[row] << 8) | blur_row_b[row]);
        }
    }
}

RuntimeValue rt_gui_box_blur(RuntimeValue xy, RuntimeValue wh,
                              RuntimeValue radius_rv, RuntimeValue unused)
{
    (void)unused;
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t r = (uint32_t)(uint64_t)radius_rv;

    if (r == 0 || w == 0 || h == 0) return 0;
    if (r > 30) r = 30; /* Clamp kernel to 61 (radius 30) — prevents artifacts with large blurs */
    if (x >= g_fb_w || y >= SCREEN_H) return 0;
    if (x + w > g_fb_w) w = (uint32_t)g_fb_w - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;

    /* 5-pass box blur: H -> V -> H -> V -> H (closer Gaussian approximation) */
    box_blur_h(x, y, w, h, r);
    box_blur_v(x, y, w, h, r);
    box_blur_h(x, y, w, h, r);
    box_blur_v(x, y, w, h, r);
    box_blur_h(x, y, w, h, r);

    return 0;
}

/* ===================================================================
 * 4. Horizontal linear gradient
 *    rt_gui_gradient_h(xy, wh, color1, color2)
 *
 *    TODO(P3/gpu): trivially parallelizable pixel shader.
 *      Depends on: virgl renderer + fragment-shader pipeline + command
 *        buffer builder (same prerequisites as box_blur; see
 *        glass_render.c:~340). Baseline transfer/flush already wired.
 *      Expected speedup: ~20x for full-width gradients.
 * =================================================================== */

RuntimeValue rt_gui_gradient_h(RuntimeValue xy, RuntimeValue wh,
                                RuntimeValue c1_rv, RuntimeValue c2_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t c1 = (uint32_t)(uint64_t)c1_rv;
    uint32_t c2 = (uint32_t)(uint64_t)c2_rv;

    if (x >= g_fb_w || y >= SCREEN_H) return 0;
    if (x + w > g_fb_w) w = (uint32_t)g_fb_w - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;

    dirty_mark(x, y, w, h);

    for (uint32_t col = 0; col < w; col++) {
        uint32_t color = lerp_color(c1, c2, col, w > 1 ? w - 1 : 1);
        for (uint32_t row = 0; row < h; row++) {
            fb_write(x + col, y + row, color);
        }
    }
    return 0;
}

/* ===================================================================
 * 5. Vertical linear gradient
 *    rt_gui_gradient_v(xy, wh, color1, color2)
 *
 *    TODO(P3/gpu): trivially parallelizable pixel shader.
 *      Depends on: same virgl pipeline prerequisites as gradient_h.
 *        Baseline transfer/flush already wired (glass_render.c:~155).
 *      Expected speedup: ~20x.
 * =================================================================== */

RuntimeValue rt_gui_gradient_v(RuntimeValue xy, RuntimeValue wh,
                                RuntimeValue c1_rv, RuntimeValue c2_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t c1 = (uint32_t)(uint64_t)c1_rv;
    uint32_t c2 = (uint32_t)(uint64_t)c2_rv;

    if (x >= g_fb_w || y >= SCREEN_H) return 0;
    if (x + w > g_fb_w) w = (uint32_t)g_fb_w - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;

    dirty_mark(x, y, w, h);

    for (uint32_t row = 0; row < h; row++) {
        uint32_t color = lerp_color(c1, c2, row, h > 1 ? h - 1 : 1);
        for (uint32_t col = 0; col < w; col++) {
            fb_write(x + col, y + row, color);
        }
    }
    return 0;
}

/* ===================================================================
 * 6. Drop shadow
 *    rt_gui_shadow(xy, wh, blur_radius, alpha)
 *    Draws a dark blurred rectangle offset from the window position.
 *
 *    TODO(P3/gpu): offload blur pass to GPU compute.
 *      Depends on: same compute-shader pipeline as box_blur
 *        (glass_render.c:~340). Baseline transfer/flush already wired.
 *      Expected speedup: 10-30x.
 * =================================================================== */

RuntimeValue rt_gui_shadow(RuntimeValue xy, RuntimeValue wh,
                            RuntimeValue blur_alpha, RuntimeValue offset_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t blur_r = (uint32_t)((uint64_t)blur_alpha >> 32);
    uint32_t alpha = (uint32_t)((uint64_t)blur_alpha & 0xFFFFFFFF);
    uint32_t offset = (uint32_t)(uint64_t)offset_rv;

    /* 1.5x stronger shadow alpha */
    alpha = alpha * 3 / 2;
    if (alpha > 255) alpha = 255;
    if (blur_r > 45) blur_r = 45;
    if (offset == 0) offset = 6;

    /* Shadow position: offset down and right, slightly larger */
    uint32_t sx = x + offset;
    uint32_t sy = y + offset;
    uint32_t sw = w + offset;
    uint32_t sh = h + offset;

    /* Clamp */
    if (sx >= g_fb_w || sy >= SCREEN_H) return 0;
    if (sx + sw > g_fb_w) sw = (uint32_t)g_fb_w - sx;
    if (sy + sh > SCREEN_H) sh = SCREEN_H - sy;

    /* Draw dark rectangle */
    for (uint32_t row = 0; row < sh; row++) {
        for (uint32_t col = 0; col < sw; col++) {
            uint32_t dst = fb_read(sx + col, sy + row);
            fb_write(sx + col, sy + row, alpha_blend(dst, 0x00000000, (uint8_t)alpha));
        }
    }

    /* Blur the shadow */
    if (blur_r > 0) {
        box_blur_h(sx, sy, sw, sh, blur_r);
        box_blur_v(sx, sy, sw, sh, blur_r);
    }

    /* Extra soft outer glow for diffusion */
    if (blur_r > 4) {
        uint32_t ex = (sx > 2) ? sx - 2 : 0;
        uint32_t ey = (sy > 2) ? sy - 2 : 0;
        uint32_t ew = sw + 4;
        uint32_t eh = sh + 4;
        if (ex + ew <= g_fb_w && ey + eh <= SCREEN_H) {
            /* Top edge */
            for (uint32_t c = 0; c < ew && ex + c < g_fb_w; c++) {
                if (ey < SCREEN_H) {
                    uint32_t dst = fb_read(ex + c, ey);
                    fb_write(ex + c, ey, alpha_blend(dst, 0x00000000, alpha / 4));
                }
            }
            /* Bottom edge */
            uint32_t bot = ey + eh - 1;
            if (bot < SCREEN_H) {
                for (uint32_t c = 0; c < ew && ex + c < g_fb_w; c++) {
                    uint32_t dst = fb_read(ex + c, bot);
                    fb_write(ex + c, bot, alpha_blend(dst, 0x00000000, alpha / 4));
                }
            }
        }
    }

    return 0;
}

/* ===================================================================
 * 7. Read pixel
 *    rt_gui_read_pixel(pack(x,y), _, _, _)
 * =================================================================== */

RuntimeValue rt_gui_read_pixel(RuntimeValue x_y, RuntimeValue u1,
                                RuntimeValue u2, RuntimeValue u3)
{
    (void)u1; (void)u2; (void)u3;
    uint32_t x = (uint32_t)((uint64_t)x_y >> 32);
    uint32_t y = (uint32_t)((uint64_t)x_y & 0xFFFFFFFF);
    if (x >= g_fb_w || y >= SCREEN_H) return 0;
    return (RuntimeValue)(uint64_t)fb_read(x, y);
}

/* ===================================================================
 * 8. Rounded rectangle (approximate with filled rects)
 *    rt_gui_rounded_rect(xy, wh, color_radius, alpha)
 *
 *    TODO(P3/gpu): Bezier curve rasterization on GPU.
 *      Depends on: virgl pipeline + tessellation/fragment shaders +
 *        command-buffer builder. This is the heaviest of the six —
 *        needs Loop-Blinn or SDF-based curve AA on the shader side.
 *        Baseline transfer/flush already wired (glass_render.c:~155).
 *      Expected speedup: ~8x for AA'd corners.
 * =================================================================== */

RuntimeValue rt_gui_rounded_rect(RuntimeValue xy, RuntimeValue wh,
                                  RuntimeValue color_radius, RuntimeValue alpha_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_radius >> 32);
    uint32_t radius = (uint32_t)((uint64_t)color_radius & 0xFFFFFFFF);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;

    if (radius > w / 2) radius = w / 2;
    if (radius > h / 2) radius = h / 2;
    if (x >= g_fb_w || y >= SCREEN_H) return 0;

    /* Draw rounded rectangle using circle-quarter masks at corners */
    for (uint32_t row = 0; row < h; row++) {
        uint32_t py = y + row;
        if (py >= SCREEN_H) break;

        /* Determine row start/end considering rounded corners */
        uint32_t x_start = 0;
        uint32_t x_end = w;

        if (row < radius) {
            /* Top corners */
            uint32_t dy = radius - row;
            /* Circle equation: dx^2 + dy^2 <= r^2 => dx = sqrt(r^2 - dy^2) */
            uint32_t dx = 0;
            while ((dx + 1) * (dx + 1) + dy * dy <= radius * radius) dx++;
            x_start = radius - dx;
            x_end = w - (radius - dx);
        } else if (row >= h - radius) {
            /* Bottom corners */
            uint32_t dy = row - (h - radius);
            uint32_t dx = 0;
            while ((dx + 1) * (dx + 1) + dy * dy <= radius * radius) dx++;
            x_start = radius - dx;
            x_end = w - (radius - dx);
        }

        for (uint32_t col = x_start; col < x_end; col++) {
            uint32_t px = x + col;
            if (px >= g_fb_w) break;
            if (alpha == 255) {
                fb_write(px, py, 0xFF000000u | color);
            } else {
                uint32_t dst = fb_read(px, py);
                fb_write(px, py, alpha_blend(dst, color, alpha));
            }
        }
    }
    return 0;
}

/* ===================================================================
 * 9. Gradient blend (vertical gradient with alpha)
 *    rt_gui_gradient_blend_v(xy, wh, c1_alpha1, c2_alpha2)
 *    Blends a vertical gradient with varying alpha onto framebuffer
 * =================================================================== */

RuntimeValue rt_gui_gradient_blend_v(RuntimeValue xy, RuntimeValue wh,
                                      RuntimeValue c1_a1, RuntimeValue c2_a2)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t c1 = (uint32_t)((uint64_t)c1_a1 >> 32);
    uint32_t a1 = (uint32_t)((uint64_t)c1_a1 & 0xFF);
    uint32_t c2 = (uint32_t)((uint64_t)c2_a2 >> 32);
    uint32_t a2 = (uint32_t)((uint64_t)c2_a2 & 0xFF);

    if (x >= g_fb_w || y >= SCREEN_H) return 0;
    if (x + w > g_fb_w) w = (uint32_t)g_fb_w - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;

    for (uint32_t row = 0; row < h; row++) {
        uint32_t color = lerp_color(c1, c2, row, h > 1 ? h - 1 : 1);
        uint32_t alpha = a1 + (a2 - a1) * row / (h > 1 ? h - 1 : 1);
        if (alpha > 255) alpha = 255;
        for (uint32_t col = 0; col < w; col++) {
            uint32_t px = x + col;
            uint32_t py = y + row;
            uint32_t dst = fb_read(px, py);
            fb_write(px, py, alpha_blend(dst, color, (uint8_t)alpha));
        }
    }
    return 0;
}

/* ===================================================================
 * 10. Shadow-buffer-aware solid fill
 *     rt_gui_shadow_fill(xy, wh, color, _)
 *     Like rt_gui_fill4 but writes to shadow buffer, not MMIO.
 * =================================================================== */

RuntimeValue rt_gui_shadow_fill(RuntimeValue xy, RuntimeValue wh,
                                 RuntimeValue color_rv, RuntimeValue unused)
{
    (void)unused;
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t c = (uint32_t)(uint64_t)color_rv;

    if (x >= g_fb_w || y >= SCREEN_H) return 0;
    if (x + w > g_fb_w) w = (uint32_t)g_fb_w - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;

    dirty_mark(x, y, w, h);

    for (uint32_t row = 0; row < h; row++) {
        for (uint32_t col = 0; col < w; col++) {
            fb_write(x + col, y + row, c);
        }
    }
    return 0;
}

/* ===================================================================
 * 11. Partial present for dirty regions only
 *     rt_gui_present_rect(x, y, w, h)
 *     Copies a rectangular region from shadow buffer to MMIO framebuffer.
 *     Used for incremental updates when only a small area changed.
 * =================================================================== */

RuntimeValue rt_gui_present_rect(RuntimeValue x_rv, RuntimeValue y_rv,
                                  RuntimeValue w_rv, RuntimeValue h_rv)
{
    uint32_t x = (uint32_t)(uint64_t)x_rv;
    uint32_t y = (uint32_t)(uint64_t)y_rv;
    uint32_t w = (uint32_t)(uint64_t)w_rv;
    uint32_t h = (uint32_t)(uint64_t)h_rv;

    if (!g_shadow_ready || !g_shadow_buf) return 0;
    if (x + w > g_shadow_w) w = g_shadow_w - x;
    if (y + h > g_shadow_h) h = g_shadow_h - y;

    for (uint32_t row = y; row < y + h; row++) {
        uint64_t mmio_row = g_fb_addr + ((uint64_t)row * g_fb_w + x) * 4;
        uint32_t *src = &g_shadow_buf[row * g_shadow_w + x];
        for (uint32_t col = 0; col < w; col++) {
            *(volatile uint32_t *)(uintptr_t)(mmio_row + col * 4) = src[col];
        }
    }
    return 0;
}

/* ===================================================================
 * 12. Rounded rectangle — top corners only
 *     rt_gui_rounded_rect_top(xy, wh, color_radius, alpha)
 *     For title bars: rounded at top, flat at bottom.
 *     Same pack as rt_gui_rounded_rect.
 * =================================================================== */

RuntimeValue rt_gui_rounded_rect_top(RuntimeValue xy, RuntimeValue wh,
                                      RuntimeValue color_radius, RuntimeValue alpha_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_radius >> 32);
    uint32_t radius = (uint32_t)((uint64_t)color_radius & 0xFFFFFFFF);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;

    if (radius > w / 2) radius = w / 2;
    if (radius > h) radius = h;
    if (x >= g_fb_w || y >= SCREEN_H) return 0;

    dirty_mark(x, y, w, h);

    for (uint32_t row = 0; row < h; row++) {
        uint32_t py = y + row;
        if (py >= SCREEN_H) break;

        uint32_t x_start = 0;
        uint32_t x_end = w;

        if (row < radius) {
            /* Top corners only — bottom is flat */
            uint32_t dy = radius - row;
            uint32_t dx = 0;
            while ((dx + 1) * (dx + 1) + dy * dy <= radius * radius) dx++;
            x_start = radius - dx;
            x_end = w - (radius - dx);
        }
        /* No bottom corner rounding — rows >= h-radius are full width */

        for (uint32_t col = x_start; col < x_end; col++) {
            uint32_t px = x + col;
            if (px >= g_fb_w) break;
            if (alpha == 255) {
                fb_write(px, py, 0xFF000000u | color);
            } else {
                uint32_t dst = fb_read(px, py);
                fb_write(px, py, alpha_blend(dst, color, alpha));
            }
        }
    }
    return 0;
}

/* ===================================================================
 * 13. Filled circle (Bresenham midpoint)
 *     rt_gui_filled_circle(pack(cx,cy), pack(diameter,color), alpha, _)
 *     Draws a filled circle centered at (cx, cy) with given diameter.
 * =================================================================== */

RuntimeValue rt_gui_filled_circle(RuntimeValue cx_cy, RuntimeValue diam_color,
                                   RuntimeValue alpha_rv, RuntimeValue unused)
{
    (void)unused;
    uint32_t cx = (uint32_t)((uint64_t)cx_cy >> 32);
    uint32_t cy = (uint32_t)((uint64_t)cx_cy & 0xFFFFFFFF);
    uint32_t diameter = (uint32_t)((uint64_t)diam_color >> 32);
    uint32_t color = (uint32_t)((uint64_t)diam_color & 0xFFFFFFFF);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;

    if (diameter == 0) return 0;
    uint32_t r = diameter / 2;
    /* Center of circle */
    uint32_t ox = cx + r;
    uint32_t oy = cy + r;

    if (ox >= g_fb_w + r || oy >= SCREEN_H + r) return 0;
    dirty_mark(cx, cy, diameter, diameter);

    /* Filled circle via scanline: for each row, compute x span */
    for (uint32_t row = 0; row < diameter; row++) {
        int32_t dy = (int32_t)row - (int32_t)r;
        /* x^2 + y^2 <= r^2 => x = sqrt(r^2 - y^2) */
        int32_t r2 = (int32_t)(r * r);
        int32_t dy2 = dy * dy;
        if (dy2 > r2) continue;

        /* Integer sqrt approximation */
        uint32_t dx = 0;
        while ((int32_t)((dx + 1) * (dx + 1)) <= r2 - dy2) dx++;

        uint32_t py = cy + row;
        if (py >= SCREEN_H) continue;

        uint32_t x_left = ox > dx ? ox - dx : 0;
        uint32_t x_right = ox + dx;
        if (x_right >= g_fb_w) x_right = (uint32_t)g_fb_w - 1;

        for (uint32_t px = x_left; px <= x_right; px++) {
            if (alpha == 255) {
                fb_write(px, py, 0xFF000000u | color);
            } else {
                uint32_t dst = fb_read(px, py);
                fb_write(px, py, alpha_blend(dst, color, alpha));
            }
        }
    }
    return 0;
}

/* ===================================================================
 * 14. Procedural wallpaper generator
 *     rt_gui_draw_wallpaper(pack(width,height), style, _, _)
 *     Generates macOS-like abstract gradient wallpaper with color blobs.
 *     style: 0=dark aurora, 1=light pastel
 * =================================================================== */

static void draw_blob(uint32_t bx, uint32_t by, uint32_t br,
                       uint32_t color, uint8_t max_alpha,
                       uint32_t sw, uint32_t sh)
{
    /* Radial gradient blob — alpha falls off quadratically from center */
    uint32_t x1 = bx > br ? bx - br : 0;
    uint32_t y1 = by > br ? by - br : 0;
    uint32_t x2 = bx + br < sw ? bx + br : sw;
    uint32_t y2 = by + br < sh ? by + br : sh;

    uint32_t r2 = br * br;
    if (r2 == 0) return;

    for (uint32_t row = y1; row < y2; row++) {
        int32_t dy = (int32_t)row - (int32_t)by;
        uint32_t dy2 = (uint32_t)(dy * dy);
        for (uint32_t col = x1; col < x2; col++) {
            int32_t dx = (int32_t)col - (int32_t)bx;
            uint32_t dist2 = (uint32_t)(dx * dx) + dy2;
            if (dist2 >= r2) continue;

            /* Quadratic falloff: alpha = max_alpha * (1 - dist2/r2) */
            uint32_t alpha = (uint32_t)max_alpha * (r2 - dist2) / r2;
            if (alpha > 255) alpha = 255;
            if (alpha < 2) continue;

            uint32_t dst = fb_read(col, row);
            fb_write(col, row, alpha_blend(dst, color, (uint8_t)alpha));
        }
    }
}

RuntimeValue rt_gui_draw_wallpaper(RuntimeValue wh, RuntimeValue style_rv,
                                    RuntimeValue unused1, RuntimeValue unused2)
{
    (void)unused1; (void)unused2;
    uint32_t sw = (uint32_t)((uint64_t)wh >> 32);
    uint32_t sh = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t style = (uint32_t)(uint64_t)style_rv;

    if (sw > SCREEN_W_MAX) sw = SCREEN_W_MAX;
    if (sh > SCREEN_H) sh = SCREEN_H;
    if (sw == 0 || sh == 0) return 0;

    dirty_mark(0, 0, sw, sh);

    if (style == 0) {
        /* Dark Aurora — deep space with colorful nebula blobs */

        /* Base gradient: midnight blue → deep purple */
        for (uint32_t row = 0; row < sh; row++) {
            uint32_t color = lerp_color(0x00060612, 0x001A0830, row, sh > 1 ? sh - 1 : 1);
            for (uint32_t col = 0; col < sw; col++) {
                fb_write(col, row, 0xFF000000u | color);
            }
        }

        /* Star field — scattered bright points (600 stars, with color variety) */
        for (uint32_t i = 0; i < 600; i++) {
            uint32_t sx = ((i * 2654435761u) ^ (i * 340573321u)) % sw;
            uint32_t sy = ((i * 1103515245u) ^ (i * 12345u)) % sh;
            uint8_t brightness = 80 + (((i * 48271u) ^ (i * 65537u)) % 80);
            /* Color variety: most white, some blue-tinted, some warm-tinted */
            uint32_t star_color = 0x00FFFFFF;
            uint32_t color_seed = (i * 48271u) % 10;
            if (color_seed == 0) star_color = 0x00C0D0FF;       /* Cool blue star */
            else if (color_seed == 1) star_color = 0x00FFE8D0;  /* Warm amber star */
            else if (color_seed == 2) star_color = 0x00D0E0FF;  /* Ice blue star */
            fb_write(sx, sy, alpha_blend(fb_read(sx, sy), star_color, brightness));
            /* Every 3rd star is 3px for variation */
            if (i % 3 == 0 && sx + 1 < sw) {
                fb_write(sx + 1, sy, alpha_blend(fb_read(sx + 1, sy), star_color, brightness / 2));
                if (sx + 2 < sw) {
                    fb_write(sx + 2, sy, alpha_blend(fb_read(sx + 2, sy), star_color, brightness / 3));
                }
            }
        }

        /* Super-bright stars — 12 extra-brilliant points for dramatic effect */
        for (uint32_t i = 0; i < 12; i++) {
            uint32_t sx = ((i * 7919u + 104729u) ^ (i * 1299709u)) % sw;
            uint32_t sy = ((i * 224737u) ^ (i * 350377u + 48611u)) % sh;
            /* Color variety: white, warm, cool */
            uint32_t star_color = 0x00FFFFFF;
            if (i % 3 == 1) star_color = 0x00FFE8D0;      /* Warm */
            else if (i % 3 == 2) star_color = 0x00C0D8FF;  /* Cool */
            /* Core pixel at max brightness */
            fb_write(sx, sy, alpha_blend(fb_read(sx, sy), star_color, 250));
            /* 3x3 cross pattern for glow */
            if (sx + 1 < sw) fb_write(sx + 1, sy, alpha_blend(fb_read(sx + 1, sy), star_color, 200));
            if (sx > 0)      fb_write(sx - 1, sy, alpha_blend(fb_read(sx - 1, sy), star_color, 200));
            if (sy + 1 < sh) fb_write(sx, sy + 1, alpha_blend(fb_read(sx, sy + 1), star_color, 180));
            if (sy > 0)      fb_write(sx, sy > 0 ? sy - 1 : 0, alpha_blend(fb_read(sx, sy > 0 ? sy - 1 : 0), star_color, 180));
            /* Diagonal pixels for softer glow */
            if (sx + 1 < sw && sy + 1 < sh) fb_write(sx + 1, sy + 1, alpha_blend(fb_read(sx + 1, sy + 1), star_color, 100));
            if (sx > 0 && sy > 0)           fb_write(sx - 1, sy - 1, alpha_blend(fb_read(sx - 1, sy - 1), star_color, 100));
        }

        /* Subtle horizon glow — brighter band in lower third */
        for (uint32_t row = sh * 2 / 3; row < sh * 2 / 3 + sh / 8; row++) {
            for (uint32_t col = 0; col < sw; col++) {
                uint32_t dist = row - sh * 2 / 3;
                uint32_t max_dist = sh / 8;
                uint32_t alpha = 12 * (max_dist - dist) / max_dist;
                if (alpha > 0) {
                    uint32_t dst = fb_read(col, row);
                    fb_write(col, row, alpha_blend(dst, 0x002040A0, (uint8_t)alpha));
                }
            }
        }

        /* Nebula blobs — larger (1.5x radius) and brighter (boosted alpha) */
        draw_blob(sw * 3 / 4, sh / 4, 330, 0x000A84FF, 44, sw, sh);   /* Blue (top-right) */
        draw_blob(sw / 5, sh * 2 / 3, 300, 0x00BB86FC, 37, sw, sh);   /* Purple (bottom-left) */
        draw_blob(sw / 2, sh / 2, 270, 0x0000D4AA, 27, sw, sh);       /* Teal (center) */
        draw_blob(sw * 4 / 5, sh * 3 / 4, 240, 0x00FF6B9D, 30, sw, sh); /* Pink (bottom-right) */
        draw_blob(sw / 3, sh / 5, 210, 0x00FFD700, 24, sw, sh);       /* Gold (top-left) */
        draw_blob(sw / 8, sh / 8, 180, 0x004ECDC4, 20, sw, sh);       /* Teal-mint (top-left) */
        draw_blob(sw * 5 / 8, sh / 3, 225, 0x00E040FB, 25, sw, sh);   /* Neon purple (center-right) */
        draw_blob(sw / 2, sh * 4 / 5, 195, 0x00FF7043, 22, sw, sh);   /* Coral (bottom-center) */

        /* Extra detail blobs for visual richness — larger (1.5x) and brighter (boosted) */
        draw_blob(sw / 6, sh * 3 / 5, 150, 0x00FFFFFF, 14, sw, sh);   /* White highlight */
        draw_blob(sw * 7 / 8, sh / 6, 135, 0x0064B5F6, 17, sw, sh);  /* Light blue accent */
        draw_blob(sw * 2 / 5, sh / 10, 120, 0x00CE93D8, 17, sw, sh);  /* Light purple */

        /* Additional nebula blobs for photorealistic richness */
        draw_blob(sw / 4, sh * 3 / 5, 100, 0x00FF4060, 12, sw, sh);   /* Red nebula */
        draw_blob(sw * 2 / 3, sh / 3, 80, 0x0040A0FF, 14, sw, sh);   /* Cyan nebula */
        draw_blob(sw / 2, sh * 4 / 5, 120, 0x00C060FF, 10, sw, sh);   /* Violet nebula near bottom */

        /* Extra bright nebula cores for dramatic effect */
        draw_blob(sw / 3, sh / 3, 200, 0x006020A0, 55, sw, sh);      /* Purple core */
        draw_blob(sw * 2 / 3, sh * 2 / 3, 180, 0x002040C0, 50, sw, sh);  /* Blue core */
        draw_blob(sw / 2, sh / 4, 150, 0x00A03060, 40, sw, sh);      /* Magenta accent */

        /* Galaxy spiral — bright core + 2 spiral arms */
        {
            uint32_t gcx = sw * 3 / 5;   /* right of center */
            uint32_t gcy = sh * 2 / 5;   /* upper area */
            /* Warm bright core */
            draw_blob(gcx, gcy, 60, 0x00FFFFFF, 20, sw, sh);
            draw_blob(gcx, gcy, 35, 0x00FFE0B2, 45, sw, sh);
            draw_blob(gcx, gcy, 18, 0x00FFFFFF, 65, sw, sh);
            /* Spiral arm 1 */
            for (int i = 0; i < 8; i++) {
                int32_t ox = i * 22 - 20;
                int32_t oy = i * 12 - 40 + (i * i * 2);
                uint32_t ax = gcx + (uint32_t)(ox > 0 ? ox : 0);
                uint32_t ay = gcy + (uint32_t)(oy > 0 ? oy : 0);
                if (ax < sw && ay < sh)
                    draw_blob(ax, ay, 35 - i * 3, 0x008080FF, 10 - i, sw, sh);
            }
            /* Spiral arm 2 (opposite) */
            for (int i = 0; i < 8; i++) {
                int32_t ox = -(i * 22 - 20);
                int32_t oy = -(i * 12 - 40 + (i * i * 2));
                int32_t raw_x = (int32_t)gcx + ox;
                int32_t raw_y = (int32_t)gcy + oy;
                if (raw_x > 0 && raw_x < (int32_t)sw && raw_y > 0 && raw_y < (int32_t)sh)
                    draw_blob((uint32_t)raw_x, (uint32_t)raw_y, 35 - i * 3, 0x00C080FF, 8 - i, sw, sh);
            }
        }

        /* Crescent moon — upper right */
        {
            uint32_t mx = sw * 7 / 8;
            uint32_t my = sh / 6;
            draw_blob(mx, my, 50, 0x00FFFFFF, 18, sw, sh);  /* glow halo */
            draw_blob(mx, my, 25, 0x00F0E8D0, 85, sw, sh);  /* moon body */
            draw_blob(mx + 12, my > 4 ? my - 4 : 0, 22, 0x00060612, 85, sw, sh);  /* shadow crescent */
        }

        /* Bright constellation stars */
        {
            uint32_t cstars[][2] = {{120,150},{140,170},{160,155},{130,200},{150,195},{140,240},{155,235}};
            for (int i = 0; i < 7; i++) {
                uint32_t sx = cstars[i][0], sy = cstars[i][1];
                if (sx + 2 < sw && sy + 2 < sh) {
                    /* 3x3 bright star with AA edges */
                    uint32_t c = fb_read(sx, sy);
                    fb_write(sx, sy, alpha_blend(c, 0x00FFFFFF, 220));
                    fb_write(sx+1, sy, alpha_blend(fb_read(sx+1,sy), 0x00FFFFFF, 180));
                    fb_write(sx > 0 ? sx-1 : 0, sy, alpha_blend(fb_read(sx > 0 ? sx-1 : 0,sy), 0x00FFFFFF, 140));
                    fb_write(sx, sy+1, alpha_blend(fb_read(sx,sy+1), 0x00FFFFFF, 180));
                    fb_write(sx, sy > 0 ? sy-1 : 0, alpha_blend(fb_read(sx, sy > 0 ? sy-1 : 0), 0x00FFFFFF, 140));
                }
            }
        }

        /* Atmospheric haze — blue band at horizon */
        {
            uint32_t haze_y = sh * 3 / 4;
            uint32_t haze_h = 40;
            for (uint32_t ay = haze_y; ay < haze_y + haze_h && ay < sh; ay++) {
                uint32_t t = ay - haze_y;
                uint8_t ha = (uint8_t)((haze_h - t) * 15 / haze_h);
                for (uint32_t ax = 0; ax < sw; ax++) {
                    fb_write(ax, ay, alpha_blend(fb_read(ax, ay), 0x004060C0, ha));
                }
            }
        }

        /* Subtle blur pass to soften blobs */
        box_blur_h(0, 0, sw, sh, 8);
        box_blur_v(0, 0, sw, sh, 8);

        /* Aurora horizon glow — bright band across lower third */
        for (uint32_t ay = sh * 2 / 3; ay < sh; ay++) {
            uint32_t intensity = (ay - sh * 2 / 3) * 40 / (sh / 3);
            for (uint32_t ax = 0; ax < sw; ax++) {
                uint32_t pixel = fb_read(ax, ay);
                uint8_t r = (pixel >> 16) & 0xFF;
                uint8_t g = (pixel >> 8) & 0xFF;
                uint8_t b = pixel & 0xFF;
                /* Add warm purple-blue glow */
                uint8_t gr = r + (intensity * 3 / 5 > 255 - r ? 255 - r : intensity * 3 / 5);
                uint8_t gg = g + (intensity / 4 > 255 - g ? 255 - g : intensity / 4);
                uint8_t gb = b + (intensity > 255 - b ? 255 - b : intensity);
                fb_write(ax, ay, 0xFF000000u | ((uint32_t)gr << 16) | ((uint32_t)gg << 8) | gb);
            }
        }

        /* Subtle noise overlay for texture */
        for (uint32_t row = 0; row < sh; row++) {
            for (uint32_t col = 0; col < sw; col++) {
                /* Simple hash-based noise */
                uint32_t noise = ((col * 2654435761u) ^ (row * 2246822519u)) & 0xFF;
                if (noise > 240) {
                    uint32_t dst = fb_read(col, row);
                    fb_write(col, row, alpha_blend(dst, 0x00FFFFFF, 6));
                }
            }
        }

        /* Vignette — darken edges for depth */
        for (uint32_t row = 0; row < sh; row++) {
            for (uint32_t col = 0; col < sw; col++) {
                int32_t dx = (int32_t)col - (int32_t)(sw / 2);
                int32_t dy = (int32_t)row - (int32_t)(sh / 2);
                uint32_t dist2 = (uint32_t)(dx * dx + dy * dy);
                uint32_t max_dist2 = (sw / 2) * (sw / 2) + (sh / 2) * (sh / 2);
                if (dist2 > max_dist2 / 3) {
                    uint32_t alpha = (dist2 - max_dist2 / 3) * 40 / (max_dist2 * 2 / 3);
                    if (alpha > 40) alpha = 40;
                    uint32_t dst = fb_read(col, row);
                    fb_write(col, row, alpha_blend(dst, 0x00000000, (uint8_t)alpha));
                }
            }
        }

    } else {
        /* Light Pastel — soft gradient with gentle color washes */

        /* Base gradient: lavender → soft white */
        for (uint32_t row = 0; row < sh; row++) {
            uint32_t color = lerp_color(0x00C8B8E8, 0x00F0ECF5, row, sh > 1 ? sh - 1 : 1);
            for (uint32_t col = 0; col < sw; col++) {
                fb_write(col, row, 0xFF000000u | color);
            }
        }

        /* Sun glow — warm circle in upper area */
        draw_blob(sw / 2, sh / 4, 300, 0x00FFFFFF, 15, sw, sh);

        /* Pastel blobs */
        draw_blob(sw * 2 / 3, sh / 3, 250, 0x00FFB5C5, 25, sw, sh);   /* Pink (top-right) */
        draw_blob(sw / 4, sh / 2, 220, 0x0099CCFF, 22, sw, sh);       /* Sky blue (left) */
        draw_blob(sw / 2, sh * 3 / 4, 200, 0x00B5FFD9, 20, sw, sh);   /* Mint (bottom-center) */
        draw_blob(sw * 3 / 4, sh * 2 / 3, 180, 0x00E8C5FF, 18, sw, sh); /* Lilac (right) */
        draw_blob(sw / 6, sh / 4, 180, 0x00FFE0B2, 18, sw, sh);       /* Peach (top-left) */
        draw_blob(sw * 3 / 4, sh / 5, 160, 0x00B2EBF2, 16, sw, sh);   /* Ice blue (top-right) */
        draw_blob(sw / 3, sh * 4 / 5, 140, 0x00F8BBD0, 14, sw, sh);   /* Rose (bottom-left) */

        /* Extra detail blobs */
        draw_blob(sw / 2, sh / 6, 160, 0x00FFFFFF, 12, sw, sh);       /* Central white glow */
        draw_blob(sw * 4 / 5, sh * 3 / 4, 120, 0x00E1BEE7, 10, sw, sh); /* Lilac accent */
        draw_blob(sw / 8, sh * 4 / 5, 100, 0x00B3E5FC, 10, sw, sh);   /* Sky blue */

        /* Soft pastel clouds for light theme (boosted alpha) */
        draw_blob(sw / 4, sh / 3, 300, 0x00FFF0F8, 18, sw, sh);      /* White-pink cloud */
        draw_blob(sw * 3 / 4, sh / 2, 250, 0x00F0F0FF, 15, sw, sh);  /* White-blue cloud */
        draw_blob(sw / 2, sh * 2 / 3, 280, 0x00F8F0FF, 12, sw, sh);  /* Soft purple cloud */

        /* Sun glow */
        draw_blob(sw / 2, sh / 4, 90, 0x00FFFFFF, 25, sw, sh);
        draw_blob(sw / 2, sh / 4, 45, 0x00FFFDE0, 40, sw, sh);
        /* Horizontal lens flare */
        for (uint32_t fx = (sw/2 > 150 ? sw/2 - 150 : 0); fx < sw/2 + 150 && fx < sw; fx++) {
            uint32_t dist = (fx > sw/2) ? fx - sw/2 : sw/2 - fx;
            if (dist < 150) {
                uint8_t fa = (uint8_t)((150 - dist) * 6 / 150);
                fb_write(fx, sh/4, alpha_blend(fb_read(fx, sh/4), 0x00FFFFFF, fa));
            }
        }

        /* Soften */
        box_blur_h(0, 0, sw, sh, 10);
        box_blur_v(0, 0, sw, sh, 10);

        /* Subtle noise overlay for texture */
        for (uint32_t row = 0; row < sh; row++) {
            for (uint32_t col = 0; col < sw; col++) {
                uint32_t noise = ((col * 2654435761u) ^ (row * 2246822519u)) & 0xFF;
                if (noise > 240) {
                    uint32_t dst = fb_read(col, row);
                    fb_write(col, row, alpha_blend(dst, 0x00000000, 4));
                }
            }
        }

        /* Subtle vignette for depth */
        for (uint32_t row = 0; row < sh; row++) {
            for (uint32_t col = 0; col < sw; col++) {
                int32_t dx = (int32_t)col - (int32_t)(sw / 2);
                int32_t dy = (int32_t)row - (int32_t)(sh / 2);
                uint32_t dist2 = (uint32_t)(dx * dx + dy * dy);
                uint32_t max_dist2 = (sw / 2) * (sw / 2) + (sh / 2) * (sh / 2);
                if (dist2 > max_dist2 / 2) {
                    uint32_t alpha = (dist2 - max_dist2 / 2) * 20 / (max_dist2 / 2);
                    if (alpha > 20) alpha = 20;
                    uint32_t dst = fb_read(col, row);
                    fb_write(col, row, alpha_blend(dst, 0x00000000, (uint8_t)alpha));
                }
            }
        }
    }

    return 0;
}

/* ===================================================================
 * 14a. Radial gradient (public wrapper around draw_blob)
 *      rt_gui_gradient_radial(pack(cx,cy), pack(radius,max_alpha), color, _)
 * =================================================================== */
RuntimeValue rt_gui_gradient_radial(RuntimeValue center_xy, RuntimeValue radius_alpha,
                                     RuntimeValue color, RuntimeValue unused)
{
    uint32_t cx = (uint32_t)((uint64_t)center_xy >> 32);
    uint32_t cy = (uint32_t)((uint64_t)center_xy & 0xFFFFFFFF);
    uint32_t r  = (uint32_t)((uint64_t)radius_alpha >> 32);
    uint32_t max_alpha = (uint32_t)((uint64_t)radius_alpha & 0xFFFFFFFF);
    uint32_t c  = (uint32_t)(uint64_t)color;

    draw_blob(cx, cy, r, c, max_alpha, g_shadow_w, g_shadow_h);
    dirty_mark(cx > r ? cx - r : 0, cy > r ? cy - r : 0, r * 2, r * 2);
    return (RuntimeValue)0;
}

/* ===================================================================
 * 14b. Vignette — darken corners/edges for depth
 *      rt_gui_vignette(pack(w,h), strength, _, _)
 * =================================================================== */
RuntimeValue rt_gui_vignette(RuntimeValue wh, RuntimeValue strength,
                              RuntimeValue unused1, RuntimeValue unused2)
{
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t s = (uint32_t)(uint64_t)strength;
    if (!g_shadow_ready) return (RuntimeValue)0;

    uint32_t cx = w / 2, cy = h / 2;
    uint32_t max_r2 = cx * cx + cy * cy;

    for (uint32_t py = 0; py < h && py < g_shadow_h; py++) {
        for (uint32_t px = 0; px < w && px < g_shadow_w; px++) {
            int32_t dx = (int32_t)px - (int32_t)cx;
            int32_t dy = (int32_t)py - (int32_t)cy;
            uint32_t dist2 = (uint32_t)(dx * dx + dy * dy);
            /* Only darken outer 40% of radius */
            if (dist2 > max_r2 * 36 / 100) {
                uint32_t excess = dist2 - max_r2 * 36 / 100;
                uint32_t range = max_r2 - max_r2 * 36 / 100;
                uint8_t alpha = (uint8_t)(excess * s / (range > 0 ? range : 1));
                if (alpha > (uint8_t)s) alpha = (uint8_t)s;
                fb_write(px, py, alpha_blend(fb_read(px, py), 0x00000000, alpha));
            }
        }
    }
    dirty_mark(0, 0, w, h);
    return (RuntimeValue)0;
}

/* ===================================================================
 * 15. Anti-aliased rounded rectangle
 *     Same signature as rt_gui_rounded_rect but with smooth edge blending.
 *     At corner edges, computes distance to circle boundary and uses
 *     fractional alpha for sub-pixel smoothing.
 * =================================================================== */
RuntimeValue rt_gui_rounded_rect_aa(RuntimeValue xy, RuntimeValue wh,
                                     RuntimeValue color_radius, RuntimeValue alpha_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_radius >> 32);
    uint32_t radius = (uint32_t)((uint64_t)color_radius & 0xFFFFFFFF);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;

    if (radius > w / 2) radius = w / 2;
    if (radius > h / 2) radius = h / 2;
    if (x >= g_fb_w || y >= SCREEN_H || w == 0 || h == 0) return 0;

    dirty_mark(x, y, w, h);

    /* Pre-compute r^4 for superellipse test using 64-bit to avoid overflow */
    uint64_t r2 = (uint64_t)radius * radius;
    uint64_t r4 = r2 * r2;

    for (uint32_t row = 0; row < h; row++) {
        uint32_t py = y + row;
        if (py >= SCREEN_H) break;

        for (uint32_t col = 0; col < w; col++) {
            uint32_t px = x + col;
            if (px >= g_fb_w) break;

            /* Check which corner region this pixel is in */
            int in_corner = 0;
            uint32_t cx = 0, cy = 0; /* distance from corner center */

            if (col < radius && row < radius) {
                /* Top-left corner */
                cx = radius - col;
                cy = radius - row;
                in_corner = 1;
            } else if (col >= w - radius && row < radius) {
                /* Top-right corner */
                cx = col - (w - radius);
                cy = radius - row;
                in_corner = 1;
            } else if (col < radius && row >= h - radius) {
                /* Bottom-left corner */
                cx = radius - col;
                cy = row - (h - radius);
                in_corner = 1;
            } else if (col >= w - radius && row >= h - radius) {
                /* Bottom-right corner */
                cx = col - (w - radius);
                cy = row - (h - radius);
                in_corner = 1;
            }

            if (in_corner) {
                /* Superellipse distance: (cx/r)^4 + (cy/r)^4 vs 1.0
                 * Equivalent: cx^4 + cy^4 vs r^4 (all integer) */
                uint64_t cx2 = (uint64_t)cx * cx;
                uint64_t cy2 = (uint64_t)cy * cy;
                uint64_t cx4 = cx2 * cx2;
                uint64_t cy4 = cy2 * cy2;
                uint64_t dist4 = cx4 + cy4;

                if (dist4 > r4) {
                    /* Outside — skip this pixel */
                    continue;
                }

                /* Anti-aliasing: 3x3 sub-pixel multi-sampling for smooth edges.
                 * For pixels near the superellipse boundary, sample a 3x3 grid
                 * of sub-pixel positions and average coverage for smoother curves. */
                uint64_t threshold = r4 - r4 * 3 / 10; /* 70% of r4 = start of AA band */
                if (dist4 > threshold) {
                    /* 3x3 sub-pixel AA: check coverage at 9 sub-pixel positions */
                    uint32_t coverage = 0;
                    for (int sy = -1; sy <= 1; sy++) {
                        for (int sx = -1; sx <= 1; sx++) {
                            /* Sub-pixel offset: ±0.33 pixels via (cx*3+sx)/3 */
                            int64_t scx = (int64_t)cx * 3 + sx;
                            int64_t scy = (int64_t)cy * 3 + sy;
                            /* Compute (scx/3)^4 + (scy/3)^4 vs r^4
                             * = scx^4/(3^4) + scy^4/(3^4) vs r^4
                             * = (scx^4 + scy^4) vs r^4 * 81 */
                            int64_t scx2 = scx * scx;
                            int64_t scy2 = scy * scy;
                            uint64_t scx4 = (uint64_t)(scx2 * scx2);
                            uint64_t scy4 = (uint64_t)(scy2 * scy2);
                            if (scx4 + scy4 <= r4 * 81) coverage++;
                        }
                    }
                    uint8_t edge_alpha = (uint8_t)((uint32_t)alpha * coverage / 9);
                    if (edge_alpha > 0) {
                        uint32_t dst = fb_read(px, py);
                        fb_write(px, py, alpha_blend(dst, color, edge_alpha));
                    }
                    continue;
                }
            }

            /* Inside the shape — draw pixel */
            if (alpha == 255) {
                fb_write(px, py, 0xFF000000u | color);
            } else {
                uint32_t dst = fb_read(px, py);
                fb_write(px, py, alpha_blend(dst, color, alpha));
            }
        }
    }
    return 0;
}

/* ===================================================================
 * 16. Line drawing (Bresenham)
 *     rt_gui_line(pack(x1,y1), pack(x2,y2), pack(color,thickness), alpha)
 * =================================================================== */
RuntimeValue rt_gui_line(RuntimeValue p1, RuntimeValue p2,
                          RuntimeValue color_thick, RuntimeValue alpha_rv)
{
    int32_t x1 = (int32_t)((uint64_t)p1 >> 32);
    int32_t y1 = (int32_t)((uint64_t)p1 & 0xFFFFFFFF);
    int32_t x2 = (int32_t)((uint64_t)p2 >> 32);
    int32_t y2 = (int32_t)((uint64_t)p2 & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_thick >> 32);
    uint32_t thick = (uint32_t)((uint64_t)color_thick & 0xFFFFFFFF);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;

    if (thick == 0) thick = 1;

    int32_t dx = x2 > x1 ? x2 - x1 : x1 - x2;
    int32_t dy = y2 > y1 ? y2 - y1 : y1 - y2;
    int32_t sx = x1 < x2 ? 1 : -1;
    int32_t sy = y1 < y2 ? 1 : -1;
    int32_t err = dx - dy;

    while (1) {
        /* Draw thick pixel (square brush) */
        for (uint32_t ty = 0; ty < thick; ty++) {
            for (uint32_t tx = 0; tx < thick; tx++) {
                int32_t px = x1 + (int32_t)tx - (int32_t)(thick / 2);
                int32_t py = y1 + (int32_t)ty - (int32_t)(thick / 2);
                if (px >= 0 && px < (int32_t)g_fb_w && py >= 0 && py < (int32_t)SCREEN_H) {
                    if (alpha == 255) {
                        fb_write((uint32_t)px, (uint32_t)py, 0xFF000000u | color);
                    } else {
                        uint32_t dst = fb_read((uint32_t)px, (uint32_t)py);
                        fb_write((uint32_t)px, (uint32_t)py, alpha_blend(dst, color, alpha));
                    }
                }
            }
        }

        if (x1 == x2 && y1 == y2) break;
        int32_t e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }

    dirty_mark(x1 < x2 ? (uint32_t)x1 : (uint32_t)x2,
               y1 < y2 ? (uint32_t)y1 : (uint32_t)y2,
               (uint32_t)dx + thick, (uint32_t)dy + thick);
    return 0;
}

/* ===================================================================
 * 17. Circle outline (ring)
 *     rt_gui_ring(pack(cx,cy), pack(diameter,thickness), pack(color,0), alpha)
 * =================================================================== */
RuntimeValue rt_gui_ring(RuntimeValue cx_cy, RuntimeValue diam_thick,
                          RuntimeValue color_rv, RuntimeValue alpha_rv)
{
    uint32_t cx = (uint32_t)((uint64_t)cx_cy >> 32);
    uint32_t cy = (uint32_t)((uint64_t)cx_cy & 0xFFFFFFFF);
    uint32_t diameter = (uint32_t)((uint64_t)diam_thick >> 32);
    uint32_t thick = (uint32_t)((uint64_t)diam_thick & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_rv >> 32);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;

    if (diameter == 0 || thick == 0) return 0;
    uint32_t r_outer = diameter / 2;
    uint32_t r_inner = r_outer > thick ? r_outer - thick : 0;
    uint32_t r_outer2 = r_outer * r_outer;
    uint32_t r_inner2 = r_inner * r_inner;

    dirty_mark(cx, cy, diameter, diameter);

    for (uint32_t row = 0; row < diameter; row++) {
        int32_t dy = (int32_t)row - (int32_t)r_outer;
        uint32_t dy2 = (uint32_t)(dy * dy);
        for (uint32_t col = 0; col < diameter; col++) {
            int32_t dx = (int32_t)col - (int32_t)r_outer;
            uint32_t dist2 = (uint32_t)(dx * dx) + dy2;
            if (dist2 <= r_outer2 && dist2 >= r_inner2) {
                uint32_t px = cx + col;
                uint32_t py = cy + row;
                if (px < g_fb_w && py < SCREEN_H) {
                    if (alpha == 255) {
                        fb_write(px, py, 0xFF000000u | color);
                    } else {
                        uint32_t dst = fb_read(px, py);
                        fb_write(px, py, alpha_blend(dst, color, alpha));
                    }
                }
            }
        }
    }
    return 0;
}

/* ===================================================================
 * 18. Rounded rectangle with vertical gradient
 *     rt_gui_gradient_rect(xy, wh, color_radius, c2_alpha)
 *     Draws rounded rect with vertical gradient from color (high bits of
 *     color_radius) to c2 (high bits of c2_alpha). Alpha from low byte of c2_alpha.
 * =================================================================== */
RuntimeValue rt_gui_gradient_rect(RuntimeValue xy, RuntimeValue wh,
                                   RuntimeValue color_radius, RuntimeValue c2_alpha)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t w = (uint32_t)((uint64_t)wh >> 32);
    uint32_t h = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);
    uint32_t c1 = (uint32_t)((uint64_t)color_radius >> 32);
    uint32_t radius = (uint32_t)((uint64_t)color_radius & 0xFFFFFFFF);
    uint32_t c2 = (uint32_t)((uint64_t)c2_alpha >> 32);
    uint8_t alpha = (uint8_t)(uint64_t)c2_alpha;

    if (radius > w / 2) radius = w / 2;
    if (radius > h / 2) radius = h / 2;
    if (x >= g_fb_w || y >= SCREEN_H) return 0;

    dirty_mark(x, y, w, h);

    for (uint32_t row = 0; row < h; row++) {
        uint32_t py = y + row;
        if (py >= SCREEN_H) break;

        uint32_t color = lerp_color(c1, c2, row, h > 1 ? h - 1 : 1);

        uint32_t x_start = 0;
        uint32_t x_end = w;

        if (row < radius) {
            uint32_t dy = radius - row;
            uint32_t dx = 0;
            while ((dx + 1) * (dx + 1) + dy * dy <= radius * radius) dx++;
            x_start = radius - dx;
            x_end = w - (radius - dx);
        } else if (row >= h - radius) {
            uint32_t dy = row - (h - radius);
            uint32_t dx = 0;
            while ((dx + 1) * (dx + 1) + dy * dy <= radius * radius) dx++;
            x_start = radius - dx;
            x_end = w - (radius - dx);
        }

        for (uint32_t col = x_start; col < x_end; col++) {
            uint32_t px = x + col;
            if (px >= g_fb_w) break;
            if (alpha == 255) {
                fb_write(px, py, 0xFF000000u | color);
            } else {
                uint32_t dst = fb_read(px, py);
                fb_write(px, py, alpha_blend(dst, color, alpha));
            }
        }
    }
    return 0;
}

/* ===================================================================
 * 19. Bitmap font text rendering
 *     rt_gui_draw_text(xy, text_ptr_len, color_size, alpha)
 *
 *     Renders ASCII text using built-in 8x16 bitmap font.
 *     xy = pack(x, y)
 *     text_ptr_len = pack(ptr_to_string, length)  -- RuntimeValue string
 *     color_size = pack(color, 0)  -- color in high 32 bits
 *     alpha = alpha value
 *
 *     Since Simple strings are RuntimeValue (tagged pointer), we receive
 *     the raw string data pointer and length from the Simple side.
 * =================================================================== */

/* Standard VGA 8x16 bitmap font for ASCII 32-126 (95 printable chars)
 * Each character is 8 pixels wide, 16 pixels tall.
 * Stored as 16 bytes per char (1 byte per row, 8 bits used).
 * This is the CP437 8x16 font used by BIOS/EFI for high-quality text. */
static const uint8_t font_8x16[95][16] = {
    /* ' ' (32) */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* '!' (33) */ {0x00,0x00,0x18,0x3C,0x3C,0x3C,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
    /* '"' (34) */ {0x00,0x66,0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* '#' (35) */ {0x00,0x00,0x00,0x6C,0x6C,0xFE,0x6C,0x6C,0x6C,0xFE,0x6C,0x6C,0x00,0x00,0x00,0x00},
    /* '$' (36) */ {0x18,0x18,0x7C,0xC6,0xC2,0xC0,0x7C,0x06,0x06,0x86,0xC6,0x7C,0x18,0x18,0x00,0x00},
    /* '%' (37) */ {0x00,0x00,0x00,0x00,0xC2,0xC6,0x0C,0x18,0x30,0x60,0xC6,0x86,0x00,0x00,0x00,0x00},
    /* '&' (38) */ {0x00,0x00,0x38,0x6C,0x6C,0x38,0x76,0xDC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00},
    /* '\'' (39)*/ {0x00,0x30,0x30,0x30,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* '(' (40) */ {0x00,0x00,0x0C,0x18,0x30,0x30,0x30,0x30,0x30,0x30,0x18,0x0C,0x00,0x00,0x00,0x00},
    /* ')' (41) */ {0x00,0x00,0x30,0x18,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x18,0x30,0x00,0x00,0x00,0x00},
    /* '*' (42) */ {0x00,0x00,0x00,0x00,0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00,0x00,0x00,0x00,0x00},
    /* '+' (43) */ {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00},
    /* ',' (44) */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x30,0x00,0x00,0x00},
    /* '-' (45) */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* '.' (46) */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
    /* '/' (47) */ {0x00,0x00,0x00,0x00,0x02,0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00,0x00,0x00,0x00},
    /* '0' (48) */ {0x00,0x00,0x7C,0xC6,0xC6,0xCE,0xDE,0xF6,0xE6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* '1' (49) */ {0x00,0x00,0x18,0x38,0x78,0x18,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00},
    /* '2' (50) */ {0x00,0x00,0x7C,0xC6,0x06,0x0C,0x18,0x30,0x60,0xC0,0xC6,0xFE,0x00,0x00,0x00,0x00},
    /* '3' (51) */ {0x00,0x00,0x7C,0xC6,0x06,0x06,0x3C,0x06,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* '4' (52) */ {0x00,0x00,0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x0C,0x1E,0x00,0x00,0x00,0x00},
    /* '5' (53) */ {0x00,0x00,0xFE,0xC0,0xC0,0xC0,0xFC,0x06,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* '6' (54) */ {0x00,0x00,0x38,0x60,0xC0,0xC0,0xFC,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* '7' (55) */ {0x00,0x00,0xFE,0xC6,0x06,0x06,0x0C,0x18,0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00},
    /* '8' (56) */ {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* '9' (57) */ {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7E,0x06,0x06,0x06,0x0C,0x78,0x00,0x00,0x00,0x00},
    /* ':' (58) */ {0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00},
    /* ';' (59) */ {0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x30,0x00,0x00,0x00,0x00},
    /* '<' (60) */ {0x00,0x00,0x00,0x06,0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x06,0x00,0x00,0x00,0x00},
    /* '=' (61) */ {0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* '>' (62) */ {0x00,0x00,0x00,0x60,0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x60,0x00,0x00,0x00,0x00},
    /* '?' (63) */ {0x00,0x00,0x7C,0xC6,0xC6,0x0C,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
    /* '@' (64) */ {0x00,0x00,0x00,0x7C,0xC6,0xC6,0xDE,0xDE,0xDE,0xDC,0xC0,0x7C,0x00,0x00,0x00,0x00},
    /* 'A' (65) */ {0x00,0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00},
    /* 'B' (66) */ {0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x66,0x66,0x66,0x66,0xFC,0x00,0x00,0x00,0x00},
    /* 'C' (67) */ {0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xC0,0xC0,0xC2,0x66,0x3C,0x00,0x00,0x00,0x00},
    /* 'D' (68) */ {0x00,0x00,0xF8,0x6C,0x66,0x66,0x66,0x66,0x66,0x66,0x6C,0xF8,0x00,0x00,0x00,0x00},
    /* 'E' (69) */ {0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x60,0x62,0x66,0xFE,0x00,0x00,0x00,0x00},
    /* 'F' (70) */ {0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00},
    /* 'G' (71) */ {0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xDE,0xC6,0xC6,0x66,0x3A,0x00,0x00,0x00,0x00},
    /* 'H' (72) */ {0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00},
    /* 'I' (73) */ {0x00,0x00,0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
    /* 'J' (74) */ {0x00,0x00,0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0xCC,0xCC,0xCC,0x78,0x00,0x00,0x00,0x00},
    /* 'K' (75) */ {0x00,0x00,0xE6,0x66,0x66,0x6C,0x78,0x78,0x6C,0x66,0x66,0xE6,0x00,0x00,0x00,0x00},
    /* 'L' (76) */ {0x00,0x00,0xF0,0x60,0x60,0x60,0x60,0x60,0x60,0x62,0x66,0xFE,0x00,0x00,0x00,0x00},
    /* 'M' (77) */ {0x00,0x00,0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00},
    /* 'N' (78) */ {0x00,0x00,0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00},
    /* 'O' (79) */ {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* 'P' (80) */ {0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x60,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00},
    /* 'Q' (81) */ {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xD6,0xDE,0x7C,0x0C,0x0E,0x00,0x00},
    /* 'R' (82) */ {0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x6C,0x66,0x66,0x66,0xE6,0x00,0x00,0x00,0x00},
    /* 'S' (83) */ {0x00,0x00,0x7C,0xC6,0xC6,0x60,0x38,0x0C,0x06,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* 'T' (84) */ {0x00,0x00,0xFF,0xDB,0x99,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
    /* 'U' (85) */ {0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* 'V' (86) */ {0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00,0x00,0x00,0x00},
    /* 'W' (87) */ {0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xD6,0xD6,0xD6,0xFE,0xEE,0x6C,0x00,0x00,0x00,0x00},
    /* 'X' (88) */ {0x00,0x00,0xC6,0xC6,0x6C,0x7C,0x38,0x38,0x7C,0x6C,0xC6,0xC6,0x00,0x00,0x00,0x00},
    /* 'Y' (89) */ {0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x78,0x30,0x30,0x30,0x30,0x78,0x00,0x00,0x00,0x00},
    /* 'Z' (90) */ {0x00,0x00,0xFE,0xC6,0x86,0x0C,0x18,0x30,0x60,0xC2,0xC6,0xFE,0x00,0x00,0x00,0x00},
    /* '[' (91) */ {0x00,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x3C,0x00,0x00,0x00,0x00},
    /* '\\' (92)*/ {0x00,0x00,0x00,0x80,0xC0,0xE0,0x70,0x38,0x1C,0x0E,0x06,0x02,0x00,0x00,0x00,0x00},
    /* ']' (93) */ {0x00,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00,0x00,0x00,0x00},
    /* '^' (94) */ {0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* '_' (95) */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00},
    /* '`' (96) */ {0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 'a' (97) */ {0x00,0x00,0x00,0x00,0x00,0x78,0x0C,0x7C,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00},
    /* 'b' (98) */ {0x00,0x00,0xE0,0x60,0x60,0x78,0x6C,0x66,0x66,0x66,0x66,0x7C,0x00,0x00,0x00,0x00},
    /* 'c' (99) */ {0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC0,0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* 'd' (100)*/ {0x00,0x00,0x1C,0x0C,0x0C,0x3C,0x6C,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00},
    /* 'e' (101)*/ {0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xFE,0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* 'f' (102)*/ {0x00,0x00,0x1C,0x36,0x32,0x30,0x78,0x30,0x30,0x30,0x30,0x78,0x00,0x00,0x00,0x00},
    /* 'g' (103)*/ {0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x0C,0xCC,0x78,0x00},
    /* 'h' (104)*/ {0x00,0x00,0xE0,0x60,0x60,0x6C,0x76,0x66,0x66,0x66,0x66,0xE6,0x00,0x00,0x00,0x00},
    /* 'i' (105)*/ {0x00,0x00,0x18,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
    /* 'j' (106)*/ {0x00,0x00,0x06,0x06,0x00,0x0E,0x06,0x06,0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00},
    /* 'k' (107)*/ {0x00,0x00,0xE0,0x60,0x60,0x66,0x6C,0x78,0x78,0x6C,0x66,0xE6,0x00,0x00,0x00,0x00},
    /* 'l' (108)*/ {0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
    /* 'm' (109)*/ {0x00,0x00,0x00,0x00,0x00,0xEC,0xFE,0xD6,0xD6,0xD6,0xD6,0xC6,0x00,0x00,0x00,0x00},
    /* 'n' (110)*/ {0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00},
    /* 'o' (111)*/ {0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* 'p' (112)*/ {0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00},
    /* 'q' (113)*/ {0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x0C,0x0C,0x1E,0x00},
    /* 'r' (114)*/ {0x00,0x00,0x00,0x00,0x00,0xDC,0x76,0x66,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00},
    /* 's' (115)*/ {0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0x60,0x38,0x0C,0xC6,0x7C,0x00,0x00,0x00,0x00},
    /* 't' (116)*/ {0x00,0x00,0x10,0x30,0x30,0xFC,0x30,0x30,0x30,0x30,0x36,0x1C,0x00,0x00,0x00,0x00},
    /* 'u' (117)*/ {0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00},
    /* 'v' (118)*/ {0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x00,0x00,0x00,0x00},
    /* 'w' (119)*/ {0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xD6,0xD6,0xD6,0xFE,0x6C,0x00,0x00,0x00,0x00},
    /* 'x' (120)*/ {0x00,0x00,0x00,0x00,0x00,0xC6,0x6C,0x38,0x38,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00},
    /* 'y' (121)*/ {0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7E,0x06,0x0C,0xF8,0x00},
    /* 'z' (122)*/ {0x00,0x00,0x00,0x00,0x00,0xFE,0xCC,0x18,0x30,0x60,0xC6,0xFE,0x00,0x00,0x00,0x00},
    /* '{' (123)*/ {0x00,0x00,0x0E,0x18,0x18,0x18,0x70,0x18,0x18,0x18,0x18,0x0E,0x00,0x00,0x00,0x00},
    /* '|' (124)*/ {0x00,0x00,0x18,0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00},
    /* '}' (125)*/ {0x00,0x00,0x70,0x18,0x18,0x18,0x0E,0x18,0x18,0x18,0x18,0x70,0x00,0x00,0x00,0x00},
    /* '~' (126)*/ {0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};

/* ===================================================================
 * UTF-8 Decoder — for Unicode text rendering
 *
 * Returns the codepoint and number of bytes consumed.
 * Invalid sequences return U+FFFD (replacement character) and advance 1 byte.
 * =================================================================== */

static uint32_t utf8_decode(const uint8_t *buf, uint32_t len, uint32_t pos, uint32_t *out_cp) {
    if (pos >= len) { *out_cp = 0; return 0; }
    uint8_t b0 = buf[pos];

    /* 1-byte: 0xxxxxxx */
    if (b0 < 0x80) { *out_cp = b0; return 1; }

    /* Continuation byte as lead — invalid */
    if (b0 < 0xC0) { *out_cp = 0xFFFD; return 1; }

    /* 2-byte: 110xxxxx 10xxxxxx */
    if (b0 < 0xE0) {
        if (pos + 1 >= len) { *out_cp = 0xFFFD; return 1; }
        uint8_t b1 = buf[pos + 1];
        if ((b1 & 0xC0) != 0x80) { *out_cp = 0xFFFD; return 1; }
        uint32_t cp = ((b0 & 0x1F) << 6) | (b1 & 0x3F);
        if (cp < 0x80) { *out_cp = 0xFFFD; return 2; } /* overlong */
        *out_cp = cp; return 2;
    }

    /* 3-byte: 1110xxxx 10xxxxxx 10xxxxxx */
    if (b0 < 0xF0) {
        if (pos + 2 >= len) { *out_cp = 0xFFFD; return 1; }
        uint8_t b1 = buf[pos + 1], b2 = buf[pos + 2];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) { *out_cp = 0xFFFD; return 1; }
        uint32_t cp = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
        if (cp < 0x800) { *out_cp = 0xFFFD; return 3; } /* overlong */
        if (cp >= 0xD800 && cp <= 0xDFFF) { *out_cp = 0xFFFD; return 3; } /* surrogate */
        *out_cp = cp; return 3;
    }

    /* 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (b0 < 0xF8) {
        if (pos + 3 >= len) { *out_cp = 0xFFFD; return 1; }
        uint8_t b1 = buf[pos + 1], b2 = buf[pos + 2], b3 = buf[pos + 3];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) {
            *out_cp = 0xFFFD; return 1;
        }
        uint32_t cp = ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) |
                      ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        if (cp < 0x10000 || cp > 0x10FFFF) { *out_cp = 0xFFFD; return 4; }
        *out_cp = cp; return 4;
    }

    *out_cp = 0xFFFD; return 1;
}

/* Check if a codepoint is East Asian Wide (CJK, Hangul syllables, fullwidth) */
static int cp_is_wide(uint32_t cp) {
    if (cp >= 0x4E00 && cp <= 0x9FFF) return 1;  /* CJK Unified Ideographs */
    if (cp >= 0x3400 && cp <= 0x4DBF) return 1;  /* CJK Extension A */
    if (cp >= 0xAC00 && cp <= 0xD7AF) return 1;  /* Hangul Syllables */
    if (cp >= 0x3000 && cp <= 0x303F) return 1;  /* CJK Symbols */
    if (cp >= 0x3040 && cp <= 0x30FF) return 1;  /* Hiragana + Katakana */
    if (cp >= 0xFF01 && cp <= 0xFF60) return 1;  /* Fullwidth Forms */
    if (cp >= 0xF900 && cp <= 0xFAFF) return 1;  /* CJK Compat Ideographs */
    if (cp >= 0x3130 && cp <= 0x318F) return 1;  /* Hangul Compat Jamo */
    if (cp >= 0x20000 && cp <= 0x2A6DF) return 1; /* CJK Extension B */
    return 0;
}

/* ===================================================================
 * TrueType Font Support — stb_truetype integration point
 *
 * Font registry for loaded TTF/OTF fonts. When a glyph is not in the
 * built-in vector font, we look it up in loaded TrueType fonts.
 * =================================================================== */

#define MAX_FONTS 16
#define GLYPH_CACHE_SIZE 256

typedef struct {
    const uint8_t *data;   /* Raw TTF/OTF file data */
    uint32_t data_len;     /* Length of font data */
    uint32_t active;       /* 1 if slot is in use */
    uint32_t priority;     /* Lower = higher priority (0 = primary) */
    /* Future: stbtt_fontinfo would go here */
} FontSlot;

static FontSlot g_fonts[MAX_FONTS];
static int g_font_count = 0;

/* Glyph bitmap cache — avoids re-rasterizing recently used glyphs */
typedef struct {
    uint32_t codepoint;     /* Unicode codepoint */
    uint32_t pixel_height;  /* Rasterized height */
    uint8_t *bitmap;        /* Alpha bitmap (pixel_width * pixel_height bytes) */
    uint16_t bmp_width;     /* Bitmap width in pixels */
    uint16_t bmp_height;    /* Bitmap height in pixels */
    int16_t  bearing_x;     /* Left side bearing */
    int16_t  advance;       /* Horizontal advance width */
    uint32_t font_idx;      /* Which font produced this glyph */
    uint32_t age;           /* LRU counter */
} GlyphCacheEntry;

static GlyphCacheEntry g_glyph_cache[GLYPH_CACHE_SIZE];
static uint32_t g_glyph_cache_age = 0;

/* rt_font_load_from_memory(data_ptr, data_len, priority, _)
 * Register a TrueType font from memory. Returns font index or -1. */
RuntimeValue rt_font_load_from_memory(RuntimeValue data_ptr_rv, RuntimeValue data_len_rv,
                                       RuntimeValue priority_rv, RuntimeValue unused) {
    (void)unused;
    if (g_font_count >= MAX_FONTS) return (RuntimeValue)-1;

    uint64_t ptr = (uint64_t)data_ptr_rv;
    uint32_t len = (uint32_t)(uint64_t)data_len_rv;
    uint32_t pri = (uint32_t)(uint64_t)priority_rv;

    int idx = g_font_count++;
    g_fonts[idx].data = (const uint8_t *)(uintptr_t)ptr;
    g_fonts[idx].data_len = len;
    g_fonts[idx].active = 1;
    g_fonts[idx].priority = pri;

    return (RuntimeValue)idx;
}

/* ===================================================================
 * Vector Font Glyph Outlines
 *
 * Simplified sans-serif font inspired by Helvetica/SF Pro proportions.
 * Each glyph is an array of path commands in a 16x24 design grid.
 * Commands: {type, x, y} where type: 0=MOVE, 1=LINE, 2=CLOSE(ignored x,y)
 *
 * The rasterizer scales these to any pixel size using integer math.
 * =================================================================== */

#define VF_MOVE  0
#define VF_LINE  1
#define VF_CLOSE 2
#define VF_END   3  /* End of glyph */

typedef struct { int8_t type, x, y; } VfCmd;

/* Glyph widths (in design units, out of 16) */
static const uint8_t vf_widths[95] = {
    5,  /* space */
    3, 6, 9, 8, 10, 10, 3, 5, 5, 7, 8, 3, 6, 3, 7,  /* ! " # $ % & ' ( ) * + , - . / */
    9, 7, 9, 9, 9, 9, 9, 9, 9, 9,  /* 0-9 */
    3, 3, 7, 8, 7, 8,  /* : ; < = > ? */
    14, /* @ */
    11, 10, 10, 10, 9, 9, 11, 10, 3, 7, 10, 9, 12, 10, 11, 10, 11, 10, 9, 9, 10, 11, 14, 10, 10, 9,  /* A-Z */
    5, 7, 5, 8, 8, 4,  /* [ \ ] ^ _ ` */
    9, 9, 8, 9, 9, 6, 9, 9, 3, 5, 8, 3, 13, 9, 9, 9, 9, 6, 8, 6, 9, 8, 12, 8, 8, 8,  /* a-z */
    5, 3, 5, 9  /* { | } ~ */
};

/* Glyph outline data — only define the most-used characters.
 * Others fall back to the bitmap font. */

static const VfCmd vf_glyph_A[] = {
    {VF_MOVE, 0, 24}, {VF_LINE, 5, 0}, {VF_LINE, 6, 0},
    {VF_LINE, 11, 24}, {VF_LINE, 9, 24}, {VF_LINE, 7, 16},
    {VF_LINE, 4, 16}, {VF_LINE, 2, 24}, {VF_CLOSE, 0, 0},
    /* Crossbar cutout */
    {VF_MOVE, 4, 14}, {VF_LINE, 5, 8}, {VF_LINE, 6, 8},
    {VF_LINE, 7, 14}, {VF_CLOSE, 0, 0},
    {VF_END, 0, 0}
};

static const VfCmd vf_glyph_E[] = {
    {VF_MOVE, 1, 0}, {VF_LINE, 9, 0}, {VF_LINE, 9, 3},
    {VF_LINE, 3, 3}, {VF_LINE, 3, 10}, {VF_LINE, 8, 10},
    {VF_LINE, 8, 13}, {VF_LINE, 3, 13}, {VF_LINE, 3, 21},
    {VF_LINE, 9, 21}, {VF_LINE, 9, 24}, {VF_LINE, 1, 24},
    {VF_CLOSE, 0, 0}, {VF_END, 0, 0}
};

static const VfCmd vf_glyph_B[] = {
    {VF_MOVE,1,0},{VF_LINE,7,0},{VF_LINE,9,2},{VF_LINE,9,10},{VF_LINE,7,12},
    {VF_LINE,9,14},{VF_LINE,9,22},{VF_LINE,7,24},{VF_LINE,1,24},{VF_CLOSE,0,0},
    {VF_MOVE,3,3},{VF_LINE,6,3},{VF_LINE,7,4},{VF_LINE,7,9},{VF_LINE,6,10},
    {VF_LINE,3,10},{VF_CLOSE,0,0},
    {VF_MOVE,3,14},{VF_LINE,6,14},{VF_LINE,7,15},{VF_LINE,7,21},{VF_LINE,6,22},
    {VF_LINE,3,22},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_C[] = {
    {VF_MOVE,9,3},{VF_LINE,9,0},{VF_LINE,2,0},{VF_LINE,0,2},{VF_LINE,0,22},
    {VF_LINE,2,24},{VF_LINE,9,24},{VF_LINE,9,21},{VF_LINE,3,21},{VF_LINE,2,20},
    {VF_LINE,2,4},{VF_LINE,3,3},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_D[] = {
    {VF_MOVE,1,0},{VF_LINE,7,0},{VF_LINE,10,3},{VF_LINE,10,21},{VF_LINE,7,24},
    {VF_LINE,1,24},{VF_CLOSE,0,0},
    {VF_MOVE,3,3},{VF_LINE,6,3},{VF_LINE,8,5},{VF_LINE,8,19},{VF_LINE,6,21},
    {VF_LINE,3,21},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_F[] = {
    {VF_MOVE,1,0},{VF_LINE,9,0},{VF_LINE,9,3},{VF_LINE,3,3},{VF_LINE,3,10},
    {VF_LINE,8,10},{VF_LINE,8,13},{VF_LINE,3,13},{VF_LINE,3,24},{VF_LINE,1,24},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_G[] = {
    {VF_MOVE,9,3},{VF_LINE,9,0},{VF_LINE,2,0},{VF_LINE,0,2},{VF_LINE,0,22},
    {VF_LINE,2,24},{VF_LINE,9,24},{VF_LINE,11,22},{VF_LINE,11,12},{VF_LINE,6,12},
    {VF_LINE,6,14},{VF_LINE,9,14},{VF_LINE,9,21},{VF_LINE,3,21},{VF_LINE,2,20},
    {VF_LINE,2,4},{VF_LINE,3,3},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_H[] = {
    {VF_MOVE,1,0},{VF_LINE,3,0},{VF_LINE,3,10},{VF_LINE,7,10},{VF_LINE,7,0},
    {VF_LINE,9,0},{VF_LINE,9,24},{VF_LINE,7,24},{VF_LINE,7,13},{VF_LINE,3,13},
    {VF_LINE,3,24},{VF_LINE,1,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_I[] = {
    {VF_MOVE,0,0},{VF_LINE,3,0},{VF_LINE,3,24},{VF_LINE,0,24},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_J[] = {
    {VF_MOVE,4,0},{VF_LINE,7,0},{VF_LINE,7,20},{VF_LINE,5,24},{VF_LINE,1,24},
    {VF_LINE,0,22},{VF_LINE,0,18},{VF_LINE,2,18},{VF_LINE,2,21},{VF_LINE,3,22},
    {VF_LINE,5,20},{VF_LINE,5,0},{VF_LINE,4,0},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_K[] = {
    {VF_MOVE,1,0},{VF_LINE,3,0},{VF_LINE,3,9},{VF_LINE,7,0},{VF_LINE,10,0},
    {VF_LINE,5,11},{VF_LINE,10,24},{VF_LINE,7,24},{VF_LINE,3,14},{VF_LINE,3,24},
    {VF_LINE,1,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_L[] = {
    {VF_MOVE,1,0},{VF_LINE,3,0},{VF_LINE,3,21},{VF_LINE,9,21},{VF_LINE,9,24},
    {VF_LINE,1,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_M[] = {
    {VF_MOVE,0,0},{VF_LINE,2,0},{VF_LINE,6,10},{VF_LINE,10,0},{VF_LINE,12,0},
    {VF_LINE,12,24},{VF_LINE,10,24},{VF_LINE,10,6},{VF_LINE,7,14},{VF_LINE,5,14},
    {VF_LINE,2,6},{VF_LINE,2,24},{VF_LINE,0,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_N[] = {
    {VF_MOVE,1,0},{VF_LINE,3,0},{VF_LINE,8,16},{VF_LINE,8,0},{VF_LINE,10,0},
    {VF_LINE,10,24},{VF_LINE,8,24},{VF_LINE,3,8},{VF_LINE,3,24},{VF_LINE,1,24},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_O[] = {
    {VF_MOVE,2,0},{VF_LINE,9,0},{VF_LINE,11,2},{VF_LINE,11,22},{VF_LINE,9,24},
    {VF_LINE,2,24},{VF_LINE,0,22},{VF_LINE,0,2},{VF_CLOSE,0,0},
    {VF_MOVE,3,3},{VF_LINE,8,3},{VF_LINE,9,4},{VF_LINE,9,20},{VF_LINE,8,21},
    {VF_LINE,3,21},{VF_LINE,2,20},{VF_LINE,2,4},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_P[] = {
    {VF_MOVE,1,0},{VF_LINE,7,0},{VF_LINE,9,2},{VF_LINE,9,12},{VF_LINE,7,14},
    {VF_LINE,3,14},{VF_LINE,3,24},{VF_LINE,1,24},{VF_CLOSE,0,0},
    {VF_MOVE,3,3},{VF_LINE,6,3},{VF_LINE,7,4},{VF_LINE,7,10},{VF_LINE,6,11},
    {VF_LINE,3,11},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_Q[] = {
    {VF_MOVE,2,0},{VF_LINE,9,0},{VF_LINE,11,2},{VF_LINE,11,19},{VF_LINE,10,20},
    {VF_LINE,12,24},{VF_LINE,10,24},{VF_LINE,8,21},{VF_LINE,2,24},{VF_LINE,0,22},
    {VF_LINE,0,2},{VF_CLOSE,0,0},
    {VF_MOVE,3,3},{VF_LINE,8,3},{VF_LINE,9,4},{VF_LINE,9,18},{VF_LINE,7,20},
    {VF_LINE,3,21},{VF_LINE,2,20},{VF_LINE,2,4},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_R[] = {
    {VF_MOVE,1,0},{VF_LINE,7,0},{VF_LINE,9,2},{VF_LINE,9,10},{VF_LINE,7,12},
    {VF_LINE,9,14},{VF_LINE,10,24},{VF_LINE,8,24},{VF_LINE,7,15},{VF_LINE,3,14},
    {VF_LINE,3,24},{VF_LINE,1,24},{VF_CLOSE,0,0},
    {VF_MOVE,3,3},{VF_LINE,6,3},{VF_LINE,7,4},{VF_LINE,7,10},{VF_LINE,6,11},
    {VF_LINE,3,11},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_S[] = {
    {VF_MOVE,8,3},{VF_LINE,3,3},{VF_LINE,2,4},{VF_LINE,2,10},{VF_LINE,3,11},
    {VF_LINE,7,11},{VF_LINE,9,13},{VF_LINE,9,22},{VF_LINE,7,24},{VF_LINE,1,24},
    {VF_LINE,1,21},{VF_LINE,7,21},{VF_LINE,7,14},{VF_LINE,6,13},{VF_LINE,2,13},
    {VF_LINE,0,11},{VF_LINE,0,2},{VF_LINE,2,0},{VF_LINE,8,0},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_T[] = {
    {VF_MOVE,0,0},{VF_LINE,9,0},{VF_LINE,9,3},{VF_LINE,6,3},{VF_LINE,6,24},
    {VF_LINE,4,24},{VF_LINE,4,3},{VF_LINE,0,3},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_U[] = {
    {VF_MOVE,1,0},{VF_LINE,3,0},{VF_LINE,3,20},{VF_LINE,4,21},{VF_LINE,7,21},
    {VF_LINE,8,20},{VF_LINE,8,0},{VF_LINE,10,0},{VF_LINE,10,22},{VF_LINE,8,24},
    {VF_LINE,3,24},{VF_LINE,1,22},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_V[] = {
    {VF_MOVE,0,0},{VF_LINE,2,0},{VF_LINE,5,18},{VF_LINE,9,0},{VF_LINE,11,0},
    {VF_LINE,6,24},{VF_LINE,5,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_W[] = {
    {VF_MOVE,0,0},{VF_LINE,2,0},{VF_LINE,4,16},{VF_LINE,6,6},{VF_LINE,7,6},
    {VF_LINE,9,16},{VF_LINE,11,0},{VF_LINE,13,0},{VF_LINE,10,24},{VF_LINE,9,24},
    {VF_LINE,7,14},{VF_LINE,5,24},{VF_LINE,4,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_X[] = {
    {VF_MOVE,0,0},{VF_LINE,2,0},{VF_LINE,5,10},{VF_LINE,8,0},{VF_LINE,10,0},
    {VF_LINE,6,12},{VF_LINE,10,24},{VF_LINE,8,24},{VF_LINE,5,14},{VF_LINE,2,24},
    {VF_LINE,0,24},{VF_LINE,4,12},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_Y[] = {
    {VF_MOVE,0,0},{VF_LINE,2,0},{VF_LINE,5,10},{VF_LINE,8,0},{VF_LINE,10,0},
    {VF_LINE,6,13},{VF_LINE,6,24},{VF_LINE,4,24},{VF_LINE,4,13},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_Z[] = {
    {VF_MOVE,0,0},{VF_LINE,9,0},{VF_LINE,9,3},{VF_LINE,3,21},{VF_LINE,9,21},
    {VF_LINE,9,24},{VF_LINE,0,24},{VF_LINE,0,21},{VF_LINE,6,3},{VF_LINE,0,3},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_d[] = {
    {VF_MOVE, 7, 0}, {VF_LINE, 9, 0}, {VF_LINE, 9, 24},
    {VF_LINE, 7, 24}, {VF_LINE, 7, 22},
    {VF_LINE, 5, 24}, {VF_LINE, 2, 24}, {VF_LINE, 0, 21},
    {VF_LINE, 0, 11}, {VF_LINE, 2, 8}, {VF_LINE, 5, 8},
    {VF_LINE, 7, 10}, {VF_CLOSE, 0, 0},
    /* Inner cutout */
    {VF_MOVE, 2, 11}, {VF_LINE, 3, 10}, {VF_LINE, 6, 10},
    {VF_LINE, 7, 11}, {VF_LINE, 7, 21}, {VF_LINE, 6, 22},
    {VF_LINE, 3, 22}, {VF_LINE, 2, 21}, {VF_CLOSE, 0, 0},
    {VF_END, 0, 0}
};

static const VfCmd vf_glyph_e[] = {
    {VF_MOVE, 0, 14}, {VF_LINE, 9, 14}, {VF_LINE, 9, 11},
    {VF_LINE, 7, 8}, {VF_LINE, 2, 8}, {VF_LINE, 0, 11},
    {VF_LINE, 0, 21}, {VF_LINE, 2, 24}, {VF_LINE, 7, 24},
    {VF_LINE, 9, 22}, {VF_LINE, 9, 20}, {VF_LINE, 7, 20},
    {VF_LINE, 7, 22}, {VF_LINE, 2, 22}, {VF_LINE, 2, 14},
    {VF_CLOSE, 0, 0},
    /* Inner top cutout */
    {VF_MOVE, 2, 12}, {VF_LINE, 2, 11}, {VF_LINE, 3, 10},
    {VF_LINE, 6, 10}, {VF_LINE, 7, 11}, {VF_LINE, 7, 12},
    {VF_CLOSE, 0, 0},
    {VF_END, 0, 0}
};

static const VfCmd vf_glyph_i[] = {
    /* Dot */
    {VF_MOVE, 0, 3}, {VF_LINE, 2, 3}, {VF_LINE, 2, 6},
    {VF_LINE, 0, 6}, {VF_CLOSE, 0, 0},
    /* Stem */
    {VF_MOVE, 0, 8}, {VF_LINE, 2, 8}, {VF_LINE, 2, 24},
    {VF_LINE, 0, 24}, {VF_CLOSE, 0, 0},
    {VF_END, 0, 0}
};

static const VfCmd vf_glyph_l[] = {
    {VF_MOVE, 0, 0}, {VF_LINE, 2, 0}, {VF_LINE, 2, 24},
    {VF_LINE, 0, 24}, {VF_CLOSE, 0, 0},
    {VF_END, 0, 0}
};

static const VfCmd vf_glyph_n[] = {
    {VF_MOVE, 0, 8}, {VF_LINE, 2, 8}, {VF_LINE, 2, 10},
    {VF_LINE, 4, 8}, {VF_LINE, 7, 8}, {VF_LINE, 9, 11},
    {VF_LINE, 9, 24}, {VF_LINE, 7, 24}, {VF_LINE, 7, 12},
    {VF_LINE, 5, 10}, {VF_LINE, 2, 12}, {VF_LINE, 2, 24},
    {VF_LINE, 0, 24}, {VF_CLOSE, 0, 0},
    {VF_END, 0, 0}
};

static const VfCmd vf_glyph_o[] = {
    {VF_MOVE, 2, 8}, {VF_LINE, 7, 8}, {VF_LINE, 9, 11},
    {VF_LINE, 9, 21}, {VF_LINE, 7, 24}, {VF_LINE, 2, 24},
    {VF_LINE, 0, 21}, {VF_LINE, 0, 11}, {VF_CLOSE, 0, 0},
    /* Inner cutout */
    {VF_MOVE, 2, 11}, {VF_LINE, 3, 10}, {VF_LINE, 6, 10},
    {VF_LINE, 7, 11}, {VF_LINE, 7, 21}, {VF_LINE, 6, 22},
    {VF_LINE, 3, 22}, {VF_LINE, 2, 21}, {VF_CLOSE, 0, 0},
    {VF_END, 0, 0}
};

static const VfCmd vf_glyph_r[] = {
    {VF_MOVE, 0, 8}, {VF_LINE, 2, 8}, {VF_LINE, 2, 11},
    {VF_LINE, 4, 8}, {VF_LINE, 6, 8}, {VF_LINE, 6, 10},
    {VF_LINE, 3, 12}, {VF_LINE, 2, 12}, {VF_LINE, 2, 24},
    {VF_LINE, 0, 24}, {VF_CLOSE, 0, 0},
    {VF_END, 0, 0}
};

static const VfCmd vf_glyph_t[] = {
    {VF_MOVE, 1, 4}, {VF_LINE, 3, 4}, {VF_LINE, 3, 8},
    {VF_LINE, 5, 8}, {VF_LINE, 5, 10}, {VF_LINE, 3, 10},
    {VF_LINE, 3, 21}, {VF_LINE, 5, 24}, {VF_LINE, 6, 24},
    {VF_LINE, 6, 22}, {VF_LINE, 5, 22}, {VF_LINE, 3, 21},
    {VF_LINE, 1, 10}, {VF_LINE, 0, 10}, {VF_LINE, 0, 8},
    {VF_LINE, 1, 8}, {VF_CLOSE, 0, 0},
    {VF_END, 0, 0}
};

static const VfCmd vf_glyph_a[] = {
    {VF_MOVE,7,8},{VF_LINE,9,8},{VF_LINE,9,24},{VF_LINE,7,24},{VF_LINE,7,22},
    {VF_LINE,5,24},{VF_LINE,2,24},{VF_LINE,0,22},{VF_LINE,0,18},{VF_LINE,2,16},
    {VF_LINE,7,16},{VF_LINE,7,14},{VF_LINE,5,14},{VF_LINE,2,14},{VF_LINE,0,12},
    {VF_LINE,0,10},{VF_LINE,2,8},{VF_LINE,5,8},{VF_LINE,7,10},{VF_CLOSE,0,0},
    {VF_MOVE,2,18},{VF_LINE,2,21},{VF_LINE,3,22},{VF_LINE,6,22},{VF_LINE,7,21},
    {VF_LINE,7,18},{VF_LINE,6,17},{VF_LINE,3,17},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_b[] = {
    {VF_MOVE,0,0},{VF_LINE,2,0},{VF_LINE,2,10},{VF_LINE,4,8},{VF_LINE,7,8},
    {VF_LINE,9,10},{VF_LINE,9,22},{VF_LINE,7,24},{VF_LINE,4,24},{VF_LINE,2,22},
    {VF_LINE,2,24},{VF_LINE,0,24},{VF_CLOSE,0,0},
    {VF_MOVE,2,12},{VF_LINE,3,10},{VF_LINE,6,10},{VF_LINE,7,12},{VF_LINE,7,21},
    {VF_LINE,6,22},{VF_LINE,3,22},{VF_LINE,2,21},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_c[] = {
    {VF_MOVE,8,10},{VF_LINE,8,8},{VF_LINE,2,8},{VF_LINE,0,10},{VF_LINE,0,22},
    {VF_LINE,2,24},{VF_LINE,8,24},{VF_LINE,8,22},{VF_LINE,3,22},{VF_LINE,2,21},
    {VF_LINE,2,11},{VF_LINE,3,10},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_f[] = {
    {VF_MOVE,3,0},{VF_LINE,6,0},{VF_LINE,6,2},{VF_LINE,4,2},{VF_LINE,3,3},
    {VF_LINE,3,8},{VF_LINE,6,8},{VF_LINE,6,10},{VF_LINE,3,10},{VF_LINE,3,24},
    {VF_LINE,1,24},{VF_LINE,1,10},{VF_LINE,0,10},{VF_LINE,0,8},{VF_LINE,1,8},
    {VF_LINE,1,3},{VF_LINE,2,1},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_g[] = {
    {VF_MOVE,7,8},{VF_LINE,9,8},{VF_LINE,9,28},{VF_LINE,7,30},{VF_LINE,2,30},
    {VF_LINE,0,28},{VF_LINE,0,26},{VF_LINE,2,26},{VF_LINE,2,28},{VF_LINE,3,29},
    {VF_LINE,7,29},{VF_LINE,7,24},{VF_LINE,5,24},{VF_LINE,2,24},{VF_LINE,0,22},
    {VF_LINE,0,10},{VF_LINE,2,8},{VF_LINE,5,8},{VF_LINE,7,10},{VF_CLOSE,0,0},
    {VF_MOVE,2,11},{VF_LINE,3,10},{VF_LINE,6,10},{VF_LINE,7,11},{VF_LINE,7,21},
    {VF_LINE,6,22},{VF_LINE,3,22},{VF_LINE,2,21},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_h[] = {
    {VF_MOVE,0,0},{VF_LINE,2,0},{VF_LINE,2,10},{VF_LINE,4,8},{VF_LINE,7,8},
    {VF_LINE,9,10},{VF_LINE,9,24},{VF_LINE,7,24},{VF_LINE,7,12},{VF_LINE,5,10},
    {VF_LINE,2,12},{VF_LINE,2,24},{VF_LINE,0,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_j[] = {
    {VF_MOVE,3,3},{VF_LINE,5,3},{VF_LINE,5,6},{VF_LINE,3,6},{VF_CLOSE,0,0},
    {VF_MOVE,3,8},{VF_LINE,5,8},{VF_LINE,5,28},{VF_LINE,3,30},{VF_LINE,0,30},
    {VF_LINE,0,28},{VF_LINE,2,28},{VF_LINE,3,27},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_k[] = {
    {VF_MOVE,0,0},{VF_LINE,2,0},{VF_LINE,2,14},{VF_LINE,6,8},{VF_LINE,8,8},
    {VF_LINE,4,15},{VF_LINE,8,24},{VF_LINE,6,24},{VF_LINE,2,17},{VF_LINE,2,24},
    {VF_LINE,0,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_m[] = {
    {VF_MOVE,0,8},{VF_LINE,2,8},{VF_LINE,2,10},{VF_LINE,4,8},{VF_LINE,6,8},
    {VF_LINE,7,10},{VF_LINE,9,8},{VF_LINE,11,8},{VF_LINE,13,10},{VF_LINE,13,24},
    {VF_LINE,11,24},{VF_LINE,11,12},{VF_LINE,9,10},{VF_LINE,8,10},{VF_LINE,7,12},
    {VF_LINE,7,24},{VF_LINE,5,24},{VF_LINE,5,12},{VF_LINE,3,10},{VF_LINE,2,12},
    {VF_LINE,2,24},{VF_LINE,0,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_p[] = {
    {VF_MOVE,0,8},{VF_LINE,2,8},{VF_LINE,2,10},{VF_LINE,4,8},{VF_LINE,7,8},
    {VF_LINE,9,10},{VF_LINE,9,22},{VF_LINE,7,24},{VF_LINE,4,24},{VF_LINE,2,22},
    {VF_LINE,2,30},{VF_LINE,0,30},{VF_CLOSE,0,0},
    {VF_MOVE,2,12},{VF_LINE,3,10},{VF_LINE,6,10},{VF_LINE,7,12},{VF_LINE,7,21},
    {VF_LINE,6,22},{VF_LINE,3,22},{VF_LINE,2,21},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_q[] = {
    {VF_MOVE,7,8},{VF_LINE,9,8},{VF_LINE,9,30},{VF_LINE,7,30},{VF_LINE,7,22},
    {VF_LINE,5,24},{VF_LINE,2,24},{VF_LINE,0,22},{VF_LINE,0,10},{VF_LINE,2,8},
    {VF_LINE,5,8},{VF_LINE,7,10},{VF_CLOSE,0,0},
    {VF_MOVE,2,11},{VF_LINE,3,10},{VF_LINE,6,10},{VF_LINE,7,11},{VF_LINE,7,21},
    {VF_LINE,6,22},{VF_LINE,3,22},{VF_LINE,2,21},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_s[] = {
    {VF_MOVE,7,10},{VF_LINE,3,10},{VF_LINE,2,11},{VF_LINE,2,14},{VF_LINE,3,15},
    {VF_LINE,6,15},{VF_LINE,8,17},{VF_LINE,8,22},{VF_LINE,6,24},{VF_LINE,1,24},
    {VF_LINE,1,22},{VF_LINE,6,22},{VF_LINE,6,18},{VF_LINE,5,17},{VF_LINE,2,17},
    {VF_LINE,0,15},{VF_LINE,0,10},{VF_LINE,2,8},{VF_LINE,7,8},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_u[] = {
    {VF_MOVE,0,8},{VF_LINE,2,8},{VF_LINE,2,21},{VF_LINE,3,22},{VF_LINE,6,22},
    {VF_LINE,7,21},{VF_LINE,7,8},{VF_LINE,9,8},{VF_LINE,9,24},{VF_LINE,7,24},
    {VF_LINE,7,22},{VF_LINE,5,24},{VF_LINE,4,24},{VF_LINE,2,22},{VF_LINE,0,24},
    {VF_LINE,0,22},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_v[] = {
    {VF_MOVE,0,8},{VF_LINE,2,8},{VF_LINE,4,20},{VF_LINE,6,8},{VF_LINE,8,8},
    {VF_LINE,5,24},{VF_LINE,3,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_w[] = {
    {VF_MOVE,0,8},{VF_LINE,2,8},{VF_LINE,3,18},{VF_LINE,5,10},{VF_LINE,7,10},
    {VF_LINE,9,18},{VF_LINE,10,8},{VF_LINE,12,8},{VF_LINE,10,24},{VF_LINE,8,24},
    {VF_LINE,6,16},{VF_LINE,4,24},{VF_LINE,2,24},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_x[] = {
    {VF_MOVE,0,8},{VF_LINE,2,8},{VF_LINE,4,14},{VF_LINE,6,8},{VF_LINE,8,8},
    {VF_LINE,5,16},{VF_LINE,8,24},{VF_LINE,6,24},{VF_LINE,4,18},{VF_LINE,2,24},
    {VF_LINE,0,24},{VF_LINE,3,16},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_y[] = {
    {VF_MOVE,0,8},{VF_LINE,2,8},{VF_LINE,5,18},{VF_LINE,7,8},{VF_LINE,9,8},
    {VF_LINE,5,24},{VF_LINE,3,30},{VF_LINE,1,30},{VF_LINE,4,22},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_z[] = {
    {VF_MOVE,0,8},{VF_LINE,8,8},{VF_LINE,8,10},{VF_LINE,2,22},{VF_LINE,8,22},
    {VF_LINE,8,24},{VF_LINE,0,24},{VF_LINE,0,22},{VF_LINE,6,10},{VF_LINE,0,10},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

/* ── Digit glyphs 0-9 ───────────────────────────────────────────────── */

static const VfCmd vf_glyph_0[] = {
    {VF_MOVE,2,0},{VF_LINE,7,0},{VF_LINE,9,2},{VF_LINE,9,22},{VF_LINE,7,24},
    {VF_LINE,2,24},{VF_LINE,0,22},{VF_LINE,0,2},{VF_CLOSE,0,0},
    {VF_MOVE,3,3},{VF_LINE,6,3},{VF_LINE,7,4},{VF_LINE,7,20},{VF_LINE,6,21},
    {VF_LINE,3,21},{VF_LINE,2,20},{VF_LINE,2,4},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_1[] = {
    {VF_MOVE,2,4},{VF_LINE,4,0},{VF_LINE,6,0},{VF_LINE,6,21},{VF_LINE,8,21},
    {VF_LINE,8,24},{VF_LINE,1,24},{VF_LINE,1,21},{VF_LINE,4,21},{VF_LINE,4,5},
    {VF_LINE,2,7},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_2[] = {
    {VF_MOVE,0,2},{VF_LINE,2,0},{VF_LINE,7,0},{VF_LINE,9,2},{VF_LINE,9,10},
    {VF_LINE,3,21},{VF_LINE,9,21},{VF_LINE,9,24},{VF_LINE,0,24},{VF_LINE,0,21},
    {VF_LINE,7,8},{VF_LINE,7,4},{VF_LINE,6,3},{VF_LINE,3,3},{VF_LINE,2,4},
    {VF_LINE,2,6},{VF_LINE,0,6},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_3[] = {
    {VF_MOVE,0,2},{VF_LINE,2,0},{VF_LINE,7,0},{VF_LINE,9,2},{VF_LINE,9,10},
    {VF_LINE,7,12},{VF_LINE,9,14},{VF_LINE,9,22},{VF_LINE,7,24},{VF_LINE,2,24},
    {VF_LINE,0,22},{VF_LINE,0,20},{VF_LINE,2,20},{VF_LINE,2,21},{VF_LINE,6,21},
    {VF_LINE,7,20},{VF_LINE,7,15},{VF_LINE,5,13},{VF_LINE,5,11},{VF_LINE,7,9},
    {VF_LINE,7,4},{VF_LINE,6,3},{VF_LINE,3,3},{VF_LINE,2,4},{VF_LINE,2,6},
    {VF_LINE,0,6},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_4[] = {
    {VF_MOVE,6,0},{VF_LINE,8,0},{VF_LINE,8,15},{VF_LINE,9,15},{VF_LINE,9,18},
    {VF_LINE,8,18},{VF_LINE,8,24},{VF_LINE,6,24},{VF_LINE,6,18},{VF_LINE,0,18},
    {VF_LINE,0,15},{VF_CLOSE,0,0},
    {VF_MOVE,6,5},{VF_LINE,2,15},{VF_LINE,6,15},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_5[] = {
    {VF_MOVE,0,0},{VF_LINE,9,0},{VF_LINE,9,3},{VF_LINE,2,3},{VF_LINE,2,10},
    {VF_LINE,7,10},{VF_LINE,9,12},{VF_LINE,9,22},{VF_LINE,7,24},{VF_LINE,2,24},
    {VF_LINE,0,22},{VF_LINE,0,20},{VF_LINE,2,20},{VF_LINE,2,21},{VF_LINE,6,21},
    {VF_LINE,7,20},{VF_LINE,7,13},{VF_LINE,6,12},{VF_LINE,0,12},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_6[] = {
    {VF_MOVE,7,0},{VF_LINE,9,0},{VF_LINE,9,3},{VF_LINE,3,3},{VF_LINE,2,4},
    {VF_LINE,2,10},{VF_LINE,4,8},{VF_LINE,7,8},{VF_LINE,9,10},{VF_LINE,9,22},
    {VF_LINE,7,24},{VF_LINE,2,24},{VF_LINE,0,22},{VF_LINE,0,2},{VF_LINE,2,0},
    {VF_CLOSE,0,0},
    {VF_MOVE,3,10},{VF_LINE,6,10},{VF_LINE,7,11},{VF_LINE,7,21},{VF_LINE,6,22},
    {VF_LINE,3,22},{VF_LINE,2,21},{VF_LINE,2,11},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_7[] = {
    {VF_MOVE,0,0},{VF_LINE,9,0},{VF_LINE,9,3},{VF_LINE,4,24},{VF_LINE,2,24},
    {VF_LINE,7,3},{VF_LINE,0,3},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_8[] = {
    {VF_MOVE,2,0},{VF_LINE,7,0},{VF_LINE,9,2},{VF_LINE,9,10},{VF_LINE,7,12},
    {VF_LINE,9,14},{VF_LINE,9,22},{VF_LINE,7,24},{VF_LINE,2,24},{VF_LINE,0,22},
    {VF_LINE,0,14},{VF_LINE,2,12},{VF_LINE,0,10},{VF_LINE,0,2},{VF_CLOSE,0,0},
    {VF_MOVE,3,3},{VF_LINE,6,3},{VF_LINE,7,4},{VF_LINE,7,9},{VF_LINE,6,10},
    {VF_LINE,3,10},{VF_LINE,2,9},{VF_LINE,2,4},{VF_CLOSE,0,0},
    {VF_MOVE,3,14},{VF_LINE,6,14},{VF_LINE,7,15},{VF_LINE,7,21},{VF_LINE,6,22},
    {VF_LINE,3,22},{VF_LINE,2,21},{VF_LINE,2,15},{VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_9[] = {
    {VF_MOVE,2,0},{VF_LINE,7,0},{VF_LINE,9,2},{VF_LINE,9,22},{VF_LINE,7,24},
    {VF_LINE,0,24},{VF_LINE,0,21},{VF_LINE,6,21},{VF_LINE,7,20},{VF_LINE,7,14},
    {VF_LINE,5,16},{VF_LINE,2,16},{VF_LINE,0,14},{VF_LINE,0,2},{VF_CLOSE,0,0},
    {VF_MOVE,2,3},{VF_LINE,6,3},{VF_LINE,7,4},{VF_LINE,7,13},{VF_LINE,6,14},
    {VF_LINE,3,14},{VF_LINE,2,13},{VF_LINE,2,4},{VF_CLOSE,0,0},{VF_END,0,0}
};

/* ── Punctuation glyphs ─────────────────────────────────────────────── */

static const VfCmd vf_glyph_colon[] = {
    {VF_MOVE,0,6},{VF_LINE,2,6},{VF_LINE,2,9},{VF_LINE,0,9},{VF_CLOSE,0,0},
    {VF_MOVE,0,17},{VF_LINE,2,17},{VF_LINE,2,20},{VF_LINE,0,20},{VF_CLOSE,0,0},
    {VF_END,0,0}
};

static const VfCmd vf_glyph_period[] = {
    {VF_MOVE,0,21},{VF_LINE,2,21},{VF_LINE,2,24},{VF_LINE,0,24},
    {VF_CLOSE,0,0},{VF_END,0,0}
};

static const VfCmd vf_glyph_space[] = {
    {VF_END,0,0}
};

/* Lookup table — maps ASCII 32-126 to glyph data. NULL = use bitmap fallback. */
static const VfCmd* vf_glyphs[95] = {
    vf_glyph_space, /* space (32) */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* ! to - (33-45) */
    vf_glyph_period, /* . (46) */
    NULL, /* / (47) */
    vf_glyph_0, vf_glyph_1, vf_glyph_2, vf_glyph_3, vf_glyph_4, /* 0-4 (48-52) */
    vf_glyph_5, vf_glyph_6, vf_glyph_7, vf_glyph_8, vf_glyph_9, /* 5-9 (53-57) */
    vf_glyph_colon, /* : (58) */
    NULL, NULL, NULL, NULL, NULL, NULL, /* ; to @ (59-64) */
    vf_glyph_A, vf_glyph_B, vf_glyph_C, vf_glyph_D, vf_glyph_E, vf_glyph_F,
    vf_glyph_G, vf_glyph_H, vf_glyph_I, vf_glyph_J, vf_glyph_K, vf_glyph_L,
    vf_glyph_M, vf_glyph_N, vf_glyph_O, vf_glyph_P, vf_glyph_Q, vf_glyph_R,
    vf_glyph_S, vf_glyph_T, vf_glyph_U, vf_glyph_V, vf_glyph_W, vf_glyph_X,
    vf_glyph_Y, vf_glyph_Z, /* A-Z (65-90) */
    NULL, NULL, NULL, NULL, NULL, NULL, /* [ to ` (91-96) */
    vf_glyph_a, vf_glyph_b, vf_glyph_c, vf_glyph_d, vf_glyph_e, vf_glyph_f,
    vf_glyph_g, vf_glyph_h, vf_glyph_i, vf_glyph_j, vf_glyph_k, vf_glyph_l,
    vf_glyph_m, vf_glyph_n, vf_glyph_o, vf_glyph_p, vf_glyph_q, vf_glyph_r,
    vf_glyph_s, vf_glyph_t, vf_glyph_u, vf_glyph_v, vf_glyph_w, vf_glyph_x,
    vf_glyph_y, vf_glyph_z, /* a-z (97-122) */
    NULL, NULL, NULL, NULL /* { to ~ (123-126) */
};

/* rt_gui_draw_text(pack(x,y), pack(ptr,len), pack(color,0), alpha)
 * Simple uses tagged RuntimeValue for strings. The caller must pass
 * the raw char pointer and byte length separately.
 * If ptr is a RuntimeValue string, the Simple side extracts data+len. */
RuntimeValue rt_gui_draw_text(RuntimeValue xy, RuntimeValue ptr_len,
                               RuntimeValue color_rv, RuntimeValue alpha_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint64_t ptr_val = (uint64_t)ptr_len >> 32;
    uint32_t len = (uint32_t)((uint64_t)ptr_len & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_rv >> 32);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;

    /* ptr_val is the high 32 bits — won't work for 64-bit pointers.
     * Instead, treat ptr_len as: high 32 = char count, low 32 = unused.
     * The actual string data comes through a different mechanism.
     *
     * For baremetal simplicity: use a static text buffer approach.
     * The Simple side calls rt_gui_set_text() first, then rt_gui_draw_text(). */

    if (x >= g_fb_w || y >= SCREEN_H) return 0;

    const char *text = (const char *)(uintptr_t)ptr_val;
    if (!text || len == 0) return 0;
    if (len > 256) len = 256;

    uint32_t cx = x;
    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = (uint8_t)text[i];
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;

        /* Draw character glyph (8x16 bitmap) */
        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            uint32_t py = y + row;
            if (py >= SCREEN_H) break;
            for (uint32_t col = 0; col < 8; col++) {
                if (bits & (0x80 >> col)) {
                    uint32_t px = cx + col;
                    if (px >= g_fb_w) break;
                    if (alpha == 255) {
                        fb_write(px, py, 0xFF000000u | color);
                    } else {
                        uint32_t dst = fb_read(px, py);
                        fb_write(px, py, alpha_blend(dst, color, alpha));
                    }
                }
            }
        }
        cx += 8; /* Advance to next character */
        if (cx >= g_fb_w) break;
    }

    dirty_mark(x, y, cx - x, 16);
    return 0;
}

/* Static text buffer for inter-function text passing */
static char g_text_buf[256];
static uint32_t g_text_len = 0;

/* rt_gui_set_text_buf(char_values_packed, _, _, _)
 * Store up to 8 characters per call from packed u64.
 * Call multiple times then rt_gui_draw_text_buf to render.
 * char_values = 8 bytes packed into u64 (byte 0 in bits 56-63, etc.) */
RuntimeValue rt_gui_set_text_buf(RuntimeValue chars, RuntimeValue offset_rv,
                                  RuntimeValue unused1, RuntimeValue unused2)
{
    (void)unused1; (void)unused2;
    uint32_t offset = (uint32_t)(uint64_t)offset_rv;
    uint64_t c = (uint64_t)chars;

    for (int i = 0; i < 8 && offset + i < 256; i++) {
        uint8_t ch = (uint8_t)(c >> (56 - i * 8));
        if (ch == 0) break;
        g_text_buf[offset + i] = (char)ch;
        if (offset + i + 1 > g_text_len) g_text_len = offset + i + 1;
    }
    return 0;
}

/* rt_gui_draw_text_buf(pack(x,y), pack(color,len), alpha, _)
 * Renders text from the static buffer at given position.
 * Now UTF-8 aware: multi-byte sequences are decoded, non-ASCII chars
 * use bitmap fallback with '?' substitution for the 8x16 bitmap font. */
RuntimeValue rt_gui_draw_text_buf(RuntimeValue xy, RuntimeValue color_len,
                                   RuntimeValue alpha_rv, RuntimeValue unused)
{
    (void)unused;
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_len >> 32);
    uint32_t len = (uint32_t)((uint64_t)color_len & 0xFFFFFFFF);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;

    if (len == 0) len = g_text_len;
    if (len > 256) len = 256;
    if (x >= g_fb_w || y >= SCREEN_H) return 0;

    /* UTF-8 decode loop for bitmap rendering */
    uint32_t cx = x;
    uint32_t pos = 0;
    while (pos < len) {
        uint32_t codepoint;
        uint32_t consumed = utf8_decode((const uint8_t *)g_text_buf, len, pos, &codepoint);
        if (consumed == 0) break;
        pos += consumed;

        /* Map to bitmap font index (ASCII 32-126 only) */
        uint32_t ch = codepoint;
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;

        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            uint32_t py = y + row;
            if (py >= SCREEN_H) break;
            for (uint32_t col = 0; col < 8; col++) {
                if (bits & (0x80 >> col)) {
                    uint32_t px = cx + col;
                    if (px >= g_fb_w) break;
                    if (alpha == 255) {
                        fb_write(px, py, 0xFF000000u | color);
                    } else {
                        uint32_t dst = fb_read(px, py);
                        fb_write(px, py, alpha_blend(dst, color, alpha));
                    }
                }
            }
        }
        /* Wide characters get double advance */
        cx += cp_is_wide(codepoint) ? 16 : 8;
        if (cx >= g_fb_w) break;
    }

    dirty_mark(x, y, cx - x, 16);
    g_text_len = 0; /* Reset buffer */
    return 0;
}

/* ===================================================================
 * 21. Text with shadow (pseudo anti-aliased appearance)
 *     rt_gui_draw_text_shadow(pack(x,y), pack(color,len), alpha, shadow_alpha)
 *     Renders text twice: shadow at (x+1,y+1) then foreground at (x,y)
 * =================================================================== */
RuntimeValue rt_gui_draw_text_shadow(RuntimeValue xy, RuntimeValue color_len,
                                      RuntimeValue alpha_rv, RuntimeValue shadow_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_len >> 32);
    uint32_t len = (uint32_t)((uint64_t)color_len & 0xFFFFFFFF);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;
    uint8_t shadow_alpha = (uint8_t)(uint64_t)shadow_rv;

    if (len == 0) len = g_text_len;
    if (len > 256) len = 256;
    if (x >= g_fb_w || y >= SCREEN_H) return 0;

    /* Pass 0: Soft shadow at (x+1, y+1) with half alpha (pseudo-AA fringe) */
    uint32_t cx = x + 1;
    uint8_t soft_alpha = shadow_alpha * 2 / 3;
    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = (uint8_t)g_text_buf[i];
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;
        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            uint32_t py = y + 1 + row;
            if (py >= SCREEN_H) break;
            for (uint32_t col = 0; col < 8; col++) {
                if (bits & (0x80 >> col)) {
                    uint32_t px = cx + col;
                    if (px < g_fb_w) {
                        uint32_t dst = fb_read(px, py);
                        fb_write(px, py, alpha_blend(dst, 0x00000000, soft_alpha));
                    }
                }
            }
        }
        cx += 8;
    }

    /* Pass 1: Main shadow at (x+2, y+2) in dark color (larger offset) */
    cx = x + 2;
    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = (uint8_t)g_text_buf[i];
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;
        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            uint32_t py = y + 2 + row;
            if (py >= SCREEN_H) break;
            for (uint32_t col = 0; col < 8; col++) {
                if (bits & (0x80 >> col)) {
                    uint32_t px = cx + col;
                    if (px < g_fb_w) {
                        uint32_t dst = fb_read(px, py);
                        fb_write(px, py, alpha_blend(dst, 0x00000000, shadow_alpha));
                    }
                }
            }
        }
        cx += 8;
    }

    /* Pass 2: Foreground at (x, y) */
    cx = x;
    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = (uint8_t)g_text_buf[i];
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;
        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            uint32_t py = y + row;
            if (py >= SCREEN_H) break;
            for (uint32_t col = 0; col < 8; col++) {
                if (bits & (0x80 >> col)) {
                    uint32_t px = cx + col;
                    if (px < g_fb_w) {
                        if (alpha == 255) {
                            fb_write(px, py, 0xFF000000u | color);
                        } else {
                            uint32_t dst = fb_read(px, py);
                            fb_write(px, py, alpha_blend(dst, color, alpha));
                        }
                    }
                }
            }
        }
        cx += 8;
    }

    dirty_mark(x, y, cx - x + 1, 18);
    g_text_len = 0;
    return 0;
}

/* ===================================================================
 * 22. Bold text (pseudo-bold via double render at x and x+1)
 *     rt_gui_draw_text_bold(pack(x,y), pack(color,len), alpha, _)
 *     Renders text twice: once at (x,y), once at (x+1,y) for thickness.
 *     Does NOT reset g_text_buf so caller can chain with shadow.
 * =================================================================== */
RuntimeValue rt_gui_draw_text_bold(RuntimeValue xy, RuntimeValue color_len,
                                    RuntimeValue alpha_rv, RuntimeValue unused)
{
    (void)unused;
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_len >> 32);
    uint32_t len = (uint32_t)((uint64_t)color_len & 0xFFFFFFFF);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;

    if (len == 0) len = g_text_len;
    if (len > 256) len = 256;
    if (x >= g_fb_w || y >= SCREEN_H) return 0;

    /* Draw at (x, y) — first pass */
    uint32_t cx = x;
    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = (uint8_t)g_text_buf[i];
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;
        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            uint32_t py = y + row;
            if (py >= SCREEN_H) break;
            for (uint32_t col = 0; col < 8; col++) {
                if (bits & (0x80 >> col)) {
                    uint32_t px = cx + col;
                    if (px >= g_fb_w) break;
                    if (alpha == 255) {
                        fb_write(px, py, 0xFF000000u | color);
                    } else {
                        uint32_t dst = fb_read(px, py);
                        fb_write(px, py, alpha_blend(dst, color, alpha));
                    }
                }
            }
        }
        cx += 8;
        if (cx >= g_fb_w) break;
    }

    /* Draw at (x+1, y) — bold effect (thicker strokes) */
    cx = x + 1;
    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = (uint8_t)g_text_buf[i];
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;
        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            uint32_t py = y + row;
            if (py >= SCREEN_H) break;
            for (uint32_t col = 0; col < 8; col++) {
                if (bits & (0x80 >> col)) {
                    uint32_t px = cx + col;
                    if (px >= g_fb_w) break;
                    if (alpha == 255) {
                        fb_write(px, py, 0xFF000000u | color);
                    } else {
                        uint32_t dst = fb_read(px, py);
                        fb_write(px, py, alpha_blend(dst, color, alpha));
                    }
                }
            }
        }
        cx += 8;
        if (cx >= g_fb_w) break;
    }

    dirty_mark(x, y, cx - x + 1, 16);
    g_text_len = 0;
    return 0;
}

/* ===================================================================
 * 25. Outlined text — renders glyphs with 1px AA outline
 *     rt_gui_draw_text_outline(pack(x,y), pack(color,len), alpha, outline_alpha)
 *     For each ON pixel: draws normally.
 *     For each OFF pixel adjacent to ON: draws at outline_alpha.
 *     Creates smooth anti-aliased appearance for bitmap fonts.
 * =================================================================== */
RuntimeValue rt_gui_draw_text_outline(RuntimeValue xy, RuntimeValue color_len,
                                       RuntimeValue alpha_rv, RuntimeValue outline_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_len >> 32);
    uint32_t len = (uint32_t)((uint64_t)color_len & 0xFFFFFFFF);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;
    uint8_t outline_alpha = (uint8_t)(uint64_t)outline_rv;

    if (len == 0) len = g_text_len;
    if (len > 256) len = 256;
    if (x >= g_fb_w || y >= SCREEN_H) return 0;

    /* Pass 1: Outline — draw fringe pixels at reduced alpha */
    uint32_t cx = x;
    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = (uint8_t)g_text_buf[i];
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;
        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            uint8_t bits_above = (row > 0) ? font_8x16[idx][row-1] : 0;
            uint8_t bits_below = (row < 15) ? font_8x16[idx][row+1] : 0;
            uint32_t py = y + row;
            if (py >= SCREEN_H) break;
            for (uint32_t col = 0; col < 8; col++) {
                uint8_t mask = 0x80 >> col;
                if (!(bits & mask)) {
                    /* OFF pixel — check 4 neighbors */
                    int neighbor = 0;
                    if (col > 0 && (bits & (mask << 1))) neighbor = 1;
                    if (col < 7 && (bits & (mask >> 1))) neighbor = 1;
                    if (bits_above & mask) neighbor = 1;
                    if (bits_below & mask) neighbor = 1;
                    if (neighbor) {
                        uint32_t px = cx + col;
                        if (px < g_fb_w) {
                            uint32_t dst = fb_read(px, py);
                            fb_write(px, py, alpha_blend(dst, color, outline_alpha));
                        }
                    }
                }
            }
        }
        cx += 8;
        if (cx >= g_fb_w) break;
    }

    /* Pass 2: Foreground — draw ON pixels at full alpha */
    cx = x;
    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = (uint8_t)g_text_buf[i];
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;
        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            uint32_t py = y + row;
            if (py >= SCREEN_H) break;
            for (uint32_t col = 0; col < 8; col++) {
                if (bits & (0x80 >> col)) {
                    uint32_t px = cx + col;
                    if (px >= g_fb_w) break;
                    if (alpha == 255) {
                        fb_write(px, py, 0xFF000000u | color);
                    } else {
                        uint32_t dst = fb_read(px, py);
                        fb_write(px, py, alpha_blend(dst, color, alpha));
                    }
                }
            }
        }
        cx += 8;
        if (cx >= g_fb_w) break;
    }

    dirty_mark(x, y, cx - x, 16);
    g_text_len = 0;
    return 0;
}

/* ===================================================================
 * 23. Scaled text (2x) — renders each font pixel as a 2x2 block
 *     rt_gui_draw_text_2x(pack(x,y), pack(color,len), alpha, shadow_alpha)
 *     Output: 16x32 per character (2x the 8x16 base font)
 *     With drop shadow for readability.
 * =================================================================== */
RuntimeValue rt_gui_draw_text_2x(RuntimeValue xy, RuntimeValue color_len,
                                  RuntimeValue alpha_rv, RuntimeValue shadow_rv)
{
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_len >> 32);
    uint32_t len = (uint32_t)((uint64_t)color_len & 0xFFFFFFFF);
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;
    uint8_t shadow_alpha = (uint8_t)(uint64_t)shadow_rv;

    if (len == 0) len = g_text_len;
    if (len > 256) len = 256;
    if (x >= g_fb_w || y >= SCREEN_H) return 0;

    /* Shadow pass at (x+1, y+1) — tighter shadow */
    if (shadow_alpha > 0) {
        uint32_t cx = x + 1;
        for (uint32_t i = 0; i < len; i++) {
            uint8_t ch = (uint8_t)g_text_buf[i];
            if (ch < 32 || ch > 126) ch = '?';
            uint32_t idx = ch - 32;
            for (uint32_t row = 0; row < 16; row++) {
                uint8_t bits = font_8x16[idx][row];
                for (uint32_t col = 0; col < 8; col++) {
                    if (bits & (0x80 >> col)) {
                        /* 2x2 block for shadow */
                        for (int dy = 0; dy < 2; dy++) {
                            uint32_t py = y + 1 + row * 2 + dy;
                            if (py >= SCREEN_H) continue;
                            for (int dx = 0; dx < 2; dx++) {
                                uint32_t px = cx + col * 2 + dx;
                                if (px < g_fb_w) {
                                    uint32_t dst = fb_read(px, py);
                                    fb_write(px, py, alpha_blend(dst, 0x00000000, shadow_alpha));
                                }
                            }
                        }
                    }
                }
            }
            cx += 16; /* 8 * 2 = 16 pixels per char */
            if (cx >= g_fb_w) break;
        }
    }

    /* Foreground pass at (x, y) — 2x scaled */
    uint32_t cx = x;
    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = (uint8_t)g_text_buf[i];
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;
        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            for (uint32_t col = 0; col < 8; col++) {
                if (bits & (0x80 >> col)) {
                    /* 2x2 block */
                    for (int dy = 0; dy < 2; dy++) {
                        uint32_t py = y + row * 2 + dy;
                        if (py >= SCREEN_H) continue;
                        for (int dx = 0; dx < 2; dx++) {
                            uint32_t px = cx + col * 2 + dx;
                            if (px < g_fb_w) {
                                if (alpha == 255) {
                                    fb_write(px, py, 0xFF000000u | color);
                                } else {
                                    uint32_t dst = fb_read(px, py);
                                    fb_write(px, py, alpha_blend(dst, color, alpha));
                                }
                            }
                        }
                    }
                }
            }
        }
        cx += 16;
        if (cx >= g_fb_w) break;
    }

    /* Pass 3: Edge smoothing (sub-pixel AA approximation)
     * For each font pixel that is ON, check if horizontal/vertical neighbors
     * are OFF. If so, reduce edge pixels' alpha for smoother appearance. */
    cx = x;
    uint8_t edge_alpha = alpha * 2 / 3; /* 66% alpha for edge pixels */
    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = (uint8_t)g_text_buf[i];
        if (ch < 32 || ch > 126) ch = '?';
        uint32_t idx = ch - 32;
        for (uint32_t row = 0; row < 16; row++) {
            uint8_t bits = font_8x16[idx][row];
            uint8_t bits_above = (row > 0) ? font_8x16[idx][row-1] : 0;
            uint8_t bits_below = (row < 15) ? font_8x16[idx][row+1] : 0;
            for (uint32_t col = 0; col < 8; col++) {
                uint8_t mask = 0x80 >> col;
                if (!(bits & mask)) {
                    /* This pixel is OFF — check if any neighbor is ON */
                    int has_neighbor = 0;
                    if (col > 0 && (bits & (mask << 1))) has_neighbor = 1;
                    if (col < 7 && (bits & (mask >> 1))) has_neighbor = 1;
                    if (bits_above & mask) has_neighbor = 1;
                    if (bits_below & mask) has_neighbor = 1;

                    if (has_neighbor) {
                        /* Draw fringe pixel at reduced alpha */
                        uint32_t py = y + row * 2;
                        uint32_t px_base = cx + col * 2;
                        for (int dy = 0; dy < 2; dy++) {
                            if (py + dy >= SCREEN_H) continue;
                            for (int dx = 0; dx < 2; dx++) {
                                uint32_t px = px_base + dx;
                                if (px < g_fb_w) {
                                    uint32_t dst = fb_read(px, py + dy);
                                    fb_write(px, py + dy, alpha_blend(dst, color, edge_alpha / 4));
                                }
                            }
                        }
                    }
                }
            }
        }
        cx += 16;
        if (cx >= g_fb_w) break;
    }

    dirty_mark(x, y, cx - x + 2, 34);
    g_text_len = 0;
    return 0;
}

/* ===================================================================
 * 26. Vector font rasterizer — CPU-based AA text from glyph outlines
 *
 *   Uses ray-casting (even-odd rule) with 2x2 sub-pixel sampling for
 *   anti-aliasing.  Each glyph is defined in a 16x24 design grid and
 *   scaled to the requested pixel height with integer arithmetic.
 *
 *   rt_gui_draw_text_vector(pack(x,y), pack(color, (height<<16)|len),
 *                           alpha, _)
 * =================================================================== */

/* Check if point (px, py) is inside the glyph outline using ray casting.
 * Coordinates are in scaled space: design_coord * scale_num / scale_den. */
static int vf_point_inside(const VfCmd *cmds, int px, int py,
                            int scale_num, int scale_den)
{
    int crossings = 0;
    int path_start_x = 0, path_start_y = 0;
    int cur_x = 0, cur_y = 0;

    for (int i = 0; cmds[i].type != VF_END; i++) {
        int sx = cmds[i].x * scale_num / scale_den;
        int sy = cmds[i].y * scale_num / scale_den;

        if (cmds[i].type == VF_MOVE) {
            cur_x = sx; cur_y = sy;
            path_start_x = sx; path_start_y = sy;
        } else if (cmds[i].type == VF_LINE || cmds[i].type == VF_CLOSE) {
            int x1, y1, x2, y2;
            if (cmds[i].type == VF_CLOSE) {
                x1 = cur_x; y1 = cur_y;
                x2 = path_start_x; y2 = path_start_y;
                cur_x = path_start_x; cur_y = path_start_y;
            } else {
                x1 = cur_x; y1 = cur_y;
                x2 = sx; y2 = sy;
                cur_x = sx; cur_y = sy;
            }

            /* Ray casting: horizontal ray from (px, py) to the right */
            if ((y1 <= py && y2 > py) || (y2 <= py && y1 > py)) {
                /* Edge crosses this scanline -- compute X intersection */
                int ix = x1 + (py - y1) * (x2 - x1) / (y2 - y1);
                if (px < ix) crossings++;
            }
        }
    }
    return crossings & 1; /* Odd = inside */
}

/* Render a single vector glyph at framebuffer position (gx, gy). */
static void vf_render_glyph(const VfCmd *cmds, uint8_t glyph_width,
                              int gx, int gy, int pixel_height,
                              uint32_t color, uint8_t alpha)
{
    int pixel_width = glyph_width * pixel_height / 24;
    if (pixel_width < 1) pixel_width = 1;

    /* For each pixel in the glyph bounding box, do 2x2 sub-sampling */
    for (int row = 0; row < pixel_height; row++) {
        int py_base = gy + row;
        if (py_base < 0 || (uint32_t)py_base >= SCREEN_H) continue;

        for (int col = 0; col < pixel_width; col++) {
            int px_base = gx + col;
            if (px_base < 0 || (uint32_t)px_base >= g_fb_w) continue;

            /* 2x2 sub-sample grid for anti-aliasing */
            int coverage = 0;
            for (int sub_y = 0; sub_y < 2; sub_y++) {
                for (int sub_x = 0; sub_x < 2; sub_x++) {
                    /* Map sub-pixel to design space via doubled resolution.
                     * Sub-pixel center = (col*2 + sub_x, row*2 + sub_y)
                     * Design scale: pixel_height*2 / 48  (since design=24) */
                    int sp_x = col * 2 + sub_x;
                    int sp_y = row * 2 + sub_y;
                    if (vf_point_inside(cmds, sp_x, sp_y,
                                        pixel_height * 2, 48)) {
                        coverage++;
                    }
                }
            }

            if (coverage > 0) {
                uint8_t pixel_alpha = (uint8_t)((uint32_t)alpha * coverage / 4);
                uint32_t dst = fb_read((uint32_t)px_base, (uint32_t)py_base);
                fb_write((uint32_t)px_base, (uint32_t)py_base,
                         alpha_blend(dst, color, pixel_alpha));
            }
        }
    }
}

/* Render a single character (ASCII or Unicode) via vector/bitmap/placeholder.
 * Returns the horizontal advance in pixels. */
static uint32_t render_one_char(uint32_t codepoint, int gx, int gy,
                                 int pixel_height, uint32_t color, uint8_t alpha)
{
    /* ASCII range: use built-in vector/bitmap font */
    if (codepoint >= 32 && codepoint <= 126) {
        uint32_t idx = codepoint - 32;
        const VfCmd *glyph = vf_glyphs[idx];
        uint8_t gw = vf_widths[idx];

        if (glyph != NULL) {
            vf_render_glyph(glyph, gw, gx, gy, pixel_height, color, alpha);
            return (uint32_t)(gw * pixel_height / 24) + 1;
        } else {
            /* Bitmap fallback for ASCII chars without vector glyphs */
            uint32_t bmp_idx = codepoint - 32;
            uint32_t scale = (uint32_t)pixel_height / 16;
            if (scale < 1) scale = 1;
            for (uint32_t row = 0; row < 16; row++) {
                uint8_t bits = font_8x16[bmp_idx][row];
                for (uint32_t col = 0; col < 8; col++) {
                    if (bits & (0x80 >> col)) {
                        for (uint32_t sy = 0; sy < scale; sy++) {
                            uint32_t py = (uint32_t)gy + row * scale + sy;
                            if (py >= SCREEN_H) continue;
                            for (uint32_t sx = 0; sx < scale; sx++) {
                                uint32_t px = (uint32_t)gx + col * scale + sx;
                                if (px >= g_fb_w) continue;
                                if (alpha == 255) {
                                    fb_write(px, py, 0xFF000000u | color);
                                } else {
                                    uint32_t dst = fb_read(px, py);
                                    fb_write(px, py, alpha_blend(dst, color, alpha));
                                }
                            }
                        }
                    }
                }
            }
            return 8 * scale + 1;
        }
    }

    /* Non-ASCII Unicode: render a placeholder box with codepoint hex.
     * Future: stb_truetype rasterization from loaded TTF fonts.
     * Wide characters (CJK, Hangul) get double-width boxes. */
    int is_wide = cp_is_wide(codepoint);
    int box_w = is_wide ? pixel_height : (pixel_height * 2 / 3);
    int box_h = pixel_height;
    if (box_w < 4) box_w = 4;

    /* Draw dotted rectangle outline as placeholder */
    uint8_t box_alpha = alpha * 2 / 3;
    for (int bx = 0; bx < box_w; bx++) {
        for (int by = 0; by < box_h; by++) {
            int is_border = (bx == 0 || bx == box_w - 1 || by == 0 || by == box_h - 1);
            if (!is_border) continue;
            uint32_t px = (uint32_t)(gx + bx);
            uint32_t py = (uint32_t)(gy + by);
            if (px >= g_fb_w || py >= SCREEN_H) continue;
            uint32_t dst = fb_read(px, py);
            fb_write(px, py, alpha_blend(dst, color, box_alpha));
        }
    }

    /* Draw small hex digits inside the box (up to 4 hex digits for BMP) */
    if (box_h >= 8 && box_w >= 6) {
        /* Render codepoint as hex in tiny 3x5 digits */
        uint32_t cp_val = codepoint;
        int ndigits = (cp_val > 0xFFF) ? 4 : (cp_val > 0xFF) ? 3 : (cp_val > 0xF) ? 2 : 1;
        /* Simple 3x5 hex digit patterns */
        static const uint16_t hex_3x5[16] = {
            0x7B6F, /* 0: 111 101 110 101 111 */
            0x2C97, /* 1: 010 110 010 010 111 */
            0x73E7, /* 2: 111 001 111 100 111 */
            0x73CF, /* 3: 111 001 111 001 111 */
            0x5BC9, /* 4: 101 101 111 001 001 */
            0x7CF3, /* 5: 111 100 111 001 111 (corrected) */
            0x7EF7, /* 6: 111 100 111 101 111 (corrected) */
            0x7249, /* 7: 111 001 010 010 010 (corrected) */
            0x7BF7, /* 8: 111 101 111 101 111 (corrected) */
            0x7BC9, /* 9: 111 101 111 001 001 (corrected) */
            0x7BFD, /* A: 111 101 111 101 101 (corrected) */
            0x6BF6, /* B: 110 101 110 101 110 (corrected) */
            0x7CE7, /* C: 111 100 100 100 111 (corrected) */
            0x6B76, /* D: 110 101 101 101 110 (corrected) */
            0x7CF7, /* E: 111 100 111 100 111 (corrected) */
            0x7CF4, /* F: 111 100 111 100 100 (corrected) */
        };
        int dx_start = (box_w - ndigits * 4) / 2;
        if (dx_start < 1) dx_start = 1;
        int dy_start = (box_h - 5) / 2;
        if (dy_start < 1) dy_start = 1;

        for (int d = 0; d < ndigits; d++) {
            int shift = (ndigits - 1 - d) * 4;
            int nibble = (cp_val >> shift) & 0xF;
            uint16_t pattern = hex_3x5[nibble];
            for (int dr = 0; dr < 5; dr++) {
                for (int dc = 0; dc < 3; dc++) {
                    int bit_idx = (4 - dr) * 3 + (2 - dc);
                    if (pattern & (1 << bit_idx)) {
                        uint32_t px = (uint32_t)(gx + dx_start + d * 4 + dc);
                        uint32_t py = (uint32_t)(gy + dy_start + dr);
                        if (px < g_fb_w && py < SCREEN_H) {
                            uint32_t dst = fb_read(px, py);
                            fb_write(px, py, alpha_blend(dst, color, alpha));
                        }
                    }
                }
            }
        }
    }

    return (uint32_t)box_w + 1;
}

/* rt_gui_draw_text_vector(pack(x,y), pack(color, (height<<16)|len), alpha, _)
 * Renders text using vector font outlines with anti-aliasing.
 * Now supports full UTF-8: ASCII chars use built-in vector/bitmap fonts,
 * non-ASCII Unicode chars render as placeholder boxes with hex codepoints.
 * Falls back to bitmap font for ASCII characters without vector outlines. */
RuntimeValue rt_gui_draw_text_vector(RuntimeValue xy, RuntimeValue color_size,
                                      RuntimeValue alpha_rv, RuntimeValue unused)
{
    (void)unused;
    uint32_t x = (uint32_t)((uint64_t)xy >> 32);
    uint32_t y = (uint32_t)((uint64_t)xy & 0xFFFFFFFF);
    uint32_t color = (uint32_t)((uint64_t)color_size >> 32);
    uint32_t size_and_len = (uint32_t)((uint64_t)color_size & 0xFFFFFFFF);
    uint32_t pixel_height = (size_and_len >> 16) & 0xFFFF;
    uint32_t len = size_and_len & 0xFFFF;
    uint8_t alpha = (uint8_t)(uint64_t)alpha_rv;

    if (pixel_height == 0) pixel_height = 16;
    if (len == 0) len = g_text_len;
    if (len > 256) len = 256;
    if (x >= g_fb_w || y >= SCREEN_H) return 0;

    /* UTF-8 decode loop: walk bytes, decode codepoints, render each */
    uint32_t cx = x;
    uint32_t pos = 0;
    while (pos < len) {
        uint32_t codepoint;
        uint32_t consumed = utf8_decode((const uint8_t *)g_text_buf, len, pos, &codepoint);
        if (consumed == 0) break;
        pos += consumed;

        /* Skip control characters */
        if (codepoint < 32 && codepoint != '\t') continue;
        /* Tab: advance by 4 spaces worth */
        if (codepoint == '\t') {
            uint32_t space_w = 5 * (uint32_t)pixel_height / 24;
            cx += space_w * 4;
            if (cx >= g_fb_w) break;
            continue;
        }

        uint32_t advance = render_one_char(codepoint, (int)cx, (int)y,
                                            (int)pixel_height, color, alpha);
        cx += advance;
        if (cx >= g_fb_w) break;
    }

    dirty_mark(x, y, cx - x, pixel_height);
    g_text_len = 0;
    return 0;
}

/* ===================================================================
 * 20. Layout Engine — Flexbox-style HStack/VStack
 *
 * Flat array of layout nodes. Simple side builds the tree via
 * function calls, then runs compute_layout, then reads results.
 *
 * Usage from Simple:
 *   rt_layout_reset(0, 0, 0, 0)
 *   val root = rt_layout_add(LAYOUT_VSTACK, -1, 0, 0)  // parent=-1 = root
 *   val bar = rt_layout_add(LAYOUT_HSTACK, root, 0, 0)
 *   rt_layout_set_size(bar, pack(FIXED, 28), pack(FILL, 0))  // h=28 fixed, w=fill
 *   rt_layout_set_padding(bar, pack(8, 8), pack(4, 4))       // lr=8, tb=4
 *   rt_layout_compute(pack(1024, 768), 0, 0, 0)              // screen size
 *   val bar_x = rt_layout_get(bar, 0, 0, 0) >> 32            // x in high bits
 *   val bar_y = rt_layout_get(bar, 0, 0, 0) & 0xFFFFFFFF     // y in low bits
 *   val bar_w = rt_layout_get_size(bar, 0, 0, 0) >> 32       // w
 *   val bar_h = rt_layout_get_size(bar, 0, 0, 0) & 0xFFFFFFFF // h
 * =================================================================== */

#define MAX_LAYOUT_NODES 128

typedef enum {
    LT_NONE = 0,
    LT_HSTACK = 1,    /* Horizontal: children laid out left-to-right */
    LT_VSTACK = 2,    /* Vertical: children laid out top-to-bottom */
    LT_ZSTACK = 3     /* Overlay: children stacked on top of each other */
} LayoutType;

typedef enum {
    SM_FIXED = 0,       /* Fixed pixel size */
    SM_FILL = 1,        /* Fill remaining space (weight-based) */
    SM_AUTO = 2         /* Size to content */
} SizeMode;

typedef enum {
    ALIGN_START = 0,
    ALIGN_CENTER = 1,
    ALIGN_END = 2
} LayoutAlign;

typedef struct {
    int active;          /* 1 if node is in use */
    int parent;          /* Parent node index (-1 = root) */
    LayoutType type;

    /* Size specification */
    SizeMode w_mode, h_mode;
    uint32_t w_value, h_value; /* Fixed size or weight */

    /* Padding */
    uint32_t pad_left, pad_right, pad_top, pad_bottom;

    /* Spacing between children */
    uint32_t spacing;

    /* Alignment of children on cross axis */
    LayoutAlign h_align, v_align;

    /* Computed results */
    int32_t cx, cy;       /* Position (relative to screen) */
    uint32_t cw, ch;      /* Computed size */

    /* Children tracking */
    int children[32];     /* Child indices */
    int child_count;

    /* Intrinsic size (from content or children) */
    uint32_t intrinsic_w, intrinsic_h;
} LayoutNode;

static LayoutNode g_layout[MAX_LAYOUT_NODES];
static int g_layout_count = 0;

/* Reset layout tree */
RuntimeValue rt_layout_reset(RuntimeValue u1, RuntimeValue u2,
                              RuntimeValue u3, RuntimeValue u4)
{
    (void)u1; (void)u2; (void)u3; (void)u4;
    for (int i = 0; i < MAX_LAYOUT_NODES; i++) {
        g_layout[i].active = 0;
        g_layout[i].child_count = 0;
    }
    g_layout_count = 0;
    return 0;
}

/* Add a node: rt_layout_add(type, parent_id, _, _) -> node_id */
RuntimeValue rt_layout_add(RuntimeValue type_rv, RuntimeValue parent_rv,
                            RuntimeValue u1, RuntimeValue u2)
{
    (void)u1; (void)u2;
    if (g_layout_count >= MAX_LAYOUT_NODES) return (RuntimeValue)-1;

    int id = g_layout_count++;
    LayoutNode *n = &g_layout[id];
    n->active = 1;
    n->type = (LayoutType)(uint64_t)type_rv;
    n->parent = (int)(int64_t)parent_rv;
    n->w_mode = SM_AUTO;
    n->h_mode = SM_AUTO;
    n->w_value = 0;
    n->h_value = 0;
    n->pad_left = n->pad_right = n->pad_top = n->pad_bottom = 0;
    n->spacing = 0;
    n->h_align = ALIGN_START;
    n->v_align = ALIGN_START;
    n->cx = n->cy = 0;
    n->cw = n->ch = 0;
    n->child_count = 0;
    n->intrinsic_w = n->intrinsic_h = 0;

    /* Register as child of parent */
    if (n->parent >= 0 && n->parent < MAX_LAYOUT_NODES) {
        LayoutNode *p = &g_layout[n->parent];
        if (p->child_count < 32) {
            p->children[p->child_count++] = id;
        }
    }

    return (RuntimeValue)id;
}

/* Set size: rt_layout_set_size(id, pack(w_mode, w_value), pack(h_mode, h_value), _) */
RuntimeValue rt_layout_set_size(RuntimeValue id_rv, RuntimeValue w_spec,
                                 RuntimeValue h_spec, RuntimeValue u)
{
    (void)u;
    int id = (int)(int64_t)id_rv;
    if (id < 0 || id >= g_layout_count) return 0;
    LayoutNode *n = &g_layout[id];

    n->w_mode = (SizeMode)((uint64_t)w_spec >> 32);
    n->w_value = (uint32_t)((uint64_t)w_spec & 0xFFFFFFFF);
    n->h_mode = (SizeMode)((uint64_t)h_spec >> 32);
    n->h_value = (uint32_t)((uint64_t)h_spec & 0xFFFFFFFF);

    return 0;
}

/* Set padding: rt_layout_set_padding(id, pack(left, right), pack(top, bottom), _) */
RuntimeValue rt_layout_set_padding(RuntimeValue id_rv, RuntimeValue lr,
                                    RuntimeValue tb, RuntimeValue u)
{
    (void)u;
    int id = (int)(int64_t)id_rv;
    if (id < 0 || id >= g_layout_count) return 0;
    LayoutNode *n = &g_layout[id];

    n->pad_left = (uint32_t)((uint64_t)lr >> 32);
    n->pad_right = (uint32_t)((uint64_t)lr & 0xFFFFFFFF);
    n->pad_top = (uint32_t)((uint64_t)tb >> 32);
    n->pad_bottom = (uint32_t)((uint64_t)tb & 0xFFFFFFFF);

    return 0;
}

/* Set spacing + alignment: rt_layout_set_props(id, spacing, pack(h_align, v_align), _) */
RuntimeValue rt_layout_set_props(RuntimeValue id_rv, RuntimeValue spacing_rv,
                                  RuntimeValue align_rv, RuntimeValue u)
{
    (void)u;
    int id = (int)(int64_t)id_rv;
    if (id < 0 || id >= g_layout_count) return 0;
    LayoutNode *n = &g_layout[id];

    n->spacing = (uint32_t)(uint64_t)spacing_rv;
    n->h_align = (LayoutAlign)((uint64_t)align_rv >> 32);
    n->v_align = (LayoutAlign)((uint64_t)align_rv & 0xFFFFFFFF);

    return 0;
}

/* Measure pass (bottom-up): compute intrinsic sizes */
static void layout_measure(int id)
{
    LayoutNode *n = &g_layout[id];
    if (!n->active) return;

    /* Measure children first */
    for (int i = 0; i < n->child_count; i++) {
        layout_measure(n->children[i]);
    }

    /* Compute intrinsic size based on children */
    uint32_t content_w = 0, content_h = 0;

    if (n->type == LT_HSTACK) {
        for (int i = 0; i < n->child_count; i++) {
            LayoutNode *c = &g_layout[n->children[i]];
            if (c->w_mode == SM_FIXED) content_w += c->w_value;
            else content_w += c->intrinsic_w;
            uint32_t ch = (c->h_mode == SM_FIXED) ? c->h_value : c->intrinsic_h;
            if (ch > content_h) content_h = ch;
        }
        if (n->child_count > 1) content_w += (n->child_count - 1) * n->spacing;
    } else if (n->type == LT_VSTACK) {
        for (int i = 0; i < n->child_count; i++) {
            LayoutNode *c = &g_layout[n->children[i]];
            if (c->h_mode == SM_FIXED) content_h += c->h_value;
            else content_h += c->intrinsic_h;
            uint32_t cw = (c->w_mode == SM_FIXED) ? c->w_value : c->intrinsic_w;
            if (cw > content_w) content_w = cw;
        }
        if (n->child_count > 1) content_h += (n->child_count - 1) * n->spacing;
    } else if (n->type == LT_ZSTACK) {
        for (int i = 0; i < n->child_count; i++) {
            LayoutNode *c = &g_layout[n->children[i]];
            uint32_t cw = (c->w_mode == SM_FIXED) ? c->w_value : c->intrinsic_w;
            uint32_t ch = (c->h_mode == SM_FIXED) ? c->h_value : c->intrinsic_h;
            if (cw > content_w) content_w = cw;
            if (ch > content_h) content_h = ch;
        }
    }

    n->intrinsic_w = content_w + n->pad_left + n->pad_right;
    n->intrinsic_h = content_h + n->pad_top + n->pad_bottom;
}

/* Layout pass (top-down): assign positions and final sizes */
static void layout_arrange(int id, int32_t x, int32_t y, uint32_t w, uint32_t h)
{
    LayoutNode *n = &g_layout[id];
    if (!n->active) return;

    n->cx = x;
    n->cy = y;
    n->cw = w;
    n->ch = h;

    if (n->child_count == 0) return;

    uint32_t content_w = w - n->pad_left - n->pad_right;
    uint32_t content_h = h - n->pad_top - n->pad_bottom;
    int32_t start_x = x + (int32_t)n->pad_left;
    int32_t start_y = y + (int32_t)n->pad_top;

    if (n->type == LT_HSTACK) {
        /* Calculate fixed vs fill space */
        uint32_t fixed_total = 0;
        uint32_t fill_weight_total = 0;
        for (int i = 0; i < n->child_count; i++) {
            LayoutNode *c = &g_layout[n->children[i]];
            if (c->w_mode == SM_FIXED) fixed_total += c->w_value;
            else if (c->w_mode == SM_FILL) fill_weight_total += (c->w_value > 0 ? c->w_value : 1);
            else fixed_total += c->intrinsic_w;
        }
        if (n->child_count > 1) fixed_total += (n->child_count - 1) * n->spacing;

        uint32_t free_space = (content_w > fixed_total) ? content_w - fixed_total : 0;

        int32_t cx = start_x;
        for (int i = 0; i < n->child_count; i++) {
            LayoutNode *c = &g_layout[n->children[i]];
            uint32_t cw, ch;

            if (c->w_mode == SM_FIXED) cw = c->w_value;
            else if (c->w_mode == SM_FILL) {
                uint32_t weight = (c->w_value > 0 ? c->w_value : 1);
                cw = (fill_weight_total > 0) ? (free_space * weight / fill_weight_total) : 0;
            } else cw = c->intrinsic_w;

            if (c->h_mode == SM_FIXED) ch = c->h_value;
            else if (c->h_mode == SM_FILL) ch = content_h;
            else ch = c->intrinsic_h;

            /* Cross-axis alignment */
            int32_t cy = start_y;
            if (n->v_align == ALIGN_CENTER) cy += (int32_t)(content_h - ch) / 2;
            else if (n->v_align == ALIGN_END) cy += (int32_t)(content_h - ch);

            layout_arrange(n->children[i], cx, cy, cw, ch);
            cx += (int32_t)cw + (int32_t)n->spacing;
        }
    } else if (n->type == LT_VSTACK) {
        uint32_t fixed_total = 0;
        uint32_t fill_weight_total = 0;
        for (int i = 0; i < n->child_count; i++) {
            LayoutNode *c = &g_layout[n->children[i]];
            if (c->h_mode == SM_FIXED) fixed_total += c->h_value;
            else if (c->h_mode == SM_FILL) fill_weight_total += (c->h_value > 0 ? c->h_value : 1);
            else fixed_total += c->intrinsic_h;
        }
        if (n->child_count > 1) fixed_total += (n->child_count - 1) * n->spacing;

        uint32_t free_space = (content_h > fixed_total) ? content_h - fixed_total : 0;

        int32_t cy = start_y;
        for (int i = 0; i < n->child_count; i++) {
            LayoutNode *c = &g_layout[n->children[i]];
            uint32_t cw, ch;

            if (c->h_mode == SM_FIXED) ch = c->h_value;
            else if (c->h_mode == SM_FILL) {
                uint32_t weight = (c->h_value > 0 ? c->h_value : 1);
                ch = (fill_weight_total > 0) ? (free_space * weight / fill_weight_total) : 0;
            } else ch = c->intrinsic_h;

            if (c->w_mode == SM_FIXED) cw = c->w_value;
            else if (c->w_mode == SM_FILL) cw = content_w;
            else cw = c->intrinsic_w;

            int32_t cx = start_x;
            if (n->h_align == ALIGN_CENTER) cx += (int32_t)(content_w - cw) / 2;
            else if (n->h_align == ALIGN_END) cx += (int32_t)(content_w - cw);

            layout_arrange(n->children[i], cx, cy, cw, ch);
            cy += (int32_t)ch + (int32_t)n->spacing;
        }
    } else if (n->type == LT_ZSTACK) {
        for (int i = 0; i < n->child_count; i++) {
            LayoutNode *c = &g_layout[n->children[i]];
            uint32_t cw = (c->w_mode == SM_FIXED) ? c->w_value : content_w;
            uint32_t ch = (c->h_mode == SM_FIXED) ? c->h_value : content_h;
            int32_t cx = start_x, cy = start_y;
            if (n->h_align == ALIGN_CENTER) cx += (int32_t)(content_w - cw) / 2;
            if (n->v_align == ALIGN_CENTER) cy += (int32_t)(content_h - ch) / 2;
            layout_arrange(n->children[i], cx, cy, cw, ch);
        }
    }
}

/* Compute layout: rt_layout_compute(pack(screen_w, screen_h), _, _, _) */
RuntimeValue rt_layout_compute(RuntimeValue wh, RuntimeValue u1,
                                RuntimeValue u2, RuntimeValue u3)
{
    (void)u1; (void)u2; (void)u3;
    uint32_t sw = (uint32_t)((uint64_t)wh >> 32);
    uint32_t sh = (uint32_t)((uint64_t)wh & 0xFFFFFFFF);

    if (g_layout_count == 0) return 0;

    /* Find root node (parent == -1) */
    int root = -1;
    for (int i = 0; i < g_layout_count; i++) {
        if (g_layout[i].active && g_layout[i].parent == -1) {
            root = i;
            break;
        }
    }
    if (root < 0) return 0;

    layout_measure(root);
    layout_arrange(root, 0, 0, sw, sh);

    return 0;
}

/* Get computed position: rt_layout_get(id, _, _, _) -> pack(x, y) */
RuntimeValue rt_layout_get(RuntimeValue id_rv, RuntimeValue u1,
                            RuntimeValue u2, RuntimeValue u3)
{
    (void)u1; (void)u2; (void)u3;
    int id = (int)(int64_t)id_rv;
    if (id < 0 || id >= g_layout_count) return 0;
    LayoutNode *n = &g_layout[id];
    uint64_t x = (uint32_t)n->cx;
    uint64_t y = (uint32_t)n->cy;
    return (RuntimeValue)((x << 32) | y);
}

/* Get computed size: rt_layout_get_size(id, _, _, _) -> pack(w, h) */
RuntimeValue rt_layout_get_size(RuntimeValue id_rv, RuntimeValue u1,
                                 RuntimeValue u2, RuntimeValue u3)
{
    (void)u1; (void)u2; (void)u3;
    int id = (int)(int64_t)id_rv;
    if (id < 0 || id >= g_layout_count) return 0;
    LayoutNode *n = &g_layout[id];
    return (RuntimeValue)(((uint64_t)n->cw << 32) | (uint64_t)n->ch);
}
