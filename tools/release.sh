#!/bin/bash

if [ "$1" == "" ] ; then
	echo "Usage (trunk|branches/name)"
	exit 1;
fi

rm -fr current-release
mkdir current-release
cd ..
git archive $1  | tar -x -C tools/current-release 
cd tools
VERSION=`grep CPPCMS_PACKAGE_VERSION current-release/CMakeLists.txt  | sed 's/.*"\(.*\)".*/\1/'`

DIRNAME=cppcms-$VERSION
mv current-release $DIRNAME
cd $DIRNAME
doxygen
cp /usr/share/doc/lmdb-doc/html/dynsections.js doc/doxygen/html
cd ..
tar -cjf $DIRNAME.tar.bz2 $DIRNAME
rm -fr $DIRNAME

pushd .
if true
then
    tar -xjf $DIRNAME.tar.bz2
    cd $DIRNAME
    mkdir -p ../../../www/$VERSION
    cp -a doc/doxygen/html/*  ../../../www/$VERSION/
    cd ../../../www
    rm -f latest
    ln -s $VERSION latest
fi
popd 
mv $DIRNAME.tar.bz2 ../../releases/

