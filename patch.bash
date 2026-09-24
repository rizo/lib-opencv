#!/usr/bin/env bash
set -euo pipefail

if ! grep -q "PATCH: enable_insource_build.patch" src/CMakeLists.txt; then
  patch -p1 < patches/enable_insource_build.patch
fi
