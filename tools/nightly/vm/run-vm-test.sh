#!/bin/bash

NOSTART=false

while true
do
	if [ "$1" == "--no-start" ]
	then 
		NOSTART=true
		shift
		continue
	else
		break
	fi
done

VM="$1"
shift

source "vm-$VM.sh"

OK=false

if [ "$VM" != "localhost" ] && ! $NOSTART
then
	VBoxManage startvm  "$VM" --type headless || exit 1
	sleep $VM_WAIT
	for x in 1..10
	do
		if ssh -p $VM_PORT localhost $VM_PING
		then
			OK=true
			break
		else
			sleep 5
		fi
	done
else
	OK=true
	NOSTART=true
fi


if $OK ; then
	./upload_version_$VM_OS.sh || OK=false
fi

while $OK && [ "$1" != "" ]
do
	TEST="$1"
	shift
	./run_test_$VM_OS.sh $TEST || OK=false
done


if ! $NOSTART 
then
	VBoxManage controlvm "$VM" acpipowerbutton || exit 1
	sleep 30
	while [ "`VBoxManage list runningvms | grep \"$VM\"  | wc -l`" != 0 ]
	do
		VBoxManage controlvm "$VM" acpipowerbutton 
		sleep 10
	done
fi

