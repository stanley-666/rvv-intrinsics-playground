cmake_minimum_required(VERSION 3.16)

project(rvv_intrinsics_playground C)

set(RVV_PLAYGROUND_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_C_COMPILER_TARGET AND NOT CMAKE_C_COMPILER MATCHES "riscv64-.*-gcc$")
  message(FATAL_ERROR
    "This project must be configured with a RISC-V cross compiler. "
    "Current C compiler is '${CMAKE_C_COMPILER}'. "
    "Use a fresh build directory and pass "
    "-DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-unknown-linux-gnu-toolchain.cmake."
  )
endif()

set(RVV_ARCH "rv64gcv_zicntr_zihpm" CACHE STRING "RISC-V ISA string passed to -march")
set(RVV_ABI "lp64d" CACHE STRING "RISC-V ABI string passed to -mabi")
set(RVV_OPT_LEVEL "3" CACHE STRING "Optimization level")
set(RVV_LINK_STATIC ON CACHE BOOL "Link statically so binaries can run on spike pk")
set(BENCHMARK_REPEAT "1024" CACHE STRING "How many times each kernel is repeated")
set(RVV_BENCHMARKS "vector_add_f32;mul_f32;saxpy_f32;reduce_sum_f32;mask_select_i32;slide1down_i32;gather_i32;scatter_i32;widen_add_i16_i32;convert_f32_i32;strided_load_f32" CACHE STRING "Semicolon-separated list of benchmarks to build")

set(RVV_COMMON_FLAGS
  -O${RVV_OPT_LEVEL}
  -Wall
  -Wextra
  -Wpedantic
  -march=${RVV_ARCH}
  -mabi=${RVV_ABI}
)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/benchmark-common.cmake)

foreach(bench IN LISTS RVV_BENCHMARKS)
  set(bench_dir "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/${bench}")
  if(EXISTS "${bench_dir}/CMakeLists.txt")
    add_subdirectory("${bench_dir}")
  else()
    message(FATAL_ERROR "Requested benchmark directory does not exist: ${bench_dir}")
  endif()
endforeach()

message(STATUS "RVV_ARCH=${RVV_ARCH}")
message(STATUS "RVV_ABI=${RVV_ABI}")
message(STATUS "RVV_BENCHMARKS=${RVV_BENCHMARKS}")
