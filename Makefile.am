SUBDIRS = ./transtext

noinst_PROGRAMS = hello_world.fcgi
noinst_HEADERS = hello_world_view.h
dist_bin_SCRIPTS = cppcms_tmpl_cc cppcms_run

hello_world_fcgi_SOURCES = hello_world.cpp hello_world_view1.cpp hello_world_view2.cpp
hello_world_fcgi_LDADD = libcppcms.la transtext/libcppcmstranstext.la
hello_world_fcgi_CXXFLAGS= -Wall -I./transtext



hello_world_view1.cpp: hello_world_skin1.tmpl hello_world_view1.tmpl
	./cppcms_tmpl_cc hello_world_skin1.tmpl hello_world_view1.tmpl -o hello_world_view1.cpp -n view1 -d test

hello_world_view2.cpp: hello_world_skin2.tmpl hello_world_view1.tmpl
	./cppcms_tmpl_cc hello_world_skin2.tmpl hello_world_view1.tmpl -o hello_world_view2.cpp -n view2 -d test

lib_LTLIBRARIES     = libcppcms.la
libcppcms_la_SOURCES = global_config.cpp manager.cpp 	url.cpp worker_thread.cpp \
	text_tool.cpp 	cache_interface.cpp base_cache.cpp thread_cache.cpp scgi.cpp \
	base_view.cpp util.cpp form.cpp application.cpp

libcppcms_la_LDFLAGS  = -no-undefined -version-info 0:0:0
libcppcms_la_LIBADD = @CPPCMS_LIBS@ transtext/libcppcmstranstext.la
libcppcms_la_CXXFLAGS = -Wall -I./transtext

if EN_FORK_CACHE
libcppcms_la_SOURCES += process_cache.cpp
endif

if EN_FCGI_BACKEND
libcppcms_la_SOURCES += fcgi.cpp
endif

nobase_pkginclude_HEADERS = global_config.h text_tool.h url.h cppcms_error.h \
	manager.h worker_thread.h fcgi.h cache_interface.h archive.h \
	base_cache.h thread_cache.h cgicc_connection.h scgi.h cgi_api.h \
	process_cache.h shmem_allocator.h posix_mutex.h config.h cgi.h base_view.h \
	util.h form.h application.h

if EN_TCP_CACHE
libcppcms_la_SOURCES += tcp_cache.cpp
bin_PROGRAMS = tcp_cache_server
tcp_cache_server_SOURCES = base_cache.cpp thread_cache.cpp tcp_cache_server.cpp
nobase_pkginclude_HEADERS += tcp_cache_protocol.h tcp_cache.h
endif


