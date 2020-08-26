LIBS=-lcppcms -lbooster

all: clean myapp

myapp: myapp.cpp
	$(CXX) $(CXXFLAGS) -std=c++11 -Wall myapp.cpp -o myapp $(LIBS)

clean:
	rm -fr myapp *.exe *.so cppcms_rundir
