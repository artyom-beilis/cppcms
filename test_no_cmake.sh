#!/bin/bash

PYTHON="$1"

./base64_test
./encryptor_test
./storage_test
./json_test
./cache_backend_test
./serialization_test
./booster/test_function_function
./booster/test_ptime_posix_time
./booster/test_thread_thread
./booster/test_smart_ptr_shared_ptr
./booster/test_smart_ptr_atomic_counter
./booster/test_smart_ptr_sp_counter
./booster/test_log_log
./booster/test_nowide_nowide
./booster/test_iostreams_streambuf
./booster/test_regex_regex
./booster/test_aio_reactor
./booster/test_aio_timer
./booster/test_aio_event_loop
./booster/test_aio_socket
./booster/test_aio_endpoint
./booster/test_locale_boundary
./booster/test_locale_codepage
./booster/test_locale_collate
./booster/test_locale_convert
./booster/test_locale_date_time
./booster/test_locale_formatting
./booster/test_locale_generator
./booster/test_locale_ios_prop
./booster/test_locale_icu_vs_os_timezone
./booster/test_locale_message ../tests
./multipart_parser_test   ../tests
./form_test -c ../tests/form_test.js "--test-exec=$PYTHON ../tests/form_test.py"
./cookie_test -c ../tests/cookie_test.js "--test-exec=$PYTHON ../tests/cookie_test.py"
./forwarder_test -c ../tests/forwarder_test.js --test-internal=true "--test-exec=$PYTHON ../tests/forwarder_test.py"
./forwarder_test -c ../tests/forwarder_test.js --test-internal=false "--test-exec=$PYTHON ../tests/forwarder_test.py"
./jsonrpc_test -c ../tests/jsonrpc_test.js "--test-exec=$PYTHON ../tests/jsonrpc_test.py"
./proto_test -c ../tests/proto_test.js --test-async=async --service-api=http --service-port=8080 "--test-exec=$PYTHON ../tests/proto_test.py http"
./proto_test -c ../tests/proto_test.js --test-async=sync --service-api=http --service-port=8080 "--test-exec=$PYTHON ../tests/proto_test.py http"
./proto_test -c ../tests/proto_test.js --test-async=async --service-api=scgi --service-port=8080 "--test-exec=$PYTHON ../tests/proto_test.py scgi_tcp"
./proto_test -c ../tests/proto_test.js --test-async=sync --service-api=scgi --service-port=8080 "--test-exec=$PYTHON ../tests/proto_test.py scgi_tcp"

