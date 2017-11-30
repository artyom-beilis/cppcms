#!/bin/bash -x

TEST="$1"

ssh -p $VM_PORT localhost mkdir  "/tmp/nightly/cppcms/$TEST" || exit 1
scp -P $VM_PORT $VM_ID/$TEST.sh localhost:/tmp/nightly/cppcms/  || exit 1
ssh -p $VM_PORT localhost "cd /tmp/nightly/cppcms/$TEST ; sh /tmp/nightly/cppcms/$TEST.sh" | tee logs/$VM_ID-$TEST.log 
if [ "${PIPESTATUS[0]}" == "0" ] ; then
	echo ok >logs/$VM_ID-${TEST}-status.txt
else
	echo fail >logs/$VM_ID-${TEST}-status.txt
fi
scp -P $VM_PORT localhost:/tmp/nightly/cppcms/"$TEST"/Testing/Temporary/LastTest.log "logs/${VM_ID}-${TEST}-ctest-log.txt"
