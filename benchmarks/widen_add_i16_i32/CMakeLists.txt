cmake_minimum_required(VERSION 3.16)

set(RVV_PLAYGROUND_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(widen_add_i16_i32 C)

  set(RVV_ARCH "rv64gcv_zicntr_zihpm" CACHE STRING "RISC-V ISA string passed to -march")
  set(RVV_ABI "lp64d" CACHE STRING "RISC-V ABI string passed to -mabi")
  set(RVV_OPT_LEVEL "3" CACHE STRING "Optimization level")
  set(BENCHMARK_REPEAT "1024" CACHE STRING "How many times each kernel is repeated")
  set(RVV_LINK_STATIC ON CACHE BOOL "Link statically so binaries can run on spike pk")
endif()

if(NOT CMAKE_C_COMPILER_TARGET AND NOT CMAKE_C_COMPILER MATCHES "riscv64-.*-gcc$")
  message(FATAL_ERROR
    "This benchmark must be configured with a RISC-V cross compiler. "
    "Current C compiler is '${CMAKE_C_COMPILER}'. "
    "Use a fresh build directory and pass "
    "-DCMAKE_TOOLCHAIN_FILE=../../cmake/riscv64-unknown-linux-gnu-toolchain.cmake."
  )
endif()

set(RVV_COMMON_FLAGS
  -O${RVV_OPT_LEVEL}
  -Wall
  -Wextra
  -Wpedantic
  -march=${RVV_ARCH}
  -mabi=${RVV_ABI}
)

include(${RVV_PLAYGROUND_ROOT_DIR}/cmake/benchmark-common.cmake)
rvv_add_benchmark(widen_add_i16_i32 "${CMAKE_CURRENT_LIST_DIR}/main.c")
