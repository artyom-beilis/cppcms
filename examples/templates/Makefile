LIBS=-lcppcms -lbooster

all: clean hello-tmpl

hello-tmpl: hello-tmpl.cpp my_skin.cpp content.h
	$(CXX) -Wall hello-tmpl.cpp my_skin.cpp -o hello-tmpl ${LIBS}

my_skin.cpp: first.tmpl content.h
	cppcms_tmpl_cc first.tmpl -o my_skin.cpp

clean:
	rm -fr hello-tmpl *.exe *.so my_skin.cpp cppcms_rundir

