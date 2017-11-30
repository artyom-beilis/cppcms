#!/usr/bin/env bash 

cmake $FLAGS -DCMAKE_INCLUDE_PATH=/opt/icu46/include -DCMAKE_LIBRARY_PATH=/opt/icu46/lib -DCMAKE_CXX_FLAGS:STRING=-I/opt/icu46/include -DDISABLE_STATIC=ON .. || exit 1
make -j 4 || exit 1
ctest || exit 1
