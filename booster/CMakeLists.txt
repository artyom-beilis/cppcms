#
#  Copyright (c) 2010 Artyom Beilis (Tonkikh)
#
#  Distributed under the Boost Software License, Version 1.0. (See
#  accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 2.6)
project(booster)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(CPack)
enable_testing()

option(DISABLE_SHARED	"Disable shared libraries build" OFF)
option(DISABLE_STATIC	"Disable static libraries build" OFF)
option(DISABLE_ICU_LOCALIZATION "Disable locale library" OFF)



#############################################################################
#
# Setup various build flags for different supported compilers and systems
#
#############################################################################

if(DISABLE_ICU_LOCALIZATION)
	set(BOOSTER_SUFFIX "-c")
endif()


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
		if(NOT BOOSTER_SUFFIX)
			set(BOOSTER_SUFFIX "-d")
		else()
			set(BOOSTER_SUFFIX "${BOOSTER_SUFFIX}d")
		endif()
	endif()
endif()

if(WIN32)
	find_library(WS2_32 ws2_32)
else()
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
endif()



check_cxx_source_compiles(
	"int main() { volatile int v=0; return __sync_bool_compare_and_swap(&v,0,1); }"
	BOOSTER_HAS_GCC_SYNC)

check_cxx_source_compiles(
	"#include <bits/atomicity.h> 
	using __gnu_cxx::__exchange_and_add;
	int main(){ volatile int x=0; return __exchange_and_add(&x,1);}"
	BOOSTER_HAVE_GCC_BITS_EXCHANGE_AND_ADD)

check_cxx_source_compiles(
	"#include <ext/atomicity.h> 
	using __gnu_cxx::__exchange_and_add;
	int main(){ volatile int x=0; return __exchange_and_add(&x,1);}"
	BOOSTER_HAVE_GCC_EXT_EXCHANGE_AND_ADD)

check_cxx_source_compiles(
	"#include <sys/types.h>
	#include <machine/atomic.h>
	int main() { volatile unsigned v=0; return atomic_cmpset_int(&v,1,0); }"
	BOOSTER_HAVE_FREEBSD_ATOMIC)

check_cxx_source_compiles(
	"#include <atomic.h>
	int main() { volatile unsigned v=0; return atomic_add_int_nv(&v,1); }"
	BOOSTER_HAVE_SOLARIS_ATOMIC)

check_cxx_source_compiles(
	"#include <libkern/OSAtomic.h>
	int main() { volatile int v=0; return OSAtomicAdd32(1,&v); }"
	BOOSTER_HAVE_MAC_OS_X_ATOMIC)

check_cxx_source_compiles(
	"#include <stdint.h>
	int main() { int64_t x=10; return 0; }"
	BOOSTER_HAVE_STDINT_H)

check_cxx_source_compiles(
	"#include <inttypes.h>
	int main() { int64_t x=10; return 0; }"
	BOOSTER_HAVE_INTTYPES_H)

if(WIN32)
	set(CMAKE_REQUIRED_LIBRARIES ${WS2_32})
	check_cxx_source_compiles(
	"#include <winsock2.h>
	#include <windows.h>
	int main()
	{ struct sockaddr_in6 in6; struct in6_addr in6addr; ::inet_pton(AF_INET6,\"::1\",&in6addr); return 0; }"
	BOOSTER_AIO_HAVE_PF_INET6)
else()
	set(CMAKE_REQUIRED_LIBRARIES ${LIB_SOCKGETHOSTBYNAME} ${LIB_SOCKET} ${LIB_NSL})
	check_cxx_source_compiles(
	"#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <sys/un.h>
	#include <netinet/in.h>
	int main()
	{ struct sockaddr_in6 in6; struct in6_addr in6addr; ::inet_pton(AF_INET6,\"::1\",&in6addr); return 0; }"
	BOOSTER_AIO_HAVE_PF_INET6)
endif()



set(BOOSTER_SRC	
	lib/thread/src/thread.cpp
	lib/ptime/src/posix_time.cpp
	lib/regex/src/pcre_regex.cpp
	lib/system/src/posix_error.cpp
	lib/system/src/windows_error.cpp
	lib/aio/src/aio_category.cpp
	lib/aio/src/deadline_timer.cpp
	lib/aio/src/endpoint.cpp
	lib/aio/src/io_service.cpp
	lib/aio/src/reactor.cpp
	lib/aio/src/select_iterrupter.cpp
	lib/aio/src/socket.cpp
	lib/smart_ptr/src/sp_counted_base.cpp
	lib/smart_ptr/src/atomic_counter.cpp
	lib/log/src/log.cpp
)

if(NOT DISABLE_ICU_LOCALIZATION)
	message("-- Looking for ICU libraries")

	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		if(MSVC)
			set(ICU_SUFFIX "d")
		endif()
	endif()

	find_library(ICU_UC icuuc${ICU_SUFFIX})
	find_library(ICU_DATA icudt NAMES icudata)
	find_library(ICU_I18N icuin${ICU_SUFFIX} NAMES icui18n${ICU_SUFFIX})
	find_path(ICU_INCLUDE_DIR unicode/unistr.h)

	if(ICU_UC AND ICU_DATA AND ICU_I18N AND ICU_INCLUDE_DIR)
		message("-- ICU Found, building booster locale")
		set(WITH_BOOSTER_LOCALE ON)
		include_directories(${ICU_INCLUDE_DIR})
		set(BOOSTER_SRC ${BOOSTER_SRC}
			lib/locale/src/boundary.cpp
			lib/locale/src/codepage.cpp
			lib/locale/src/collator.cpp
			lib/locale/src/conversion.cpp
			lib/locale/src/date_time.cpp
			lib/locale/src/format.cpp
			lib/locale/src/formatter.cpp
			lib/locale/src/formatting.cpp
			lib/locale/src/generator.cpp
			lib/locale/src/info.cpp
			lib/locale/src/message.cpp
			lib/locale/src/mo_lambda.cpp
			lib/locale/src/time_zone.cpp)

	endif()
endif()








if(NOT HAVE_PTHREAD)
	if(MSVC)
		find_library(LIB_PTHREAD pthreadVC2)
	else()
		find_library(LIB_PTHREAD pthread NAMES thr kse pthreadGC2)
	endif()
endif(NOT HAVE_PTHREAD)




find_path(PTHREAD_INC pthread.h)
include_directories(${PTHREAD_INC})

find_path(PCRE_INCLUDE pcre.h)
if(PCRE_INCLUDE)
	include_directories(${PCRE_INCLUDE})
endif()

if(NOT PCRE_INCLUDE)
	find_path(PCRE_INCLUDE2 pcre/pcre.h)
	include_directories(${PCRE_INCLUDE2}/pcre)
endif()

find_library(PCRE_LIB pcre)


include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)
include_directories(lib/test)

set(LINK_LIBS )

if(NOT DISABLE_SHARED)
	add_library(booster SHARED ${BOOSTER_SRC})
	set(LINK_LIBS ${LINK_LIBS} booster)
	if(WIN32)
		set(BOOSTER_SHARED_DEFS ${BOOSTER_SHARED_DEFS} DLL_EXPORT)
	endif(WIN32)
	set_target_properties(booster PROPERTIES COMPILE_DEFINITIONS "${BOOSTER_SHARED_DEFS}")
	set_target_properties(booster PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(booster PROPERTIES OUTPUT_NAME "booster${BOOSTER_SUFFIX}")

endif(NOT DISABLE_SHARED)	

if(NOT DISABLE_STATIC)
	add_library(booster-static STATIC ${BOOSTER_SRC})
	set(LINK_LIBS ${LINK_LIBS} booster-static)
	set_target_properties(booster-static PROPERTIES COMPILE_DEFINITIONS "${BOOST_STATIC_DEFS}")
	set_target_properties(booster-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(booster-static PROPERTIES OUTPUT_NAME "booster${BOOSTER_SUFFIX}")
endif(NOT DISABLE_STATIC)

install(DIRECTORY booster DESTINATION include
        PATTERN ".svn" EXCLUDE)


foreach(ALIB ${LINK_LIBS})
	if(LIB_SOCKGETHOSTBYNAME)
		target_link_libraries(${ALIB} ${LIB_SOCKGETHOSTBYNAME})
	endif(LIB_SOCKGETHOSTBYNAME)
	if(LIB_NSL)
		target_link_libraries(${ALIB} ${LIB_NSL})
	endif(LIB_NSL)

	if(LIB_SOCKET)
		target_link_libraries(${ALIB} ${LIB_SOCKET})
	endif(LIB_SOCKET)
	if(LIB_PTHREAD)
		target_link_libraries(${ALIB} ${LIB_PTHREAD})
	endif(LIB_PTHREAD)
	if(WS2_32)
		target_link_libraries(${ALIB} ${WS2_32})
	endif()
	target_link_libraries(${ALIB} ${PCRE_LIB})
	if(WITH_BOOSTER_LOCALE)
		target_link_libraries(${ALIB} ${ICU_UC})
		target_link_libraries(${ALIB} ${ICU_I18N})
		target_link_libraries(${ALIB} ${ICU_DATA})
	endif()
	if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.6)
		set_target_properties(${ALIB} PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_BINARY_DIR}/booster/build_config.h")
	else()
		set_target_properties(${ALIB} PROPERTIES PUBLIC_HEADER "${CMAKE_BINARY_DIR}/booster/build_config.h")
	endif()
	
	if(WIN32)
		set_target_properties(${ALIB} PROPERTIES VERSION 0 SOVERSION 0)
	else()
		set_target_properties(${ALIB} PROPERTIES VERSION 0.0.0 SOVERSION 0)
	endif()


endforeach()


if(MSVC AND NOT DISABLE_STATIC)
	set_target_properties(booster-static PROPERTIES PREFIX "lib")
endif(MSVC AND NOT DISABLE_STATIC)

set(EXE_COM_DEFS "")

if(NOT DISABLE_SHARED)
	set(BOOSTER_LIB booster)
	if(WIN32)
		set(EXE_COM_DEFS "${EXE_COM_DEFS}" "DLL_EXPORT")
	endif()
else()
	set(BOOSTER_LIB booster-static)
endif()

macro(add_booster_param_test MODULE TEST PARAMETER)
	set(TEST_SRC "lib/${MODULE}/test/test_${TEST}.cpp")
	set(TEST_NAME "test_${MODULE}_${TEST}")
	add_executable(${TEST_NAME} ${TEST_SRC})
	target_link_libraries(${TEST_NAME} ${BOOSTER_LIB})
	set_target_properties(${TEST_NAME} PROPERTIES COMPILE_DEFINITIONS "${EXE_COM_DEFS}")
	add_test(${TEST_NAME} ${TEST_NAME} ${PARAMETER})
	set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 10)
endmacro()

macro(add_booster_test MODULE TEST)
	add_booster_param_test(${MODULE} ${TEST} "")
endmacro()

add_booster_test(function function)
add_booster_test(ptime posix_time)
add_booster_test(thread thread)
if(UNIX)
	add_booster_test(thread fork)
endif()

add_booster_test(smart_ptr shared_ptr)
add_booster_test(smart_ptr atomic_counter)
add_booster_test(smart_ptr sp_counter)

add_booster_test(log log)

add_booster_test(regex regex)
add_booster_test(aio reactor)
add_booster_test(aio timer)
add_booster_test(aio event_loop)
add_booster_test(aio socket)
add_booster_test(aio endpoint)
if(NOT WIN32)
	add_booster_test(aio prefork)
endif()

if(WITH_BOOSTER_LOCALE)
	add_booster_test(locale boundary)
	add_booster_test(locale codepage)
	add_booster_test(locale collate)
	add_booster_test(locale convert)
	add_booster_test(locale date_time)
	add_booster_test(locale formatting)
	add_booster_test(locale generator)
	add_booster_test(locale ios_prop)
	add_booster_test(locale icu_vs_os_timezone)
	add_booster_param_test(locale message "${CMAKE_CURRENT_SOURCE_DIR}/lib/locale/test")
endif()

configure_file(lib/booster_build_config.cmake.h booster/build_config.h)
install(TARGETS ${LINK_LIBS}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	PUBLIC_HEADER DESTINATION include/booster)

