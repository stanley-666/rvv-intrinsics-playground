/*
 * Demonstrates an RVV reduction kernel.
 * The vector loop consumes float32 input elements and produces one scalar sum.
 */
#include <riscv_vector.h>
#include <math.h>
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

static void fill_linear_f32(float *dst, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = (float)(i + 1);
  }
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

static void print_sample(const float *x, size_t n, float sum) {
  print_array_f32("src", x, n);
  printf("dst_sum=%f\n", sum);
}

static float reduce_sum_scalar(const float *x, size_t n) {
  float sum = 0.0f;
  for (size_t i = 0; i < n; ++i) {
    sum += x[i];
  }
  return sum;
}

static float reduce_sum_rvv(const float *x, size_t n) {
  size_t i = 0;
  vfloat32m1_t vacc = __riscv_vfmv_v_f_f32m1(0.0f, 1);

  while (i < n) {
    size_t vl = __riscv_vsetvl_e32m1(n - i);
    vfloat32m1_t vx = __riscv_vle32_v_f32m1(x + i, vl);
    vacc = __riscv_vfredusum_vs_f32m1_f32m1(vx, vacc, vl);
    i += vl;
  }

  return __riscv_vfmv_f_s_f32m1_f32(vacc);
}

int main(void) {
  float x[kLength];
  float sum = 0.0f;
  float golden = 0.0f;
  uint64_t cycle_start;
  uint64_t cycle_end;

  fill_linear_f32(x, kLength);

  cycle_start = read_cycle();
  for (int iter = 0; iter < BENCHMARK_REPEAT; ++iter) {
    sum = reduce_sum_rvv(x, kLength);
  }
  cycle_end = read_cycle();
  golden = reduce_sum_scalar(x, kLength);

  print_result_header("reduce_sum_f32", kLength);
  print_cycle_stats(cycle_end - cycle_start);
  print_sample(x, kLength, sum);
  if (((sum - golden) > 1e-3f) || ((golden - sum) > 1e-3f)) {
    printf("status=FAIL sum=%f golden=%f\n", sum, golden);
    return 1;
  }

  printf("status=PASS sum=%f\n", sum);
  return 0;
}
