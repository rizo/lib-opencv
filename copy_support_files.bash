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
cp support/Makefile.uk src/Makefile.uk
cp support/Kraftfile src/Kraftfile
