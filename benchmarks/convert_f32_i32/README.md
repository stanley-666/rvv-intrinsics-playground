# convert_f32_i32

This benchmark demonstrates type conversion from floating-point to integer.

What it does:

- loads a `float32` vector,
- converts each lane to `int32`,
- stores the converted result.

What the printed output is trying to teach:

- `src` shows the original floating-point values,
- `dst` shows the converted integer values,
- the reader should focus on how the data type changes even though the number of lanes stays the same.

Why it exists:

- it introduces conversion intrinsics as a class separate from arithmetic,
- it is useful when explaining how type changes happen without widening add or reduction,
- it provides a minimal float/int conversion example.

What to focus on in the code:

- this benchmark is not about changing memory order or reducing lanes,
- the key operation is `vfcvt`, which changes the interpretation and representation of each lane,
- the scalar reference is there to make the conversion semantics explicit and checkable.

Why this benchmark matters:

- conversion is common in quantization, post-processing, and mixed-type pipelines,
- it teaches that RVV kernels are not limited to add/mul style operators,
- it is the cleanest type-conversion example in this repo.

Main intrinsics:

- `__riscv_vsetvl_e32m1`
- `__riscv_vle32_v_f32m1`
- `__riscv_vfcvt_x_f_v_i32m1`
- `__riscv_vse32_v_i32m1`

Step-by-step intrinsic walkthrough:

- `__riscv_vsetvl_e32m1(n - i)`
  - chooses how many `float32` lanes will be processed in this iteration.
- `__riscv_vle32_v_f32m1(src + i, vl)`
  - loads the next chunk of floating-point source values.
- `__riscv_vfcvt_x_f_v_i32m1(vf, vl)`
  - converts each floating-point lane into a signed `int32` lane,
  - this is the key type-conversion step in the benchmark.
- `__riscv_vse32_v_i32m1(out + i, vi, vl)`
  - stores the converted integer lanes to the destination array.
