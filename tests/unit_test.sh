#!/usr/bin/env bash 
BIN="$1"
WIN32=0
case "`uname`" in
	*win*)	
		WIN32=1 
esac



run()
{
	EXE=$1
	CONF=$2
	TEST=$3
	PARAM=$4
	$BIN/$EXE -c $CONF &
	PID=$!
	ERROR=0
	sleep 1
	echo $TEST $PARAM with $EXE -c $CONF
	./$TEST $PARAM 
	if [ ! $? ]; then
		ERROR=1
	fi
	kill $PID
	wait $PID
	if [ "$ERROR" == "1" ]; then
		exit 1
	fi
}


run form_test form_test.js form_test.py
run proto_test proto_test_http.js proto_test.py http
run proto_test proto_test_scgi_tcp.js proto_test.py scgi_tcp
if [ $WIN32 == 0 ]; then
	run proto_test proto_test_scgi_unix.js proto_test.py scgi_unix
fi

