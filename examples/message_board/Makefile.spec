LIBS=-lcppcms -lbooster

CXXFLAGS=-Wall -g -O3 -I../apps

EXEC_CXXFLAGS=$(CXXFLAGS)
EXEC_LDFLAGS=-rdynamic
EXEC_LIBS=$(LIBS) -lcppdb

VIEW_CXXFLAGS=$(CXXFLAGS) -fPIC -DPIC
VIEW_LDFLAGS=-shared
VIEW_LIBS=$(LIBS)

CXX=g++
CTMPL=cppcms_tmpl_cc
GETTEXT_DOMAIN=mb


