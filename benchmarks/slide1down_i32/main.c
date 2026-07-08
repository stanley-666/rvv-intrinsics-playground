/*
 * Demonstrates slide-style data movement on int32 vectors.
 * Each lane takes the next element, and the last lane is filled with zero.
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

static void fill_linear_i32(int32_t *dst, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = (int32_t)(i + 1);
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

static void print_sample(const int32_t *src, const int32_t *out, size_t n) {
  print_array_i32("src", src, n);
  print_array_i32("dst", out, n);
}

static void slide1down_scalar(const int32_t *src, int32_t *out, size_t n) {
  for (size_t i = 0; i + 1 < n; ++i) {
    out[i] = src[i + 1];
  }
  out[n - 1] = 0;
}

static void slide1down_rvv(const int32_t *src, int32_t *out, size_t n) {
  size_t i = 0;
  while (i < n) {
    size_t vl = __riscv_vsetvl_e32m1(n - i);
    vint32m1_t vx = __riscv_vle32_v_i32m1(src + i, vl);
    int32_t tail = (i + vl < n) ? src[i + vl] : 0;
    vint32m1_t vy = __riscv_vslide1down_vx_i32m1(vx, tail, vl);
    __riscv_vse32_v_i32m1(out + i, vy, vl);
    i += vl;
  }
}

int main(void) {
  int32_t src[kLength];
  int32_t out[kLength];
  int32_t golden[kLength];
  uint64_t cycle_start;
  uint64_t cycle_end;

  fill_linear_i32(src, kLength);

  cycle_start = read_cycle();
  for (int iter = 0; iter < BENCHMARK_REPEAT; ++iter) {
    slide1down_rvv(src, out, kLength);
  }
  cycle_end = read_cycle();
  slide1down_scalar(src, golden, kLength);

  print_result_header("slide1down_i32", kLength);
  print_cycle_stats(cycle_end - cycle_start);
  print_sample(src, out, kLength);
  if (!check_equal_i32(out, golden, kLength)) {
    puts("status=FAIL");
    return 1;
  }

  printf("status=PASS checksum=%f\n", checksum_i32_as_f32(out, kLength));
  return 0;
}
