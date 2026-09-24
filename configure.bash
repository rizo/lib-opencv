#!/usr/bin/env bash

cd src

cmake \
  -DCMAKE_INSTALL_PREFIX=install \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_opencv_world=OFF \
  `cat ../options.txt` \
  .

