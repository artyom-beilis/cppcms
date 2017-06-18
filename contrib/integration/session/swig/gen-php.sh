#!/bin/bash

if [ "$SWIG" == "" ]
then
	SWIG=swig
fi

$SWIG -php -module cppcms_api  -outdir ../php/php5  -o ../php/php5/cppcms_api.c cppcms.i
