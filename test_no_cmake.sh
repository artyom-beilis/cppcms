#!/bin/bash

if test "x$1" == "x" ;
then 
	echo "Provide a path to python interpreter"
	exit 1
fi

PYTHON="$1"

rm -f Test.Log
rm -f fail.flag

run_test()
{
	echo "Testing    "$@""
	echo "Starting $@"  >> Test.Log
	echo ---------------------- >>Test.Log
	if "$@"  >> Test.Log 2>>Test.Log
	then
		echo ---------------------- >>Test.Log
		echo Passed >>Test.Log
		echo "Ok"
	else
		echo ---------------------- >>Test.Log
		echo "Failed !!!!!!!!!!!!!!!!!!!!!!" >>Test.Log
		echo "Failed !!!!!!!!!!!!!!!!!!!!!!"
		echo "$@" >> fail.flag
	fi
	echo >>Test.Log
}

run_test ./booster/test_function_function
run_test ./booster/test_ptime_posix_time
run_test ./booster/test_thread_thread
run_test ./booster/test_smart_ptr_shared_ptr
run_test ./booster/test_smart_ptr_atomic_counter
run_test ./booster/test_smart_ptr_sp_counter
run_test ./booster/test_log_log
run_test ./booster/test_nowide_nowide
run_test ./booster/test_iostreams_streambuf
run_test ./booster/test_regex_regex
run_test ./booster/test_aio_reactor
run_test ./booster/test_aio_timer
run_test ./booster/test_aio_event_loop
run_test ./booster/test_aio_socket
run_test ./booster/test_aio_endpoint
if test "x$NOICU" = "x" 
then
	run_test ./booster/test_locale_boundary
	run_test ./booster/test_locale_codepage
	run_test ./booster/test_locale_collate
	run_test ./booster/test_locale_convert
	run_test ./booster/test_locale_date_time
	run_test ./booster/test_locale_formatting
	run_test ./booster/test_locale_generator
	run_test ./booster/test_locale_ios_prop
	run_test ./booster/test_locale_icu_vs_os_timezone
	run_test ./booster/test_locale_message ../tests
else
	run_test ./locale_src_test_convert
	run_test ./locale_src_test_formatting
	run_test ./locale_src_test_ios_prop
	run_test ./locale_src_test_message ../tests
fi

run_test ./base64_test
run_test ./encryptor_test
run_test ./storage_test
run_test ./json_test
run_test ./cache_backend_test
run_test ./serialization_test
run_test ./multipart_parser_test   ../tests



run_test ./form_test -c ../tests/form_test.js "--test-exec=$PYTHON ../tests/form_test.py"
run_test ./cookie_test -c ../tests/cookie_test.js "--test-exec=$PYTHON ../tests/cookie_test.py"
run_test ./forwarder_test -c ../tests/forwarder_test.js --test-internal=true "--test-exec=$PYTHON ../tests/forwarder_test.py"
run_test ./forwarder_test -c ../tests/forwarder_test.js --test-internal=false "--test-exec=$PYTHON ../tests/forwarder_test.py"
run_test ./jsonrpc_test -c ../tests/jsonrpc_test.js "--test-exec=$PYTHON ../tests/jsonrpc_test.py"
run_test ./proto_test -c ../tests/proto_test.js --test-async=async --service-api=http --service-port=8080 "--test-exec=$PYTHON ../tests/proto_test.py http"
run_test ./proto_test -c ../tests/proto_test.js --test-async=sync --service-api=http --service-port=8080 "--test-exec=$PYTHON ../tests/proto_test.py http"
run_test ./proto_test -c ../tests/proto_test.js --test-async=async --service-api=scgi --service-port=8080 "--test-exec=$PYTHON ../tests/proto_test.py scgi_tcp"
run_test ./proto_test -c ../tests/proto_test.js --test-async=sync --service-api=scgi --service-port=8080 "--test-exec=$PYTHON ../tests/proto_test.py scgi_tcp"

if test "x$NOUNIX" = "x"
then
	run_test ./proto_test -c ../tests/proto_test.js --test-async=async --service-api=scgi --service-socket=/tmp/cppcms_test_socket "--test-exec=$PYTHON ../tests/proto_test.py scgi_unix"
	run_test ./proto_test -c ../tests/proto_test.js --test-async=sync --service-api=scgi --service-socket=/tmp/cppcms_test_socket "--test-exec=$PYTHON ../tests/proto_test.py scgi_unix"
fi

if test -e fail.flag 
then
	echo "`cat fail.flag | wc -l` Tests are failed"
	exit 1;
else
	echo "All tests are passed"
fi

