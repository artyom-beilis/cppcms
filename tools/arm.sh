#!/bin/bash 


#####################
# Armel/Linux       #
#####################

armel_build()
{
	cd /tmp
	rm -fr nb.tar.bz2
	cd /tmp/nb
	rm -fr build
	mkdir build
	cd build
	if cmake -DCMAKE_TOOLCHAIN_FILE=../tools/ArmelToolChain.cmake $FLAGS -DDISABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Debug .. && make 
	then
		return 0
	else
		return 1
	fi
}

TEST=arm
FILE=/tmp/$TEST.txt
if armel_build &> $FILE 
then
	cd /tmp
	tar -cjf nb.tar.bz2 nb/build nb/tests nb/booster/lib/locale/test
	cd /home/artik/OSZoo/ArmelSquezzy
	/opt/qemu14/bin/qemu-system-arm \
		-M versatilepb \
		-kernel vmlinuz-2.6.32-5-versatile \
		-initrd initrd.img-2.6.32-5-versatile \
		-hda debian_squeeze_armel_standard.qcow2 \
		-nographic \
		-append "root=/dev/sda1" \
		-redir tcp:2224::22 &
	
	PID=$!
	sleep 60

	scp -P 2224 /tmp/nb.tar.bz2 artik@localhost:/tmp
	ssh -p 2224 artik@localhost "cd /tmp; tar -xjf nb.tar.bz2"
	if ssh -p 2224 artik@localhost "cd /tmp/nb/build; ctest " >> $FILE
	then
		echo $TEST - pass >>/tmp/report.txt
	else
		echo $TEST - fail >>/tmp/report.txt
	fi

	ssh -p 2224 root@localhost "poweroff"
	sleep 30
	kill $PID
	wait $PID
else
	echo $TEST - fail >>/tmp/report.txt
fi
cp $FILE /tmp/faults/

