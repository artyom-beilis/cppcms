#!/usr/bin/env bash 


cmake $FLAGS -DCMAKE_CXX_FLAGS=-std=c++0x -DCMAKE_INCLUDE_PATH=$HOME/gcc/include -DCMAKE_LIBRARY_PATH=$HOME/gcc/lib -DDISABLE_STATIC=ON .. || exit 1
make  || exit 1
ctest || exit 1
