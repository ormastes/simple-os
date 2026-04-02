# Minimal QEMU Semihosting Test
# Tests serial output and QEMU debug exit
#
# Build: as --32 -o test.o qemu_semihost_test.S
# Link:  ld -m elf_i386 -T linker.ld -o test.elf test.o
# Run:   qemu-system-i386 -kernel test.elf -serial stdio \
#        -device isa-debug-exit,iobase=0xf4,iosize=0x04 -display none

.set MULTIBOOT_MAGIC, 0x1BADB002
.set MULTIBOOT_FLAGS, 0x00000003
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

# COM1 serial port
.set COM1_DATA, 0x3F8
.set COM1_LSR, 0x3FD
.set LSR_THRE, 0x20

# QEMU debug exit
.set DEBUG_EXIT_PORT, 0xF4

.section .multiboot, "a"
.align 4
multiboot_header:
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHECKSUM

.section .text
.global _start
.code32

_start:
    # Set up stack
    movl $stack_top, %esp

    # Print test message
    movl $msg_start, %esi
    call print_string

    # Print success message
    movl $msg_success, %esi
    call print_string

    # Exit QEMU with success (code 0)
    # Note: QEMU exit code = (value << 1) | 1, so 0 -> 1
    movw $DEBUG_EXIT_PORT, %dx
    movb $0, %al
    outb %al, %dx

    # Fallback: halt if debug exit not available
halt:
    cli
    hlt
    jmp halt

# Print null-terminated string at %esi
print_string:
    pushl %eax
    pushl %edx
.print_loop:
    movb (%esi), %al        # Load character
    testb %al, %al          # Check for null
    jz .print_done

    # Wait for transmit buffer empty
.wait_tx:
    movw $COM1_LSR, %dx
    inb %dx, %al
    testb $LSR_THRE, %al
    jz .wait_tx

    # Send character
    movb (%esi), %al
    movw $COM1_DATA, %dx
    outb %al, %dx

    incl %esi
    jmp .print_loop

.print_done:
    popl %edx
    popl %eax
    ret

.section .rodata
msg_start:
    .asciz "[SEMIHOST TEST] Starting QEMU semihosting test...\r\n"
msg_success:
    .asciz "[SEMIHOST TEST] Serial output works!\r\n[SEMIHOST TEST] Exiting with success code.\r\n"

.section .bss
.align 16
stack_bottom:
    .space 16384    # 16KB stack
stack_top:
