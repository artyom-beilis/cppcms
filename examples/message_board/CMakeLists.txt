cmake_minimum_required(VERSION 2.6)
project(cppcms)

include_directories(${CMAKE_SOURCE_DIR}/apps)

option(STATIC_VIEW "Build Static View instead of dynamically loaded one" OFF)


find_library(LIB_BOOSTER booster)
find_library(LIB_CPPCMS cppcms)
find_library(LIB_CPPDB cppdb)
find_program(EXE_TMPL_CC cppcms_tmpl_cc)

find_path(INC_BOOSTER booster/config.h)
find_path(INC_CPPCMS cppcms/config.h)
find_path(INC_CPPDB cppdb/frontend.h)

include_directories(${INC_BOOSTER})
include_directories(${INC_CPPCMS})
include_directories(${INC_CPPDB})

set(VIEW_NAME simple)

set(VIEW_TEMPLATES 
	view/master.tmpl
	view/forums.tmpl 
	view/thread.tmpl)

set(MB_SRC
	apps/master.cpp
	apps/thread.cpp
	apps/forums.cpp
	apps/mb.cpp
	apps/main.cpp)

set(MB_LANG he)

# Actual Build

set(VIEW_SRC "${CMAKE_CURRENT_BINARY_DIR}/${VIEW_NAME}.cpp")


if(WIN32 AND CMAKE_COMPILER_IS_GNUCXX AND NOT STATIC_VIEW)
	set(TMPL_CC_PARAMS "-u" )
else()
	set(TMPL_CC_PARAMS)
endif()

add_custom_command(
	OUTPUT ${VIEW_SRC}
	COMMAND ${EXE_TMPL_CC}
		-s "${VIEW_NAME}"
		${TMPL_CC_PARAMS}
		-o ${VIEW_SRC}
		${VIEW_TEMPLATES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${VIEW_TEMPLATES})

if(STATIC_VIEW)
	set(MB_SRC ${MB_SRC} ${VIEW_SRC})
else()
	add_library(${VIEW_NAME} SHARED ${VIEW_SRC})
	target_link_libraries(${VIEW_NAME} ${LIB_CPPCMS})
	target_link_libraries(${VIEW_NAME} ${LIB_BOOSTER})
endif()


add_executable(mb ${MB_SRC} ${VIEW_SRC})
target_link_libraries(mb ${LIB_CPPCMS})
target_link_libraries(mb ${LIB_BOOSTER})
target_link_libraries(mb ${LIB_CPPDB})



