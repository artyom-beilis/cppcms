LIBS=-lcppcms -lbooster

all: clean hello 

hello: hello.cpp view.cpp content.h
	$(CXX) -Wall hello.cpp view.cpp -o hello $(LIBS)

view.cpp: view.tmpl content.h
	cppcms_tmpl_cc view.tmpl -o view.cpp

clean:
	rm -fr hello *.exe *.so view.cpp cppcms_rundir
