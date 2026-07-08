/*
 * Demonstrates the most basic RVV pointwise kernel:
 * load two float32 vectors, add them, and store the result.
 * This is the first benchmark to read in this repo.
 */
#include <riscv_vector.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

enum { kLength = 256 };
enum { kPrintCount = 8 };

static size_t current_vlen_bytes(void) { return __riscv_vlenb(); }

static uint64_t read_cycle(void) {
  uint64_t value;
  __asm__ volatile("rdcycle %0" : "=r"(value));
  return value;
}

static void fill_linear_f32(float *dst, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = (float)(i + 1);
  }
}

static int check_close_f32(const float *lhs, const float *rhs, size_t n, float tolerance) {
  for (size_t i = 0; i < n; ++i) {
    float diff = fabsf(lhs[i] - rhs[i]);
    if (diff > tolerance) {
      printf("mismatch at %zu: lhs=%f rhs=%f diff=%f\n", i, lhs[i], rhs[i], diff);
      return 0;
    }
  }
  return 1;
}

static float checksum_f32(const float *src, size_t n) {
  float sum = 0.0f;
  for (size_t i = 0; i < n; ++i) {
    sum += src[i];
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

static void print_array_f32(const char *label, const float *src, size_t n) {
  size_t count = n < kPrintCount ? n : kPrintCount;
  printf("%s=[", label);
  for (size_t i = 0; i < count; ++i) {
    printf("%s%.2f", i == 0 ? "" : ", ", src[i]);
  }
  printf("]\n");
}

static void print_sample(const float *a, const float *b, const float *out, size_t n) {
  print_array_f32("src_a", a, n);
  print_array_f32("src_b", b, n);
  print_array_f32("dst", out, n);
}

static void vector_add_scalar(const float *a, const float *b, float *c, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

static void vector_add_rvv(const float *a, const float *b, float *c, size_t n) {
  size_t i = 0;
  while (i < n) {
    size_t vl = __riscv_vsetvl_e32m1(n - i);
    vfloat32m1_t va = __riscv_vle32_v_f32m1(a + i, vl);
    vfloat32m1_t vb = __riscv_vle32_v_f32m1(b + i, vl);
    vfloat32m1_t vc = __riscv_vfadd_vv_f32m1(va, vb, vl);
    __riscv_vse32_v_f32m1(c + i, vc, vl);
    i += vl;
  }
}

int main(void) {
  float a[kLength];
  float b[kLength];
  float out[kLength];
  float golden[kLength];
  uint64_t cycle_start;
  uint64_t cycle_end;

  fill_linear_f32(a, kLength);
  fill_linear_f32(b, kLength);

  cycle_start = read_cycle();
  for (int iter = 0; iter < BENCHMARK_REPEAT; ++iter) {
    vector_add_rvv(a, b, out, kLength);
  }
  cycle_end = read_cycle();
  vector_add_scalar(a, b, golden, kLength);

  print_result_header("vector_add_f32", kLength);
  print_cycle_stats(cycle_end - cycle_start);
  print_sample(a, b, out, kLength);
  if (!check_close_f32(out, golden, kLength, 1e-5f)) {
    puts("status=FAIL");
    return 1;
  }

  printf("status=PASS checksum=%f\n", checksum_f32(out, kLength));
  return 0;
}
