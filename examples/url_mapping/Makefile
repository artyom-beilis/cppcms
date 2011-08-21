LIBS=-lcppcms -lbooster

all: clean hello

hello: hello.cpp
	$(CXX) -DDLL_EXPORT -Wall hello.cpp -o hello $(LIBS)

clean:
	rm -fr hello *.exe *.so cppcms_rundir
