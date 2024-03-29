#!/usr/bin/env bash

set -euo pipefail

if [[ ! -d out ]]; then
    PKG_CONFIG_PATH=./libelfin/elf:./libelfin/dwarf \
    CC=clang \
    CXX=clang++ \
        meson setup out
fi
meson install -C out --only-changed

./packaged/bin/elfy-test ./packaged/bin/elfy-test
