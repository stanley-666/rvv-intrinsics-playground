/*
 * Demonstrates widening arithmetic.
 * Two int16 input vectors are added into an int32 output vector.
 */
#include <riscv_vector.h>
#include <stdint.h>
#include <stdio.h>

enum { kLength = 128 };
enum { kPrintCount = 8 };

static size_t current_vlen_bytes(void) { return __riscv_vlenb(); }

static uint64_t read_cycle(void) {
  uint64_t value;
  __asm__ volatile("rdcycle %0" : "=r"(value));
  return value;
}

static void fill_linear_i16(int16_t *dst, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = (int16_t)(i + 1);
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

static void print_array_i16(const char *label, const int16_t *src, size_t n) {
  size_t count = n < kPrintCount ? n : kPrintCount;
  printf("%s=[", label);
  for (size_t i = 0; i < count; ++i) {
    printf("%s%d", i == 0 ? "" : ", ", src[i]);
  }
  printf("]\n");
}

static void print_array_i32(const char *label, const int32_t *src, size_t n) {
  size_t count = n < kPrintCount ? n : kPrintCount;
  printf("%s=[", label);
  for (size_t i = 0; i < count; ++i) {
    printf("%s%d", i == 0 ? "" : ", ", src[i]);
  }
  printf("]\n");
}

static void print_sample(const int16_t *a, const int16_t *b, const int32_t *out, size_t n) {
  print_array_i16("src_a", a, n);
  print_array_i16("src_b", b, n);
  print_array_i32("dst", out, n);
}

static void widen_add_scalar(const int16_t *a, const int16_t *b, int32_t *out, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    out[i] = (int32_t)a[i] + (int32_t)b[i];
  }
}

static void widen_add_rvv(const int16_t *a, const int16_t *b, int32_t *out, size_t n) {
  size_t i = 0;
  while (i < n) {
    size_t vl = __riscv_vsetvl_e16m1(n - i);
    vint16m1_t va = __riscv_vle16_v_i16m1(a + i, vl);
    vint16m1_t vb = __riscv_vle16_v_i16m1(b + i, vl);
    vint32m2_t vo = __riscv_vwadd_vv_i32m2(va, vb, vl);
    __riscv_vse32_v_i32m2(out + i, vo, vl);
    i += vl;
  }
}

int main(void) {
  int16_t a[kLength];
  int16_t b[kLength];
  int32_t out[kLength];
  int32_t golden[kLength];
  uint64_t cycle_start;
  uint64_t cycle_end;

  fill_linear_i16(a, kLength);
  fill_linear_i16(b, kLength);

  cycle_start = read_cycle();
  for (int iter = 0; iter < BENCHMARK_REPEAT; ++iter) {
    widen_add_rvv(a, b, out, kLength);
  }
  cycle_end = read_cycle();
  widen_add_scalar(a, b, golden, kLength);

  print_result_header("widen_add_i16_i32", kLength);
  print_cycle_stats(cycle_end - cycle_start);
  print_sample(a, b, out, kLength);
  if (!check_equal_i32(out, golden, kLength)) {
    puts("status=FAIL");
    return 1;
  }

  printf("status=PASS checksum=%f\n", checksum_i32_as_f32(out, kLength));
  return 0;
}
