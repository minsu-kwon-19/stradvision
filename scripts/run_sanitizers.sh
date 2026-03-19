#!/bin/bash
set -ex
mkdir -p build_san
cd build_san
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -fsanitize=undefined -g" -DCMAKE_BUILD_TYPE=Debug ..
make -j4
ctest --output-on-failure
