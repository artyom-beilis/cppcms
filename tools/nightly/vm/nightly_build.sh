#!/bin/bash  -x

ROOT_PATH=`dirname $0`
cd $ROOT_PATH


./export_version.sh || exit 1

#####################
# Report
#####################

rm -f logs/*

for VM in localhost freebsd solaris win7
do
	./run-vm-test.sh $VM  $(ls $VM | sed 's/\.sh//' | sed 's/\.bat//')
done

#
# Create Report
#

./upload_report.sh
