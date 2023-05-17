LIBS=-lcppcms -lbooster


all: xss

xss: xss.cpp
	$(CXX) $(CXXFLAGS) -std=c++11 -O2 -Wall -g xss.cpp -o xss ${LIBS}

clean:
	rm -fr xss xss.exe cppcms_rundir
