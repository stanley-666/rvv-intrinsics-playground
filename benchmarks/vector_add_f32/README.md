# vector_add_f32

This benchmark is the first walkthrough target in this repo.

What it does:

- loads two `float32` vectors,
- adds them lane by lane,
- stores the result to an output vector.

What the printed output is trying to teach:

- `src_a` and `src_b` are the original input arrays,
- `dst` is the array after the RVV operator runs,
- because both inputs are `1, 2, 3, 4, ...`, the reader can immediately see that the output becomes `2, 4, 6, 8, ...`.

Why it exists:

- it is the smallest complete RVV example in this repo,
- it introduces `vsetvl`, vector load, arithmetic, and vector store,
- it is the easiest place to compare scalar code against RVV code.

What to focus on in the code:

- the `while (i < n)` loop is the standard RVV loop shape,
- `__riscv_vsetvl_e32m1(...)` chooses how many lanes this iteration will process,
- `vle32 -> vfadd -> vse32` is the simplest possible load -> compute -> store pipeline,
- the scalar version and RVV version do exactly the same operation, only the implementation style differs.

Why this benchmark should be read first:

- there are no masks, reductions, or mixed-width types yet,
- the output transformation is visually obvious from the printed arrays,
- almost every later benchmark in this repo reuses the same loop skeleton.

Main intrinsics:

- `__riscv_vsetvl_e32m1`
- `__riscv_vle32_v_f32m1`
- `__riscv_vfadd_vv_f32m1`
- `__riscv_vse32_v_f32m1`

Step-by-step intrinsic walkthrough:

- `__riscv_vsetvl_e32m1(n - i)`
  - asks the implementation how many `e32, m1` lanes can be processed in this loop iteration,
  - this is why the loop is vector-length agnostic instead of assuming a fixed vector width.
- `__riscv_vle32_v_f32m1(a + i, vl)`
  - loads `vl` lanes of `float32` data from `src_a`,
  - in this benchmark that means reading the next chunk of `1, 2, 3, 4, ...`.
- `__riscv_vle32_v_f32m1(b + i, vl)`
  - loads the matching `vl` lanes from `src_b`,
  - both source vectors stay aligned lane by lane for the later add.
- `__riscv_vfadd_vv_f32m1(va, vb, vl)`
  - performs lane-wise floating-point add,
  - lane 0 computes `a[0] + b[0]`, lane 1 computes `a[1] + b[1]`, and so on.
- `__riscv_vse32_v_f32m1(c + i, vc, vl)`
  - stores the result vector back to memory,
  - this is the step that turns the computed lanes into the printed `dst` array.
