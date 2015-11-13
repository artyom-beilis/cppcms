#!/bin/bash

parsed() 
{
grep -E '(CPPCMS_API|define.*[0-9])' "$1" | \
	sed 's/char const \*\([a-z_]*\)\([,\)]\)/string \1\2/g' | \
	sed 's/char const \*cppcms/string cppcms/g' | \
	sed 's/const//' | \
	sed 's/void  *\*\([a-z_]*\)\([,\)]\)/byte[] \1\2/g' | \
	sed 's/int  *\([a-z_]*\)\([,)]\)/int \1\2/g' | \
	sed 's/cppcms_capi[a-z_]* *\*\([a-z_]*\)\([,\)]\)/pointer \1\2/g' | \
	sed 's/cppcms_capi[a-z_]* *\*cppcms_capi/pointer cppcms_capi/g' | \
	sed 's/cppcms_capi_object  *\([a-zA-Z_]*\)/pointer \1/g' | \
	sed 's/CPPCMS_API //' | \
	sed 's/long long/llong/g' | \
	sed 's/\t/ /g' | \
	sed 's/  / /g' 
}

FILE=$1
if [ "$FILE" == "" ]
then
	FILE=../../../../cppcms/capi/session.h
fi

parsed $FILE \
	| sed 's/pointer/Pointer/g' \
	| sed 's/string/String/g' \
	| sed 's/llong/long/g' \
	| sed 's/int/int/g' \
	| sed 's/unsigned/int/g' \
	| sed 's/#define  *CPPCMS_CAPI_\([a-zA-Z0-9_]*\)  *\([0-9\-]*\)/\        public final static int \1=\2;/' \
	| sed 's/\([a-zA-Z_][a-z_]*\)  *\([a-z_][a-z_]*\)(\(.*\));/        \1 \2(\3);/' >Api.java
