LIBS=-lcppcms -lbooster


all: xss

xss: xss.cpp
	$(CXX) $(CXXFLAGS) -O2 -Wall -g xss.cpp -o xss ${LIBS}

clean:
	rm -fr xss xss.exe cppcms_rundir
