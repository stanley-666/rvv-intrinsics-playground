# reduce_sum_f32

This benchmark demonstrates vector reduction.

What it does:

- reads a `float32` input vector,
- accumulates all lanes into one scalar sum,
- compares the RVV result against a scalar reference.

What the printed output is trying to teach:

- `src` shows the original array,
- `dst_sum` shows that the result is no longer another vector but one scalar,
- this is the first benchmark where the output shape changes.

Why it exists:

- it is the first benchmark here where the output is not another vector buffer,
- it shows how reduction changes the loop pattern,
- it is useful after the reader already understands load/compute/store kernels.

What to focus on in the code:

- the loop still uses `vsetvl` and `vle32`, so the front half looks familiar,
- the important change is that each chunk is folded into an accumulator with `vfredusum`,
- the final scalar result is extracted from the vector accumulator at the end.

Why this benchmark matters:

- many useful vector kernels eventually need a scalar answer,
- reduction is a common point where readers realize RVV is not only about producing another full vector,
- it is the cleanest introduction in this repo to the idea of vector accumulation.

Main intrinsics:

- `__riscv_vsetvl_e32m1`
- `__riscv_vle32_v_f32m1`
- `__riscv_vfredusum_vs_f32m1_f32m1`
- `__riscv_vfmv_f_s_f32m1_f32`

Step-by-step intrinsic walkthrough:

- `__riscv_vfmv_v_f_f32m1(0.0f, 1)`
  - creates the initial accumulator vector and seeds lane 0 with `0.0`,
  - this is the starting value for the reduction.
- `__riscv_vsetvl_e32m1(n - i)`
  - chooses how many `float32` lanes to load in this iteration.
- `__riscv_vle32_v_f32m1(x + i, vl)`
  - loads the next chunk of the source array.
- `__riscv_vfredusum_vs_f32m1_f32m1(vx, vacc, vl)`
  - reduces the newly loaded vector chunk into the accumulator,
  - conceptually this folds many lanes into one running sum.
- `__riscv_vfmv_f_s_f32m1_f32(vacc)`
  - extracts the scalar result from the accumulator vector after the loop finishes,
  - this becomes the printed `dst_sum`.
