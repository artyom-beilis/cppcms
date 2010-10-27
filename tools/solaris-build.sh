#!/usr/bin/env bash 

cd /tmp
rm -fr /tmp/faults
mkdir /tmp/faults

rm -f report.txt

FLAGS=

rm -fr /tmp/nb
svn export https://cppcms.svn.sourceforge.net/svnroot/cppcms/framework/trunk nb
cd nb
/usr/gnu/bin/tar -xjf cppcms_boost.tar.bz2

cd /tmp

solaris_gcc()
{
	cd /tmp/nb
	rm -fr build
	mkdir build
	cd build

	if cmake $FLAGS -DCMAKE_INCLUDE_PATH=$HOME/gcc/include -DCMAKE_LIBRARY_PATH=$HOME/gcc/lib -DDISABLE_STATIC=ON .. && make && make test
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

for TEST in solaris_gcc
do
	FILE=/tmp/$TEST.txt
	if $TEST &> $FILE 
	then
		echo $TEST - pass >>/tmp/report.txt
	else
		echo $TEST - fail >>/tmp/report.txt
	fi
	cp $FILE /tmp/faults/
done

cp /tmp/report.txt /tmp/faults


