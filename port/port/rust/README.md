# Porting Rust to SimpleOS

Build the Rust toolchain (`core`, `alloc`) for SimpleOS using the `ormastes/rust` fork.

## Overview

SimpleOS uses a custom Rust toolchain to compile `no_std` Rust code for kernel modules and system services. The fork adds SimpleOS as a recognized target with appropriate defaults for freestanding execution.

## Custom Target Specs

Pre-built target JSON specs are in `target/`:

| File | Architecture | Notes |
|------|-------------|-------|
| `x86_64-simpleos.json` | x86-64 | Desktop/server, soft-float, no red zone |
| `aarch64-simpleos.json` | AArch64 | ARM64, NEON disabled by default |
| `riscv64gc-simpleos.json` | RV64GC | 64-bit RISC-V with G+C extensions |
| `riscv32imac-simpleos.json` | RV32IMAC | 32-bit RISC-V embedded |

## Prerequisites

- Rust nightly toolchain (host)
- `rust-src` component: `rustup component add rust-src`
- The `ormastes/rust` fork (for full toolchain build)
- LLVM SimpleOS toolchain (see `../llvm/README.md`)

## Quick Start: Using Target Specs Directly

For building individual crates without a full toolchain build:

```bash
# Point to the target spec
export SIMPLEOS_TARGET=$(pwd)/src/os/port/rust/target/x86_64-simpleos.json

# Build a no_std crate
cargo build -Z build-std=core,alloc --target $SIMPLEOS_TARGET
```

## Full Toolchain Build

### Clone the Fork

```bash
git clone https://github.com/ormastes/rust.git
cd rust
git checkout simpleos
```

### Configure

Create `config.toml`:

```toml
[llvm]
link-shared = false
targets = "X86;AArch64;RISCV"

[build]
target = ["x86_64-simpleos"]
docs = false
extended = false

[install]
prefix = "/opt/simpleos-rust"

[rust]
lld = true
```

### Build

```bash
# Build core and alloc for SimpleOS
python3 x.py build library/core library/alloc --target x86_64-simpleos

# Install
python3 x.py install --target x86_64-simpleos
```

### Multi-Target Build

```bash
python3 x.py build library/core library/alloc \
  --target x86_64-simpleos \
  --target aarch64-simpleos \
  --target riscv64gc-simpleos \
  --target riscv32imac-simpleos
```

## Fork Changes

The `ormastes/rust` fork includes:

1. **Target definitions** in `compiler/rustc_target/src/spec/` for all SimpleOS triples
2. **core/alloc patches** for SimpleOS-specific memory allocator hooks
3. **Linker defaults** pointing to `ld.lld` with SimpleOS linker scripts

## Using with SimpleOS Kernel

```rust
#![no_std]
#![no_main]

extern crate alloc;

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[no_mangle]
pub extern "C" fn _start() -> ! {
    // SimpleOS entry point
    loop {}
}
```

## Automated Build

Use `build.spl` in this directory:

```bash
bin/simple run src/os/port/rust/build.spl -- --target x86_64-simpleos
```
