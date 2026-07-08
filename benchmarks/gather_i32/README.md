# gather_i32

This benchmark demonstrates indexed gather / permutation.

What it does:

- loads an `int32` source vector,
- loads a vector of indices,
- gathers lanes according to those indices.

What the printed output is trying to teach:

- `src` is the original data array,
- `idx` shows which lane positions are requested,
- `dst` is the reordered vector produced by that index pattern.

Why it exists:

- it is the first explicit permutation-style example in this repo,
- it shows how one vector can control lane selection in another vector,
- it is a useful bridge from simple arithmetic kernels to more irregular data access.

What to focus on in the code:

- one vector carries data and another vector carries the selection pattern,
- `vrgather` is not a memory load from arbitrary addresses here; it is lane selection from a source vector,
- the main teaching point is that vector lanes do not have to stay in their original order.

Why this benchmark matters:

- gather-like behavior shows up in lookup, permutation, and rearrangement workloads,
- it makes readers confront the difference between contiguous processing and indexed selection,
- it pairs naturally with `scatter_i32`, which shows the write-side counterpart.

Main intrinsics:

- `__riscv_vsetvl_e32m1`
- `__riscv_vle32_v_i32m1`
- `__riscv_vle32_v_u32m1`
- `__riscv_vrgather_vv_i32m1`
- `__riscv_vse32_v_i32m1`

Step-by-step intrinsic walkthrough:

- `__riscv_vsetvl_e32m1(n - i)`
  - chooses the active lane count for this chunk.
- `__riscv_vle32_v_i32m1(src + i, vl)`
  - loads the source data lanes that can be selected from in this chunk.
- `__riscv_vle32_v_u32m1(idx + i, vl)`
  - loads the index vector that tells each lane which source lane to pick.
- `__riscv_vrgather_vv_i32m1(vs, vi, vl)`
  - gathers lanes from `vs` according to the per-lane indices in `vi`,
  - if an index lane is `3`, that result lane takes the value from source lane 3 of the current vector chunk.
- `__riscv_vse32_v_i32m1(out + i, vo, vl)`
  - stores the gathered, reordered lanes to the destination array.
