#ifndef RVV_PLAYGROUND_BENCH_UTILS_H
#define RVV_PLAYGROUND_BENCH_UTILS_H

#include <stddef.h>
#include <stdint.h>

size_t current_vlen_bytes(void);
void fill_linear_f32(float *dst, size_t n, float base, float step);
int check_close_f32(const float *lhs, const float *rhs, size_t n, float tolerance);
float checksum_f32(const float *src, size_t n);
void fill_linear_i32(int32_t *dst, size_t n, int32_t base, int32_t step);
void fill_linear_i16(int16_t *dst, size_t n, int16_t base, int16_t step);
int check_equal_i32(const int32_t *lhs, const int32_t *rhs, size_t n);
float checksum_i32_as_f32(const int32_t *src, size_t n);
void print_result_header(const char *name, size_t n);

#endif
