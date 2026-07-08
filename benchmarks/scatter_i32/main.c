/*
 * Demonstrates indexed scatter store on int32 vectors.
 * Values from the source vector are written into output positions selected by an index vector.
 */
#include <riscv_vector.h>
#include <stdint.h>
#include <stdio.h>

enum { kLength = 16 };
enum { kPrintCount = 16 };

static size_t current_vlen_bytes(void) { return __riscv_vlenb(); }

static uint64_t read_cycle(void) {
  uint64_t value;
  __asm__ volatile("rdcycle %0" : "=r"(value));
  return value;
}

static void fill_linear_i32(int32_t *dst, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = (int32_t)(i + 1);
  }
}

static void fill_reverse_byte_indices(uint32_t *dst, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = (uint32_t)((n - 1 - i) * sizeof(int32_t));
  }
}

static void fill_zero_i32(int32_t *dst, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = 0;
  }
}

static int check_equal_i32(const int32_t *lhs, const int32_t *rhs, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    if (lhs[i] != rhs[i]) {
      printf("mismatch at %zu: lhs=%d rhs=%d\n", i, lhs[i], rhs[i]);
      return 0;
    }
  }
  return 1;
}

static float checksum_i32_as_f32(const int32_t *src, size_t n) {
  float sum = 0.0f;
  for (size_t i = 0; i < n; ++i) {
    sum += (float)src[i];
  }
  return sum;
}

static void print_result_header(const char *name, size_t n) {
  printf("benchmark=%s length=%zu repeat=%d vlenb=%zu\n", name, n, BENCHMARK_REPEAT,
         current_vlen_bytes());
}

static void print_cycle_stats(uint64_t cycles_total) {
  printf("cycles_total=%llu cycles_per_iter=%.2f\n", (unsigned long long)cycles_total,
         (double)cycles_total / (double)BENCHMARK_REPEAT);
}

static void print_array_i32(const char *label, const int32_t *src, size_t n) {
  size_t count = n < kPrintCount ? n : kPrintCount;
  printf("%s=[", label);
  for (size_t i = 0; i < count; ++i) {
    printf("%s%d", i == 0 ? "" : ", ", src[i]);
  }
  printf("]\n");
}

static void print_array_u32_as_elem_index(const char *label, const uint32_t *src, size_t n) {
  size_t count = n < kPrintCount ? n : kPrintCount;
  printf("%s=[", label);
  for (size_t i = 0; i < count; ++i) {
    printf("%s%u", i == 0 ? "" : ", ", src[i] / (uint32_t)sizeof(int32_t));
  }
  printf("]\n");
}

static void print_sample(const int32_t *src, const uint32_t *idx_bytes, const int32_t *out,
                         size_t n) {
  print_array_i32("src", src, n);
  print_array_u32_as_elem_index("scatter_idx", idx_bytes, n);
  print_array_i32("dst", out, n);
}

static void scatter_scalar(const int32_t *src, const uint32_t *idx_bytes, int32_t *out, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    out[idx_bytes[i] / sizeof(int32_t)] = src[i];
  }
}

static void scatter_rvv(const int32_t *src, const uint32_t *idx_bytes, int32_t *out, size_t n) {
  size_t i = 0;
  while (i < n) {
    size_t vl = __riscv_vsetvl_e32m1(n - i);
    vint32m1_t vs = __riscv_vle32_v_i32m1(src + i, vl);
    vuint32m1_t vi = __riscv_vle32_v_u32m1(idx_bytes + i, vl);
    /* Store each lane of vs to out + byte_offset from the matching lane in vi. */
    __riscv_vsuxei32_v_i32m1(out, vi, vs, vl);
    i += vl;
  }
}

int main(void) {
  int32_t src[kLength];
  uint32_t idx_bytes[kLength];
  int32_t out[kLength];
  int32_t golden[kLength];
  uint64_t cycle_start;
  uint64_t cycle_end;

  fill_linear_i32(src, kLength);
  fill_reverse_byte_indices(idx_bytes, kLength);
  fill_zero_i32(out, kLength);
  fill_zero_i32(golden, kLength);

  cycle_start = read_cycle();
  for (int iter = 0; iter < BENCHMARK_REPEAT; ++iter) {
    scatter_rvv(src, idx_bytes, out, kLength);
  }
  cycle_end = read_cycle();
  scatter_scalar(src, idx_bytes, golden, kLength);

  print_result_header("scatter_i32", kLength);
  print_cycle_stats(cycle_end - cycle_start);
  print_sample(src, idx_bytes, out, kLength);
  if (!check_equal_i32(out, golden, kLength)) {
    puts("status=FAIL");
    return 1;
  }

  printf("status=PASS checksum=%f\n", checksum_i32_as_f32(out, kLength));
  return 0;
}
