/*
 * x86_64 Bare-Metal Linker Script
 *
 * Target: x86_64 processors (QEMU, physical hardware)
 * Bootloader: Multiboot1 compatible (QEMU -kernel)
 *
 * Memory Layout:
 *   0x00100000 - Kernel load address (1MB mark)
 *   0x00200000 - Stack (grows down)
 *   Identity-mapped first 1GB via page tables
 *
 * Sections:
 *   .text.entry - Multiboot1 header + _start (must be in first 8KB)
 *   .text       - Code and read-only data
 *   .rodata     - Read-only data (GDT, etc.)
 *   .data       - Initialized data
 *   .bss        - Uninitialized data (page tables, stack)
 *   .heap       - Dynamic memory allocation
 */

ENTRY(_entry32)

SECTIONS
{
    /* Kernel starts at 1MB (standard for multiboot) */
    . = 0x100000;
    _kernel_start = .;

    /* Code section — .text.entry first for Multiboot header */
    .text ALIGN(4K) :
    {
        KEEP(*(.text.entry))
        KEEP(*(.multiboot))
        *(.text)
        *(.text.*)
    }
    /* The x86_64 boot crt0 places a fixed 28-byte Multiboot1 header at the
     * start of .text.entry, followed immediately by the 32-bit boot entry.
     * Some current toolchain paths are dropping the assembler's _entry32
     * symbol from the final ELF symbol table, which leaves e_entry as 0 and
     * makes QEMU reject the kernel image. Recreate the symbol at link time so
     * ENTRY(_entry32) always resolves to the actual boot code. */
    _entry32 = ADDR(.text) + 28;

    /* Read-only data */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data */
    .bss ALIGN(4K) :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        __bss_end = .;
    }

    /* Heap (grows upward) — NOLOAD so ld emits SHT_NOBITS and QEMU's
     * multiboot loader does not have to read/write megabytes of zero
     * content out of the ELF file. */
    .heap ALIGN(4K) (NOLOAD) :
    {
        __heap_start = .;
        . = . + 16M;  /* 16MB linker-script heap (in addition to _heap[]) */
        __heap_end = .;
    }

    /* Stack (grows downward) — NOLOAD for the same reason. Without this
     * ld marks the section PROGBITS, inflates the ELF file by the stack
     * size, and QEMU either truncates the kernel load or extends the
     * kernel image beyond the configured -m memory. The resulting
     * stack-pointer-above-physical-RAM was the root cause of Agent M's
     * 0x950a cascading fault. */
    .stack ALIGN(4K) (NOLOAD) :
    {
        . = . + 8M;  /* 8MB stack — Simple code uses large stack frames */
        _stack_top = .;
    }

    /* End of kernel */
    _kernel_end = .;

    /*
     * Strong C-ABI aliases for spl_handle_* syscall shims.
     *
     * The Simple compiler's --entry-closure DCE retains the functions (visible
     * as kernel__abi__syscall_shim__spl_handle_* W symbols) but does NOT emit
     * strong --defsym aliases for @export("C") on non-entry functions.
     * The weak stubs in baremetal_stubs.c therefore win the W vs W arbitration.
     *
     * These linker-script assignments create strong (T) aliases that override
     * both the weak baremetal stubs and the internal mangled names.
     *
     * One entry per spl_handle_* function defined in
     * src/os/kernel/abi/syscall_shim.spl (62 shims, Wave 10D).
     */
    /* Point to the inner mangled name (no os__ prefix) which is the real Simple implementation.
     * The os__kernel__arch__x86_64__interrupt__spl_x86_dispatch_installed_syscall_abi is a weak
     * freestanding stub; kernel__arch__x86_64__interrupt__spl_x86_dispatch_installed_syscall_abi
     * is the actual compiled function.  This strong assignment wins over all weak stubs. */
    spl_x86_dispatch_installed_syscall_abi = kernel__arch__x86_64__interrupt__spl_x86_dispatch_installed_syscall_abi;
    spl_handle_debug_write = kernel__abi__syscall_shim__spl_handle_debug_write;
    spl_handle_exit = kernel__abi__syscall_shim__spl_handle_exit;
    spl_handle_yield = kernel__abi__syscall_shim__spl_handle_yield;
    spl_handle_spawn = kernel__abi__syscall_shim__spl_handle_spawn;
    spl_handle_wait = kernel__abi__syscall_shim__spl_handle_wait;
    spl_handle_getpid = kernel__abi__syscall_shim__spl_handle_getpid;
    spl_handle_list_tasks = kernel__abi__syscall_shim__spl_handle_list_tasks;
    spl_handle_get_task_info = kernel__abi__syscall_shim__spl_handle_get_task_info;
    spl_handle_signal = kernel__abi__syscall_shim__spl_handle_signal;
    spl_handle_set_priority = kernel__abi__syscall_shim__spl_handle_set_priority;
    spl_handle_get_parent_pid = kernel__abi__syscall_shim__spl_handle_get_parent_pid;
    spl_handle_mmap = kernel__abi__syscall_shim__spl_handle_mmap;
    spl_handle_munmap = kernel__abi__syscall_shim__spl_handle_munmap;
    spl_handle_mprotect = kernel__abi__syscall_shim__spl_handle_mprotect;
    spl_handle_spawn_binary = kernel__abi__syscall_shim__spl_handle_spawn_binary;
    spl_handle_ipc_send = kernel__abi__syscall_shim__spl_handle_ipc_send;
    spl_handle_ipc_recv = kernel__abi__syscall_shim__spl_handle_ipc_recv;
    spl_handle_ipc_create_port = kernel__abi__syscall_shim__spl_handle_ipc_create_port;
    spl_handle_ipc_connect = kernel__abi__syscall_shim__spl_handle_ipc_connect;
    spl_handle_notification_create = kernel__abi__syscall_shim__spl_handle_notification_create;
    spl_handle_notification_signal = kernel__abi__syscall_shim__spl_handle_notification_signal;
    spl_handle_notification_wait = kernel__abi__syscall_shim__spl_handle_notification_wait;
    spl_handle_notification_poll = kernel__abi__syscall_shim__spl_handle_notification_poll;
    spl_handle_notification_destroy = kernel__abi__syscall_shim__spl_handle_notification_destroy;
    spl_handle_notification_wait_any = kernel__abi__syscall_shim__spl_handle_notification_wait_any;
    spl_handle_file_open = kernel__abi__syscall_shim__spl_handle_file_open;
    spl_handle_file_read = kernel__abi__syscall_shim__spl_handle_file_read;
    spl_handle_file_write = kernel__abi__syscall_shim__spl_handle_file_write;
    spl_handle_file_close = kernel__abi__syscall_shim__spl_handle_file_close;
    spl_handle_file_stat = kernel__abi__syscall_shim__spl_handle_file_stat;
    spl_handle_file_mkdir = kernel__abi__syscall_shim__spl_handle_file_mkdir;
    spl_handle_file_readdir = kernel__abi__syscall_shim__spl_handle_file_readdir;
    spl_handle_mount = kernel__abi__syscall_shim__spl_handle_mount;
    spl_handle_unmount = kernel__abi__syscall_shim__spl_handle_unmount;
    spl_handle_unlink = kernel__abi__syscall_shim__spl_handle_unlink;
    spl_handle_pledge = kernel__abi__syscall_shim__spl_handle_pledge;
    spl_handle_unveil = kernel__abi__syscall_shim__spl_handle_unveil;
    spl_handle_cap_grant = kernel__abi__syscall_shim__spl_handle_cap_grant;
    spl_handle_ftruncate = kernel__abi__syscall_shim__spl_handle_ftruncate;
    spl_handle_rename = kernel__abi__syscall_shim__spl_handle_rename;
    spl_handle_rmdir = kernel__abi__syscall_shim__spl_handle_rmdir;
    spl_handle_lseek = kernel__abi__syscall_shim__spl_handle_lseek;
    spl_handle_getcwd = kernel__abi__syscall_shim__spl_handle_getcwd;
    spl_handle_chdir = kernel__abi__syscall_shim__spl_handle_chdir;
    spl_handle_clock_gettime = kernel__abi__syscall_shim__spl_handle_clock_gettime;
    spl_handle_sleep = kernel__abi__syscall_shim__spl_handle_sleep;
    spl_handle_net_socket = kernel__abi__syscall_shim__spl_handle_net_socket;
    spl_handle_net_bind = kernel__abi__syscall_shim__spl_handle_net_bind;
    spl_handle_net_listen = kernel__abi__syscall_shim__spl_handle_net_listen;
    spl_handle_net_connect = kernel__abi__syscall_shim__spl_handle_net_connect;
    spl_handle_net_accept = kernel__abi__syscall_shim__spl_handle_net_accept;
    spl_handle_net_send_to = kernel__abi__syscall_shim__spl_handle_net_send_to;
    spl_handle_net_recv_from = kernel__abi__syscall_shim__spl_handle_net_recv_from;
    spl_handle_net_if_config = kernel__abi__syscall_shim__spl_handle_net_if_config;
    spl_handle_dev_enumerate = kernel__abi__syscall_shim__spl_handle_dev_enumerate;
    spl_handle_dev_get_info = kernel__abi__syscall_shim__spl_handle_dev_get_info;
    spl_handle_device_grant = kernel__abi__syscall_shim__spl_handle_device_grant;
    spl_handle_map_bar = kernel__abi__syscall_shim__spl_handle_map_bar;
    spl_handle_alloc_dma = kernel__abi__syscall_shim__spl_handle_alloc_dma;
    spl_handle_free_dma = kernel__abi__syscall_shim__spl_handle_free_dma;
    spl_handle_log_write = kernel__abi__syscall_shim__spl_handle_log_write;
    spl_handle_log_read = kernel__abi__syscall_shim__spl_handle_log_read;
    spl_handle_sysinfo = kernel__abi__syscall_shim__spl_handle_sysinfo;
    spl_handle_get_hostname = kernel__abi__syscall_shim__spl_handle_get_hostname;
    spl_handle_set_hostname = kernel__abi__syscall_shim__spl_handle_set_hostname;

    /* Debug sections */
    .debug_info 0 : { *(.debug_info) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_line 0 : { *(.debug_line) }
    .debug_str 0 : { *(.debug_str) }
    .debug_frame 0 : { *(.debug_frame) }
    .debug_loc 0 : { *(.debug_loc) }
    .debug_ranges 0 : { *(.debug_ranges) }

    /* Discard unwanted sections */
    /DISCARD/ :
    {
        *(.note*)
        *(.eh_frame*)
        *(.comment)
    }
}
