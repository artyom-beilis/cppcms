#!/bin/bash -x

ssh -p $VM_PORT localhost "$VM_EXE\\rm" -fr "/c/nightly/*" || exit 1
rm -fr /mnt/raid1/storage/cppcms.tar.gz
cp cppcms.tar.gz /mnt/raid1/storage/ || exit 1
sleep 5
ssh -p $VM_PORT localhost 'cmd /c copy \\192.168.2.100\storage\cppcms.tar.gz c:\nightly\' || exit 1
#[ "${PIPESTATUS[1]}" == "0" ] || exit 1
#echo "put cppcms.tar.gz /c:/nightly/" | sftp -P $VM_PORT localhost  

sleep 5

ssh -p $VM_PORT localhost "$VM_EXE\\tar" -I $VM_MSYS_EXE/gzip -xvf "/c/nightly/cppcms.tar.gz" -C  "/c/nightly"  || exit 1
sleep 5
