#!/usr/bin/env bash

cd src
mkdir -p build
cd build

cmake -DCMAKE_INSTALL_PREFIX=install \
  -DCMAKE_BUILD_TYPE=Release \
  `cat ../options.txt` \
  -DBUILD_opencv_world=OFF \
  ..

