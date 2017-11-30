cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_CXX_FLAGS="-stdlib=libc++ -I /usr/include/libcxxabi" -DDISABLE_STATIC=ON -DDISABLE_ICU_LOCALE=ON .. || exit 1
make -j 4 || exit 1
ctest  || exit 1
