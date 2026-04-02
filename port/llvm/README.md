# Porting LLVM to SimpleOS

Build LLVM from the `ormastes/llvm-project` fork for SimpleOS cross-compilation targets.

## Custom Target Triples

| Triple | Architecture | Notes |
|--------|-------------|-------|
| `x86_64-simpleos` | x86-64 | Primary desktop/server target |
| `aarch64-simpleos` | AArch64 | ARM64 boards and SBCs |
| `riscv64gc-simpleos` | RV64GC | RISC-V 64-bit with GC extensions |
| `riscv32imac-simpleos` | RV32IMAC | RISC-V 32-bit embedded |

## Prerequisites

- CMake >= 3.20
- Ninja
- Host Clang/LLVM >= 17 (for cross-compiling)
- Python 3.8+
- Git

## Getting the Source

```bash
git clone https://github.com/ormastes/llvm-project.git
cd llvm-project
git checkout simpleos
```

## Building LLVM for SimpleOS

### Quick Build (x86_64)

```bash
cmake -S llvm -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_PROJECTS="clang;lld" \
  -DLLVM_TARGETS_TO_BUILD="X86;AArch64;RISCV" \
  -DLLVM_DEFAULT_TARGET_TRIPLE="x86_64-simpleos" \
  -DCMAKE_INSTALL_PREFIX=/opt/simpleos-toolchain

ninja -C build
ninja -C build install
```

### Cross-Compiling for All SimpleOS Targets

```bash
cmake -S llvm -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_PROJECTS="clang;lld;compiler-rt" \
  -DLLVM_TARGETS_TO_BUILD="X86;AArch64;RISCV" \
  -DLLVM_DEFAULT_TARGET_TRIPLE="x86_64-simpleos" \
  -DCOMPILER_RT_DEFAULT_TARGET_ONLY=OFF \
  -DCOMPILER_RT_BAREMETAL_BUILD=ON \
  -DCMAKE_INSTALL_PREFIX=/opt/simpleos-toolchain

ninja -C build
ninja -C build install
```

### Building compiler-rt for SimpleOS

compiler-rt provides builtins needed by SimpleOS kernel and userspace:

```bash
cmake -S compiler-rt -B build-rt -G Ninja \
  -DCMAKE_C_COMPILER=/opt/simpleos-toolchain/bin/clang \
  -DCMAKE_AR=/opt/simpleos-toolchain/bin/llvm-ar \
  -DCMAKE_NM=/opt/simpleos-toolchain/bin/llvm-nm \
  -DCMAKE_RANLIB=/opt/simpleos-toolchain/bin/llvm-ranlib \
  -DCOMPILER_RT_BAREMETAL_BUILD=ON \
  -DCOMPILER_RT_BUILD_BUILTINS=ON \
  -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
  -DCOMPILER_RT_BUILD_XRAY=OFF \
  -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
  -DCOMPILER_RT_BUILD_PROFILE=OFF \
  -DCMAKE_C_FLAGS="--target=x86_64-simpleos -ffreestanding -nostdlib" \
  -DCMAKE_INSTALL_PREFIX=/opt/simpleos-toolchain

ninja -C build-rt
ninja -C build-rt install
```

## Applying SimpleOS Patches

The `simpleos` branch in `ormastes/llvm-project` contains all necessary patches. Key changes:

1. **Target triple recognition** -- Adds `simpleos` as a known OS in `llvm/lib/TargetParser/Triple.cpp`
2. **Clang driver** -- SimpleOS driver in `clang/lib/Driver/ToolChains/SimpleOS.cpp` sets default flags (`-ffreestanding`, `-nostdlib`, correct linker)
3. **LLD support** -- ELF linker recognizes SimpleOS output format and default linker script locations
4. **compiler-rt** -- Builtins build configuration for freestanding SimpleOS targets

To update patches from upstream:

```bash
git fetch upstream
git rebase upstream/main
# Resolve conflicts in SimpleOS-specific files
git push origin simpleos
```

## Using the Toolchain

After installation, compile SimpleOS code with:

```bash
/opt/simpleos-toolchain/bin/clang \
  --target=x86_64-simpleos \
  -ffreestanding -nostdlib \
  -o output.o -c input.c

/opt/simpleos-toolchain/bin/ld.lld \
  -T linker.ld -o kernel.elf output.o
```

## Automated Build

Use `build.spl` in this directory to automate the full build:

```bash
bin/simple run src/os/port/llvm/build.spl -- --target x86_64-simpleos
```
