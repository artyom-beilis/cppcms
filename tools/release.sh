#!/bin/bash

SVNROOT=https://cppcms.svn.sourceforge.net/svnroot/cppcms/framework

svn export $SVNROOT/trunk current-release
VERSION=`grep CPPCMS_PACKAGE_VERSION current-release/CMakeLists.txt  | sed 's/.*"\(.*\)".*/\1/'`

if ! svn copy $SVNROOT/trunk $SVNROOT/tags/v$VERSION -m "Tagged release $VERSION"
then
	echo failed to make a tag
fi

DIRNAME=cppcms-$VERSION
mv current-release $DIRNAME
cd $DIRNAME
tar -xjf cppcms_boost.tar.bz2
rm cppcms_boost.tar.bz2
doxygen
cd ..
tar -cjf $DIRNAME.tar.bz2 $DIRNAME
rm -fr $DIRNAME

tar -xjf $DIRNAME.tar.bz2
cd $DIRNAME
mkdir -p ../www/$VERSION
cp -a doc/doxygen/html/*  ../www/$VERSION/
cd ../www
rm -f latest
ln -s $VERSION latest


