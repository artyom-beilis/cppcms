#!/usr/bin/env bash 
BIN="$1"

if [ "$BIN" == "" ]; then
	echo "Usege unit_test.sh /path/to/build/directory [fast-only]"
	exit 1;
fi

FAST="$2"


WIN32=0
case "`uname`" in
	*CYGWIN*)
		WIN32=1 ;;
	*MINGW*)	
		WIN32=1 ;;
	*Windows*)
		WIN32=1
esac

run()
{
	EXE=$1
	CONF=$2
	PARAM=$3
	
	echo $BIN/$EXE $CONF -c $EXE.js
	echo ./$EXE.py $PARAM

	$BIN/$EXE $CONF -c $EXE.js &
	PID=$!
	ERROR=0
	sleep 1
	if  ! ./$EXE.py $PARAM ; then
		ERROR=1
	fi
	kill $PID
	wait $PID
	if [ "$ERROR" == "1" ]; then
		echo "Failed!"
		exit 1
	fi
}

basic_test()
{
	EXE=$1
	echo /BIN/$EXE
	if ! $BIN/$EXE ; then
		echo "Failed!"
		exit 1
	fi
}


basic_test atomic_test
run form_test "" ""

if [ "$FAST" != "fast-only" ]; then
	for ASYNC in true false 
	do
		run proto_test "--test-async=$ASYNC --service-api=http --service-port=8080 --service-ip=127.0.0.1" http
		run proto_test "--test-async=$ASYNC --service-api=scgi --service-port=8080 --service-ip=127.0.0.1" scgi_tcp
		if [ $WIN32 == 0 ]; then
			run proto_test "--test-async=$ASYNC --service-api=scgi --service-socket=/tmp/cppcms_test_socket" scgi_unix
		fi
	done
fi

