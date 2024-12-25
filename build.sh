#!/bin/bash

mkdir -p build

cmake -S . -B build/ -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cd build/
ninja

cd ..

python3 shader_build.py
