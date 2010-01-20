# SUBDIRS = ./
srcdir = @srcdir@
VPATH = @srcdir@

noinst_PROGRAMS = hello_world
bin_PROGRAMS = cppcms_config_find_param

#EXTRA_DIST = hello_world_skin1.tmpl hello_world_view1.tmpl hello_world_skin2.tmpl Changelog autogen.sh config.txt

hello_world_SOURCES= hello_world.cpp hello_world_view1.cpp hello_world_view2.cpp
hello_world_LDADD= libcppcms.la

hello_world_view1.cpp: hello_world_skin1.tmpl hello_world_view1.tmpl
	$(srcdir)/cppcms_tmpl_cc $(srcdir)/hello_world_skin1.tmpl $(srcdir)/hello_world_view1.tmpl -o hello_world_view1.cpp -s view1 -d test

hello_world_view2.cpp: hello_world_skin2.tmpl hello_world_view1.tmpl
	$(srcdir)/cppcms_tmpl_cc $(srcdir)/hello_world_skin2.tmpl $(srcdir)/hello_world_view1.tmpl -o hello_world_view2.cpp -s view2 -d test


lib_LTLIBRARIES = libcppcms_boost.la libcppcms.la
libcppcms_boost_la_SOURCES = \
	cppcms_boost/libs/system/src/error_code.cpp \
	cppcms_boost/libs/regex/src/winstances.cpp \
	cppcms_boost/libs/regex/src/wide_posix_api.cpp \
	cppcms_boost/libs/regex/src/wc_regex_traits.cpp \
	cppcms_boost/libs/regex/src/w32_regex_traits.cpp \
	cppcms_boost/libs/regex/src/usinstances.cpp \
	cppcms_boost/libs/regex/src/static_mutex.cpp \
	cppcms_boost/libs/regex/src/regex_traits_defaults.cpp \
	cppcms_boost/libs/regex/src/regex_raw_buffer.cpp \
	cppcms_boost/libs/regex/src/regex_debug.cpp \
	cppcms_boost/libs/regex/src/regex.cpp \
	cppcms_boost/libs/regex/src/posix_api.cpp \
	cppcms_boost/libs/regex/src/instances.cpp \
	cppcms_boost/libs/regex/src/icu.cpp \
	cppcms_boost/libs/regex/src/fileiter.cpp \
	cppcms_boost/libs/regex/src/cregex.cpp \
	cppcms_boost/libs/regex/src/cpp_regex_traits.cpp \
	cppcms_boost/libs/regex/src/c_regex_traits.cpp \
	cppcms_boost/libs/iostreams/src/zlib.cpp \
	cppcms_boost/libs/iostreams/src/mapped_file.cpp \
	cppcms_boost/libs/iostreams/src/gzip.cpp \
	cppcms_boost/libs/iostreams/src/file_descriptor.cpp \
	cppcms_boost/libs/filesystem/src/utf8_codecvt_facet.hpp
	cppcms_boost/libs/filesystem/src/utf8_codecvt_facet.cpp \
	cppcms_boost/libs/filesystem/src/portability.cpp \
	cppcms_boost/libs/filesystem/src/path.cpp \
	cppcms_boost/libs/filesystem/src/operations.cpp \
	cppcms_boost/libs/date_time/src/gregorian/date_generators.cpp \
	cppcms_boost/libs/date_time/src/gregorian/greg_month.cpp \
	cppcms_boost/libs/date_time/src/gregorian/greg_weekday.cpp \
	cppcms_boost/libs/date_time/src/gregorian/gregorian_types.cpp \
	cppcms_boost/libs/date_time/src/posix_time/posix_time_types.cpp 


libcppcms_la_SOURCES = \
	localization/locale_src_boundary.cpp \
	localization/locale_src_codepage.cpp \
	localization/locale_src_collator.cpp \
	localization/locale_src_conversion.cpp \
	localization/locale_src_format.cpp \
	localization/locale_src_formatter.cpp \
	localization/locale_src_formatting.cpp \
	localization/locale_src_generator.cpp \
	localization/locale_src_info.cpp \
	localization/locale_src_message.cpp \
	localization/locale_src_mo_lambda.cpp \
	localization/locale_src_timezone.cpp \
	service.cpp \
	cgi_api.cpp \
	http_request.cpp \
	http_response.cpp \
	http_context.cpp \
	cppcms_error.cpp \
	cppcms_error_category.cpp \
	thread_pool.cpp \
	applications_pool.cpp \
	application.cpp \
	url_dispatcher.cpp \
	http_cookie.cpp \
	util.cpp \
	base64.cpp \
	base_view.cpp \
	views_pool.cpp \
	internal_file_server.cpp \
	scgi_api.cpp \
	fastcgi_api.cpp \
	http_api.cpp \
	atomic_counter.cpp \
	aio_timer.cpp \
	json.cpp \
	encoding.cpp \
	regex.cpp \
	form.cpp \
	filters.cpp \
	cache_pool.cpp \
	cache_interface.cpp \
	thread_cache.cpp \
	connection_forwarder.cpp \
	session_pool.cpp \
	base_encryptor.cpp \
	hmac_encryptor.cpp \
	md5.cpp \
	urandom.cpp \
	session_cookies.cpp \
	session_dual.cpp \
	session_sid.cpp \
	session_interface.cpp \
	session_memory_storage.cpp

if ENABLE_GCRYPT
 libcppcms_la_SOURCES += aes_encryptor.cpp
endif

if WIN32
 libcppcms_la_SOURCES += session_win32_file_storage.cpp
else
 libcppcms_la_SOURCES += session_posix_file_storage.cpp
endif

if WIN_NOT_CYGWIN
 libcppcms_boost_la_SOURCES += \
	cppcms_boost/libs/thread/src/win32/tss_pe.cpp \
	cppcms_boost/libs/thread/src/win32/tss_dll.cpp \
	cppcms_boost/libs/thread/src/win32/thread.cpp

else
 libcppcms_boost_la_SOURCES += \
	cppcms_boost/libs/thread/src/pthread/thread.cpp \
	cppcms_boost/libs/thread/src/pthread/once.cpp

endif

cppcms_config_find_param_SOURCES=cppcms_config_find_param.cpp json.cpp
cppcms_config_find_param_CXXFLAGS=-g -Wall -I$(srcdir)/cppcms_boost -I$(srcdir) -I$(srcdir)/localization
hello_world_CXXFLAGS=-g -Wall -I$(srcdir)/cppcms_boost -I$(srcdir) -I$(srcdir)/localization

libcppcms_la_LDFLAGS  = -no-undefined -version-info 1:0:0
libcppcms_la_LIBADD = @CPPCMS_LIBS@ libcppcms_boost.la
libcppcms_la_CXXFLAGS =-g -Wall -I$(srcdir)/cppcms_boost -I$(srcdir) -I$(srcdir)/localization

libcppcms_boost_la_CXXFLAGS = -g -Wall -I$(srcdir)/cppcms_boost -I$(srcdir) -I$(srcdir)/localization
libcppcms_boost_la_LIBADD = @BOOST_LIBS@ 
libcppcms_boost_la_LDFLAGS  = -no-undefined -version-info 1:0:0



