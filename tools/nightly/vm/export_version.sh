#!/bin/bash

source url.sh

rm -rf cppcms.tar.gz cppcms
svn export $REPO/$BRANCH cppcms || exit 1
tar -czvf cppcms.tar.gz cppcms
rm -fr cppcms

