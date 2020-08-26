LIBS=-lcppcms -lbooster


all: hello

hello: hello.cpp
	$(CXX) -std=c++11 -O2 -Wall -g hello.cpp -o hello ${LIBS}

clean:
	rm -fr hello hello.exe cppcms_rundir
