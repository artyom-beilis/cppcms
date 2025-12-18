cmake_minimum_required(VERSION 3.5)

if(WIN32 OR CYGWIN)
	add_definitions(-DDLL_EXPORT)
endif()

add_library(cppcms_session_cppdb SHARED cppdb_storage.cpp)
target_link_libraries(cppcms_session_cppdb cppcms booster cppdb)
