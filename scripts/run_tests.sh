#!/bin/bash
gcc --version
clang --version
cat /etc/os-release

ls -lah
mkdir build_gcc
mkdir build_clang

cd build_gcc
cmake -DCMAKE_BUILD_TYPE="Release" -DBUILD_FAST="ON" ..
make

ctest --verbose

cd ../build_clang
export CC=/usr/bin/clang && cmake ..
make

ctest --verbose
