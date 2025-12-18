cmake_minimum_required(VERSION 3.5)
if(WIN32 OR CYGWIN)
	add_definitions(-DDLL_EXPORT)
endif()

add_library(cppcms_session_sqlite3 SHARED session_sqlite_storage.cpp)
target_link_libraries(cppcms_session_sqlite3 cppcms booster sqlite3)
