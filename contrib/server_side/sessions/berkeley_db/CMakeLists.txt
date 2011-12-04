cmake_minimum_required(VERSION 2.6)

if(WIN32 OR CYGWIN)
	add_definitions(-DDLL_EXPORT)
endif()

add_library(cppcms_session_bdb SHARED bdb.cpp)
target_link_libraries(cppcms_session_bdb cppcms booster db)
