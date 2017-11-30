#!/bin/bash -x
TEST="$1"

sleep 5
ssh -p $VM_PORT localhost "$VM_EXE\mkdir" "c:/nightly/cppcms/$TEST" || exit 1
sleep 5

rm -f /mnt/raid1/storage/log.txt

echo "cd c:\\nightly\\cppcms\\$TEST || exit 1" >/tmp/tmp.bat
cat  $VM_ID/$TEST.bat >>/tmp/tmp.bat

echo "put /tmp/tmp.bat /c:/nightly/$TEST.bat" | sftp -P $VM_PORT localhost 
[ "${PIPESTATUS[1]}" == "0" ] || exit 1
sleep 5

ssh -p $VM_PORT localhost 'c:\nightly\'$TEST.bat | tee logs/$VM_ID-$TEST.log 
if [ "${PIPESTATUS[0]}" == "0" ] ; then
	echo ok >logs/$VM_ID-${TEST}-status.txt
else
	echo fail >logs/$VM_ID-${TEST}-status.txt
fi

sleep 5

ssh -p $VM_PORT localhost  "cmd /c copy c:\\nightly\\cppcms\\$TEST\\Testing\\Temporary\\LastTest.log \\\\192.168.2.100\\storage\log.txt" 

sleep 5

cp /mnt/raid1/storage/log.txt logs/${VM_ID}-${TEST}-ctest-log.txt 

sleep 5
