#!/usr/bin/env bash

set -euo pipefail

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
  echo "Usage: $0 <benchmark-name> [build-dir]" >&2
  echo "Example: $0 saxpy_f32" >&2
  exit 1
fi

BENCHMARK_NAME="$1"
BUILD_DIR="${2:-build}"
ISA_STRING="${ISA_STRING:-rv64gcv_zicntr_zihpm}"
BINARY_PATH="${BUILD_DIR}/benchmarks/${BENCHMARK_NAME}/${BENCHMARK_NAME}"
CHIPYARD_DIR="${CHIPYARD_DIR:-/home/stanley/chipyard_1.13.0/chipyard}"

resolve_from_path_or_candidates() {
  local tool_name="$1"
  shift

  if command -v "$tool_name" >/dev/null 2>&1; then
    command -v "$tool_name"
    return 0
  fi

  for candidate in "$@"; do
    if [ -x "$candidate" ]; then
      printf '%s\n' "$candidate"
      return 0
    fi
  done

  return 1
}

SPIKE_BIN="$(resolve_from_path_or_candidates spike \
  "${CHIPYARD_DIR}/.conda-env/riscv-tools/bin/spike" \
  "${CHIPYARD_DIR}/.conda-env/riscv-tools/riscv-isa-sim/bin/spike" \
  "${CHIPYARD_DIR}/toolchains/riscv-tools/riscv-isa-sim/build/spike")" || {
  echo "spike not found in PATH or known Chipyard locations." >&2
  echo "Source your environment first, or set CHIPYARD_DIR correctly." >&2
  exit 1
}

PK_BIN="$(resolve_from_path_or_candidates pk \
  "${CHIPYARD_DIR}/.conda-env/riscv-tools/riscv64-unknown-elf/bin/pk" \
  "${CHIPYARD_DIR}/toolchains/riscv-tools/riscv-pk/build/pk")" || {
  echo "pk not found in PATH or known Chipyard locations." >&2
  echo "Source your environment first, or set CHIPYARD_DIR correctly." >&2
  exit 1
}

if [ ! -x "$BINARY_PATH" ]; then
  echo "Benchmark binary not found: $BINARY_PATH" >&2
  echo "Build it first with CMake, for example:" >&2
  echo "  cmake -S . -B ${BUILD_DIR} -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-unknown-linux-gnu-toolchain.cmake" >&2
  echo "  cmake --build ${BUILD_DIR} -j" >&2
  exit 1
fi

echo "using spike: ${SPIKE_BIN}"
echo "using pk: ${PK_BIN}"
exec "${SPIKE_BIN}" --isa="${ISA_STRING}" "${PK_BIN}" "$BINARY_PATH"
