#!/usr/bin/env bash
set -euo pipefail

rm -rf src/unikraft-support

mkdir src/unikraft-support
mkdir src/unikraft-support/include

cp -r src/opencv2 ./src/unikraft-support/include
cp src/custom_hal.hpp ./src/unikraft-support/include
cp src/cv_cpu_config.h ./src/unikraft-support/include
cp src/cvconfig.h ./src/unikraft-support/include
cp src/opencv_data_config.hpp ./src/unikraft-support/include

