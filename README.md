# RVV Intrinsics Playground

This repo is a small playground for learning how to write RISC-V Vector (RVV) operators with C intrinsics, build several benchmark-style binaries with CMake, and run the generated ELF files on `spike pk`.

The goal is not to hide the toolchain details. This repo keeps the build flow explicit so a new user can see:

- how an RVV kernel is structured,
- how `-march` / `-mabi` are passed through CMake,
- how to switch between several small kernels,
- how to compile a RISC-V ELF program,
- how to run that program on Spike with the proxy kernel.

## Repo Layout

```text
.
├── benchmarks/
│   ├── vector_add_f32/
│   ├── saxpy_f32/
│   └── reduce_sum_f32/
├── cmake/
│   ├── benchmark-common.cmake
│   └── riscv64-unknown-linux-gnu-toolchain.cmake
├── docs/
│   ├── riscv-v-spec-1.0.pdf       # RVV ISA spec
│   ├── setup.md                   # Compiler / Spike / pk installation notes
│   ├── v-intrinsic-spec.pdf       # RVV intrinsic API spec
│   └── writing-rvv-kernels.md     # How the intrinsic loops are written
├── scripts/
│   ├── install_riscv_toolchain.sh
│   └── run_on_spike.sh
├── include/rvv_playground/
└── src/
```

## Current Benchmarks

- `vector_add_f32`: basic vector load, add, store
- `saxpy_f32`: fused multiply-add style kernel `alpha * x + y`
- `reduce_sum_f32`: vector reduction into one scalar
- `mask_select_i32`: compare + mask + merge
- `slide1down_i32`: slide/per-lane shift style data movement
- `gather_i32`: indexed gather / permutation
- `scatter_i32`: indexed scatter store
- `widen_add_i16_i32`: widening arithmetic
- `convert_f32_i32`: floating-point to integer conversion
- `strided_load_f32`: strided memory access

These are small correctness-oriented kernels. They repeat the operator many times so the binary behaves more like a benchmark target, but `spike pk` should still be treated mainly as a functional simulation path. If you want trustworthy performance numbers, use a simulator or platform with a timing model.

## Reading Order

If someone is new to this repo, the recommended order is:

1. Start from `vector_add_f32`. It is the smallest load -> compute -> store example and is the first benchmark to read.
2. Then read `saxpy_f32`. It keeps the same loop skeleton, but changes the compute step into a fused multiply-accumulate.
3. Read `reduce_sum_f32` after that. This is the first example where the result is not another vector array, but one reduced scalar.
4. After those three, move to `mask_select_i32`, `slide1down_i32`, `gather_i32`, and `scatter_i32` for control-style and data-movement style intrinsics.
5. Then read `widen_add_i16_i32`, `convert_f32_i32`, and `strided_load_f32` for type-conversion, widening, and memory-pattern examples.
6. After the code shape is familiar, use `docs/v-intrinsic-spec.pdf` to look up exact intrinsic names and signatures.
7. Use `docs/riscv-v-spec-1.0.pdf` when you want the ISA-level meaning of `vl`, element width, LMUL, reduction behavior, and vector-length-agnostic execution.

That order matters. `vector_add_f32` is the benchmark that should be treated as the first walkthrough target in this repo.

## Intrinsic Class Coverage

This repo now treats the following as the core starter-set of RVV intrinsic classes, and each one has at least one benchmark example:

- Load/store: `vector_add_f32`
- Floating arithmetic: `vector_add_f32`
- Fused multiply-accumulate: `saxpy_f32`
- Reduction: `reduce_sum_f32`
- Compare/mask/merge: `mask_select_i32`
- Slide: `slide1down_i32`
- Gather/permutation: `gather_i32`
- Scatter/indexed store: `scatter_i32`
- Widening arithmetic: `widen_add_i16_i32`
- Type conversion: `convert_f32_i32`
- Strided memory access: `strided_load_f32`

This is a curated starter set, not a claim that every intrinsic in the full RVV spec is already covered.

## Build With CMake

Recommended flow for `spike pk` is to use `riscv64-unknown-linux-gnu-gcc`. This repo links benchmark binaries statically by default.

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-unknown-linux-gnu-toolchain.cmake \
  -DRVV_ARCH=rv64gcv_zicntr_zihpm \
  -DRVV_ABI=lp64d \
  -DRVV_BENCHMARKS="vector_add_f32;saxpy_f32;reduce_sum_f32"

cmake --build build -j
```

If `build/` was previously configured with host `cc`, do not reuse it. Remove that build directory and configure again, otherwise CMake will keep the old compiler selection.

```bash
rm -rf build
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-unknown-linux-gnu-toolchain.cmake
```

You can build only one benchmark:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-unknown-linux-gnu-toolchain.cmake \
  -DRVV_BENCHMARKS="saxpy_f32"

cmake --build build -j
```

If you want the generated `Makefile` and build artifacts to live directly under one operator directory, configure that operator in place:

```bash
cmake -S benchmarks/saxpy_f32 -B benchmarks/saxpy_f32 \
  -DCMAKE_TOOLCHAIN_FILE=../../cmake/riscv64-unknown-linux-gnu-toolchain.cmake \
  -DRVV_ARCH=rv64gcv_zicntr_zihpm \
  -DRVV_ABI=lp64d

cmake --build benchmarks/saxpy_f32 -j
```

That flow matches the per-operator teaching layout: source and generated `Makefile` are both under `benchmarks/saxpy_f32/`.

You can also change the ISA string from CMake instead of editing source code:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-unknown-linux-gnu-toolchain.cmake \
  -DRVV_ARCH=rv64gcv_zicntr_zihpm_zvl256b \
  -DRVV_ABI=lp64d
```

## Run On Spike + pk

After building from the repo root, each benchmark is under `build/benchmarks/<name>/<name>`. If you configure in-place, the binary lands in that operator directory.

```bash
spike pk build/vector_add_f32
spike pk build/saxpy_f32
spike pk build/reduce_sum_f32
```

With the current output layout, the direct root-build path is:

```bash
spike --isa=rv64gcv_zicntr_zihpm pk build/benchmarks/saxpy_f32/saxpy_f32
```

For an in-place per-operator build:

```bash
spike pk benchmarks/saxpy_f32/saxpy_f32
```

## Automate Spike Run

If the user has already manually sourced an environment that provides `spike` and `pk` such as Chipyard's `env.sh`, this repo provides a small wrapper:

```bash
source /path/to/chipyard/env.sh
./scripts/run_on_spike.sh saxpy_f32
```

The script assumes `source` already made both `spike` and `pk` available in `PATH`.

The script defaults to:

- ISA: `rv64gcv_zicntr_zihpm`
- binary path: `build/benchmarks/<benchmark>/<benchmark>`

You can also override the build directory:

```bash
./scripts/run_on_spike.sh saxpy_f32 build
```

Expected output looks like this:

```text
benchmark=vector_add_f32 length=256 repeat=1024 vlenb=...
cycles_total=... cycles_per_iter=...
src_a=[...]
src_b=[...]
dst=[...]
status=PASS checksum=...
```

## Installation Notes

See [docs/setup.md](docs/setup.md) for the full setup flow.

If you want a repo-local starting point, there is also a helper script at [scripts/install_riscv_toolchain.sh](/home/stanley/rvv-intrinsics-playground/scripts/install_riscv_toolchain.sh:1).

Important detail: on the current local machine used for this repo update, `riscv64-unknown-linux-gnu-gcc` exists, but `spike` and `pk` are not installed yet. This repo is prepared around the `unknown-linux-gnu + spike pk` flow, and uses static linking by default for those benchmark binaries.

## Learn The Kernel Pattern

See [docs/writing-rvv-kernels.md](docs/writing-rvv-kernels.md) for the core RVV loop structure:

1. choose `vl` with `vsetvl`
2. load a chunk
3. apply one intrinsic operator
4. store or reduce
5. advance the scalar index by `vl`

The two spec references shipped in this repo are:

- [docs/v-intrinsic-spec.pdf](/home/stanley/rvv-intrinsics-playground/docs/v-intrinsic-spec.pdf)
- [docs/riscv-v-spec-1.0.pdf](/home/stanley/rvv-intrinsics-playground/docs/riscv-v-spec-1.0.pdf)

The benchmark header also prints `vlenb`, which comes from `__riscv_vlenb()`. That is the current vector register width in bytes for the implementation you are running on.

Each benchmark also prints `cycles_total` and `cycles_per_iter`, measured with `rdcycle`. Treat these numbers as simple teaching-oriented relative costs for the current run environment, not as portable performance claims across different implementations.
