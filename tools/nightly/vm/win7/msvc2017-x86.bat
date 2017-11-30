@call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars32.bat"

cmake -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDISABLE_STATIC=ON -DCMAKE_INCLUDE_PATH=c:\Users\artik\msvc2017\x86\Release\include -DCMAKE_LIBRARY_PATH=c:\Users\artik\msvc2017\x86\Release\lib .. || exit 1
ninja || exit 1
SET PATH=%PATH%;c:\Users\artik\msvc2017\x86\Release\bin;.\booster\
ctest || exit 1

