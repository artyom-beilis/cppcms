#!/bin/bash

rm -f cppcms_api.so

if [ "$CPPCMS_PATH" != "" ]
then
    CPPCMS_INC=-I$CPPCMS_PATH/include
    CPPCMS_LIB=-L$CPPCMS_PATH/lib
    CPPCMS_LINK_FLAGS=-Wl,-rpath=$CPPCMS_PATH/lib
fi

PHP_FLAGS=`php-config --includes`

gcc -fPIC -shared -O2 -g $CPPCMS_INC $PHP_FLAGS $CPPCMS_LIB cppcms_api.c -o cppcms_api.so $CPPCMS_LINK_FLAGS -lcppcms -lbooster
