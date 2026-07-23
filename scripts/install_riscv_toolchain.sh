#!/usr/bin/env bash

set -euo pipefail

RISCV_PREFIX="${RISCV_PREFIX:-/opt/RISCV/riscv64-gcv}"
RISCV_ARCH="${RISCV_ARCH:-rv64gcv_zicntr_zihpm}"
RISCV_ABI="${RISCV_ABI:-lp64d}"
TOOLCHAIN_REPO="${TOOLCHAIN_REPO:-https://github.com/riscv/riscv-gnu-toolchain}"
TOOLCHAIN_SRC_DIR="${TOOLCHAIN_SRC_DIR:-$HOME/riscv-gnu-toolchain}"
BUILD_DIR="${BUILD_DIR:-$TOOLCHAIN_SRC_DIR/build}"
JOBS="${JOBS:-$(nproc)}"

install_prereqs() {
  sudo apt-get update
  sudo apt-get install -y \
    autoconf automake autotools-dev curl python3 python3-pip \
    libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex \
    texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev \
    ninja-build git cmake libglib2.0-dev libslirp-dev libncurses-dev

  if ! python3 -c 'import tomli' >/dev/null 2>&1; then
    if apt-cache show python3-tomli >/dev/null 2>&1; then
      sudo apt-get install -y python3-tomli
    else
      python3 -m pip install --user tomli
    fi
  fi
}

check_existing_toolchain() {
  if command -v riscv64-unknown-linux-gnu-gcc >/dev/null 2>&1; then
    echo "Found existing toolchain: $(command -v riscv64-unknown-linux-gnu-gcc)"
    riscv64-unknown-linux-gnu-gcc --version
    return 0
  fi
  return 1
}

clone_toolchain_repo() {
  if [ ! -d "$TOOLCHAIN_SRC_DIR/.git" ]; then
    git clone "$TOOLCHAIN_REPO" "$TOOLCHAIN_SRC_DIR"
  fi
}

build_toolchain() {
  mkdir -p "$BUILD_DIR"
  mkdir -p "$RISCV_PREFIX"

  cd "$BUILD_DIR"
  ../configure \
    --prefix="$RISCV_PREFIX" \
    --with-arch="$RISCV_ARCH" \
    --with-abi="$RISCV_ABI" \
    --enable-linux \
    --disable-multilib

  make linux -j"$JOBS"
}

print_env_hint() {
  cat <<EOF

Toolchain installed under:
  $RISCV_PREFIX

Add this to your shell startup file if needed:
  export RISCV="$RISCV_PREFIX"
  export PATH="\$RISCV/bin:\$PATH"

Quick checks:
  riscv64-unknown-linux-gnu-gcc --version
  find "$RISCV_PREFIX" -name riscv_vector.h
EOF
}

main() {
  install_prereqs

  if check_existing_toolchain; then
    print_env_hint
    exit 0
  fi

  clone_toolchain_repo
  build_toolchain
  print_env_hint
}

main "$@"
