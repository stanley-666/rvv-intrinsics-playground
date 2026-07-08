# mask_select_i32

This benchmark demonstrates compare, mask generation, and masked selection.

What it does:

- compares two `int32` input vectors lane by lane,
- builds a mask from `a < b`,
- selects one lane from `a` or `b` based on that mask.

What the printed output is trying to teach:

- `src_a` and `src_b` are shown before the operator,
- `dst` shows which lane survived after the compare + mask + merge decision,
- with the simple input pattern in this repo, the output makes the lane-wise selection visible immediately.

Why it exists:

- it introduces a control-style intrinsic pattern instead of pure arithmetic,
- it shows how mask types appear in RVV code,
- it is a simple entry point for masked execution.

What to focus on in the code:

- the compare intrinsic creates a mask rather than a normal data vector,
- the merge intrinsic uses that mask to choose between two candidate vectors,
- this benchmark is about control flow at lane granularity, not about arithmetic complexity.

Why this benchmark matters:

- many real vector kernels need conditions, not only math,
- mask types are one of the first RVV concepts that feel different from scalar C,
- this example keeps that idea small enough to inspect by eye from the printed arrays.

Main intrinsics:

- `__riscv_vsetvl_e32m1`
- `__riscv_vle32_v_i32m1`
- `__riscv_vmslt_vv_i32m1_b32`
- `__riscv_vmerge_vvm_i32m1`
- `__riscv_vse32_v_i32m1`

Step-by-step intrinsic walkthrough:

- `__riscv_vsetvl_e32m1(n - i)`
  - chooses the active lane count for this chunk.
- `__riscv_vle32_v_i32m1(a + i, vl)`
  - loads the next `int32` chunk from `src_a`.
- `__riscv_vle32_v_i32m1(b + i, vl)`
  - loads the matching `int32` chunk from `src_b`.
- `__riscv_vmslt_vv_i32m1_b32(va, vb, vl)`
  - compares `va < vb` lane by lane,
  - the result is not an `int32` vector but a mask vector that says which lanes are true.
- `__riscv_vmerge_vvm_i32m1(vb, va, mask, vl)`
  - chooses between `vb` and `va` for each lane,
  - if the mask lane is true, the result lane comes from `va`; otherwise it comes from `vb`.
- `__riscv_vse32_v_i32m1(out + i, vo, vl)`
  - stores the selected result lanes to the destination array.
