#!/usr/bin/env bash

mkdir -p backup
mv ./src ./backup/src.$(date +%s)

SRC="https://github.com/nihui/opencv-mobile/releases/latest/download/opencv-mobile-5.0.0.zip"

mkdir fetch_tmp
cd fetch_tmp

wget "$SRC"
mv * src.zip
unzip src.zip
rm src.zip
mv * src
mv src ../src

cd ..
rmdir fetch_tmp
