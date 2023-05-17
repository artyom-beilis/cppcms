#!/bin/bash

source url.sh

REPO_REV="GIT $GIT_BRANCH/`git ls-remote $REPO.git $GIT_BRANCH | awk '{print $1}' `"

TGT=/tmp/nightly-build-report
rm -fr $TGT
mkdir $TGT
for f in logs/*-status.txt
do
	base=${f%-status.txt}
	base=$(basename $base)
	if [ "$(cat $f)" == "ok" ]
	then
		cp logs/$base.log $TGT/$base.txt
	else
		cat logs/$base.log logs/${base}-ctest-log.txt >$TGT/$base.txt
	fi
done

./report_to_html.py "$REPO" "$REPO_REV"  >/home/artik/nightly-build-report.html
$HOME/bin/sync_build.sh

