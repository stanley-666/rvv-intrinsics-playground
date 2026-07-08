# widen_add_i16_i32

This benchmark demonstrates widening arithmetic.

What it does:

- loads two `int16` input vectors,
- adds them with widening,
- stores the result as `int32`.

What the printed output is trying to teach:

- `src_a` and `src_b` are narrow input arrays,
- `dst` is the wider output array,
- the reader should notice that the values are easy to understand, but the important structural change is actually the type width.

Why it exists:

- it shows that output element width can differ from input element width,
- it is a clean example of widening semantics before more complex mixed-width kernels,
- it helps connect LMUL and type-width changes to a concrete operator.

What to focus on in the code:

- the loop is configured with `e16`, not `e32`,
- the output vector type is wider than the input vector type,
- `vwadd` is the key intrinsic because it combines arithmetic with width expansion.

Why this benchmark matters:

- mixed-width behavior is one of the core RVV ideas that beginners often miss,
- this example shows width change without adding masking, reduction, or irregular memory access,
- it is a good first example for connecting data type width to vector type naming.

Main intrinsics:

- `__riscv_vsetvl_e16m1`
- `__riscv_vle16_v_i16m1`
- `__riscv_vwadd_vv_i32m2`
- `__riscv_vse32_v_i32m2`

Step-by-step intrinsic walkthrough:

- `__riscv_vsetvl_e16m1(n - i)`
  - chooses the active lane count using `e16`, because the inputs are `int16`.
- `__riscv_vle16_v_i16m1(a + i, vl)`
  - loads the next chunk of `int16` values from `src_a`.
- `__riscv_vle16_v_i16m1(b + i, vl)`
  - loads the matching `int16` chunk from `src_b`.
- `__riscv_vwadd_vv_i32m2(va, vb, vl)`
  - performs widening add,
  - the arithmetic result is promoted from 16-bit inputs to a 32-bit output vector.
- `__riscv_vse32_v_i32m2(out + i, vo, vl)`
  - stores the widened `int32` results to memory.
