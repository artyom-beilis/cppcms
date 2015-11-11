#!/bin/bash

parsed() 
{
grep -E '(CPPCMS_API|define.*[0-9])' "$1" | \
	sed 's/char const \*[a-z_]*\([,\)]\)/string\1/g' | \
	sed 's/char const \*cppcms/string cppcms/g' | \
	sed 's/const//' | \
	sed 's/void  *\*[a-z_]*\([,\)]\)/pointer\1/g' | \
	sed 's/int  *[a-z_]*\([,)]\)/int\1/g' | \
	sed 's/cppcms_capi[a-z_]* *\*[a-z_]*\([,\)]\)/pointer\1/g' | \
	sed 's/cppcms_capi[a-z_]* *\*cppcms_capi/pointer cppcms_capi/g' | \
	sed 's/cppcms_capi_object  *[a-zA-Z_]*/pointer/g' | \
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
	| sed 's/void/None/g' \
	| sed 's/pointer/c_void_p/g' \
	| sed 's/string/c_char_p/g' \
	| sed 's/llong/c_longlong/g' \
	| sed 's/int/c_int/g' \
	| sed 's/unsigned/c_uint/g' \
	| sed 's/#define  *CPPCMS_CAPI_\([a-zA-Z0-9_]*\)  *\([0-9\-]*\)/\        cls.\1=\2/' \
	| sed 's/\([a-zA-Z_][a-z_]*\)  *\([a-z_][a-z_]*\)(\(.*\));/        cls.capi.\2.restype=\1\n        cls.capi.\2.argtypes=[ \3 ]/' >python.py
