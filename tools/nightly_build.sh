#!/bin/bash 

ROOT_PATH=`dirname $0`
BSD_PATH=`dirname $0`/bsd-build.sh
SOLARIS_PATH=`dirname $0`/solaris-build.sh

cd /tmp
rm -fr /tmp/faults
mkdir /tmp/faults

rm -f report.txt

FLAGS=

rm -fr /tmp/nb
svn export https://cppcms.svn.sourceforge.net/svnroot/cppcms/framework/trunk nb
cd nb
tar -xjf cppcms_boost.tar.bz2

cd /tmp

gcc_44_stlport()
{
	cd /tmp/nb
	rm -fr build
	mkdir build
	cd build


	if cmake $FLAGS -DUSE_STLPORT=ON -DDISABLE_ICU_LOCALE=ON -DDISABLE_STD_LOCALE=ON -DCMAKE_BUILD_TYPE=Debug -DDISABLE_STATIC=ON .. && make && make test
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
gcc_44()
{
	cd /tmp/nb
	rm -fr build
	mkdir build
	cd build


	if cmake $FLAGS -DDISABLE_STATIC=ON .. && make && make test
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

gcc_45()
{
	source ~/bin/env_gcc

	cd /tmp/nb

	rm -fr build
	mkdir build
	cd build


	if cmake $FLAGS -DDISABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/opt/gcc45/bin/gcc-4.5 -DCMAKE_CXX_COMPILER=/opt/gcc45/bin/g++-4.5 .. && make && make test
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


clangcc()
{
	source ~/bin/env_clang

	cd /tmp/nb
	rm -fr build
	mkdir build
	cd build

	if cmake $FLAGS -DCMAKE_C_COMPILER=`which clang` -DCMAKE_CXX_COMPILER=`which clang++` -DDISABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Debug .. \
		 && make && make test
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


intel()
{
	source ~/bin/env_intel

	cd /tmp/nb
	rm -fr build
	mkdir build
	cd build

	if cmake $FLAGS -DCMAKE_C_COMPILER=`which icc` -DCMAKE_CXX_COMPILER=`which icpc` -DDISABLE_STATIC=ON .. \
		 && make && make test
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

gcc_450x()
{
	source ~/bin/env_gcc

	cd /tmp/nb
	rm -fr build
	mkdir build
	cd build


	if cmake $FLAGS -DDISABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS:STRING=-std=c++0x  -DCMAKE_C_COMPILER=/opt/gcc45/bin/gcc-4.5 -DCMAKE_CXX_COMPILER=/opt/gcc45/bin/g++-4.5 .. && make && make test
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

cd /tmp

#####################
# LINUX 
#####################

for TEST in gcc_44 intel gcc_44_stlport clangcc
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

#####################
# FreeBSD
#####################

VBoxHeadless -s FreeBSD &
PID=$!
sleep 60

scp -P 2223 $BSD_PATH artik@localhost:/tmp
ssh -p 2223 artik@localhost /tmp/bsd-build.sh
scp -P 2223 artik@localhost:/tmp/report.txt /tmp/tmp.txt
cat /tmp/tmp.txt >>/tmp/report.txt
scp -P 2223 'artik@localhost:/tmp/faults/*' /tmp/faults/

VBoxManage controlvm FreeBSD acpipowerbutton
wait $PID


#####################
# Solaris
#####################

VBoxHeadless -s Solaris &
PID=$!
sleep 300

scp -P 2222 $SOLARIS_PATH artik@localhost:/tmp
ssh -p 2222 artik@localhost /tmp/solaris-build.sh
scp -P 2222 artik@localhost:/tmp/report.txt /tmp/tmp.txt
cat /tmp/tmp.txt >>/tmp/report.txt
scp -P 2222 'artik@localhost:/tmp/faults/*' /tmp/faults/

VBoxManage controlvm Solaris acpipowerbutton
wait $PID

#####################
# Debian Armel
#####################

# $ROOT_PATH/arm.sh

#####################
# Windows
#####################

cd /tmp
rm -fr nb/build
rm -f nb.tar.gz
tar -czf nb.tar.gz nb
cp $ROOT_PATH/mingw-build.sh .
smbclient -U guest -N '//192.168.2.100/storage' -c 'put nb.tar.gz'
smbclient -U guest -N '//192.168.2.100/storage' -c 'put mingw-build.sh'
smbclient -U guest -N '//192.168.2.100/storage' -c 'del faults.tar.gz'

VBoxHeadless -s XP &
PID=$!

for x in {1..240}
do
	if [ -e /mnt/raid/storage/faults.tar.gz ] 
	then
		sleep 1
		break
	else
		sleep 30
	fi
done

VBoxManage controlvm XP acpipowerbutton
wait $PID

if [ -e /mnt/raid/storage/faults.tar.gz ] 
then
	mkdir temp
	cp /mnt/raid/storage/faults.tar.gz temp
	cd temp
	tar -xzf faults.tar.gz
	cat faults/report.txt >>/tmp/report.txt
	rm faults/report.txt
	cp faults/* /tmp/faults/
	cd ..
	rm -fr temp 
fi


#####################
# Report
#####################


cp /tmp/report.txt /tmp/faults
cp /tmp/report.txt /home/artik/vserver-www/www/files/nightly-build-report.txt

cd /tmp
rm -fr /tmp/nightly-build-report
mv /tmp/faults /tmp/nightly-build-report

tar -czvf nightly-build-report.tar.gz nightly-build-report
cp nightly-build-report.tar.gz ~/
cp nightly-build-report.tar.gz /home/artik/vserver-www/www/files/
cp -a nightly-build-report /home/artik/vserver-www/www/files/
$ROOT_PATH/report_to_html.py < ~/vserver-www/www/files/nightly-build-report.txt >~/vserver-www/www/files/nightly-build-report.html

$HOME/bin/sync_build.sh



