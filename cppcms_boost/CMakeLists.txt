cmake_minimum_required(VERSION 2.6)


set(BOOST_SRC 
	libs/system/src/error_code.cpp
	libs/iostreams/src/zlib.cpp
	libs/iostreams/src/mapped_file.cpp
	libs/iostreams/src/gzip.cpp
	libs/iostreams/src/file_descriptor.cpp
	libs/filesystem/src/utf8_codecvt_facet.hpp
	libs/filesystem/src/utf8_codecvt_facet.cpp
	libs/filesystem/src/portability.cpp
	libs/filesystem/src/path.cpp
	libs/filesystem/src/operations.cpp
)


include_directories(.)
find_library(ZLIB z NAMES zdll zlib)
find_path(ZLIB_INCLUDE_DIR zlib.h)
include_directories(${ZLIB_INCLUDE_DIR})

if(NOT DISABLE_SHARED)
	add_library(boost SHARED ${BOOST_SRC})
	set(BOOST_SHARED_DEFS CPPCMS_BOOST_ALL_DYN_LINK CPPCMS_BOOST_ALL_NO_LIB)
	if(WIN32 AND NOT CYGWIN)
		set(BOOST_SHARED_DEFS ${BOOST_SHARED_DEFS} CPPCMS_BOOST_THREAD_BUILD_DLL)
	endif(WIN32 AND NOT CYGWIN)
	set_target_properties(boost PROPERTIES COMPILE_DEFINITIONS "${BOOST_SHARED_DEFS}")
	set_target_properties(boost PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(boost PROPERTIES OUTPUT_NAME "cppcms_boost${CPPCMS_SUFFIX}")
	target_link_libraries(boost ${ZLIB})
	if(LIB_PTHREAD)
		target_link_libraries(boost ${LIB_PTHREAD})
	endif(LIB_PTHREAD)
	install(TARGETS boost 
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)
endif(NOT DISABLE_SHARED)	

if(NOT DISABLE_STATIC)
	add_library(boost-static STATIC ${BOOST_SRC})
	set(BOOST_STATIC_DEFS CPPCMS_BOOST_ALL_NO_LIB)
	if(WIN32 AND NOT CYGWIN)
		set(BOOST_STATIC_DEFS ${BOOST_STATIC_DEFS} CPPCMS_BOOST_THREAD_BUILD_LIB)
	endif(WIN32 AND NOT CYGWIN)
	set_target_properties(boost-static PROPERTIES COMPILE_DEFINITIONS "${BOOST_STATIC_DEFS}")
	set_target_properties(boost-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(boost-static PROPERTIES OUTPUT_NAME "cppcms_boost")
	target_link_libraries(boost-static ${ZLIB})
	if(LIB_PTHREAD)
		target_link_libraries(boost-static ${LIB_PTHREAD})
	endif(LIB_PTHREAD)
	install(TARGETS boost-static
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)
endif(NOT DISABLE_STATIC)	

if(MSVC AND NOT DISABLE_STATIC)
	set_target_properties(boost-static PROPERTIES PREFIX "lib")
endif(MSVC AND NOT DISABLE_STATIC)

