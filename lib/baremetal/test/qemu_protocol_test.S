# QEMU Semihosting Protocol Test
# Tests our custom binary protocol over serial
#
# Protocol format:
#   [0xAB] Magic
#   [0x01] Version
#   [u32]  Op code (little-endian)
#   [u32]  Payload length (little-endian)
#   [...]  Payload

.set MULTIBOOT_MAGIC, 0x1BADB002
.set MULTIBOOT_FLAGS, 0x00000003
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

.set COM1_DATA, 0x3F8
.set COM1_LSR, 0x3FD
.set LSR_THRE, 0x20
.set DEBUG_EXIT_PORT, 0xF4

# Protocol constants
.set PROTO_MAGIC, 0xAB
.set PROTO_VERSION, 0x01
.set SYS_WRITE_HANDLE, 0x100
.set SYS_WRITE_HANDLE_P1, 0x101

.section .multiboot, "a"
.align 4
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHECKSUM

.section .text
.global _start
.code32

_start:
    movl $stack_top, %esp

    # Print text header
    movl $msg_header, %esi
    call print_string

    # Send binary protocol message: semi_print(handle=0x42)
    # Format: [AB][01][00 01 00 00][04 00 00 00][42 00 00 00]
    movl $msg_sending, %esi
    call print_string

    # Send magic
    movb $PROTO_MAGIC, %al
    call serial_write_byte

    # Send version
    movb $PROTO_VERSION, %al
    call serial_write_byte

    # Send op code (0x100 = SYS_WRITE_HANDLE) little-endian
    movb $0x00, %al
    call serial_write_byte
    movb $0x01, %al
    call serial_write_byte
    movb $0x00, %al
    call serial_write_byte
    movb $0x00, %al
    call serial_write_byte

    # Send payload length (4 bytes for handle)
    movb $0x04, %al
    call serial_write_byte
    movb $0x00, %al
    call serial_write_byte
    movb $0x00, %al
    call serial_write_byte
    movb $0x00, %al
    call serial_write_byte

    # Send handle (0x42)
    movb $0x42, %al
    call serial_write_byte
    movb $0x00, %al
    call serial_write_byte
    movb $0x00, %al
    call serial_write_byte
    movb $0x00, %al
    call serial_write_byte

    # Print newline
    movl $newline, %esi
    call print_string

    # Print success
    movl $msg_success, %esi
    call print_string

    # Show hex dump of what was sent
    movl $msg_hexdump, %esi
    call print_string

    # Exit with success
    movw $DEBUG_EXIT_PORT, %dx
    movb $0, %al
    outb %al, %dx

halt:
    cli
    hlt
    jmp halt

# Write byte in %al to serial
serial_write_byte:
    pushl %edx
    pushl %eax
.wait_tx:
    movw $COM1_LSR, %dx
    inb %dx, %al
    testb $LSR_THRE, %al
    jz .wait_tx
    popl %eax
    movw $COM1_DATA, %dx
    outb %al, %dx
    popl %edx
    ret

# Print null-terminated string at %esi
print_string:
    pushl %eax
.print_loop:
    movb (%esi), %al
    testb %al, %al
    jz .print_done
    call serial_write_byte
    incl %esi
    jmp .print_loop
.print_done:
    popl %eax
    ret

.section .rodata
msg_header:
    .asciz "\r\n=== QEMU Semihosting Protocol Test ===\r\n\r\n"
msg_sending:
    .asciz "Sending binary protocol message: semi_print(0x42)\r\nBinary data: "
newline:
    .asciz "\r\n"
msg_success:
    .asciz "\r\nProtocol message sent successfully!\r\n"
msg_hexdump:
    .asciz "Expected hex: AB 01 00 01 00 00 04 00 00 00 42 00 00 00\r\n"

.section .bss
.align 16
stack_bottom:
    .space 16384
stack_top:
