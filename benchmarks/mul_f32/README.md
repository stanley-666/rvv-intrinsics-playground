# mul_f32

This benchmark demonstrates pointwise floating-point multiply.

What it does:

- loads two `float32` vectors,
- multiplies them lane by lane,
- stores the result vector.

What the printed output is trying to teach:

- `src_a` and `src_b` are the original input arrays,
- `dst` is the output after pointwise multiply,
- because both inputs are `1, 2, 3, 4, ...`, the output becomes `1, 4, 9, 16, ...`.

Why it exists:

- it isolates multiply as its own teaching step instead of hiding it inside FMA,
- it gives readers a direct comparison point between add, multiply, and fused multiply-accumulate,
- it is one of the most common building blocks in vector math kernels.

What to focus on in the code:

- the loop structure is the same as `vector_add_f32`,
- the only real change is the compute intrinsic: `__riscv_vfmul_vv_f32m1`,
- this is useful for teaching that many RVV kernels differ by one operator line.

Why this benchmark matters:

- multiply is the basis of dot product, GEMM, convolution, and MAC-style code,
- it should be understood separately before moving on to fused operators,
- its cycle count can also be compared against add and gather/scatter style operations.

Main intrinsics:

- `__riscv_vsetvl_e32m1`
- `__riscv_vle32_v_f32m1`
- `__riscv_vfmul_vv_f32m1`
- `__riscv_vse32_v_f32m1`

Step-by-step intrinsic walkthrough:

- `__riscv_vsetvl_e32m1(n - i)`
  - chooses the active lane count for this chunk of the loop.
- `__riscv_vle32_v_f32m1(a + i, vl)`
  - loads the next chunk of `src_a`.
- `__riscv_vle32_v_f32m1(b + i, vl)`
  - loads the matching chunk of `src_b`.
- `__riscv_vfmul_vv_f32m1(va, vb, vl)`
  - multiplies the two vectors lane by lane,
  - lane 0 computes `1 * 1`, lane 1 computes `2 * 2`, lane 2 computes `3 * 3`, and so on.
- `__riscv_vse32_v_f32m1(out + i, vc, vl)`
  - stores the multiplied result vector to the destination array.
