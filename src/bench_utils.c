#include "rvv_playground/bench_utils.h"

#include <math.h>
#include <stdint.h>
#include <riscv_vector.h>
#include <stdio.h>

size_t current_vlen_bytes(void) {
  return __riscv_vlenb();
}

void fill_linear_f32(float *dst, size_t n, float base, float step) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = base + step * (float)i;
  }
}

int check_close_f32(const float *lhs, const float *rhs, size_t n, float tolerance) {
  for (size_t i = 0; i < n; ++i) {
    float diff = fabsf(lhs[i] - rhs[i]);
    if (diff > tolerance) {
      printf("mismatch at %zu: lhs=%f rhs=%f diff=%f\n", i, lhs[i], rhs[i], diff);
      return 0;
    }
  }
  return 1;
}

float checksum_f32(const float *src, size_t n) {
  float sum = 0.0f;
  for (size_t i = 0; i < n; ++i) {
    sum += src[i];
  }
  return sum;
}

void fill_linear_i32(int32_t *dst, size_t n, int32_t base, int32_t step) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = base + step * (int32_t)i;
  }
}

void fill_linear_i16(int16_t *dst, size_t n, int16_t base, int16_t step) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = (int16_t)(base + step * (int16_t)i);
  }
}

int check_equal_i32(const int32_t *lhs, const int32_t *rhs, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    if (lhs[i] != rhs[i]) {
      printf("mismatch at %zu: lhs=%d rhs=%d\n", i, lhs[i], rhs[i]);
      return 0;
    }
  }
  return 1;
}

float checksum_i32_as_f32(const int32_t *src, size_t n) {
  float sum = 0.0f;
  for (size_t i = 0; i < n; ++i) {
    sum += (float)src[i];
  }
  return sum;
}

void print_result_header(const char *name, size_t n) {
  printf("benchmark=%s length=%zu repeat=%d vlenb=%zu\n", name, n, BENCHMARK_REPEAT,
         current_vlen_bytes());
}
