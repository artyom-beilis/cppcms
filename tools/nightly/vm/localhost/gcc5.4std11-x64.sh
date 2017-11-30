cmake -DCMAKE_CXX_FLAGS="-std=c++11" -DDISABLE_STATIC=ON .. || exit 1
make -j 4 || exit 1
ctest  || exit 1
