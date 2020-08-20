#!/bin/bash

if [ ! -d "build" ]; then
  mkdir build
fi

cd build
rm -rf *

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake .. -DX86=ON
make -j4