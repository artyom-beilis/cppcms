cmake_minimum_required(VERSION 2.6)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)


project(cppcms)
set(CPPCMS_PACKAGE_NAME "CppCMS")
set(CPPCMS_PACKAGE_STRING "CppCMS/0.99.10")
set(CPPCMS_PACKAGE_VERSION "0.99.10")

include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(CPack)
enable_testing()

#############################################################################
#
# Setup default build options
#
#############################################################################

option(DISABLE_SHARED		"Disable shared libraries build" OFF)
option(DISABLE_STATIC		"Disable static libraries build" OFF)
option(DISABLE_GCRYPT		"Disable usage of gcrypt library" OFF)
option(DISABLE_OPENSSL		"Disable usage of openssl library" OFF)
option(DISABLE_FCGI		"Disable fastcgi web server api" OFF)
option(DISABLE_SCGI		"Disable scgi web server api" OFF)
option(DISABLE_HTTP		"Disable http web server" OFF)
option(DISABLE_CACHE		"Disable cache beckend" OFF)
option(DISABLE_TCPCACHE		"Disable distributed cache beckend" OFF)
option(DISABLE_GZIP		"Disable support of gzip output compression" OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
	option(DISABLE_PREFORK_CACHE	"Disable prefork-cache beckend" ON)
else()
	option(DISABLE_PREFORK_CACHE	"Disable prefork-cache beckend" OFF)
endif()

add_subdirectory(booster)

include_directories(booster)
include_directories(src)
include_directories(private)
include_directories(cppcms_boost)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/booster)
include_directories(${CMAKE_SOURCE_DIR})

if(DISABLE_CACHE)
	set(DISABLE_PREFORK_CACHE ON)
	set(DISABLE_TCPCACHE ON)
endif()

if(WIN32 OR CYGWIN)
	set(IS_WINDOWS TRUE)
else()
	set(IS_WINDOWS FALSE)
endif()

if(WIN32 AND NOT CYGWIN)
	set(WINDOWS_NATIVE TRUE)
else()
	set(WINDOWS_NATIVE FALSE)
endif()

if(CYGWIN)
	set(IS_CYGWIN TRUE)
else()
	set(IS_CYGWIN FALSE)
endif()






#############################################################################
#
# Setup various build flags for different supported compilers and systems
#
#############################################################################

if(CMAKE_COMPILER_IS_GNUCXX)
	check_cxx_source_compiles(
		"#if __GNUC__ < 4
		#error
		#endif
		int main() {}"
		GCC_IS_GCC4)

	set(CXX_FLAGS "-Wall -Wextra")

	if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
		set(CXX_FLAGS "${CXX_FLAGS} -pthreads")
	endif()

	if(NOT GCC_IS_GCC4)
		# Uninitalized checks are bogous under gcc-3.4
		set(CXX_FLAGS "${CXX_FLAGS} -Wno-uninitialized")
	endif()

	if(IS_WINDOWS)
		if(GCC_IS_GCC4) 
			# Very important, otherwise process would not start under cygwin
			set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--enable-auto-import")
		else()
			# gcc-3 does not have shared library for libstdc++ -- cause dll faitures with locale
			set(DISABLE_SHARED ON)
		endif()
	endif()

	if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
		set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -rdynamic")
	endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	set(CXX_FLAGS "-Wall")
elseif(MSVC)
	set(CXX_FLAGS "/EHsc /W3")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
	#
	# We use STL port under Sun Studio, standard library is broken
	#
	
	set(CXX_FLAGS "-library=stlport4 -xannotate=no")

	if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
		set(CXX_FLAGS "${CXX_FLAGS} -mt")
	endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

#############################################################################
#
# Set default RelWithDebInfo build
#
#############################################################################

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	if(MSVC)
		set(ZLIB_SUFFIX "d")
		if(NOT CPPCMS_SUFFIX)
			set(CPPCMS_SUFFIX "-d")
		else()
			set(CPPCMS_SUFFIX "${CPPCMS_SUFFIX}d")
		endif()
	endif()
endif()

check_cxx_source_compiles(
	"#include <stdint.h> 
	int main(){int32_t x; int64_t y; return 0; }"
	CPPCMS_HAVE_STDINT_H)

check_cxx_source_compiles(
	"#include <inttypes.h> 
	int main(){int32_t x; int64_t y; return 0; }"
	CPPCMS_HAVE_INTTYPES_H)

check_function_exists(atoll CPPCMS_HAVE_ATOLL)
check_function_exists(_atoi64 CPPCMS_HAVE_ATOI64)
check_function_exists(snprintf CPPCMS_HAVE_SNPRINTF)
check_function_exists(stat CPPCMS_HAVE_STAT)
check_function_exists(_stat CPPCMS_HAVE__STAT)

set(CPPCMS_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
set(CPPCMS_LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})

#
# Atomic API tests
#


check_cxx_source_compiles(
	"#include <string> 
	int main() { std::wstring s; }"
	CPPCMS_HAVE_STD_WSTRING)

check_cxx_source_compiles(
	"#include <time.h> 
	int main() { struct tm t; char const *p=t.tm_zone; long o=t.tm_gmtoff; }"
	CPPCMS_HAVE_BSD_TM)

check_cxx_source_compiles(
	"#include <string> 
	int main() { std::u16string s16; std::u32string s32; }"
	CPPCMS_HAVE_CPP0X_UXSTRING)

check_cxx_source_compiles(
	"#include <list> 
	int main(){std::list<int> l; auto p=l.begin();}"
	CPPCMS_HAVE_CPP_0X_AUTO)

check_cxx_source_compiles(
	"#include <list> 
	int main(){std::list<int> l; decltype(l.begin()) p=l.begin();}"
	CPPCMS_HAVE_CPP_0X_DECLTYPE)

check_cxx_source_compiles(
	"#include <list> 
	int main(){std::list<int> l; typeof(l.begin()) p=l.begin();}"
	CPPCMS_HAVE_GCC_TYPEOF)

check_cxx_source_compiles(
	"#include <list> 
	int main(){std::list<int> l; __typeof__(l.begin()) p=l.begin();}"
	CPPCMS_HAVE_UNDERSCORE_TYPEOF)

if(NOT DISABLE_GZIP)
	find_library(ZLIB NAMES z zdll zlib${ZLIB_SUFFIX} zlib)
	find_path(ZLIB_INCLUDE_DIR zlib.h)
	include_directories(${ZLIB_INCLUDE_DIR})
endif()


if(IS_WINDOWS)
	find_library(WS2_32 ws2_32)
	find_library(WSOCK32 wsock32)
else()
	check_function_exists(dlopen CPPCMS_HAVE_DLOPEN)
	if(NOT CPPCMS_HAVE_DLOPEN)
		check_library_exists(dl dlopen "" CPPCMS_HAVE_LIB_DL)
		if(NOT CPPCMS_HAVE_LIB_DL)
			message(FATAL " No library with dlopen found")
		else(NOT CPPCMS_HAVE_LIB_DL)
			find_library(LIB_DL dl)
		endif(NOT CPPCMS_HAVE_LIB_DL)
	endif(NOT CPPCMS_HAVE_DLOPEN)
	
	check_function_exists(socket CPPCMS_HAVE_SOCKET)
	if(NOT CPPCMS_HAVE_SOCKET)
		check_library_exists(socket socket "" CPPCMS_HAVE_LIB_SOCKET)
		if(NOT CPPCMS_HAVE_LIB_SOCKET)
			message(FATAL " No library with socket found")
		else(NOT CPPCMS_HAVE_LIB_SOCKET)
			find_library(LIB_SOCKET socket)
		endif(NOT CPPCMS_HAVE_LIB_SOCKET)
	endif(NOT CPPCMS_HAVE_SOCKET)

	check_function_exists(gethostbyname CPPCMS_HAVE_GETHOSTBYNAME)
	if(NOT CPPCMS_HAVE_GETHOSTBYNAME)
		check_library_exists(socket gethostbyname "" LIB_SOCKGETHOSTBYNAME)
		if(NOT LIB_SOCKGETHOSTBYNAME)
			check_library_exists(nsl gethostbyname "" CPPCMS_HAVE_LIB_NSL)
			if(NOT CPPCMS_HAVE_LIB_NSL)
				message(FATAL " No library with gethostbyname found")
			else(NOT CPPCMS_HAVE_LIB_NSL)
				find_library(LIB_NSL nsl)
			endif(NOT CPPCMS_HAVE_LIB_NSL)
		endif(NOT LIB_SOCKGETHOSTBYNAME)
	endif(NOT CPPCMS_HAVE_GETHOSTBYNAME)
endif() 


check_function_exists(canonicalize_file_name CPPCMS_HAVE_CANONICALIZE_FILE_NAME)

add_definitions(-DCPPCMS_BOOST_ALL_NO_LIB)

if(DISABLE_TCPCACHE)
	set(CPPCMS_NO_TCP_CACHE 1)
endif()

if(DISABLE_CACHE)
	set(CPPCMS_NO_CACHE 1)
endif()

if(DISABLE_PREFORK_CACHE)
	set(CPPCMS_NO_PREFOK_CACHE 1)
endif()

if(DISABLE_GZIP)
	set(CPPCMS_NO_GZIP 1)
endif()



set(CPPCMS_PUBLIC_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/cppcms/config.h)


set(CPPCMS_SOURCES 
	src/service.cpp 
	src/cgi_api.cpp 
	src/http_request.cpp 
	src/http_response.cpp 
	src/http_context.cpp 
	src/cppcms_error.cpp 
	src/cppcms_error_category.cpp 
	src/thread_pool.cpp 
	src/applications_pool.cpp 
	src/application.cpp 
	src/url_dispatcher.cpp 
	src/url_mapper.cpp 
	src/http_cookie.cpp  
	src/http_file.cpp  
	src/http_content_type.cpp
	src/util.cpp 
	src/base64.cpp 
	src/base_view.cpp 
	src/base_content.cpp 
	src/views_pool.cpp
	src/internal_file_server.cpp 
	src/json.cpp 
	src/encoding.cpp 
	src/form.cpp 
	src/filters.cpp
	src/cache_pool.cpp
	src/cache_interface.cpp
	src/cache_storage.cpp
	src/forwarder.cpp
	src/session_pool.cpp
	src/hmac_encryptor.cpp
	src/aes_encryptor.cpp
	src/md5.cpp
	src/urandom.cpp
	src/aes.cpp
	src/session_cookies.cpp
	src/session_dual.cpp
	src/session_sid.cpp
	src/session_interface.cpp
	src/session_memory_storage.cpp
	src/rpc_json.cpp
	src/mount_point.cpp
	src/archive.cpp
	src/crypto.cpp
	src/daemonize.cpp
	src/xss.cpp
	src/copy_filter.cpp
	src/send_timeout.cpp
)

if(NOT DISABLE_GZIP)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES}
		cppcms_boost/src/zlib.cpp
		cppcms_boost/src/gzip.cpp)
endif()


if(NOT DISABLE_TCPCACHE)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES}
		src/tcp_cache_client.cpp
		src/cache_over_ip.cpp
		src/tcp_messenger.cpp
		src/tcp_connector.cpp
		src/tcp_cache_server.cpp)
endif()

if(NOT DISABLE_GCRYPT)
	find_path(GCRYPT_INCLUDE_DIR gcrypt.h)
	find_library(LIB_GCRYPT gcrypt)
endif()

if(NOT DISABLE_OPENSSL)
	find_path(OPENSSL_INCLUDE_DIR openssl/aes.h)
	find_library(LIB_OPENSSL crypto)
endif()

if(GCRYPT_INCLUDE_DIR AND LIB_GCRYPT)
	set(LIB_CRYPT ${LIB_GCRYPT})
	set(CPPCMS_HAVE_GCRYPT 1)
	include_directories(${GCRYPT_INCLUDE_DIR})
elseif (OPENSSL_INCLUDE_DIR AND LIB_OPENSSL)
	set(LIB_CRYPT ${LIB_OPENSSL})
	set(CPPCMS_HAVE_OPENSSL 1)
	include_directories(${OPENSSL_INCLUDE_DIR})
endif()


if(NOT LIB_CRYPT)
	message("GNU-TLS Gcrypt or OpenSSL librarys are not found, disabling AES encryption support")
endif()



if(NOT DISABLE_FCGI)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} src/fastcgi_api.cpp)
	set(CPPCMS_HAS_FCGI 1)
endif()

if(NOT DISABLE_SCGI)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} src/scgi_api.cpp)
	set(CPPCMS_HAS_SCGI 1)
endif()


if(NOT DISABLE_HTTP)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} src/http_api.cpp)
	set(CPPCMS_HAS_HTTP 1)
endif()
	

if(WINDOWS_NATIVE)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} src/session_win32_file_storage.cpp)
else()
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} src/session_posix_file_storage.cpp)
endif()

if(NOT DISABLE_SHARED)
	add_library(cppcms SHARED ${CPPCMS_SOURCES})
	set(CPPCMS_LIB cppcms)
else(NOT DISABLE_SHARED)
	set(CPPCMS_LIB cppcms-static)
endif(NOT DISABLE_SHARED)

if(NOT DISABLE_STATIC)
	add_library(cppcms-static STATIC ${CPPCMS_SOURCES})
endif(NOT DISABLE_STATIC)

if(DISABLE_SHARED OR DISABLE_STATIC)
	set(CPPCMS_LIBS ${CPPCMS_LIB})
else(DISABLE_SHARED OR DISABLE_STATIC)
	set(CPPCMS_LIBS cppcms; cppcms-static)
endif(DISABLE_SHARED OR DISABLE_STATIC)


foreach(ALIB ${CPPCMS_LIBS})
	if(DISABLE_SHARED)
		target_link_libraries(${ALIB} booster-static)
	else(DISABLE_SHARED)
		target_link_libraries(${ALIB} booster)
	endif(DISABLE_SHARED)
	if(LIB_CRYPT)
		target_link_libraries(${ALIB} ${LIB_CRYPT})
	endif(LIB_CRYPT)
	
	if(WS2_32 AND WSOCK32)
		target_link_libraries(${ALIB} ${WS2_32})
		target_link_libraries(${ALIB} ${WSOCK32})
	endif(WS2_32 AND WSOCK32) 

	if(LIB_SOCKGETHOSTBYNAME)
		target_link_libraries(${ALIB} ${LIB_SOCKGETHOSTBYNAME})
	endif(LIB_SOCKGETHOSTBYNAME)
	if(LIB_NSL)
		target_link_libraries(${ALIB} ${LIB_NSL})
	endif(LIB_NSL)

	if(LIB_SOCKET)
		target_link_libraries(${ALIB} ${LIB_SOCKET})
	endif(LIB_SOCKET)
	
	if(LIB_DL)
		target_link_libraries(${ALIB} ${LIB_DL})
	endif(LIB_DL)

	if(NOT DISABLE_GZIP)
		target_link_libraries(${ALIB} ${ZLIB})
	endif()
endforeach(ALIB)


find_program(PYTHON NAMES python2 python)
if(NOT PYTHON)
	message(FATAL "Can't build without Python interpreter")
endif(NOT PYTHON)


# Templates builds

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/skin1.cpp
	COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
		-s skin1 
		-o ${CMAKE_CURRENT_BINARY_DIR}/skin1.cpp 
			${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_skin1.tmpl 
			${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_view1.tmpl
	DEPENDS 
		${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
		${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_skin1.tmpl 
		${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_view1.tmpl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/skin2.cpp
	COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
		-s skin2 
		-o ${CMAKE_CURRENT_BINARY_DIR}/skin2.cpp 
			${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_skin2.tmpl 
			${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_view1.tmpl
	DEPENDS 
		${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
		${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_skin2.tmpl 
		${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_view1.tmpl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tc_skin_a.cpp
	COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
		-o ${CMAKE_CURRENT_BINARY_DIR}/tc_skin_a.cpp 
			${CMAKE_CURRENT_SOURCE_DIR}/tests/tc_skin_a.tmpl
	DEPENDS 
		${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
		${CMAKE_CURRENT_SOURCE_DIR}/tests/tc_skin_a.tmpl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tc_skin_b.cpp
	COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
		-o ${CMAKE_CURRENT_BINARY_DIR}/tc_skin_b.cpp 
			${CMAKE_CURRENT_SOURCE_DIR}/tests/tc_skin_b.tmpl
	DEPENDS 
		${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
		${CMAKE_CURRENT_SOURCE_DIR}/tests/tc_skin_b.tmpl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tc_skin.cpp
	COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
		-o ${CMAKE_CURRENT_BINARY_DIR}/tc_skin.cpp 
			${CMAKE_CURRENT_SOURCE_DIR}/tests/tc_skin.tmpl
	DEPENDS 
		${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
		${CMAKE_CURRENT_SOURCE_DIR}/tests/tc_skin.tmpl)



if(NOT DISABLE_SHARED)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/skin3.cpp
		COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
			-s skin3 
			-o ${CMAKE_CURRENT_BINARY_DIR}/skin3.cpp 
				${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_skin2.tmpl 
				${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_view1.tmpl
		DEPENDS 
			${CMAKE_CURRENT_SOURCE_DIR}/bin/cppcms_tmpl_cc 
			${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_skin2.tmpl 
			${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world_view1.tmpl)
	add_library(skin3 SHARED skin3.cpp)
	set(SKIN3_DEFS DLL_EXPORT)
	set_target_properties(skin3 PROPERTIES
				COMPILE_DEFINITIONS "${SKIN3_DEFS}")
	target_link_libraries(skin3 ${CPPCMS_LIB})
endif(NOT DISABLE_SHARED)


foreach(SKIN tc_skin_a tc_skin_b tc_skin)
	add_library(${SKIN} SHARED ${CMAKE_CURRENT_BINARY_DIR}/${SKIN}.cpp)
	target_link_libraries(${SKIN} ${CPPCMS_LIB})
	if(IS_WINDOWS)
		set_target_properties(${SKIN} PROPERTIES COMPILE_DEFINITIONS DLL_EXPORT)
	endif()
endforeach()


add_executable(hello_world src/hello_world.cpp skin1.cpp skin2.cpp)
target_link_libraries(hello_world ${CPPCMS_LIB})

if(NOT DISABLE_TCPCACHE)
	add_executable(cppcms_scale src/cache_server_main.cpp)
	target_link_libraries(cppcms_scale ${CPPCMS_LIB})
endif()

add_executable(cppcms_make_key src/make_key.cpp)
target_link_libraries(cppcms_make_key ${CPPCMS_LIB})

add_executable(cppcms_config_find_param src/cppcms_config_find_param.cpp src/json.cpp)
if(DISABLE_SHARED)
	target_link_libraries(cppcms_config_find_param booster-static)
else(DISABLE_SHARED)
	target_link_libraries(cppcms_config_find_param booster)
endif(DISABLE_SHARED)

#########################################
# Tests
#########################################

set(ALL_TESTS 
	form_test
	proto_test
	disco_test
	http_timeouts_test
	file_server_test
	encryptor_test
	storage_test
	json_test
	base64_test
	forwarder_test
	jsonrpc_test
	cookie_test
	multipart_parser_test
	content_type_parser_test
	serialization_test
	status_test
	xss_test
	url_mapper_test
	copy_filter_test
	tc_test
	)

if(NOT DISABLE_TCPCACHE)
	set(ALL_TESTS ${ALL_TESTS} cache_backend_test)
endif()

foreach(TEST ${ALL_TESTS})
	add_executable(${TEST} tests/${TEST}.cpp)
	target_link_libraries(${TEST} ${CPPCMS_LIB})
endforeach()

foreach(LFILE ${LOCALE_TEST_SOURCES})
	string(REGEX REPLACE ".*noicu/src/(.*)\\.cpp" "\\1" TEST_NAME ${LFILE})
	add_executable(${TEST_NAME} ${LFILE})
	target_link_libraries(${TEST_NAME} ${CPPCMS_LIB})
	set(LOCALE_TESTS "${LOCALE_TESTS} ${TEST_NAME}")
	add_test(${TEST_NAME} ${TEST_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/tests")
endforeach()



#####################################
# End of tests
#####################################

# These are use export
if(IS_WINDOWS AND NOT DISABLE_SHARED)
	set_target_properties(${ALL_TESTS} hello_world cppcms PROPERTIES
				COMPILE_DEFINITIONS DLL_EXPORT)
endif()

set_target_properties(${CPPCMS_LIBS} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(${CPPCMS_LIBS} PROPERTIES OUTPUT_NAME "cppcms${CPPCMS_SUFFIX}")
if(IS_WINDOWS)
	set_target_properties(${CPPCMS_LIBS} PROPERTIES VERSION 1 SOVERSION 1)
else()
	set_target_properties(${CPPCMS_LIBS} PROPERTIES VERSION 1.0.0 SOVERSION 1)
endif()

set_target_properties(${CPPCMS_LIBS} PROPERTIES 
			PUBLIC_HEADER "${CPPCMS_PUBLIC_HEADERS}")
			
if(MSVC AND NOT DISABLE_STATIC)
	set_target_properties(cppcms-static PROPERTIES PREFIX "lib")
endif(MSVC AND NOT DISABLE_STATIC)

configure_file(config.cmake.h cppcms/config.h)

set(CPPCMS_INSTALL_TARGETS ${CPPCMS_LIBS} cppcms_config_find_param cppcms_make_key)
if(NOT DISABLE_TCPCACHE)
	set(CPPCMS_INSTALL_TARGETS ${CPPCMS_INSTALL_TARGETS} cppcms_scale)
endif()

#################
# Installation
#################


install(TARGETS ${CPPCMS_INSTALL_TARGETS} 
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	PUBLIC_HEADER DESTINATION include/cppcms)
install(PROGRAMS bin/cppcms_tmpl_cc bin/cppcms_run
	DESTINATION bin)

install(DIRECTORY cppcms DESTINATION include
        PATTERN ".svn" EXCLUDE)


#############
# TESTS
#############
		


set(CNF "${CMAKE_CURRENT_SOURCE_DIR}/tests")

add_test(base64_test base64_test)
add_test(encryptor_test encryptor_test "${CNF}")
add_test(storage_test storage_test)
add_test(json_test json_test)
add_test(multipart_parser_test multipart_parser_test "${CNF}")
add_test(content_type_parser_test content_type_parser_test)
if(NOT DISABLE_TCPCACHE)
	add_test(cache_backend_test cache_backend_test)
endif()
add_test(serialization_test serialization_test)
add_test(xss_test xss_test)
add_test(url_mapper_test url_mapper_test)
add_test(copy_filter_test copy_filter_test)
add_test(tc_test tc_test)

add_test(status_test
	status_test "-c" "${CNF}/status_test.js" 
	"--test-exec=${PYTHON} ${CNF}/status_test.py")

add_test(async_status_test
	status_test "-c" "${CNF}/status_test.js" "--test-async=async"
	"--test-exec=${PYTHON} ${CNF}/status_test.py async")

add_test(form_test
	form_test "-c" "${CNF}/form_test.js" 
	"--test-exec=${PYTHON} ${CNF}/form_test.py")

add_test(cookie_test
	cookie_test "-c" "${CNF}/cookie_test.js" 
	"--test-exec=${PYTHON} ${CNF}/cookie_test.py")


add_test(internal_forwarder_test 
	forwarder_test "-c" "${CNF}/forwarder_test.js" 
	"--test-exec=${PYTHON} ${CNF}/forwarder_test.py"
	"--test-internal=true")

add_test(forwarder_test
	forwarder_test "-c" "${CNF}/forwarder_test.js" 
	"--test-exec=${PYTHON} ${CNF}/forwarder_test.py"
	"--test-internal=false")

add_test(jsonrpc_test
	jsonrpc_test "-c" "${CNF}/jsonrpc_test.js"
	"--test-exec=${PYTHON} ${CNF}/jsonrpc_test.py")

add_test(http_timeouts_test_read
	http_timeouts_test "-c" "${CNF}/proto_test.js" 
	"--service-api=http" "--service-port=8080" "--service-ip=127.0.0.1" "--http-timeout=5" "--logging-level=debug"
	"--test-write=false"
	"--test-exec=${PYTHON} ${CNF}/http_timeouts_test.py read")

add_test(http_timeouts_test_write
	http_timeouts_test "-c" "${CNF}/proto_test.js" 
	"--service-api=http" "--service-port=8080" "--service-ip=127.0.0.1" "--http-timeout=5" "--logging-level=debug"
	"--test-write=true"
	"--test-exec=${PYTHON} ${CNF}/http_timeouts_test.py write")

add_test(file_server_test
	file_server_test "-c" "${CNF}/file_server_test.js" 
	"--test-exec=${PYTHON} ${CNF}/file_server_test.py"
	"-U" "${CNF}"
	)

add_test(file_server_with_listing_test
	file_server_test "-c" "${CNF}/file_server_test.js" 
	"--file_server-listing=true"
	"--test-exec=${PYTHON} ${CNF}/file_server_test.py listing"
	"-U" "${CNF}"
	)

add_test(disco_test_http 
	disco_test "-c" "${CNF}/proto_test.js" 
	"--service-api=http" "--service-port=8080" "--service-ip=127.0.0.1"
	"--test-exec=${PYTHON} ${CNF}/disco_test.py http")

add_test(disco_test_fastcgi_tcp 
	disco_test "-c" "${CNF}/proto_test.js" 
	"--service-api=fastcgi" "--service-port=8080" "--service-ip=127.0.0.1"
	"--test-exec=${PYTHON} ${CNF}/disco_test.py fastcgi_tcp")

add_test(disco_test_scgi_tcp 
	disco_test "-c" "${CNF}/proto_test.js" 
	"--service-api=scgi" "--service-port=8080" "--service-ip=127.0.0.1"
	"--test-exec=${PYTHON} ${CNF}/disco_test.py scgi_tcp")

if(NOT IS_WINDOWS)
	add_test(disco_test_fastcgi_unix 
		disco_test "-c" "${CNF}/proto_test.js" 
		"--service-api=fastcgi" "--service-socket=/tmp/cppcms_test_socket"
		"--test-exec=${PYTHON} ${CNF}/disco_test.py fastcgi_unix")

	add_test(disco_test_scgi_unix 
		disco_test "-c" "${CNF}/proto_test.js" 
		"--service-api=scgi" "--service-socket=/tmp/cppcms_test_socket"
		"--test-exec=${PYTHON} ${CNF}/disco_test.py scgi_unix")
endif()

foreach(TYPE async sync)
	add_test(proto_test_${TYPE}_http 
		proto_test "-c" "${CNF}/proto_test.js" 
		"--test-async=${TYPE}" "--service-api=http" "--service-port=8080" "--service-ip=127.0.0.1"
		"--test-exec=${PYTHON} ${CNF}/proto_test.py http")
	add_test(proto_test_${TYPE}_scgi 
		proto_test "-c" "${CNF}/proto_test.js" 
		"--test-async=${TYPE}" "--service-api=scgi" "--service-port=8080" "--service-ip=127.0.0.1"
		"--test-exec=${PYTHON} ${CNF}/proto_test.py scgi_tcp")
	add_test(proto_test_${TYPE}_fastcgi 
		proto_test "-c" "${CNF}/proto_test.js" 
		"--test-async=${TYPE}" "--service-api=fastcgi" "--service-port=8080" "--service-ip=127.0.0.1"
		"--test-exec=${PYTHON} ${CNF}/proto_test.py fastcgi_tcp")
	if(NOT IS_WINDOWS)
		add_test(proto_test_${TYPE}_scgi_unix
			proto_test "-c" "${CNF}/proto_test.js" 
			"--test-async=${TYPE}" "--service-api=scgi" "--service-socket=/tmp/cppcms_test_socket"
			"--test-exec=${PYTHON} ${CNF}/proto_test.py scgi_unix")
		add_test(proto_test_${TYPE}_fastcgi_unix
			proto_test "-c" "${CNF}/proto_test.js" 
			"--test-async=${TYPE}" "--service-api=fastcgi" "--service-socket=/tmp/cppcms_test_socket"
			"--test-exec=${PYTHON} ${CNF}/proto_test.py fastcgi_unix")
	endif()
endforeach()

set(TESTS_WITH_TIMEOUT
	forwarder_test 
	jsonrpc_test 
	proto_test_sync_http
	proto_test_async_http
	proto_test_sync_scgi
	proto_test_async_scgi)

if(NOT DISABLE_TCPCACHE)
	set(TESTS_WITH_TIMEOUT ${TESTS_WITH_TIMEOUT} cache_backend_test)
endif()

set_tests_properties( 
	${TESTS_WITH_TIMEOUT}
	PROPERTIES TIMEOUT 120)

if(NOT IS_WINDOWS)
	set_tests_properties(
		proto_test_sync_scgi_unix
		proto_test_async_scgi_unix
		PROPERTIES TIMEOUT 120)

endif()

