cmake_minimum_required(VERSION 2.4)
project(cppcms)

include(CheckTypeSize)
#include(CheckFunctionExists)
#include(CheckLibraryExists)

set(PACKAGE_NAME "CppCMS")
set(PACKAGE_STRING "CppCMS/0.99.1")
set(PACKAGE_VERSION "0.99.1")

find_package(Boost 1.36.0 COMPONENTS system filesystem regex iostreams thread date_time)  

find_library(ICU_UC icuuc)
find_library(ICU_DATA icudata NAMES icudt)
find_library(ICU_I18N icui18n NAMES icuin)

if(NOT ICU_UC OR NOT ICU_DATA OR NOT ICU_I18N)
	message(FATAL " Failed to find icu")
endif(NOT ICU_UC OR NOT ICU_DATA OR NOT ICU_I18N)

set(HAVE_ICU 1)

if(NOT Boost_FOUND)
	message(FATAL " Failed to find Boost >= 1.36 ")
endif(NOT Boost_FOUND)

find_library(ICONV_LIB iconv)

include_directories(${CMAKE_BINARY_DIR})
include_directories(boost_locale)
include_directories(${Boost_INCLUDE_DIRS})


check_type_size(wchar_t SIZEOF_WCHAR_T) 

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
	locale_mo_file.cpp 
	locale_gettext.cpp 
	locale_environment.cpp 
	locale_pool.cpp 
	base64.cpp 
	locale_info.cpp 
	base_view.cpp 
	internal_file_server.cpp 
	scgi_api.cpp 
	fastcgi_api.cpp 
	http_api.cpp 
	atomic_counter.cpp 
	aio_timer.cpp 
	json.cpp 
	encoding.cpp 
	locale_charset.cpp 
	locale_icu_locale.cpp 
	locale_convert.cpp 
	locale_numeric.cpp 
	locale_collate.cpp 
	icu_util.cpp 
	form.cpp 
	filters.cpp 
	utf8_iterator.cpp)


add_definitions(-DDLL_EXPORT)


add_library(cppcms SHARED ${CPPCMS_SOURCES})

target_link_libraries(cppcms ${ICU_UC})
target_link_libraries(cppcms ${ICU_DATA})
target_link_libraries(cppcms ${ICU_I18N})
target_link_libraries(cppcms ${Boost_LIBRARIES})

target_link_libraries(cppcms ${ICONV_LIB})

add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world cppcms)


if(${CMAKE_HOST_WIN32})
	target_link_libraries(cppcms ws2_32)
	target_link_libraries(cppcms wsock32)
endif(${CMAKE_HOST_WIN32}) 


