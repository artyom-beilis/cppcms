#!/bin/bash

if [ "$SWIG" == "" ]
then
	SWIG=swig
fi

$SWIG -php7 -module cppcms_api  -outdir ../php/php7  -o ../php/php7/cppcms_api.c cppcms.i
