# scatter_i32

This benchmark demonstrates indexed scatter store.

What it does:

- loads a source `int32` vector,
- loads a vector of destination indices,
- stores each lane into an output array position chosen by that index vector.

What the printed output is trying to teach:

- `src` is the original value vector,
- `scatter_idx` shows the destination positions,
- `dst` shows how the output array changes after indexed stores land in those positions.

Why it exists:

- it is the write-side counterpart to `gather_i32`,
- it shows indexed memory access where lanes do not stay in sequential order,
- it makes scatter behavior visible by reversing the destination order.

What to focus on in the code:

- the source values stay packed in one vector, but the stores do not go to contiguous output positions,
- `vsuxei32` is the key intrinsic because the index vector controls where each lane is written,
- unlike gather, this benchmark changes an output array layout rather than reading a rearranged vector from one source vector.

What `vsuxei32` means here:

- `v`: vector instruction
- `s`: store
- `uxei32`: unordered indexed store, where each index element is 32 bits wide

In this benchmark, the call

```c
__riscv_vsuxei32_v_i32m1(out, vi, vs, vl);
```

means:

- `out` is the base address of the destination array,
- `vi` is the vector of byte offsets,
- `vs` is the vector of values to write,
- each lane stores one value to `out + vi[lane]`.

So if:

- `src = [1, 2, 3, 4, ...]`
- `scatter_idx = [7, 6, 5, 4, ...]`

then the first few lanes write:

- lane 0: write `1` to destination element 7
- lane 1: write `2` to destination element 6
- lane 2: write `3` to destination element 5
- lane 3: write `4` to destination element 4

That is why the output array looks reordered after the scatter.

Why this benchmark matters:

- scatter is a natural complement to gather when teaching indexed access,
- it shows the write-side cost and behavior of irregular access patterns,
- the reversed destination order makes the effect easy to verify from the printed before/after arrays.

Main intrinsics:

- `__riscv_vsetvl_e32m1`
- `__riscv_vle32_v_i32m1`
- `__riscv_vle32_v_u32m1`
- `__riscv_vsuxei32_v_i32m1`

Step-by-step intrinsic walkthrough:

- `__riscv_vsetvl_e32m1(n - i)`
  - chooses the active lane count for this chunk.
- `__riscv_vle32_v_i32m1(src + i, vl)`
  - loads the vector of source values that will be written out.
- `__riscv_vle32_v_u32m1(idx_bytes + i, vl)`
  - loads the vector of byte offsets that specify where each lane should be stored.
- `__riscv_vsuxei32_v_i32m1(out, vi, vs, vl)`
  - performs the indexed scatter store,
  - each lane writes one value from `vs` into `out + vi[lane]`,
  - unlike a normal contiguous store, neighboring lanes may go to unrelated output positions.
