LIBS=-lcppcms -lbooster

all: clean hello.fcgi locale/he/LC_MESSAGES/hello.mo

hello.fcgi: hello.cpp view.cpp content.h
	$(CXX) -Wall hello.cpp view.cpp -o hello.fcgi $(LIBS)

view.cpp: view.tmpl content.h
	cppcms_tmpl_cc -d hello view.tmpl -o view.cpp

locale/he/LC_MESSAGES/hello.mo: locale/he/LC_MESSAGES/hello.po
	msgfmt locale/he/LC_MESSAGES/hello.po -o locale/he/LC_MESSAGES/hello.mo

clean:
	rm -fr *.fcgi *.exe *.so view.cpp locale/he/LC_MESSAGES/hello.mo cppcms_rundir
