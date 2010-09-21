LIBS=-lcppcms -lbooster

CXXFLAGS=-Wall -g -O3 -I../inc

EXEC_CXXFLAGS=$(CXXFLAGS)
EXEC_LDFLAGS=-export-dynamic
EXEC_LIBS=$(LIBS) -ldbixx 

VIEW_CXXFLAGS=$(CXXFLAGS) -fPIC -DPIC
VIEW_LDFLAGS=-shared
VIEW_LIBS=$(LIBS)

CXX=g++
CTMPL=cppcms_tmpl_cc
GETTEXT_DOMAIN=mb


