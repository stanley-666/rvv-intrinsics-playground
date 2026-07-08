# saxpy_f32

This benchmark demonstrates fused multiply-accumulate on `float32` vectors.

What it does:

- reads `x` and `y`,
- computes `alpha * x + y`,
- stores the result vector.

What the printed output is trying to teach:

- `alpha` is printed separately so the reader knows the scalar multiplier,
- `src_x` and `src_y` are the original arrays,
- `dst` is the result after fused multiply-accumulate,
- with `alpha = 2.0` and both inputs `1, 2, 3, 4, ...`, the result becomes `3, 6, 9, 12, ...`.

Why it exists:

- it reuses the same loop shape as `vector_add_f32`,
- it shows how a pointwise arithmetic benchmark changes when the compute intrinsic becomes FMA,
- it is a natural second example after basic vector add.

What to focus on in the code:

- the loop structure is intentionally almost identical to `vector_add_f32`,
- the main conceptual change is the compute intrinsic: `__riscv_vfmacc_vf_f32m1`,
- this is a good example of how many RVV kernels share one skeleton and differ mainly in the operator inside the loop.

What this benchmark adds beyond vector_add:

- one scalar value participates in every lane computation,
- the arithmetic becomes more realistic than pure add,
- the reader can start seeing how an operator changes while the vector-control structure stays stable.

Main intrinsics:

- `__riscv_vsetvl_e32m1`
- `__riscv_vle32_v_f32m1`
- `__riscv_vfmacc_vf_f32m1`
- `__riscv_vse32_v_f32m1`

Step-by-step intrinsic walkthrough:

- `__riscv_vsetvl_e32m1(n - i)`
  - chooses the active lane count for this chunk of the loop.
- `__riscv_vle32_v_f32m1(x + i, vl)`
  - loads the next chunk of `src_x`.
- `__riscv_vle32_v_f32m1(y + i, vl)`
  - loads the matching chunk of `src_y`.
- `__riscv_vfmacc_vf_f32m1(vy, alpha, vx, vl)`
  - computes fused multiply-accumulate lane by lane,
  - in this benchmark the meaning is `dst = y + alpha * x`,
  - with `alpha = 2`, the first lanes become `1 + 2*1`, `2 + 2*2`, `3 + 2*3`, and so on.
- `__riscv_vse32_v_f32m1(out + i, vz, vl)`
  - stores the computed vector chunk into the destination array.
