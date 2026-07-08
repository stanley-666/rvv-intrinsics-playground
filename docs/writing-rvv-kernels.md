# Writing RVV Kernels

This repo uses the same core pattern in every operator.

## 0. Which Benchmark To Read First

Do not open the benchmarks in random order.

Recommended order:

1. `benchmarks/vector_add_f32/main.c`
2. `benchmarks/saxpy_f32/main.c`
3. `benchmarks/reduce_sum_f32/main.c`
4. `benchmarks/mask_select_i32/main.c`
5. `benchmarks/slide1down_i32/main.c`
6. `benchmarks/gather_i32/main.c`
7. `benchmarks/scatter_i32/main.c`
8. `benchmarks/widen_add_i16_i32/main.c`
9. `benchmarks/convert_f32_i32/main.c`
10. `benchmarks/strided_load_f32/main.c`

Why this order:

- `vector_add_f32` is the first benchmark because it is the cleanest introduction to `vsetvl`, vector load, one arithmetic intrinsic, and vector store.
- `saxpy_f32` should be second because the loop skeleton is almost the same, so the reader can focus on how the compute intrinsic changes.
- `reduce_sum_f32` should be third because reduction is conceptually harder: the output is one scalar, not one vector buffer.
- `mask_select_i32`, `slide1down_i32`, `gather_i32`, and `scatter_i32` are the next layer because they introduce masks and data movement instead of only arithmetic.
- `widen_add_i16_i32`, `convert_f32_i32`, and `strided_load_f32` are later because they mix element-width changes, conversion semantics, and non-unit-stride memory access.

When reading the code together with the official references in this repo:

- use `docs/v-intrinsic-spec.pdf` when you want to check the exact intrinsic prototype or naming pattern,
- use `docs/riscv-v-spec-1.0.pdf` when you want the ISA-level explanation behind `vl`, vector types, and reduction semantics.

## 0.5 Core Intrinsic Classes Covered In This Repo

This repo uses one benchmark per core starter class:

- load/store plus basic arithmetic: `vector_add_f32`
- fused multiply-accumulate: `saxpy_f32`
- reduction: `reduce_sum_f32`
- compare/mask/merge: `mask_select_i32`
- slide: `slide1down_i32`
- gather/permutation: `gather_i32`
- scatter/indexed store: `scatter_i32`
- widening arithmetic: `widen_add_i16_i32`
- float/int conversion: `convert_f32_i32`
- strided load: `strided_load_f32`

That is the current teaching scope. It is broader than the original three benchmarks, but it is still not the full RVV intrinsic surface.

## 1. Scalar Reference First

Write a plain scalar version first. That gives you:

- a correctness baseline,
- a reference output for comparison,
- a way to explain the operator before talking about vector registers.

Example:

```c
static void vector_add_scalar(const float *a, const float *b, float *c, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}
```

## 2. RVV Version Uses A `while` Loop

RVV code usually does not hardcode one vector length. Instead, it repeatedly asks the hardware/compiler ABI layer what `vl` is valid for the remaining work.

Example shape:

```c
size_t i = 0;
while (i < n) {
  size_t vl = __riscv_vsetvl_e32m1(n - i);
  i += vl;
}
```

This is the key idea: RVV is vector-length agnostic. The loop progresses by the returned `vl`, not by a hardcoded constant like 4, 8, or 16.

If you want to see the implementation's vector register width directly, use:

```c
size_t vlenb = __riscv_vlenb();
```

`vlenb` is the vector register width in bytes. In this repo, benchmark output prints it in the header so you can see what the current Spike or hardware configuration is exposing.

This repo also measures cycles with `rdcycle` around the repeated RVV loop in each benchmark and prints:

```text
cycles_total=...
cycles_per_iter=...
```

That is mainly for relative comparison inside this teaching repo. A larger cycle count can help readers notice that some operations, such as gather, scatter, or reduction-heavy code, are more expensive than plain pointwise add. It should not be treated as a universal performance claim for every RVV implementation.

## 3. Load -> Compute -> Store

For a pointwise operator, the loop body is usually:

```c
size_t vl = __riscv_vsetvl_e32m1(n - i);
vfloat32m1_t va = __riscv_vle32_v_f32m1(a + i, vl);
vfloat32m1_t vb = __riscv_vle32_v_f32m1(b + i, vl);
vfloat32m1_t vc = __riscv_vfadd_vv_f32m1(va, vb, vl);
__riscv_vse32_v_f32m1(c + i, vc, vl);
```

Breakdown:

- `__riscv_vsetvl_e32m1(...)`: choose a legal vector length for `e32, m1`
- `__riscv_vle32...`: load a chunk of 32-bit elements
- `__riscv_vfadd...`: apply one vector floating-point operator
- `__riscv_vse32...`: write the result back

## 4. Fused Operators Reuse The Same Skeleton

`saxpy_f32` changes only the compute line:

```c
vfloat32m1_t vz = __riscv_vfmacc_vf_f32m1(vy, alpha, vx, vl);
```

That corresponds to:

```text
z = y + alpha * x
```

So once the load/store skeleton is clear, most new operators are just a change in:

- the intrinsic name,
- scalar operands,
- whether the result is stored or reduced.

## 5. Reduction Kernels End In A Scalar

`reduce_sum_f32` still iterates by `vl`, but instead of storing a full vector each round, it folds the chunk into an accumulator.

That is why its pattern becomes:

- load a vector,
- reduce into accumulator,
- continue,
- extract one scalar at the end.

## 6. CMake Separates Build Parameters From Source Code

This repo keeps:

- benchmark selection in `RVV_BENCHMARKS`,
- ISA string in `RVV_ARCH`,
- ABI string in `RVV_ABI`.

That means you can teach or test several combinations without editing every source file.

## 7. What To Add Next

If you want to extend this repo, the next useful kernels are usually:

- integer add / multiply,
- masked operations,
- strided load / store,
- widening arithmetic,
- GEMV or small reduction-heavy kernels.
