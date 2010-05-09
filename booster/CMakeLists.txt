cmake_minimum_required(VERSION 2.6)
project(booster)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(CPack)
enable_testing()

option(DISABLE_SHARED	"Disable shared libraries build" OFF)
option(DISABLE_STATIC	"Disable static libraries build" OFF)



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

if(${CMAKE_HOST_WIN32})
	find_library(WS2_32 ws2_32)
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
)


if(NOT HAVE_PTHREAD)
	find_library(LIB_PTHREAD pthread NAMES thr kse pthreadGC2)
endif(NOT HAVE_PTHREAD)


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
include_directories(.)
include_directories(lib/test)

set(LINK_LIBS )

if(NOT DISABLE_SHARED)
	add_library(booster SHARED ${BOOSTER_SRC})
	set(LINK_LIBS ${LINK_LIBS} booster)
	if(WIN32)
		set(BOOSTER_SHARED_DEFS ${BOOSTER_SHARED_DEFS} DLL_EXPORT)
	endif(WIN32)
	set_target_properties(booster PROPERTIES COMPILE_DEFINITIONS "${BOOST_SHARED_DEFS}")
	set_target_properties(booster PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(booster PROPERTIES OUTPUT_NAME "booster")

	install(TARGETS booster
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)
endif(NOT DISABLE_SHARED)	

if(NOT DISABLE_STATIC)
	add_library(booster-static STATIC ${BOOSTER_SRC})
	set(LINK_LIBS ${LINK_LIBS} booster-static)
	set_target_properties(booster-static PROPERTIES COMPILE_DEFINITIONS "${BOOST_STATIC_DEFS}")
	set_target_properties(booster-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(booster-static PROPERTIES OUTPUT_NAME "cppcms_boost")
	install(TARGETS booster-static
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)
endif(NOT DISABLE_STATIC)	


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
endforeach()


if(MSVC AND NOT DISABLE_STATIC)
	set_target_properties(booster-static PROPERTIES PREFIX "lib")
endif(MSVC AND NOT DISABLE_STATIC)

set(EXE_COM_DEFS "")

if(NOT DISABLE_SHARED)
	set(BOOSTER_LIB booster)
	if(WIN32)
		set(EXE_COM_DEFS "${EXE_COM_DEFS} DLL_EXPORT")
	endif()
else()
	set(BOOSTER_LIB booster-static)
endif()

macro(add_booster_test MODULE TEST)
	set(TEST_SRC "lib/${MODULE}/test/test_${TEST}.cpp")
	set(TEST_NAME "test_${MODULE}_${TEST}")
	add_executable(${TEST_NAME} ${TEST_SRC})
	target_link_libraries(${TEST_NAME} ${BOOSTER_LIB})
	set_target_properties(${TEST_NAME} PROPERTIES COMPILE_DEFINITIONS "${EXE_COM_DEFS}")
	add_test(${TEST_NAME} ${TEST_NAME})
	set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 10)
endmacro()

add_booster_test(function function)
add_booster_test(ptime posix_time)
add_booster_test(thread thread)
if(UNIX)
	add_booster_test(thread fork)
endif()
add_booster_test(regex regex)
add_booster_test(aio reactor)
add_booster_test(aio timer)
add_booster_test(aio event_loop)
add_booster_test(aio socket)


