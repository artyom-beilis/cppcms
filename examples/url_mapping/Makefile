LIBS=-lcppcms -lbooster

all: clean hello

hello: hello.cpp
	$(CXX) $(CXXFLAGS) -std=c++11 -Wall hello.cpp -o hello $(LIBS)

clean:
	rm -fr hello *.exe *.so cppcms_rundir
