# slide1down_i32

This benchmark demonstrates slide-style lane movement.

What it does:

- loads an `int32` vector,
- shifts every lane down by one position,
- fills the last lane with a scalar tail value.

What the printed output is trying to teach:

- `src` is the original lane order,
- `dst` is the lane order after the slide operation,
- because the source is `1, 2, 3, 4, ...`, the reader can immediately see the result become `2, 3, 4, 5, ...`.

Why it exists:

- it shows data movement that is not just load/store,
- it is a compact example of lane rearrangement,
- it helps readers see how neighboring-lane operations differ from arithmetic kernels.

What to focus on in the code:

- the interesting intrinsic is `__riscv_vslide1down_vx_i32m1`,
- unlike add or FMA, this operator does not create new numeric values from arithmetic,
- it repositions existing lane values and injects one scalar at the tail.

Why this benchmark matters:

- many vector kernels need lane movement in addition to arithmetic,
- slide operations are a simple way to introduce permutation-like behavior,
- the before/after arrays are especially easy to understand visually in this example.

Main intrinsics:

- `__riscv_vsetvl_e32m1`
- `__riscv_vle32_v_i32m1`
- `__riscv_vslide1down_vx_i32m1`
- `__riscv_vse32_v_i32m1`

Step-by-step intrinsic walkthrough:

- `__riscv_vsetvl_e32m1(n - i)`
  - chooses how many `int32` lanes are active in this iteration.
- `__riscv_vle32_v_i32m1(src + i, vl)`
  - loads the current chunk of the source array into a vector register.
- `__riscv_vslide1down_vx_i32m1(vx, tail, vl)`
  - shifts every lane down by one position,
  - lane 0 takes old lane 1, lane 1 takes old lane 2, and so on,
  - the last lane is filled by the scalar `tail` value.
- `__riscv_vse32_v_i32m1(out + i, vy, vl)`
  - stores the shifted vector chunk to the destination array.
