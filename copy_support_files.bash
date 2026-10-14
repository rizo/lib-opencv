#!/usr/bin/env bash
set -euo pipefail

rm -rf src/unikraft-support

mkdir src/unikraft-support
mkdir src/unikraft-support/include

# Copy files generated during configure.
cp -r src/opencv2 src/unikraft-support/include
cp src/custom_hal.hpp src/unikraft-support/include
cp src/cv_cpu_config.h src/unikraft-support/include
cp src/cvconfig.h src/unikraft-support/include
cp src/opencv_data_config.hpp src/unikraft-support/include

# Copy unikraft files
cp support/Makefile.bearuk src/Makefile.bearuk
cp support/Makefile.uk src/Makefile.uk
cp support/Config.uk src/Config.uk
cp support/Kraftfile src/Kraftfile

# Copy dummy "linux/auxvec.h".
# Included in "modules/core/src/system.cpp", if __linux__ is defined.
mkdir -p src/unikraft-support/include/linux
cp support/dummy.h src/unikraft-support/include/linux/auxvec.h

# Symlink the full includes from the compiler. See Makefile.uk.
if test "${CC}" = "clang"; then
  CLANG_RESOURCE_DIR="$(clang -print-resource-dir)"
  ln -s "${CLANG_RESOURCE_DIR}/include" src/unikraft-support/compiler-include
elif test "${CC}" = "gcc"; then
  GCC_INCLUDE_DIR="$(gcc -print-file-name=include)"
  ln -s "${GCC_INCLUDE_DIR}" src/unikraft-support/compiler-include

  # Copy shim for older gcc.
  # TODO: Check gcc version first.
  mkdir -p src/unikraft-support/src
  cp support/cxxabi_stub.cpp src/unikraft-support/src/cxxabi_stub.cpp
fi
