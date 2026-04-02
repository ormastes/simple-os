# SimpleOS

Multi-architecture microkernel OS written in the [Simple language](https://github.com/ormastes/simple).

## Supported Architectures
- x86_64 (QEMU q35)
- ARM64/AArch64 (QEMU virt)
- RISC-V 64 (QEMU virt + OpenSBI)
- RISC-V 32 (QEMU virt + OpenSBI, requires LLVM backend)

## Features
- Capability-based microkernel
- Desktop GUI with compositor, window manager, 16 apps
- PS/2 keyboard + mouse input
- Framebuffer graphics (ramfb, VBE, Limine)
- Multi-architecture boot (Multiboot, OpenSBI, direct ELF)

## Prerequisites
- Simple language compiler (`bin/simple` from https://github.com/ormastes/simple)
- Clang + LLD (cross-compilation linker)
- QEMU system emulators

## Quick Start
```bash
# Build for x86_64
simple native-build --entry arch/x86_64/entry.spl \
  --target x86_64-unknown-none -o build/simpleos.elf \
  --linker-script arch/x86_64/linker.ld --entry-closure

# Run in QEMU
qemu-system-x86_64 -machine q35 -m 128M -serial stdio \
  -display none -kernel build/simpleos.elf
```

## Project Structure
- `kernel/` — Microkernel (boot, memory, scheduler, IPC, interrupts)
- `drivers/` — Device drivers (framebuffer, input, PCI, virtio)
- `compositor/` — Window compositor with Z-ordering
- `desktop/` — Desktop shell, dock, app launcher
- `apps/` — 16 GUI applications (calculator, terminal, editor, paint, ...)
- `services/` — System services (VFS, FAT32, init)
- `arch/` — Per-architecture boot entries and linker scripts
- `lib/` — Vendored stdlib subset (UI widgets, baremetal runtime)
- `port/` — Porting infrastructure (LLVM, Rust, GUI apps)
- `sdk/` — SimpleOS SDK for app development
- `tools/` — Build and test tools
