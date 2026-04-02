/**
 * SimpleOS SDK - Main C Header
 *
 * Provides FFI bindings for developing SimpleOS kernel modules,
 * drivers, and system services in C/C++.
 *
 * Usage:
 *   #include <simpleos.h>
 *
 * Link with:
 *   clang --target=x86_64-simpleos -ffreestanding -nostdlib \
 *       -I/opt/simpleos-sdk/include -o module.o -c module.c
 */

#ifndef SIMPLEOS_H
#define SIMPLEOS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Version ===== */

#define SIMPLEOS_VERSION_MAJOR 0
#define SIMPLEOS_VERSION_MINOR 1
#define SIMPLEOS_VERSION_PATCH 0

/* ===== Framebuffer API ===== */

/**
 * Copy pixel data to the framebuffer.
 * @param dst   Destination offset in the framebuffer (in bytes)
 * @param src   Source pixel buffer (ARGB32 format)
 * @param count Number of pixels to copy
 */
void rt_framebuffer_copy(uint64_t dst, uint32_t *src, uint32_t count);

/**
 * Get framebuffer dimensions.
 * @param width   Output: framebuffer width in pixels
 * @param height  Output: framebuffer height in pixels
 * @param pitch   Output: bytes per scanline
 */
void rt_framebuffer_info(uint32_t *width, uint32_t *height, uint32_t *pitch);

/**
 * Set a single pixel in the framebuffer.
 * @param x     X coordinate
 * @param y     Y coordinate
 * @param color ARGB32 color value
 */
void rt_framebuffer_set_pixel(uint32_t x, uint32_t y, uint32_t color);

/* ===== Serial I/O ===== */

/**
 * Write a single character to the serial console.
 * @param c Character to write
 */
void serial_putchar(char c);

/**
 * Write a null-terminated string to the serial console.
 * @param s String to write
 */
void serial_puts(const char *s);

/**
 * Read a character from the serial console (blocking).
 * @return Character read
 */
char serial_getchar(void);

/* ===== Memory Management ===== */

/**
 * Allocate memory from the SimpleOS heap.
 * @param size  Number of bytes to allocate
 * @return      Pointer to allocated memory, or NULL on failure
 */
void *rt_alloc(uint64_t size);

/**
 * Allocate aligned memory.
 * @param size      Number of bytes to allocate
 * @param alignment Required alignment (must be power of 2)
 * @return          Aligned pointer, or NULL on failure
 */
void *rt_alloc_aligned(uint64_t size, uint64_t alignment);

/**
 * Free previously allocated memory.
 * @param ptr Pointer returned by rt_alloc or rt_alloc_aligned
 */
void rt_free(void *ptr);

/**
 * Reallocate memory block.
 * @param ptr      Previously allocated pointer (or NULL)
 * @param new_size New size in bytes
 * @return         Pointer to reallocated memory, or NULL on failure
 */
void *rt_realloc(void *ptr, uint64_t new_size);

/* ===== Process / Task ===== */

/**
 * Exit the current process.
 * @param code Exit status code
 */
void rt_exit(int32_t code) __attribute__((noreturn));

/**
 * Yield the current time slice to the scheduler.
 */
void rt_yield(void);

/**
 * Sleep for the specified number of milliseconds.
 * @param ms Milliseconds to sleep
 */
void rt_sleep(uint64_t ms);

/* ===== IPC ===== */

/**
 * Send a message to another process.
 * @param target_pid  Target process ID
 * @param data        Message data
 * @param len         Message length in bytes
 * @return            0 on success, negative on error
 */
int32_t rt_ipc_send(uint32_t target_pid, const void *data, uint32_t len);

/**
 * Receive a message (blocking).
 * @param buf         Buffer to receive into
 * @param buf_len     Buffer capacity
 * @param sender_pid  Output: sender's process ID
 * @return            Number of bytes received, negative on error
 */
int32_t rt_ipc_recv(void *buf, uint32_t buf_len, uint32_t *sender_pid);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEOS_H */
