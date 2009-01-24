SUBDIRS = ./transtext

noinst_PROGRAMS = hello_world.fcgi
noinst_HEADERS = hello_world_view.h
dist_bin_SCRIPTS = cppcms_tmpl_cc cppcms_run
bin_PROGRAMS = cppcms_make_key

EXTRA_DIST = hello_world_skin1.tmpl hello_world_view1.tmpl hello_world_skin2.tmpl Changelog

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
	base_view.cpp util.cpp form.cpp application.cpp session_interface.cpp \
	session_cookies.cpp hmac_encryptor.cpp encryptor.cpp md5.c base64.cpp \
	session_sid.cpp session_file_storage.cpp session_dual.cpp cppcms_error.cpp

libcppcms_la_LDFLAGS  = -no-undefined -version-info 0:0:0
libcppcms_la_LIBADD = @CPPCMS_LIBS@ transtext/libcppcmstranstext.la
libcppcms_la_CXXFLAGS = -Wall -I./transtext

if EN_FORK_CACHE
libcppcms_la_SOURCES += process_cache.cpp
endif

if EN_FCGI_BACKEND
libcppcms_la_SOURCES += fcgi.cpp
endif

if EN_ENCR_SESSIONS
libcppcms_la_SOURCES += aes_encryptor.cpp
endif

if EN_SQLITE_SESSIONS
libcppcms_la_SOURCES += session_sqlite_storage.cpp
endif

nobase_pkginclude_HEADERS = global_config.h text_tool.h url.h cppcms_error.h \
	manager.h worker_thread.h fcgi.h cache_interface.h archive.h \
	base_cache.h thread_cache.h cgicc_connection.h scgi.h cgi_api.h \
	process_cache.h shmem_allocator.h posix_mutex.h config.h cgi.h base_view.h \
	util.h form.h application.h session_interface.h session_api.h session_cookies.h \
	hmac_encryptor.h aes_encryptor.h encryptor.h md5.h base64.h session_backend_factory.h \
	session_sid.h session_storage.h session_file_storage.h session_dual.h \
	session_cache_backend.h session_sqlite_storage.h tcp_cache_protocol.h tcp_cache.h \
	tcp_connector.h session_tcp_storage.h tcp_messenger.h asio_config.h fcntl_mutex.h

if EN_TCP_CACHE
libcppcms_la_SOURCES += tcp_cache.cpp tcp_messenger.cpp session_tcp_storage.cpp tcp_connector.cpp
bin_PROGRAMS += cppcms_tcp_scale
cppcms_tcp_scale_SOURCES = base_cache.cpp thread_cache.cpp tcp_cache_server.cpp session_file_storage.cpp cppcms_error.cpp
cppcms_tcp_scale_CXXFLAGS=-DNO_BUILDER_INTERFACE
cppcms_tcp_scale_LDADD = @TCPSCALE_LIBS@ 
if EN_SQLITE_SESSIONS
cppcms_tcp_scale_SOURCES += session_sqlite_storage.cpp
endif

endif


