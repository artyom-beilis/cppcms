LIBS=-lcppcms

all: clean hello.fcgi 

hello.fcgi: hello.cpp view.cpp content.h
	$(CXX) -Wall hello.cpp view.cpp -o hello.fcgi $(LIBS)

view.cpp: view.tmpl content.h
	cppcms_tmpl_cc view.tmpl -o view.cpp

clean:
	rm -fr *.fcgi *.exe *.so view.cpp cppcms_rundir
