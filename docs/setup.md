# Setup

This document shows the recommended setup for compiling RVV intrinsic programs into RISC-V ELF binaries and running them on `spike pk`.

## 1. Build A RISC-V Cross Compiler

Recommended source:

- `riscv-collab/riscv-gnu-toolchain`
- https://github.com/riscv-collab/riscv-gnu-toolchain

The upstream README currently documents a Newlib build flow with:

```bash
git clone https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain
./configure --prefix=/opt/riscv
make
```

The same README also lists Ubuntu prerequisites:

```bash
sudo apt-get install autoconf automake autotools-dev curl python3 python3-pip python3-tomli \
  libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool \
  patchutils bc zlib1g-dev libexpat-dev ninja-build git cmake libglib2.0-dev libslirp-dev \
  libncurses-dev
```

After installation, make sure the cross compiler is in `PATH`:

```bash
export RISCV=/opt/riscv
export PATH="$RISCV/bin:$PATH"
riscv64-unknown-linux-gnu-gcc --version
```

If you want a repo-local helper as a starting point, see [scripts/install_riscv_toolchain.sh](/home/stanley/rvv-intrinsics-playground/scripts/install_riscv_toolchain.sh:1). That script is intentionally aligned with the `riscv64-unknown-linux-gnu-gcc` flow used by this repo.

## 2. Build Spike

Recommended source:

- `riscv-software-src/riscv-isa-sim`
- https://github.com/riscv-software-src/riscv-isa-sim

The upstream README currently documents:

```bash
git clone https://github.com/riscv-software-src/riscv-isa-sim.git
cd riscv-isa-sim
mkdir build
cd build
../configure --prefix=$RISCV
make
make install
```

The same README lists these extra packages on Debian/Ubuntu style systems:

```bash
sudo apt-get install device-tree-compiler libboost-regex-dev libboost-system-dev
```

Check that Spike is installed:

```bash
spike --help
```

## 3. Build pk

Recommended source:

- `riscv-software-src/riscv-pk`
- https://github.com/riscv-software-src/riscv-pk

The upstream README currently documents:

```bash
git clone https://github.com/riscv-software-src/riscv-pk.git
cd riscv-pk
mkdir build
cd build
../configure --prefix=$RISCV --host=riscv64-unknown-linux-gnu
make
make install
```

Check that `pk` is installed:

```bash
pk --help
```

## 4. Build This Repo

```bash
git clone <your-repo-url>
cd rvv-intrinsics-playground

cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-unknown-linux-gnu-toolchain.cmake \
  -DRVV_ARCH=rv64gcv_zicntr_zihpm \
  -DRVV_ABI=lp64d

cmake --build build -j
```

Important: if a build directory was first configured with host `cc`, CMake will keep using that compiler. Recreate the build directory when switching to the RISC-V toolchain.

```bash
rm -rf build
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-unknown-linux-gnu-toolchain.cmake \
  -DRVV_ARCH=rv64gcv_zicntr_zihpm \
  -DRVV_ABI=lp64d
```

If you want one operator per folder, with the generated `Makefile` directly under that operator directory:

```bash
cmake -S benchmarks/vector_add_f32 -B benchmarks/vector_add_f32 \
  -DCMAKE_TOOLCHAIN_FILE=../../cmake/riscv64-unknown-linux-gnu-toolchain.cmake \
  -DRVV_ARCH=rv64gcv_zicntr_zihpm \
  -DRVV_ABI=lp64d

cmake --build benchmarks/vector_add_f32 -j
```

## 5. Run A Benchmark On Spike

```bash
spike pk build/vector_add_f32
```

For a root build, the binary layout used by this repo is:

```bash
spike --isa=rv64gcv_zicntr_zihpm pk build/benchmarks/vector_add_f32/vector_add_f32
```

Or, if you configured that operator in-place:

```bash
spike pk benchmarks/vector_add_f32/vector_add_f32
```

If `spike` and `pk` come from a separately managed environment such as Chipyard, let the user `source` that environment manually first, then run:

```bash
./scripts/run_on_spike.sh vector_add_f32
```

## 6. Validate Your Environment Quickly

If something fails, first check the toolchain path:

```bash
which riscv64-unknown-linux-gnu-gcc
which spike
which pk
```

Then check whether your compiler provides `riscv_vector.h`:

```bash
printf '#include <riscv_vector.h>\n' | \
  riscv64-unknown-linux-gnu-gcc -march=rv64gcv_zicntr_zihpm -mabi=lp64d -E -xc -
```

If this include fails, your compiler does not yet have the RVV intrinsic header in the active configuration.

## Notes

- `spike pk` is a convenient functional path for verifying that an RVV program builds and runs.
- It is not the right place to make strong performance claims.
- This repo is documented around `riscv64-unknown-linux-gnu-gcc + spike + pk`, and the benchmark targets are linked statically by default so they are suitable for `pk`.
