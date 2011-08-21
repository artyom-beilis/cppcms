cmake_minimum_required(VERSION 2.6)
project(cppcms)

include_directories(${CMAKE_SOURCE_DIR})

# Options 

option(STATIC_VIEW "Build Static View instead of dynamically loaded one" OFF)

if(WIN32 OR CYGWIN)
	add_definitions(-DDLL_EXPORT)
endif()


# Dependencies

find_library(LIB_BOOSTER booster)
find_library(LIB_CPPCMS cppcms)
find_library(LIB_CPPDB cppdb)
find_program(EXE_TMPL_CC cppcms_tmpl_cc)
find_program(EXE_MSGFMT msgfmt)
find_program(EXE_SQLITE3 sqlite3)

find_path(INC_BOOSTER booster/config.h)
find_path(INC_CPPCMS cppcms/config.h)
find_path(INC_CPPDB cppdb/frontend.h)

# Sources

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

include_directories(${INC_BOOSTER})
include_directories(${INC_CPPCMS})
include_directories(${INC_CPPDB})

set(VIEW_SRC "${CMAKE_CURRENT_BINARY_DIR}/${VIEW_NAME}.cpp")

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


add_executable(mb ${MB_SRC})
target_link_libraries(mb ${LIB_CPPCMS})
target_link_libraries(mb ${LIB_BOOSTER})
target_link_libraries(mb ${LIB_CPPDB})

if(EXE_MSGFMT)
	set(MO_FILES)
	foreach(LANG ${MB_LANG}) 
		set(PO locale/${LANG}/LC_MESSAGES/mb.po)
		set(MO ${CMAKE_CURRENT_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/mb.mo)
		file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/locale/${LANG}/LC_MESSAGES")
		set(MO_FILES ${MO_FILES} ${MO})
		add_custom_command(
			OUTPUT ${MO}
			COMMAND ${EXE_MSGFMT} ${PO} -o ${MO}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			DEPENDS ${PO})
	endforeach()
	add_custom_target(create-mo ALL DEPENDS ${MO_FILES})
else()
	message("-- msgfmt not found, translations not generatied")
endif()

if(EXE_SQLITE3)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mb.db
		COMMAND ${EXE_SQLITE3} < ${CMAKE_CURRENT_SOURCE_DIR}/model/sqlite3.sql ${CMAKE_CURRENT_BINARY_DIR}/mb.db)
	add_custom_target(sqlitedb ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mb.db)
endif()
