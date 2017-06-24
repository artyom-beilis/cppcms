#!/bin/bash

source config.sh

rm -f com/cppcms/session/*.class cppcms.jar

if ! javac -cp $SERVLET:$JNA com/cppcms/session/*.java
then
	exit 1
fi
jar cvf cppcms.jar com/cppcms/session/*.class

