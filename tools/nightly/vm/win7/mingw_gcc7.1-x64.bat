set PATH=%PATH%;c:\msys64\mingw64\bin

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDISABLE_STATIC=ON -DCMAKE_INCLUDE_PATH=c:\msys64\mingw64\include -DCMAKE_LIBRARY_PATH=c:\msys64\mingw64\lib .. || exit 1
mingw32-make -j 4 || exit 1
SET PATH=%PATH%;.\booster\
ctest  || exit 1

