#!/usr/bin/env bash
set -euo pipefail

if ! grep -q "PATCH: enable_insource_build" src/CMakeLists.txt; then
  patch -p1 < patches/enable_insource_build.patch
fi

if ! grep -q "PATCH: disable_x86_intrinsics_headers" src/modules/core/include/opencv2/core/fast_math.hpp; then
  patch -p1 < patches/disable_x86_intrinsics_headers.patch
fi
