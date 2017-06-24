#!/bin/bash
rm *.class
source  ../config.sh

if ! javac -cp $SERVLET:$JNA:../cppcms.jar UnitTest.java
then 
	exit 1
fi


rm -fr jct

mkdir jct
mkdir jct/WEB-INF
mkdir jct/WEB-INF/classes
mkdir jct/WEB-INF/lib

cp web.xml jct/WEB-INF/
cp *.class jct/WEB-INF/classes/
cp ../cppcms.jar jct/WEB-INF/lib/
cp $JNA  jct/WEB-INF/lib/
cp ../../wwwtest/*.html jct/
cp ../../wwwtest/*.js jct/
cp ../../reference/config.js jct/WEB-INF/
