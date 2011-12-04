cmake_minimum_required(VERSION 2.6)

add_subdirectory(sqlite3)
add_subdirectory(cppdb)
add_subdirectory(berkeley_db)

add_executable(tester storage_test.cpp)
target_link_libraries(tester cppcms booster)
