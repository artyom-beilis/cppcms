noinst_PROGRAMS = hello_world.fcgi
hello_world_fcgi_SOURCES = hello_world.cpp
hello_world_fcgi_LDADD = libcppcms.la
hello_world_fcgi_CXXFLAGS=-Wall

lib_LTLIBRARIES     = libcppcms.la
libcppcms_la_SOURCES = fcgi_stream.cpp  global_config.cpp thread_pool.cpp  url.cpp  worker_thread.cpp text_tool.cpp cache_interface.cpp base_cache.cpp thread_cache.cpp
libcppcms_la_CXXFLAGS=-Wall

nobase_pkginclude_HEADERS = global_config.h text_tool.h url.h cppcms_error.h thread_pool.h worker_thread.h fcgi_stream.h cache_interface.h archive.h base_cache.h thread_cache.h
