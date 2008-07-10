noinst_PROGRAMS = hello_world.fcgi

hello_world_fcgi_SOURCES = hello_world.cpp
hello_world_fcgi_LDADD = libcppcms.la
hello_world_fcgi_CXXFLAGS=-Wall

lib_LTLIBRARIES     = libcppcms.la
libcppcms_la_SOURCES = global_config.cpp manager.cpp \
 	url.cpp  worker_thread.cpp text_tool.cpp\
 	cache_interface.cpp base_cache.cpp thread_cache.cpp scgi.cpp 

libcppcms_la_LDFLAGS  = -version-info 1:0:0
libcppcms_la_CXXFLAGS = -Wall

if  EN_FORK_CACHE
libcppcms_la_SOURCES += process_cache.cpp
endif

if EN_FCGI_BACKEND
libcppcms_la_SOURCES += fcgi.cpp
endif	


nobase_pkginclude_HEADERS = global_config.h text_tool.h url.h cppcms_error.h \
	manager.h worker_thread.h fcgi.h cache_interface.h archive.h \
	base_cache.h thread_cache.h cgicc_connection.h scgi.h cgi_api.h \
	process_cache.h shmem_allocator.h posix_mutex.h config.h cgi.h

