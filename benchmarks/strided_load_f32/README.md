# strided_load_f32

This benchmark demonstrates non-unit-stride memory access.

What it does:

- reads a larger `float32` source buffer,
- loads every second element with a strided RVV load,
- writes the gathered logical vector into a dense output buffer.

What the printed output is trying to teach:

- `src_stride2` shows the source elements that the strided load will actually touch,
- `dst` shows the dense result after those spaced-apart values are packed together,
- the reader can see that the data path is regular in the output even though the input access pattern is not contiguous.

Why it exists:

- it shows that RVV memory access patterns are not limited to contiguous loads,
- it is a compact introduction to stride-in-bytes semantics,
- it helps readers reason about vector code that touches sparse or interleaved layouts.

What to focus on in the code:

- the stride is expressed in bytes, not in element count,
- the output array remains dense even though the source access is strided,
- this example is about memory pattern control more than arithmetic.

Why this benchmark matters:

- real data layouts are often interleaved or padded,
- a strided load is one of the first non-trivial memory operations readers should see,
- this example keeps the arithmetic simple so the memory access pattern is the only new idea.

Main intrinsics:

- `__riscv_vsetvl_e32m1`
- `__riscv_vlse32_v_f32m1`
- `__riscv_vse32_v_f32m1`

Step-by-step intrinsic walkthrough:

- `__riscv_vsetvl_e32m1(n - i)`
  - chooses the active lane count for this chunk.
- `__riscv_vlse32_v_f32m1(src + i * 2, stride_bytes, vl)`
  - loads `float32` lanes from memory with a fixed byte stride between lanes,
  - in this benchmark the stride is `sizeof(float) * 2`, so the load touches every second element.
- `__riscv_vse32_v_f32m1(out + i, vf, vl)`
  - stores the stridedly loaded values into a dense destination vector.
