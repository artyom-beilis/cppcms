cmake_minimum_required(VERSION 2.6)
project(cppcms)
set(PACKAGE_NAME "CppCMS")
set(PACKAGE_STRING "CppCMS/0.99.1")
set(PACKAGE_VERSION "0.99.1")

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

option(USE_STD_LOCALES	"Use standard std::locales instead of ICU" OFF)
option(DISABLE_SHARED	"Disable shared libraries build" OFF)
option(DISABLE_STATIC	"Disable static libraries build" OFF)
option(DISABLE_ICONV	"Disable usage iconv library - use ICU for conversion" OFF)
option(DISABLE_GCRYPT	"Disable usage of gcrypt library, no AES cookies encryption would be available" OFF)
option(USE_EXTERNAL_BOOST "Use external Boost library and not internal one, boost version provided by BOOST_SUFFIX variable" OFF)
option(DISABLE_FCGI	"Disable fastcgi web server api" OFF)
option(DISABLE_SCGI	"Disable scgi web server api" OFF)
option(DISABLE_HTTP	"Disable http web server" OFF)


if(USE_STD_LOCALES OR USE_EXTERNAL_BOOST)
	set(CPPCMS_SUFFIX "-c")
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

	set(CXX_FLAGS "-Wall")

	if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
		set(CXX_FLAGS "${CXX_FLAGS} -pthreads")
	endif()

	if(NOT GCC_IS_GCC4)
		# Uninitalized checks are bogous under gcc-3.4
		set(CXX_FLAGS "${CXX_FLAGS} -Wno-uninitialized")
	endif()

	if(WIN32)
		if(GCC_IS_GCC4) 
			# Very important, otherwise process would not start under cygwin
			set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--enable-auto-import")
		else()
			# gcc-3 does not have shared library for libstdc++ -- cause dll faitures with locale
			set(DISABLE_SHARED ON)
		endif()
	endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	set(CXX_FLAGS "-Wall")
elseif(MSVC)
	set(CXX_FLAGS "/EHsc /W3")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
	#
	# We use STL port under Sun Studio, standard library is broken
	#
	
	set(CXX_FLAGS "-library=stlport4")

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
		set(ICU_SUFFIX "d")
		if(NOT CPPCMS_SUFFIX)
			set(CPPCMS_SUFFIX "-d")
		else()
			set(CPPCMS_SUFFIX "${CPPCMS_SUFFIX}d")
		endif()
	endif()
endif()


if(USE_EXTERNAL_BOOST)
	set(CPPCMS_USE_EXTERNAL_BOOST 1)
endif(USE_EXTERNAL_BOOST)


if(NOT USE_EXTERNAL_BOOST)
	add_subdirectory(cppcms_boost)
	include_directories(cppcms_boost)
else(NOT USE_EXTERNAL_BOOST)
	find_path(BOOST_INCLUDE_DIR boost/shared_ptr.hpp)
	find_library(BOOST_SYSTEM	boost_system-${BOOST_SUFFIX} 	NAMES boost_system 	boost_system-mt)
	find_library(BOOST_FILESYSTEM 	boost_filesystem-${BOOST_SUFFIX} NAMES boost_filesystem boost_filesystem-mt)
	find_library(BOOST_REGEX	boost_regex-${BOOST_SUFFIX} 	NAMES boost_regex 	boost_regex-mt)
	find_library(BOOST_IOSTREAMS	boost_iostreams-${BOOST_SUFFIX} NAMES boost_iostreams 	boost_iostreams-mt)
	find_library(BOOST_THREAD	boost_thread-${BOOST_SUFFIX} 	NAMES boost_thread 	boost_thread-mt)
	find_library(BOOST_DATE_TIME	boost_date_time-${BOOST_SUFFIX} NAMES boost_date_time 	boost_date_time-mt)
	find_library(BOOST_ZLIB		boost_zlib-${BOOST_SUFFIX} 	NAMES boost_zlib 	boost_zlib)
	include_directories(${BOOST_INCLUDE_DIR})
endif(NOT USE_EXTERNAL_BOOST)


if(NOT USE_STD_LOCALES)
	find_library(ICU_UC icuuc${ICU_SUFFIX})
	find_library(ICU_DATA icudata NAMES icudt)
	find_library(ICU_I18N icui18n${ICU_SUFFIX} NAMES icuin${ICU_SUFFIX})
	find_path(ICU_INCLUDE_DIR unicode/unistr.h)

	if(NOT ICU_UC OR NOT ICU_DATA OR NOT ICU_I18N OR NOT ICU_INCLUDE_DIR)
		message(FATAL " Failed to find ICU library, can't continue")
	endif()
	set(HAVE_ICU 1)
else()
	set(CPPCMS_USE_STD_LOCALES 1)
endif()


find_path(HAVE_STDINT_H stdint.h)
find_path(HAVE_INTTYPES_H inttypes.h)

check_function_exists(atoll HAVE_ATOLL)
check_function_exists(_atoi64 HAVE_ATOI64)
check_function_exists(gmtime_r HAVE_GMTIME_R)
check_function_exists(localtime_r HAVE_LOCALTIME_R)
check_function_exists(strerror_r HAVE_STRERROR_R)
check_function_exists(snprintf HAVE_SNPRINTF)
check_function_exists(stat HAVE_STAT)
check_function_exists(_stat HAVE__STAT)
include_directories(${ICU_INCLUDE_DIR})


if(NOT DISABLE_ICONV) 

	check_cxx_source_compiles(
		"#include <iconv.h> 
		 int main() { iconv_t v=iconv_open((char *)0,(char *)0); }"
		 LIBC_ICONV)
	if(NOT LIBC_ICONV)
		find_path(ICONV_INCLUDE_DIR iconv.h)
		find_library(ICONV_LIB iconv)
		if(ICONV_LIB AND ICONV_INCLUDE_DIR)
			set(HAVE_ICONV 1)
			include_directories(${ICONV_INCLUDE_DIR})
		endif()
	else(NOT LIBC_ICONV)
		set(HAVE_ICONV 1)
	endif(NOT LIBC_ICONV)

	if(NOT HAVE_ICONV)
		message(STATUS " iconv not found, falling back to ICU")
	endif(NOT HAVE_ICONV)
endif(NOT DISABLE_ICONV)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

if(USE_STD_LOCALES)
	include_directories(noicu)
else()
	include_directories(localization)
endif()



set(CPPCMS_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
set(CPPCMS_LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})

#
# Atomic API tests
#

check_cxx_source_compiles(
	"#include <windows.h>
	int main() { volatile long v=0; return InterlockedCompareExchange(&v,1,0); }"
	HAVE_WIN32_INTERLOCKED)

check_cxx_source_compiles(
	"int main() { volatile long v=0; return __sync_add_and_fetch(&v,1); }"
	HAVE_SYNC_FETCH_AND_ADD)

check_cxx_source_compiles(
	"#include <bits/atomicity.h> 
	using __gnu_cxx::__exchange_and_add;
	int main(){ volatile int x=0; return __exchange_and_add(&x,1);}"
	HAVE_GCC_BITS_EXCHANGE_AND_ADD)

check_cxx_source_compiles(
	"#include <ext/atomicity.h> 
	using __gnu_cxx::__exchange_and_add;
	int main(){ volatile int x=0; return __exchange_and_add(&x,1);}"
	HAVE_GCC_EXT_EXCHANGE_AND_ADD)

check_cxx_source_compiles(
	"#include <sys/types.h>
	#include <machine/atomic.h>
	int main() { volatile unsigned v=0; return atomic_fetchadd_int(&v,1); }"
	HAVE_FREEBSD_ATOMIC)

check_cxx_source_compiles(
	"#include <atomic.h>
	int main() { volatile unsigned v=0; return atomic_add_int_nv(&v,1); }"
	HAVE_SOLARIS_ATOMIC)

check_cxx_source_compiles(
	"#include <libkern/OSAtomic.h>
	int main() { volatile int v=0; return OSAtomicAdd32(1,&v); }"
	HAVE_MAC_OS_X_ATOMIC)

check_cxx_source_compiles(
	"#include <string> 
	int main() { std::wstring s; }"
	HAVE_STD_WSTRING)

check_cxx_source_compiles(
	"#include <time.h> 
	int main() { struct tm t; char const *p=t.tm_zone; long o=t.tm_gmtoff; }"
	HAVE_BSD_TM)

check_cxx_source_compiles(
	"#include <string> 
	int main() { std::u16string s16; std::u32string s32; }"
	HAVE_CPP0X_UXSTRING)

check_cxx_source_compiles(
	"#include <list> 
	int main(){std::list<int> l; auto p=l.begin();}"
	HAVE_CPP_0X_AUTO)

check_cxx_source_compiles(
	"#include <list> 
	int main(){std::list<int> l; decltype(l.begin()) p=l.begin();}"
	HAVE_CPP_0X_DECLTYPE)

check_cxx_source_compiles(
	"#include <list> 
	int main(){std::list<int> l; typeof(l.begin()) p=l.begin();}"
	HAVE_GCC_TYPEOF)

check_cxx_source_compiles(
	"#include <list> 
	int main(){std::list<int> l; __typeof__(l.begin()) p=l.begin();}"
	HAVE_UNDERSCORE_TYPEOF)

if(${CMAKE_HOST_WIN32})
	find_library(WS2_32 ws2_32)
	find_library(WSOCK32 wsock32)
else(${CMAKE_HOST_WIN32})
	check_function_exists(dlopen HAVE_DLOPEN)
	if(NOT HAVE_DLOPEN)
		check_library_exists(dl dlopen "" HAVE_LIB_DL)
		if(NOT HAVE_LIB_DL)
			message(FATAL " No library with dlopen found")
		else(NOT HAVE_LIB_DL)
			find_library(LIB_DL dl)
		endif(NOT HAVE_LIB_DL)
	endif(NOT HAVE_DLOPEN)
	
	check_function_exists(socket HAVE_SOCKET)
	if(NOT HAVE_SOCKET)
		check_library_exists(socket socket "" HAVE_LIB_SOCKET)
		if(NOT HAVE_LIB_SOCKET)
			message(FATAL " No library with socket found")
		else(NOT HAVE_LIB_SOCKET)
			find_library(LIB_SOCKET socket)
		endif(NOT HAVE_LIB_SOCKET)
	endif(NOT HAVE_SOCKET)

	check_function_exists(gethostbyname HAVE_GETHOSTBYNAME)
	if(NOT HAVE_GETHOSTBYNAME)
		check_library_exists(socket gethostbyname "" LIB_SOCKGETHOSTBYNAME)
		if(NOT LIB_SOCKGETHOSTBYNAME)
			check_library_exists(nsl gethostbyname "" HAVE_LIB_NSL)
			if(NOT HAVE_LIB_NSL)
				message(FATAL " No library with gethostbyname found")
			else(NOT HAVE_LIB_NSL)
				find_library(LIB_NSL nsl)
			endif(NOT HAVE_LIB_NSL)
		endif(NOT LIB_SOCKGETHOSTBYNAME)
	endif(NOT HAVE_GETHOSTBYNAME)
endif(${CMAKE_HOST_WIN32}) 


check_function_exists(canonicalize_file_name HAVE_CANONICALIZE_FILE_NAME)

if(USE_EXTERNAL_BOOST)
	if(WIN32)
		add_definitions(-DBOOST_ALL_NO_LIB)
		add_definitions(-DBOOST_REGEX_DYN_LINK)
		add_definitions(-DBOOST_FILESYSTEM_DYN_LINK)
		add_definitions(-DBOOST_SYSTEM_DYN_LINK)
		add_definitions(-DBOOST_IOSTREAMS_DYN_LINK)
		add_definitions(-DBOOST_THREAD_DYN_LINK)
		add_definitions(-DBOOST_DATE_TIME_DYN_LINK)
		add_definitions(-DBOOST_ZLIB_DYN_LINK)
	endif(WIN32)
else(USE_EXTERNAL_BOOST)
	add_definitions(-DCPPCMS_BOOST_ALL_NO_LIB)
	if(NOT DISABLE_SHARED)
		add_definitions(-DCPPCMS_BOOST_ALL_DYN_LINK)
	endif(NOT DISABLE_SHARED)
endif(USE_EXTERNAL_BOOST)

set(CPPCMS_PUBLIC_HEADERS
	aio_timer.h
	application.h
	applications_pool.h
	atomic_counter.h
	base64.h
	base_content.h
	base_view.h
	cache_interface.h
	cache_pool.h
	callback.h
	callback0.h
	callback1.h
	callback2.h
	callback3.h
	callback4.h
	clone_ptr.h
	# connection_forwarder.h
	copy_ptr.h
	cppcms_error.h
	cstdint.h
	defs.h
	encoding.h
	filters.h
	form.h
	hold_ptr.h
	http_context.h
	http_cookie.h
	http_file.h
	http_request.h
	http_response.h
	intrusive_ptr.h
	json.h
	mem_bind.h
	noncopyable.h
	refcounted.h
	regex.h
	rpc_json.h
	service.h
	session_api.h
	session_cookies.h
	session_dual.h
	session_interface.h
	session_pool.h
	session_sid.h
	session_storage.h
	signal0.h
	thread_pool.h
	urandom.h
	url_dispatcher.h
	util.h
	view.h
	views_pool.h
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)


set(CPPCMS_SOURCES 
	service.cpp 
	cgi_api.cpp 
	http_request.cpp 
	http_response.cpp 
	http_context.cpp 
	cppcms_error.cpp 
	cppcms_error_category.cpp 
	thread_pool.cpp 
	applications_pool.cpp 
	application.cpp 
	url_dispatcher.cpp 
	http_cookie.cpp  
	util.cpp 
	base64.cpp 
	base_view.cpp 
	views_pool.cpp
	internal_file_server.cpp 
	atomic_counter.cpp 
	aio_timer.cpp 
	json.cpp 
	encoding.cpp 
	regex.cpp
	form.cpp 
	filters.cpp
	cache_pool.cpp
	cache_interface.cpp
	cache_storage.cpp
	tcp_cache_client.cpp
	cache_over_ip.cpp
	tcp_messenger.cpp
	tcp_connector.cpp
	# connection_forwarder.cpp
	session_pool.cpp
	base_encryptor.cpp
	hmac_encryptor.cpp
	md5.cpp
	urandom.cpp
	session_cookies.cpp
	session_dual.cpp
	session_sid.cpp
	session_interface.cpp
	session_memory_storage.cpp
)

if(NOT USE_STD_LOCALES)

	set(CPPCMS_SOURCES ${CPPCMS_SOURCES}
		localization/locale_src_boundary.cpp
		localization/locale_src_codepage.cpp
		localization/locale_src_collator.cpp
		localization/locale_src_conversion.cpp
		localization/locale_src_date_time.cpp
		localization/locale_src_format.cpp
		localization/locale_src_formatter.cpp
		localization/locale_src_formatting.cpp
		localization/locale_src_generator.cpp
		localization/locale_src_info.cpp
		localization/locale_src_message.cpp
		localization/locale_src_mo_lambda.cpp
		localization/locale_src_time_zone.cpp)

	set(CPPCMS_PUBLIC_HEADERS ${CPPCMS_PUBLIC_HEADERS}
		localization/locale_boundary.h
		localization/locale_codepage.h
		localization/locale_collator.h
		localization/locale_conversion.h
		localization/locale_date_time.h
		localization/locale_format.h
		localization/locale_formatting.h
		localization/locale_generator.h
		localization/locale_info.h
		localization/locale_message.h
		localization/locale_time_zone.h
		localization/localization.h)


else(NOT USE_STD_LOCALES)

	set(CPPCMS_SOURCES ${CPPCMS_SOURCES}
		noicu/locale_src_conversion.cpp
		noicu/locale_src_format.cpp
		noicu/locale_src_formatting.cpp
		noicu/locale_src_generator.cpp
		noicu/locale_src_info.cpp
		noicu/locale_src_message.cpp
		noicu/locale_src_mo_lambda.cpp
		noicu/locale_src_numeric.cpp)

	set(CPPCMS_PUBLIC_HEADERS ${CPPCMS_PUBLIC_HEADERS}
		noicu/locale_conversion.h
		noicu/locale_format.h
		noicu/locale_formatting.h
		noicu/locale_generator.h
		noicu/locale_info.h
		noicu/locale_message.h
		noicu/locale_numeric.h
		noicu/localization.h)

endif(NOT USE_STD_LOCALES)

if(NOT DISABLE_GCRYPT)
	find_path(GCRYPT_INCLUDE_DIR gcrypt.h)
	find_library(LIB_GCRYPT gcrypt)
	if(LIB_GCRYPT AND GCRYPT_INCLUDE_DIR)
		include_directories(${GCRYPT_INCLUDE_DIR})
	endif()
endif(NOT DISABLE_GCRYPT)

if(NOT DISABLE_FCGI)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} fastcgi_api.cpp)
	set(CPPCMS_HAS_FCGI 1)
endif()

if(NOT DISABLE_SCGI)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} scgi_api.cpp)
	set(CPPCMS_HAS_SCGI 1)
endif()


if(NOT DISABLE_HTTP)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} http_api.cpp)
	set(CPPCMS_HAS_HTTP 1)
endif()
	


if(LIB_GCRYPT)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} aes_encryptor.cpp)
	set(HAVE_GCRYPT 1)
else(LIB_GCRYPT)
	message("GNU Gcrypt library not found, disabling aes_encryptor")
endif(LIB_GCRYPT)


if(WIN32 AND NOT CYGWIN)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} session_win32_file_storage.cpp)
else(WIN32 AND NOT CYGWIN)
	set(CPPCMS_SOURCES ${CPPCMS_SOURCES} session_posix_file_storage.cpp)
endif(WIN32 AND NOT CYGWIN)

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
	target_link_libraries(${ALIB} ${ICU_UC})
	target_link_libraries(${ALIB} ${ICU_DATA})
	target_link_libraries(${ALIB} ${ICU_I18N})
	if(NOT USE_EXTERNAL_BOOST)
		if(DISABLE_SHARED)
			target_link_libraries(${ALIB} boost-static)
		else(DISABLE_SHARED)
			target_link_libraries(${ALIB} boost)
		endif(DISABLE_SHARED)
	else(NOT USE_EXTERNAL_BOOST)
		target_link_libraries(${ALIB} ${ICU_UC})
		target_link_libraries(${ALIB} ${ICU_DATA})
		target_link_libraries(${ALIB} ${ICU_I18N})
		target_link_libraries(${ALIB} ${BOOST_REGEX})
		target_link_libraries(${ALIB} ${BOOST_IOSTREAMS})
		target_link_libraries(${ALIB} ${BOOST_DATE_TIME})
		target_link_libraries(${ALIB} ${BOOST_THREAD})
		target_link_libraries(${ALIB} ${BOOST_FILESYSTEM})
		target_link_libraries(${ALIB} ${BOOST_SYSTEM})
		if(BOOST_ZLIB)
			target_link_libraries(${ALIB} ${BOOST_ZLIB})
		endif(BOOST_ZLIB)
	endif(NOT USE_EXTERNAL_BOOST)

	if(LIB_GCRYPT)
		target_link_libraries(${ALIB} ${LIB_GCRYPT})
	endif(LIB_GCRYPT)
	
	if(ICONV_LIB)
		target_link_libraries(${ALIB} ${ICONV_LIB})
	endif(ICONV_LIB)

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
endforeach(ALIB)


find_program(PYTHON python)
if(NOT PYTHON)
	message(FATAL "Can't build without Python interpreter")
endif(NOT PYTHON)


# Templates builds

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/skin1.cpp
	COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/cppcms_tmpl_cc 
		-s skin1 
		-o ${CMAKE_CURRENT_BINARY_DIR}/skin1.cpp 
			${CMAKE_CURRENT_SOURCE_DIR}/hello_world_skin1.tmpl 
			${CMAKE_CURRENT_SOURCE_DIR}/hello_world_view1.tmpl
	DEPENDS 
		${CMAKE_CURRENT_SOURCE_DIR}/cppcms_tmpl_cc 
		${CMAKE_CURRENT_SOURCE_DIR}/hello_world_skin1.tmpl 
		${CMAKE_CURRENT_SOURCE_DIR}/hello_world_view1.tmpl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/skin2.cpp
	COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/cppcms_tmpl_cc 
		-s skin2 
		-o ${CMAKE_CURRENT_BINARY_DIR}/skin2.cpp 
			${CMAKE_CURRENT_SOURCE_DIR}/hello_world_skin2.tmpl 
			${CMAKE_CURRENT_SOURCE_DIR}/hello_world_view1.tmpl
	DEPENDS 
		${CMAKE_CURRENT_SOURCE_DIR}/cppcms_tmpl_cc 
		${CMAKE_CURRENT_SOURCE_DIR}/hello_world_skin2.tmpl 
		${CMAKE_CURRENT_SOURCE_DIR}/hello_world_view1.tmpl)

if(NOT DISABLE_SHARED)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/skin3.cpp
		COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/cppcms_tmpl_cc 
			-s skin3 
			-o ${CMAKE_CURRENT_BINARY_DIR}/skin3.cpp 
				${CMAKE_CURRENT_SOURCE_DIR}/hello_world_skin2.tmpl 
				${CMAKE_CURRENT_SOURCE_DIR}/hello_world_view1.tmpl
		DEPENDS 
			${CMAKE_CURRENT_SOURCE_DIR}/cppcms_tmpl_cc 
			${CMAKE_CURRENT_SOURCE_DIR}/hello_world_skin2.tmpl 
			${CMAKE_CURRENT_SOURCE_DIR}/hello_world_view1.tmpl)
	add_library(skin3 MODULE skin3.cpp)
	set(SKIN3_DEFS CPPCMS_SKIN_MODULE DLL_EXPORT)
	set_target_properties(skin3 PROPERTIES
				COMPILE_DEFINITIONS "${SKIN3_DEFS}")
	target_link_libraries(skin3 ${CPPCMS_LIB})
endif(NOT DISABLE_SHARED)



add_executable(hello_world hello_world.cpp skin1.cpp skin2.cpp)
target_link_libraries(hello_world ${CPPCMS_LIB})

add_executable(cppcms_scale tcp_cache_server.cpp)
target_link_libraries(cppcms_scale ${CPPCMS_LIB})

add_executable(cppcms_config_find_param cppcms_config_find_param.cpp json.cpp)

#########################################
# Tests
#########################################

set(ALL_TESTS 
	form_test
	proto_test
	atomic_test
	encryptor_test
	storage_test
	cache_backend_test
	json_test
	)
foreach(TEST ${ALL_TESTS})
	add_executable(${TEST} tests/${TEST}.cpp)
	target_link_libraries(${TEST} ${CPPCMS_LIB})
endforeach()

if(NOT USE_STD_LOCALES)
	file(GLOB LOCALE_TEST_SOURCES "localization/locale_src_test_*.cpp")
else()
	file(GLOB LOCALE_TEST_SOURCES "noicu/locale_src_test_*.cpp")
endif()

foreach(LFILE ${LOCALE_TEST_SOURCES})
	string(REGEX REPLACE ".*(localization|noicu)/(.*)\\.cpp" "\\2" TEST_NAME ${LFILE})
	add_executable(${TEST_NAME} ${LFILE})
	target_link_libraries(${TEST_NAME} ${CPPCMS_LIB})
	set(LOCALE_TESTS "${LOCALE_TESTS} ${TEST_NAME}")
	add_test(${TEST_NAME} ${TEST_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/tests")
endforeach()



#####################################
# End of tests
#####################################

# These are use export
if(WIN32 AND NOT DISABLE_SHARED)
	set_target_properties(${ALL_TESTS} hello_world cppcms PROPERTIES
				COMPILE_DEFINITIONS DLL_EXPORT)
endif(WIN32 AND NOT DISABLE_SHARED)

set_target_properties(${CPPCMS_LIBS} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(${CPPCMS_LIBS} PROPERTIES OUTPUT_NAME "cppcms${CPPCMS_SUFFIX}")
if(WIN32)
	set_target_properties(${CPPCMS_LIBS} PROPERTIES VERSION 1 SOVERSION 1)
else()
	set_target_properties(${CPPCMS_LIBS} PROPERTIES VERSION 1.0.0 SOVERSION 1)
endif()

set_target_properties(${CPPCMS_LIBS} PROPERTIES 
			PUBLIC_HEADER "${CPPCMS_PUBLIC_HEADERS}")
			
if(MSVC AND NOT DISABLE_STATIC)
	set_target_properties(cppcms-static PROPERTIES PREFIX "lib")
endif(MSVC AND NOT DISABLE_STATIC)

configure_file(config.cmake.h config.h)


install(TARGETS ${CPPCMS_LIBS} cppcms_config_find_param cppcms_scale
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	PUBLIC_HEADER DESTINATION include/cppcms)

install(PROGRAMS cppcms_tmpl_cc cppcms_run
	DESTINATION bin)

#############
# TESTS
#############
		

set(CNF "${CMAKE_CURRENT_SOURCE_DIR}/tests")

add_test(atomic_test atomic_test)
add_test(encryptor_test encryptor_test)
add_test(storage_test storage_test)
add_test(json_test json_test)
add_test(cache_backend_test cache_backend_test)

add_test(form_test
	form_test "-c" "${CNF}/form_test.js" 
	"--test-exec=${PYTHON} ${CNF}/form_test.py")

foreach(TYPE async sync)
	add_test(proto_test_${TYPE}_http 
		proto_test "-c" "${CNF}/proto_test.js" 
		"--test-async=${TYPE}" "--service-api=http" "--service-port=8080" "--service-ip=127.0.0.1"
		"--test-exec=${PYTHON} ${CNF}/proto_test.py http")
	add_test(proto_test_${TYPE}_scgi 
		proto_test "-c" "${CNF}/proto_test.js" 
		"--test-async=${TYPE}" "--service-api=scgi" "--service-port=8080" "--service-ip=127.0.0.1"
		"--test-exec=${PYTHON} ${CNF}/proto_test.py scgi_tcp")
	if(NOT WIN32)
		add_test(proto_test_${TYPE}_scgi_unix
			proto_test "-c" "${CNF}/proto_test.js" 
			"--test-async=${TYPE}" "--service-api=scgi" "--service-socket=/tmp/cppcms_test_socket"
			"--test-exec=${PYTHON} ${CNF}/proto_test.py scgi_unix")
	endif()
endforeach()

