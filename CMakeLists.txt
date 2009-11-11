cmake_minimum_required(VERSION 2.4)
project(cppcms)
add_subdirectory(boost_locale/libs/locale)

include(CheckFunctionExists)
include(CheckCXXSourceCompiles)

set(PACKAGE_NAME "CppCMS")
set(PACKAGE_STRING "CppCMS/0.99.1")
set(PACKAGE_VERSION "0.99.1")


find_library(BOOST_SYSTEM	boost_system-${BOOST_SUFFIX} 	NAMES boost_system 	boost_system-mt)
find_library(BOOST_FILESYSTEM 	boost_filesystem-${BOOST_SUFFIX} NAMES boost_filesystem boost_filesystem-mt)
find_library(BOOST_REGEX	boost_regex-${BOOST_SUFFIX} 	NAMES boost_regex 	boost_regex-mt)
find_library(BOOST_IOSTREAMS	boost_iostreams-${BOOST_SUFFIX} NAMES boost_iostreams 	boost_iostreams-mt)
find_library(BOOST_THREAD	boost_thread-${BOOST_SUFFIX} 	NAMES boost_thread 	boost_thread-mt)
find_library(BOOST_DATE_TIME	boost_date_time-${BOOST_SUFFIX} NAMES boost_date_time 	boost_date_time-mt)

find_library(ICU_UC icuuc)
find_library(ICU_DATA icudata NAMES icudt)
find_library(ICU_I18N icui18n NAMES icuin)

if(NOT ICU_UC OR NOT ICU_DATA OR NOT ICU_I18N)
	message(FATAL " Failed to find icu")
endif(NOT ICU_UC OR NOT ICU_DATA OR NOT ICU_I18N)

set(HAVE_ICU 1)

if(NOT DISABLE_ICONV) 

	check_cxx_source_compiles(
		"#include <iconv.h> 
		 int main() { iconv_t v=iconv_open((char *)0,(char *)0); }"
		 LIBC_ICONV)
	if(NOT LIBC_ICONV)
		find_library(ICONV_LIB iconv)
		if(ICONV_LIB)
			set(HAVE_ICONV 1)
		endif(ICONV_LIB)
	else(NOT LIBC_ICONV)
		set(HAVE_ICONV 1)
	endif(NOT LIBC_ICONV)

	if(NOT HAVE_ICONV)
		message(STATUS " iconv not found, falling back to ICU")
	endif(NOT HAVE_ICONV)
endif(NOT DISABLE_ICONV)

include_directories(${CMAKE_BINARY_DIR})
include_directories(boost_locale)


check_cxx_source_compiles(
	"int main() { volatile long v;__sync_add_and_fetch(&v,1); }"
	HAVE_SYNC_FETCH_AND_ADD)
check_cxx_source_compiles(
	"#include <string> 
	int main() { std::wstring s; }"
	HAVE_STD_WSTRING)

check_cxx_source_compiles(
	"#include <string> 
	int main() { std::u16string s16; std::u32string s32; }"
	HAVE_CPP0X_UXSTRING)

Check_cxx_source_compiles(
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
	check_function_exists(socket HAVE_SOCKET)
	if(NOT HAVE_SOCKET)
		check_library_exists(socket socket "" LIB_SOCKET)
		if(NOT HAVE_SOCKET)
			message(FATAL " No library with socket found")
		endif(NOT HAVE_SOCKET)
	endif(NOT HAVE_SOCKET)

	check_function_exists(gethostbyname HAVE_GETHOSTBYNAME)
	if(NOT HAVE_GETHOSTBYNAME)
		check_library_exists(socket gethostbyname "" LIB_GETHOSTBYNAME)
		if(NOT LIB_GETHOSTBYNAME)
			check_library_exists(nsl gethostbyname "" LIB_GETHOSTBYNAME)
			if(NOT LIB_GETHOSTBYNAME)
				message(FATAL " No library with gethostbyname found")
			endif(NOT LIB_GETHOSTBYNAME)
		endif(NOT LIB_GETHOSTBYNAME)
	endif(NOT HAVE_GETHOSTBYNAME)
endif(${CMAKE_HOST_WIN32}) 


configure_file(config.cmake.h config.h)


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
	internal_file_server.cpp 
	scgi_api.cpp 
	fastcgi_api.cpp 
	http_api.cpp 
	atomic_counter.cpp 
	aio_timer.cpp 
	json.cpp 
	encoding.cpp 
	form.cpp 
	filters.cpp)


add_library(cppcms SHARED ${CPPCMS_SOURCES})

target_link_libraries(cppcms ${ICU_UC})
target_link_libraries(cppcms ${ICU_DATA})
target_link_libraries(cppcms ${ICU_I18N})
target_link_libraries(cppcms ${BOOST_REGEX})
target_link_libraries(cppcms ${BOOST_IOSTREAMS})
target_link_libraries(cppcms ${BOOST_DATE_TIME})
target_link_libraries(cppcms ${BOOST_THREAD})
target_link_libraries(cppcms ${BOOST_FILESYSTEM})
target_link_libraries(cppcms ${BOOST_SYSTEM})
target_link_libraries(cppcms boost_locale)


if(ICONV_LIB)
	target_link_libraries(cppcms ${ICONV_LIB})
endif(ICONV_LIB)

if(WS2_32 AND WSOCK32)
	target_link_libraries(cppcms ${WS2_32})
	target_link_libraries(cppcms ${WSOCK32})
endif(WS2_32 AND WSOCK32) 

if(LIB_GETHOSTBYNAME)
	target_link_libraries(cppcms LIB_GETHOSTBYNAME)
endif(LIB_GETHOSTBYNAME)

if(LIB_SOCKET)
	target_link_libraries(cppcms LIB_SOCKET)
endif(LIB_SOCKET)

add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world cppcms)


add_executable(cppcms_config_find_param cppcms_config_find_param.cpp json.cpp)

# These are use export
if(WIN32)
	set_target_properties(hello_world cppcms PROPERTIES
				COMPILE_DEFINITIONS DLL_EXPORT)
endif(WIN32)

