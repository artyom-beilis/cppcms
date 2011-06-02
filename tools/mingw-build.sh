#!/usr/bin/env bash 

cd e:/tmp
rm -fr faults
mkdir faults
rm -f report.txt

FLAGS=

win_gcc()
{
	export PATH=/e/msys/bin:/e/mingw/bin:/e/tmp/nb/build:/e/tmp/nb/build/booster:/e/projects/mingw/lib:$PATH
	cd /e/tmp/nb
	rm -fr build
	mkdir build
	cd build

	if cmake $FLAGS \
		-G "MSYS Makefiles" \
		-DCMAKE_INCLUDE_PATH=e:/projects/mingw/include \
		-DCMAKE_LIBRARY_PATH=e:/projects/mingw/lib \
		-DDISABLE_STATIC=ON .. \
		-DCMAKE_BUILD_TYPE=Debug \
		&& make && ctest -E icu_vs_os_timezone
	then
		return 0;
	else
		if [ -e Testing/Temporary/LastTest.log ]
		then
			cat Testing/Temporary/LastTest.log
		fi
		return 1
	fi

}

win_msvc()
{
	export PATH=/e/tmp/nb/build:/e/tmp/nb/build/booster:/e/projects/msvc9/packages/release/bin:$PATH
	cd /e/tmp/nb
	rm -fr build
	mkdir build
	cd build

	if cmake $FLAGS \
		-G "NMake Makefiles" \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DCMAKE_INCLUDE_PATH='e:\projects\msvc9\packages\release\include' \
		-DCMAKE_LIBRARY_PATH='e:\projects\msvc9\packages\release\lib' \
		-DDISABLE_STATIC=ON .. \
		&& nmake && ctest -E icu_vs_os_timezone
	then
		return 0;
	else
		if [ -e Testing/Temporary/LastTest.log ]
		then
			cat Testing/Temporary/LastTest.log
		fi
		return 1
	fi
}


for TEST in win_msvc win_gcc 
do
	FILE=/e/tmp/$TEST.txt
	if $TEST &> $FILE 
	then
		echo $TEST - pass >>/e/tmp/report.txt
	else
		echo $TEST - fail >>/e/tmp/report.txt
	fi
	cp $FILE /e/tmp/faults/
done


cp /e/tmp/report.txt /e/tmp/faults


